#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)reg_split.cpp	1.52 03/03/12 01:30:37 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_reg_split.cpp.incl"

//------------------------------Split--------------------------------------
// Walk the graph in RPO and for each lrg which spills, propogate reaching
// definitions.  During propogation, split the live range around regions of
// High Register Pressure (HRP).  If a Def is in a region of Low Register
// Pressure (LRP), it will not get spilled until we encounter a region of
// HRP between it and one of its uses.  We will spill at the transition
// point between LRP and HRP.  Uses in the HRP region will use the spilled
// Def.  The first Use outside the HRP region will generate a SpillCopy to
// hoist the live range back up into a register, and all subsequent uses
// will use that new Def until another HRP region is encountered.  Defs in
// HRP regions will get trailing SpillCopies to push the LRG down into the
// stack immediately.
//
// As a side effect, unlink from (hence make dead) coalesced copies.
//

//------------------------------get_spillcopy_wide-----------------------------
// Get a SpillCopy node with wide-enough masks.  Use the 'wide-mask', the
// wide ideal-register spill-mask if possible.  If the 'wide-mask' does
// not cover the input (or output), use the input (or output) mask instead.
Node *PhaseChaitin::get_spillcopy_wide( Node *def, Node *use, uint uidx ) {
  // If ideal reg doesn't exist we've got a bad schedule happening
  // that is forcing us to spill something that isn't spillable.
  // Bail rather than abort
  int ireg = def->ideal_reg();
  if( ireg == 0 || ireg == Op_RegFlags ) {
    prepare_to_bailout("Attempted to spill a non-spillable item");
    return NULL;
  }
  const RegMask *i_mask = &def->out_RegMask();
  const RegMask *w_mask = C->matcher()->idealreg2spillmask[ireg];
  const RegMask *o_mask = use ? &use->in_RegMask(uidx) : w_mask;
  const RegMask *w_i_mask = w_mask->overlap( *i_mask ) ? w_mask : i_mask;
  const RegMask *w_o_mask;

  if( w_mask->overlap( *o_mask ) && // Overlap AND
      ((ireg != Op_RegL && ireg != Op_RegD // Single use or aligned
#ifdef _LP64
        && ireg != Op_RegP
#endif
         ) || o_mask->is_aligned_Pairs()) ) {
    // Don't come here for mis-aligned doubles
    w_o_mask = w_mask;
  } else {                      // wide ideal mask does not overlap with o_mask
    // Mis-aligned doubles come here.  Must target desired registers.
    w_o_mask = o_mask;          // Must target desired registers
    // Does the ideal-reg-mask overlap with o_mask?  I.e., can I use
    // a reg-reg move or do I need a trip across register classes
    // (and thus through memory)?
    if( !C->matcher()->idealreg2regmask[ireg]->overlap( *o_mask) && o_mask->is_UP() )
      // Here we assume a trip through memory is required.
      w_i_mask = &C->FIRST_STACK_mask();
  }
  return new MachSpillCopyNode( def, *w_i_mask, *w_o_mask );
}

//------------------------------insert_proj------------------------------------
// Insert the spill at chosen location.  Skip over any interveneing Proj's or
// Phis.  Skip over a CatchNode and projs, inserting in the fall-through block
// instead.  Update high-pressure indices.  Create a new live range.
void PhaseChaitin::insert_proj( Block *b, uint i, Node *spill, uint maxlrg ) {
  // Skip intervening ProjNodes.  Do not insert between a ProjNode and
  // its definer.
  while( i < b->_nodes.size() && 
         (b->_nodes[i]->is_Proj() ||
          b->_nodes[i]->is_Phi() ) )
    i++;

  // Do not insert between a call and his Catch
  if( b->_nodes[i]->is_Catch() ) {
    // Put the instruction at the top of the fall-thru block.
    // Find the fall-thru projection
    while( 1 ) {
      const CatchProjNode *cp = b->_nodes[++i]->is_CatchProj();
      assert( cp, "" );
      if( cp->_con == CatchProjNode::fall_through_index )
        break;
    }
    int sidx = i - b->end_idx()-1;
    b = b->_succs[sidx];        // Switch to successor block
    i = 1;                      // Right at start of block
  }

  b->_nodes.insert(i,spill);    // Insert node in block
  _cfg._bbs.map(spill->_idx,b); // Update node->block mapping to reflect
  // Adjust the point where we go hi-pressure
  if( i <= b->_ihrp_index ) b->_ihrp_index++;
  if( i <= b->_fhrp_index ) b->_fhrp_index++;

  // Assign a new Live Range Number to the SpillCopy and grow
  // the node->live range mapping.
  new_lrg(spill,maxlrg);
}

//------------------------------split_DEF--------------------------------------
// There are four catagories of Split; UP/DOWN x DEF/USE
// Only three of these really occur as DOWN/USE will always color
// Any Split with a DEF cannot CISC-Spill now.  Thus we need
// two helper routines, one for Split DEFS (insert after instruction),
// one for Split USES (insert before instruction).  DEF insertion
// happens inside Split, where the Leaveblock array is updated.
uint PhaseChaitin::split_DEF( Node *def, Block *b, int loc, uint maxlrg, Node **Reachblock, Node **debug_defs, GrowableArray<uint> splits, int slidx ) {
#ifdef ASSERT
  // Increment the counter for this lrg
  splits.at_put(slidx, splits.at(slidx)+1);
#endif
  // If we are spilling the memory op for an implicit null check, at the
  // null check location (ie - null check is in HRP block) we need to do
  // the null-check first, then spill-down in the following block.
  MachNode *nul = b->end()->is_Mach();
  if( nul && nul->is_MachNullCheck() && nul->in(1) == def && def == b->_nodes[loc] ) {
    // Spill goes in the branch-not-taken block
    b = b->_succs[b->_nodes[b->end_idx()+1]->Opcode() == Op_IfTrue];
    loc = 0;                    // Just past the Region
  }
  assert( loc >= 0, "must insert past block head" );

  // Get a def-side SpillCopy
  Node *spill = get_spillcopy_wide(def,NULL,0);
  // Did we fail to split?, then bail
  if (!spill) {
    return 0;
  }

  // Insert the spill at chosen location
  insert_proj( b, loc+1, spill, maxlrg++);

  // Insert new node into Reaches array
  Reachblock[slidx] = spill;
  // Update debug list of reaching down definitions by adding this one
  debug_defs[slidx] = spill;

  // return updated count of live ranges
  return maxlrg;
}

//------------------------------split_USE--------------------------------------
// Splits at uses can involve redeffing the LRG, so no CISC Spilling there.
// Debug uses want to know if def is already stack enabled.
uint PhaseChaitin::split_USE( Node *def, Block *b, Node *use, uint useidx, uint maxlrg, bool def_down, bool cisc_sp, GrowableArray<uint> splits, int slidx ) {
#ifdef ASSERT
  // Increment the counter for this lrg
  splits.at_put(slidx, splits.at(slidx)+1);
#endif

  // Some setup stuff for handling debug node uses
  JVMState* jvms = use->jvms();
  uint debug_start = jvms ? jvms->debug_start() : 999999;
  uint debug_end   = jvms ? jvms->debug_end()   : 999999;
  
  //-------------------------------------------
  // Check for use of debug info
  if (useidx >= debug_start && useidx < debug_end) {
    // Actually it's perfectly legal for constant debug info to appear
    // just unlikely.  In this case the optimizer left a ConI of a 4
    // as both inputs to a Phi with only a debug use.  It's a single-def
    // live range of a rematerializable value.  The live range spills, 
    // rematerializes and now the ConI directly feeds into the debug info.
    // assert(!def->is_Con(), "constant debug info already constructed directly");

    // Special split handling for Debug Info
    // If DEF is DOWN, just hook the edge and return
    // If DEF is UP, Split it DOWN for this USE.
    if( def->is_Mach() ) {
      if( def_down ) {
        // DEF is DOWN, so connect USE directly to the DEF
        use->set_req(useidx, def);
      } else {
        // Block and index where the use occurs.
        Block *b = _cfg._bbs[use->_idx];
        // Put the clone just prior to use
        int bindex = b->find_node(use);
        // DEF is UP, so must copy it DOWN and hook in USE
        // Insert SpillCopy before the USE, which uses DEF as its input,
        // and defs a new live range, which is used by this node.
        Node *spill = get_spillcopy_wide(def,use,useidx); 
	// did we fail to split?
	if (!spill) {
	  // Bail
	  return 0;
	}
        // insert into basic block
        insert_proj( b, bindex, spill, maxlrg++ );
        // Use the new split
        use->set_req(useidx,spill);
      }
      // No further split handling needed for this use
      return maxlrg;
    }  // End special splitting for debug info live range
  }  // If debug info

  // CISC-SPILLING
  // Finally, check to see if USE is CISC-Spillable, and if so,
  // gather_lrg_masks will add the flags bit to its mask, and
  // no use side copy is needed.  This frees up the live range
  // register choices without causing copy coalescing, etc.
  if( UseCISCSpill && cisc_sp ) {
    int inp = use->cisc_operand();
    if( inp != AdlcVMDeps::Not_cisc_spillable )
      // Convert operand number to edge index number
      inp = use->is_Mach()->operand_index(inp);
    if( inp == (int)useidx ) {
      use->set_req(useidx, def);
#ifndef PRODUCT
      if( TraceCISCSpill ) {
	tty->print("  set_split: ");
	use->dump();
      }
#endif
      return maxlrg;
    }
  }

  //-------------------------------------------
  // Insert a Copy before the use

  // Block and index where the use occurs.
  int bindex;
  // Phi input spill-copys belong at the end of the prior block
  if( use->is_Phi() ) {
    b = _cfg._bbs[b->pred(useidx)->_idx];
    bindex = b->end_idx();
  } else {
    // Put the clone just prior to use
    bindex = b->find_node(use);
  }

  Node *spill = get_spillcopy_wide( def, use, useidx );
  if( !spill ) return 0;        // Bailed out
  // Insert SpillCopy before the USE, which uses the reaching DEF as
  // its input, and defs a new live range, which is used by this node.
  insert_proj( b, bindex, spill, maxlrg++ );
  // Use the spill/clone
  use->set_req(useidx,spill);

  // return updated live range count
  return maxlrg;
}

//------------------------------split_Rematerialize----------------------------
// Clone a local copy of the def.  
Node *PhaseChaitin::split_Rematerialize( Node *def, Block *b, uint insidx, uint &maxlrg, GrowableArray<uint> splits, int slidx, uint *lrg2reach, Node **Reachblock, bool walkThru ) {
  // The input live ranges will be stretched to the site of the new
  // instruction.  They might be stretched past a def and will thus
  // have the old and new values of the same live range alive at the
  // same time - a definite no-no.  Split out private copies of
  // the inputs.
  if( def->req() > 1 ) {
    for( uint i = 1; i < def->req(); i++ ) {
      Node *in = def->in(i);
      // Check for def as only user, if true means LRG cannot be
      // redefined except by def itself.
      if( in->outcnt() == 1 ) continue; // Only 1 user (def)
      uint lidx = n2lidx(in);
      if( lidx >= _maxlrg ) continue; // Value is a recent spill-copy
      // Check for single-def (LRG cannot redefined)
      if( lrgs(lidx)._def != (Node*)-1 ) continue;

      Block *b_def = _cfg._bbs[def->_idx];
      int idx_def = b_def->find_node(def);
      Node *in_spill = get_spillcopy_wide( in, def, i );
      if( !in_spill ) return 0; // Bailed out
      insert_proj(b_def,idx_def,in_spill,maxlrg++);
      if( b_def == b )
        insidx++;
      def->set_req(i,in_spill);
    }
  }

  Node *spill = def->clone();
  if (C->unique() >= 64000) {  // Check when generating nodes
    prepare_to_bailout("Out of nodes during rematerialize");
    return 0;
  }

  // See if any inputs are currently being spilled, and take the 
  // latest copy of spilled inputs.
  if( spill->req() > 1 ) {
    for( uint i = 1; i < spill->req(); i++ ) {
      Node *in = spill->in(i);
      uint lidx = Find_id(in);

      // Walk backwards thru spill copy node intermediates
      if( walkThru )
        while ( in->is_SpillCopy() && lidx >= _maxlrg ) {
          in = in->in(1);
          lidx = Find_id(in);
        }

      if( lidx < _maxlrg && lrgs(lidx).reg() >= LRG::SPILL_REG ) {
        Node *rdef = Reachblock[lrg2reach[lidx]];
        if( rdef ) spill->set_req(i,rdef);
      }
    }
  }


  assert( spill->out_RegMask().is_UP(), "rematerialize to a reg" );
  // Rematerialized op is def->spilled+1
  set_was_spilled(spill);
  if( _spilled_once.test(def->_idx) )
    set_was_spilled(spill);

  insert_proj( b, insidx, spill, maxlrg++ );
#ifdef ASSERT
  // Increment the counter for this lrg
  splits.at_put(slidx, splits.at(slidx)+1);
#endif
  // See if the cloned def kills any flags, and copy those kills as well
  uint i = insidx+1;
  if( clone_projs( b, i, def, spill, maxlrg ) ) {
    // Adjust the point where we go hi-pressure
    if( i <= b->_ihrp_index ) b->_ihrp_index++;
    if( i <= b->_fhrp_index ) b->_fhrp_index++;
  }

  return spill;
}

//------------------------------is_high_pressure-------------------------------
// Function to compute whether or not this live range is "high pressure"
// in this block - whether it spills eagerly or not.
bool PhaseChaitin::is_high_pressure( Block *b, LRG *lrg, uint insidx ) {
  if( lrg->_was_spilled1 ) return true;
  // Forced spilling due to conflict?  Then split only at binding uses
  // or defs, not for supposed capacity problems.
  // CNC - Turned off 7/8/99, causes too much spilling
  // if( lrg->_is_bound ) return false;

  // Not yet reached the high-pressure cutoff point, so low pressure
  uint hrp_idx = lrg->_is_float ? b->_fhrp_index : b->_ihrp_index;
  if( insidx < hrp_idx ) return false;
  // Register pressure for the block as a whole depends on reg class
  int block_pres = lrg->_is_float ? b->_freg_pressure : b->_reg_pressure;
  // Bound live ranges will split at the binding points first;
  // Intermediate splits should assume the live range's register set
  // got "freed up" and that num_regs will become INT_PRESSURE.
  int bound_pres = lrg->_is_float ? FLOATPRESSURE : INTPRESSURE;
  // Effective register pressure limit.
  int lrg_pres = (lrg->get_invalid_mask_size() > lrg->num_regs()) 
    ? (lrg->get_invalid_mask_size() >> (lrg->num_regs()-1)) : bound_pres;
  // High pressure if block pressure requires more register freedom
  // than live range has.
  return block_pres >= lrg_pres;
}

//------------------------------hi_pressure_before_first_use-------------------
// Search to see if we encounter high pressure before the first use of
// the live range.  If so, we will spill it down early (before the loop).
// Search until the search path branches too wide or becomes too infrequent.
bool PhaseChaitin::hi_pressure_before_first_use( Block *b, uint lidx ) {
  // Search until too infrequent.
  float cutoff_freq = b->_freq * 0.2;
  while( b->_freq > cutoff_freq ) {

    // Scan block for 1st use.
    for( uint i = 1; i <= b->end_idx(); i++ ) {
      Node *n = b->_nodes[i];
      // Ignore PHI use, these can be up or down
      if( n->is_Phi() ) continue;
      for( uint j = 1; j < n->req(); j++ )
        if( Find_id(n->in(j)) == lidx )
          // Found 1st use!  Return true if high pressure before there
          return is_high_pressure( b, &lrgs(lidx), j );
    }
    // No uses in block.  Quit if block is high pressure at all.
    if( is_high_pressure( b, &lrgs(lidx), 9999 ) )
      return true;

    // Try advancing the scan.  Pick most common successor.
    Block *bs = b->_succs[0];
    if( b->_num_succs > 1 ) {
      if( b->_succs[1]->_freq > bs->_freq )
        bs = b->_succs[1];
    }
    b = bs;
    if( b->num_preds()-1 > 1 )  // Merge point; give up search
      return false;
  }
  // Did not find either high pressure or a use; no change.
  return false;
}

//------------------------------prompt_use---------------------------------
// True if lidx is used before any real register is def'd in the block
bool PhaseChaitin::prompt_use( Block *b, uint lidx ) {
  if( lrgs(lidx)._was_spilled2 ) return false;

  // Scan block for 1st use.
  for( uint i = 1; i <= b->end_idx(); i++ ) {
    Node *n = b->_nodes[i];
    // Ignore PHI use, these can be up or down
    if( n->is_Phi() ) continue;
    for( uint j = 1; j < n->req(); j++ )
      if( Find_id(n->in(j)) == lidx )
        return true;          // Found 1st use!  
    if( n->out_RegMask().is_NotEmpty() ) return false;
  }
  return false;
}

//------------------------------Split--------------------------------------
//----------Split Routine----------
// ***** NEW SPLITTING HEURISTIC *****
// DEFS: If the DEF is in a High Register Pressure(HRP) Block, split there.
//        Else, no split unless there is a HRP block between a DEF and
//        one of its uses, and then split at the HRP block.
//
// USES: If USE is in HRP, split at use to leave main LRG on stack.
//       Else, hoist LRG back up to register only (ie - split is also DEF)
// We will compute a new maxlrg as we go
uint PhaseChaitin::Split( uint maxlrg ) {
  Compile::TracePhase t3("regAllocSplit", &_t_regAllocSplit, TimeCompiler);

  uint                 bidx, pidx, slidx, insidx, inpidx, twoidx;
  uint                 non_phi = 1, spill_cnt = 0;
  Node               **Reachblock;
  Node                *n1, *n2, *n3;
  Node_List           *defs,*phis;
  bool                *UPblock;
  bool                 u1, u2, u3;
  Block               *b, *pred;
  PhiNode             *phi;
  GrowableArray<uint>  lidxs;

  // Array of counters to count splits per live range
  GrowableArray<uint>  splits;

  //----------Setup Code----------
  // Create a convenient mapping from lrg numbers to reaches/leaves indices
  uint *lrg2reach = NEW_RESOURCE_ARRAY( uint, _maxlrg );
  // Keep track of DEFS & Phis for later passes
  defs = new Node_List();
  phis = new Node_List();
  // Gather info on which LRG's are spilling, and build maps
  for( bidx = 1; bidx < _maxlrg; bidx++ ) {
    if( lrgs(bidx).alive() && lrgs(bidx).reg() >= LRG::SPILL_REG ) { 
      assert(!lrgs(bidx).mask().is_AllStack(),"AllStack should color");
      lrg2reach[bidx] = spill_cnt;
      spill_cnt++;
      lidxs.append(bidx);
#ifdef ASSERT
      // Initialize the split counts to zero
      splits.append(0);
#endif
#ifndef PRODUCT
      if( PrintOpto && WizardMode && lrgs(bidx)._was_spilled1 ) 
        tty->print_cr("Warning, 2nd spill of L%d",bidx);
#endif
    }
  }

  // Pre-split colored live ranges around loops that are spilling.
  pre_split_around_loops(lidxs, spill_cnt);


  // Create side arrays for propagating reaching defs info.
  // Each block needs a node pointer for each spilling live range for the
  // Def which is live into the block.  Phi nodes handle multiple input
  // Defs by querying the output of their predecessor blocks and resolving
  // them to a single Def at the phi.  The pointer is updated for each
  // Def in the block, and then becomes the output for the block when
  // processing of the block is complete.  We also need to track whether
  // a Def is UP or DOWN.  UP means that it should get a register (ie -
  // it is always in LRP regions), and DOWN means that it is probably
  // on the stack (ie - it crosses HRP regions).
  Node ***Reaches     = NEW_RESOURCE_ARRAY( Node**, _cfg._num_blocks+1 );
  bool  **UP          = NEW_RESOURCE_ARRAY( bool*, _cfg._num_blocks+1 );
  Node  **debug_defs  = NEW_RESOURCE_ARRAY( Node*, spill_cnt );
  VectorSet **UP_entry= NEW_RESOURCE_ARRAY( VectorSet*, spill_cnt );

  // Initialize Reaches & UP
  for( bidx = 0; bidx < _cfg._num_blocks+1; bidx++ ) {
    Reaches[bidx]     = NEW_RESOURCE_ARRAY( Node*, spill_cnt );
    UP[bidx]          = NEW_RESOURCE_ARRAY( bool, spill_cnt );
    Node **Reachblock = Reaches[bidx];
    bool *UPblock     = UP[bidx];
    for( slidx = 0; slidx < spill_cnt; slidx++ ) {
      UPblock[slidx] = true;     // Assume they start in registers
      Reachblock[slidx] = NULL;  // Assume that no def is present
    }
  }

  // Initialize to array of empty vectorsets
  for( slidx = 0; slidx < spill_cnt; slidx++ ) 
    UP_entry[slidx] = new VectorSet(Thread::current()->resource_area());

  //----------PASS 1----------
  //----------Propagation & Node Insertion Code----------
  // Walk the Blocks in RPO for DEF & USE info
  for( bidx = 0; bidx < _cfg._num_blocks; bidx++ ) {

    if( C->unique()+spill_cnt >= 65000 ) {
      prepare_to_bailout("Out of Nodes during split while inserting Phis");
      return 0;
    }

    b  = _cfg._blocks[bidx];
    // Reaches & UP arrays for this block
    Reachblock = Reaches[b->_pre_order];
    UPblock    = UP[b->_pre_order];
    // Reset counter of start of non-Phi nodes in block
    non_phi = 1;
    //----------Block Entry Handling----------
    // Check for need to insert a new phi
    // Cycle through this block's predecessors, collecting Reaches
    // info for each spilled LRG.  If they are identical, no phi is
    // needed.  If they differ, check for a phi, and insert if missing,
    // or update edges if present.  Set current block's Reaches set to
    // be either the phi's or the reaching def, as appropriate.
    // If no Phi is needed, check if the LRG needs to spill on entry
    // to the block due to HRP.
    for( slidx = 0; slidx < spill_cnt; slidx++ ) {
      // Grab the live range number
      uint lidx = lidxs.at(slidx);
      // Do not bother splitting or putting in Phis for single-def
      // rematerialized live ranges.  This happens alot to constants
      // with long live ranges.
      if( lrgs(lidx)._def != (Node*)-1 &&
          lrgs(lidx)._def->rematerialize() ) {
        // reset the Reaches & UP entries
        Reachblock[slidx] = lrgs(lidx)._def;
        UPblock[slidx] = true;
	// Record following instruction in case 'n' rematerializes and
	// kills flags
	Block *pred1 = _cfg._bbs[b->pred(1)->_idx];
        continue;
      }

      // Initialize needs_phi and needs_split
      bool needs_phi = false;
      bool needs_split = false;
      // Walk the predecessor blocks to check inputs for that live range
      // Grab predecessor block header
      n1 = b->pred(1);
      // Grab the appropriate reaching def info for inpidx
      pred = _cfg._bbs[n1->_idx];
      pidx = pred->_pre_order;
      Node **Ltmp = Reaches[pidx];
      bool  *Utmp = UP[pidx];
      n1 = Ltmp[slidx];
      u1 = Utmp[slidx];
      // Initialize node for saving type info
      n3 = n1;
      u3 = u1;

      // Compare inputs to see if a Phi is needed
      for( inpidx = 2; inpidx < b->num_preds(); inpidx++ ) {
        // Grab predecessor block headers
        n2 = b->pred(inpidx);
        // Grab the appropriate reaching def info for inpidx
        pred = _cfg._bbs[n2->_idx];
        pidx = pred->_pre_order;
        Ltmp = Reaches[pidx];
        Utmp = UP[pidx];
        n2 = Ltmp[slidx];
        u2 = Utmp[slidx];
        // For each LRG, decide if a phi is necessary
        if( n1 != n2 ) {
          needs_phi = true;
        }
        // See if the phi has mismatched inputs, UP vs. DOWN
        if( n1 && n2 && (u1 != u2) ) {
          needs_split = true;
        }
        // Move n2/u2 to n1/u1 for next iteration
        n1 = n2;
        u1 = u2;
        // Preserve a non-NULL predecessor for later type referencing
        if( (n3 == NULL) && (n2 != NULL) ){
          n3 = n2;
          u3 = u2;
        }
      }  // End for all potential Phi inputs
      
      // If a phi is needed, check for it
      if( needs_phi ) {
        // check block for appropriate phinode & update edges
        for( insidx = 1; insidx <= b->end_idx(); insidx++ ) {
          n1 = b->_nodes[insidx];
          // bail if this is not a phi
          phi = n1->is_Phi();
          if( phi == NULL ) {
            // Keep track of index of first non-PhiNode instruction in block
            non_phi = insidx;
            // break out of the for loop as we have handled all phi nodes
            break;
          }
          // must be looking at a phi
          if( Find_id(n1) == lidxs.at(slidx) ) {
            // found the necessary phi
            needs_phi = false;
            // initialize the Reaches entry for this LRG
            Reachblock[slidx] = phi;
            break;
          }  // end if found correct phi
        }  // end for all phi's
        // add new phinode if one not already found
        if( needs_phi ) {
          // create a new phi node and insert it into the block
          // type is taken from left over pointer to a predecessor
          assert(n3,"No non-NULL reaching DEF for a Phi");
          phi = new PhiNode(b->head(), n3->bottom_type());
          // initialize input edges to NULL
          for( uint tmpidx = 1; tmpidx < b->num_preds(); tmpidx++ ) {
            phi->set_req(tmpidx, NULL);
          }
          // initialize the Reaches entry for this LRG
          Reachblock[slidx] = phi;

          // add node to block & node_to_block mapping
          insert_proj( b, insidx++, phi, maxlrg++ );
          non_phi++;
          // Reset new phi's mapping to be the spilling live range
          _names.map(phi->_idx, lidx);
          assert(Find_id(phi) == lidx,"Bad update on Union-Find mapping");
        }  // end if not found correct phi
        // Here you have either found or created the Phi, so record it
        assert(phi != NULL,"Must have a Phi Node here");
        phis->push(phi);
        // PhiNodes should either force the LRG UP or DOWN depending
        // on its inputs and the register pressure in the Phi's block.
	UPblock[slidx] = true;	// Assume new DEF is UP
	// If entering a high-pressure area with no immediate use,
	// assume Phi is DOWN
        if( is_high_pressure( b, &lrgs(lidx), b->end_idx()) && !prompt_use(b,lidx) )
          UPblock[slidx] = false;
	// If we are not split up/down and all inputs are down, then we
	// are down
	if( !needs_split && !u3 ) 
	  UPblock[slidx] = false;
	// If we are an 'isolated' live range, our up/down status
	// depends on whether we are isolated up or down.
	if( b->_inner_loop && b->_inner_loop->_isolate_lrgs.test(lidx) ) 
	  UPblock[slidx] = (b->_inner_loop->_ud_lrgs.test(lidx) != 0);

      }  // end if phi is needed

      // Do not need a phi, so grab the reaching DEF
      else {
        // Grab predecessor block header
        n1 = b->pred(1);
        // Grab the appropriate reaching def info for k
        pred = _cfg._bbs[n1->_idx];
        pidx = pred->_pre_order;
        Node **Ltmp = Reaches[pidx];
        bool  *Utmp = UP[pidx];
        // reset the Reaches & UP entries
        Reachblock[slidx] = Ltmp[slidx];
        UPblock[slidx] = Utmp[slidx];
      }  // end else no Phi is needed
    }  // end for all spilling live ranges
    // DEBUG
#ifndef PRODUCT
    if(TraceSpilling) {
      tty->print("/`\nBlock %d: ", b->_pre_order);
      tty->print("Reaching Definitions after Phi handling\n");
      for( uint x = 0; x < spill_cnt; x++ ) {
        tty->print("Spill Idx %d: UP %d: Node\n",x,UPblock[x]);
        if( Reachblock[x] )
          Reachblock[x]->dump();
        else
          tty->print("Undefined\n");
      }
    }
#endif

    //----------Non-Phi Node Splitting----------
    // Since phi-nodes have now been handled, the Reachblock array for this
    // block is initialized with the correct starting value for the defs which
    // reach non-phi instructions in this block.  Thus, process non-phi
    // instructions normally, inserting SpillCopy nodes for all spill
    // locations.

    // Memoize any DOWN reaching definitions for use as DEBUG info
    for( insidx = 0; insidx < spill_cnt; insidx++ ) {
      debug_defs[insidx] = (UPblock[insidx]) ? NULL : Reachblock[insidx];
      if( UPblock[insidx] )     // Memoize UP decision at block start
        UP_entry[insidx]->set( b->_pre_order );
    }

    //----------Walk Instructions in the Block and Split----------
    // For all non-phi instructions in the block
    for( insidx = 1; insidx <= b->end_idx(); insidx++ ) {
      Node *n = b->_nodes[insidx];
      // Find the defining Node's live range index
      uint defidx = Find_id(n);
      uint cnt = n->req();

      if( n->is_Phi() ) {
	// Skip phi nodes after removing dead copies.
	if( defidx < _maxlrg ) {
          // Check for useless Phis.  These appear if we spill, then
          // coalesce away copies.  Dont touch Phis in spilling live
          // ranges; they are busy getting modifed in this pass.
          if( lrgs(defidx).reg() < LRG::SPILL_REG ) {
            uint i;
            Node *u = NULL;
            // Look for the Phi merging 2 unique inputs
            for( i = 1; i < cnt; i++ ) {
              // Ignore repeats and self
              if( n->in(i) != u && n->in(i) != n ) {
                // Found a unique input
                if( u != NULL ) // If it's the 2nd, bail out
                  break;
                u = n->in(i);   // Else record it
              }
            }
            assert( u, "at least 1 valid input expected" );
            if( i >= cnt ) {    // Didn't find 2+ unique inputs?
              n->replace_by(u); // Then replace with unique input
              n->disconnect_inputs(NULL);
              b->_nodes.remove(insidx);
              insidx--;
              b->_ihrp_index--;
              b->_fhrp_index--;
            }
          }
        }
	continue;
      }
      assert( insidx > b->_ihrp_index || 
              (b->_reg_pressure < (uint)INTPRESSURE) ||
              b->_ihrp_index > 4000000 ||
              b->_ihrp_index >= b->end_idx() ||
              !b->_nodes[b->_ihrp_index]->is_Proj(), "" );
      assert( insidx > b->_fhrp_index || 
              (b->_freg_pressure < (uint)FLOATPRESSURE) ||
              b->_fhrp_index > 4000000 ||
              b->_fhrp_index >= b->end_idx() ||
              !b->_nodes[b->_fhrp_index]->is_Proj(), "" );

      // ********** Handle Crossing HRP Boundry **********
      if( (insidx == b->_ihrp_index) || (insidx == b->_fhrp_index) ) {
        for( slidx = 0; slidx < spill_cnt; slidx++ ) {
          // Check for need to split at HRP boundry - split if UP
          n1 = Reachblock[slidx];
          // bail out if no reaching DEF
          if( n1 == NULL ) continue;
	  // bail out if live range is 'isolated' around inner loop
	  uint lidx = lidxs.at(slidx);
#ifndef IA64
	  if( b->_inner_loop && b->_inner_loop->_isolate_lrgs.test(lidx) )
	    continue;
#endif
	  // If live range is currently UP
          if( UPblock[slidx] ) {
            // set location to insert spills at
            // SPLIT DOWN HERE - NO CISC SPILL
            if( is_high_pressure( b, &lrgs(lidx), insidx ) && 
                !n1->rematerialize() ) {
              // If there is already a valid stack definition available, use it
              if( debug_defs[slidx] != NULL ) {
                Reachblock[slidx] = debug_defs[slidx];
                //tty->print("Using reaching stack definition instead of spilling\n");
              }
              else {
                // Insert point is just past last use or def in the block
                int insert_point = insidx-1;
                while( insert_point > 0 ) {
                  Node *n = b->_nodes[insert_point];
                  // Hit top of block?  Quit going backwards
                  if( n->is_Phi() ) break;
                  // Found a def?  Better split after it.
                  if( n2lidx(n) == lidx ) break;
                  // Look for a use
                  uint i;
                  for( i = 1; i < n->req(); i++ )
                    if( n2lidx(n->in(i)) == lidx )
                      break;
                  // Found a use?  Better split after it.
                  if( i < n->req() ) break;
                  insert_point--;
                }
                maxlrg = split_DEF( n1, b, insert_point, maxlrg, Reachblock, debug_defs, splits, slidx);
		// If it wasn't split bail
		if (!maxlrg) {
		  return 0;
		}
                insidx++;
              }
              // This is a new DEF, so update UP
              UPblock[slidx] = false;
#ifndef PRODUCT
              // DEBUG
              if( TraceSpilling ) {
                tty->print("\nNew Split DOWN DEF of Spill Idx ");
                tty->print("%d, UP %d:\n",slidx,false);
                n1->dump();
              }
#endif
            }
          }  // end if LRG is UP
        }  // end for all spilling live ranges
        assert( b->_nodes[insidx] == n, "got insidx set incorrectly" );
      }  // end if crossing HRP Boundry

      // If the LRG index is oob, then this is a new spillcopy, skip it.
      if( defidx >= _maxlrg ) {
        continue;
      }
      LRG &deflrg = lrgs(defidx);
      uint copyidx = n->is_Copy();
      // Remove coalesced copy from CFG
      if( copyidx && defidx == n2lidx(n->in(copyidx)) ) {
        n->replace_by( n->in(copyidx) );
        n->set_req( copyidx, NULL );
        b->_nodes.remove(insidx--);
        b->_ihrp_index--; // Adjust the point where we go hi-pressure
        b->_fhrp_index--;
        continue;
      }
      // ********** Handle USES **********
      const MachNode *mach = n->is_Mach();
      bool nullcheck = false;
      // Implicit null checks never use the spilled value
      if( mach && ((MachNode*)mach)->is_MachNullCheck() )
        nullcheck = true;
      if( !nullcheck ) {
        // Search all inputs for a Spill-USE
        JVMState* jvms = n->jvms();
        uint oopoff = jvms ? jvms->oopoff() : cnt;
        for( inpidx = 1; inpidx < cnt; inpidx++ ) {
          // Get lidx of input
          uint useidx = Find_id(n->in(inpidx));
          // Not a brand-new split, and it is a spill use
          if( useidx < _maxlrg && lrgs(useidx).reg() >= LRG::SPILL_REG ) {
            // Check for valid reaching DEF
            slidx = lrg2reach[useidx];
            Node *def = Reachblock[slidx];
            assert( def != NULL, "Using Undefined Value in Split()\n");

            // (+++) %%%% remove this in favor of pre-pass in matcher.cpp
            // monitor references do not care where they live, so just hook
            if ( jvms && jvms->is_monitor_use(inpidx) ) {
              // The effect of this clone is to drop the node out of the block,
              // so that the allocator does not see it anymore, and therefore
              // does not attempt to assign it a register.
              def = def->clone();
              _names.extend(def->_idx,0);
              _cfg._bbs.map(def->_idx,b);
              n->set_req(inpidx, def);
              if (C->unique() >= 64000) {  // Check when generating nodes
                prepare_to_bailout("Out of nodes during split");
                return 0;
              }
              continue;
            }
 
            // Rematerializable?  Then clone def at use site instead
            // of store/load
            if( def->rematerialize() ) {
	      int old_size = b->_nodes.size();
	      def = split_Rematerialize( def, b, insidx, maxlrg, splits, slidx, lrg2reach, Reachblock, true );
              if( !def ) return 0; // Bail out
	      insidx += b->_nodes.size()-old_size;
            }

            // Base pointers do not care where they live, so just hook
            if( mach && mach->ideal_Opcode() == Op_AddP && inpidx == AddPNode::Base ) {
              n->set_req(inpidx, def);
              continue;
            }
            // oop map references do not care where they live, so just hook
            if( inpidx >= oopoff ) {
              n->set_req(inpidx, def);
	      // After oopoff, we have derived/base pairs.  We must mention all
	      // derived pointers here as derived/base pairs for GC.  If the
	      // derived value is spilling and we have a copy both in Reachblock
	      // (called here 'def') and debug_defs[slidx] we need to mention
	      // both in derived/base pairs or kill one.
	      Node *derived_debug = debug_defs[slidx];
	      if( ((inpidx - oopoff) & 1) == 0 &&// derived vs base?
		  mach && mach->ideal_Opcode() != Op_Halt &&
		  derived_debug != NULL && 
		  derived_debug != def ) { // Actual 2nd value appears
		// We have already set 'def' as a derived value.
		// Also set debug_defs[slidx] as a derived value.
		uint k;
		for( k = oopoff; k < cnt; k += 2 )
		  if( n->in(k) == derived_debug )
		    break;	// Found an instance of debug derived
		if( k == cnt ) {// No instance of debug_defs[slidx]
		  // Add a derived/base pair to cover the debug info
		  n->add_req( derived_debug );
		  n->add_req( n->in(inpidx+1) );
		}
	      }
              continue;
            }
            // Special logic for DEBUG info
            if( jvms && b->_freq > 0.5 ) {
              uint debug_start = jvms->debug_start();
              // If this is debug info use & there is a reaching DOWN def
              if ((debug_start <= inpidx) && (debug_defs[slidx] != NULL)) {
                assert(inpidx < oopoff, "handle only debug info here");
                // Just hook it in & move on
                n->set_req(inpidx, debug_defs[slidx]);
                // (Note that this can make two sides of a split live at the
                // same time: The debug def on stack, and another def in a
                // register.  The GC needs to know about both of them, but any
                // derived pointers after oopoff will refer to only one of the
                // two defs and the GC would therefore miss the other.  Thus
		// this hack is only allowed for debug info which is Java state
		// and therefore never a derived pointer.)
                continue;
              }
            }
            // Grab register mask info
            const RegMask &dmask = def->out_RegMask();
            const RegMask &umask = n->in_RegMask(inpidx);

            assert(inpidx < oopoff, "cannot use-split oop map info");

            bool dup = UPblock[slidx];
            bool uup = umask.is_UP();

            // Need special logic to handle bound USES
            if( !umask.is_AllStack() && (int)umask.Size() <= lrgs(useidx).num_regs() && !def->rematerialize()) {
              // These need a Split regardless of overlap or pressure
              // SPLIT - NO DEF - NO CISC SPILL
              maxlrg = split_USE(def,b,n,inpidx,maxlrg,dup,false, splits,slidx);
	      // If it wasn't split bail
	      if (!maxlrg) {
		return 0;
	      }
              insidx++;  // Reset iterator to skip USE side split
              if( C->unique() >= 64000 ) {  // Check when generating nodes
                C->set_root(NULL);
                return 0;
              }
              continue;
            }
            // Here is the logic chart which describes USE Splitting:
            // 0 = false or DOWN, 1 = true or UP
            //
            // Overlap | DEF | USE | Action
            //-------------------------------------------------------
            //    0    |  0  |  0  | Copy - mem -> mem
            //    0    |  0  |  1  | Split-UP - Check HRP
            //    0    |  1  |  0  | Split-DOWN - Debug Info?
            //    0    |  1  |  1  | Copy - reg -> reg
            //    1    |  0  |  0  | Reset Input Edge (no Split)
            //    1    |  0  |  1  | Split-UP - Check HRP
            //    1    |  1  |  0  | Split-DOWN - Debug Info?
            //    1    |  1  |  1  | Reset Input Edge (no Split)
            //
            // So, if (dup == uup), then overlap test determines action,
            // with true being no split, and false being copy. Else,
            // if DEF is DOWN, Split-UP, and check HRP to decide on
            // resetting DEF. Finally if DEF is UP, Split-DOWN, with
            // special handling for Debug Info.
            if( dup == uup ) {
              if( dmask.overlap(umask) ) {
                // Both are either up or down, and there is overlap, No Split
                n->set_req(inpidx, def);
              }
              else {  // Both are either up or down, and there is no overlap
                if( dup ) {  // If UP, reg->reg copy
                  // COPY ACROSS HERE - NO DEF - NO CISC SPILL
                  maxlrg = split_USE(def,b,n,inpidx,maxlrg,false,false, splits,slidx);
		  // If it wasn't split bail
		  if (!maxlrg) {
		    return 0;
		  }
                  insidx++;  // Reset iterator to skip USE side split
                }
                else {       // DOWN, mem->mem copy
                  // COPY UP & DOWN HERE - NO DEF - NO CISC SPILL
                  // First Split-UP to move value into Register
                  uint def_ideal = def->ideal_reg();
                  const RegMask* tmp_rm = Matcher::idealreg2regmask[def_ideal];
                  Node *spill = new MachSpillCopyNode(def, dmask, *tmp_rm);
                  insert_proj( b, insidx, spill, maxlrg );
                  // Then Split-DOWN as if previous Split was DEF
                  maxlrg = split_USE(spill,b,n,inpidx,maxlrg,false,false, splits,slidx);
		  // If it wasn't split bail
		  if (!maxlrg) {
		    return 0;
		  }
                  insidx += 2;  // Reset iterator to skip USE side splits
                }
              }  // End else no overlap
            }  // End if dup == uup
            // dup != uup, so check dup for direction of Split
            else {
              if( dup ) {  // If UP, Split-DOWN and check Debug Info
                // If this node is already a SpillCopy, just patch the edge
                if( n->is_SpillCopy() && dmask.overlap(umask) ) {
                  n->set_req(inpidx, def);
                  continue;
                }
                // COPY DOWN HERE - NO DEF - NO CISC SPILL
                maxlrg = split_USE(def,b,n,inpidx,maxlrg,false,false, splits,slidx);
		// If it wasn't split bail
		if (!maxlrg) {
		  return 0;
		}
                insidx++;  // Reset iterator to skip USE side split
                // Check for debug-info split.  Capture it for later
                // debug splits of the same value
                if (jvms && jvms->debug_start() <= inpidx && inpidx < oopoff)
                  debug_defs[slidx] = n->in(inpidx);

              }
              else {       // DOWN, Split-UP and check register pressure
                if( is_high_pressure( b, &lrgs(useidx), insidx ) ) {
                  // COPY UP HERE - NO DEF - CISC SPILL
                  maxlrg = split_USE(def,b,n,inpidx,maxlrg,true,true, splits,slidx);
		  // If it wasn't split bail
		  if (!maxlrg) {
		    return 0;
		  }
                  insidx++;  // Reset iterator to skip USE side split
                } else {                          // LRP
                  // COPY UP HERE - WITH DEF - NO CISC SPILL
                  maxlrg = split_USE(def,b,n,inpidx,maxlrg,true,false, splits,slidx);
		  // If it wasn't split bail
		  if (!maxlrg) {
		    return 0;
		  }
                  // Flag this lift-up in a low-pressure block as
                  // already-spilled, so if it spills again it will
                  // spill hard (instead of not spilling hard and
                  // coalescing away).
                  set_was_spilled(n->in(inpidx));
                  // Since this is a new DEF, update Reachblock & UP
                  Reachblock[slidx] = n->in(inpidx);
                  UPblock[slidx] = true;
                  insidx++;  // Reset iterator to skip USE side split
                }
              }  // End else DOWN
            }  // End dup != uup
          }  // End if Spill USE
        }  // End For All Inputs
      }  // End If not nullcheck

      // ********** Handle DEFS **********
      // DEFS either Split DOWN in HRP regions or when the LRG is bound, or
      // just reset the Reaches info in LRP regions.  DEFS must always update
      // UP info.
      if( deflrg.reg() >= LRG::SPILL_REG ) {    // Spilled?
        uint slidx = lrg2reach[defidx];
        // Add to defs list for later assignment of new live range number
        defs->push(n);
        // Set a flag on the Node indicating it has already spilled.
        // Only do it for capacity spills not conflict spills.
        if( !deflrg._direct_conflict )
          set_was_spilled(n);
        assert(!n->is_Phi(),"Cannot insert Phi into DEFS list");
        // Grab UP info for DEF
        const RegMask &dmask = n->out_RegMask();
        bool defup = dmask.is_UP();
        // Only split at Def if this is a HRP block or bound (and spilled once)
        if( !n->rematerialize() &&
	    (((dmask.is_bound1() || dmask.is_bound2()) && (deflrg._direct_conflict || deflrg._must_spill)) ||
	     // Check for LRG being up in a register and we are inside a high 
	     // pressure area.  Spill it down immediately.
	     (defup && is_high_pressure(b,&deflrg,insidx))) ) {
          assert( !n->rematerialize(), "" );
          assert( !n->is_SpillCopy(), "" );
          // Do a split at the def site.
          maxlrg = split_DEF( n, b, insidx, maxlrg, Reachblock, debug_defs, splits, slidx );
	  // If it wasn't split bail
	  if (!maxlrg) {
	    return 0;
	  }
          // Split DEF's Down
          UPblock[slidx] = 0;
#ifndef PRODUCT
          // DEBUG
          if( TraceSpilling ) {
            tty->print("\nNew Split DOWN DEF of Spill Idx ");
            tty->print("%d, UP %d:\n",slidx,false);
            n->dump();
          }
#endif
        }
        else {                  // Neither bound nor HRP, must be LRP
          // otherwise, just record the def
          Reachblock[slidx] = n;
          // UP should come from the outRegmask() of the DEF
          UPblock[slidx] = defup;
          // Update debug list of reaching down definitions, kill if DEF is UP
          debug_defs[slidx] = defup ? NULL : n;
#ifndef PRODUCT
          // DEBUG
          if( TraceSpilling ) {
            tty->print("\nNew DEF of Spill Idx ");
            tty->print("%d, UP %d:\n",slidx,defup);
            n->dump();
          }
#endif
        }  // End else LRP
      }  // End if spill def

      // ********** Split Left Over Mem-Mem Moves **********
      // Check for mem-mem copies and split them now.  Do not do this
      // to copies about to be spilled; they will be Split shortly.
      if( copyidx ) {
        Node *use = n->in(copyidx);
        uint useidx = Find_id(use);
        if( useidx < _maxlrg &&       // This is not a new split
            deflrg.reg() >= SharedInfo::stack0 &&
            deflrg.reg() < LRG::SPILL_REG ) { // And DEF is from stack
          LRG &uselrg = lrgs(useidx);
          if( uselrg.reg() >= SharedInfo::stack0 &&
              uselrg.reg() < LRG::SPILL_REG && // USE is from stack
              deflrg.reg() != uselrg.reg() ) { // Not trivially removed
            uint def_ideal_reg = Matcher::base2reg[n->bottom_type()->base()];
            const RegMask &def_rm = *Matcher::idealreg2regmask[def_ideal_reg];
            const RegMask &use_rm = n->in_RegMask(copyidx);
            if( def_rm.overlap(use_rm) && (n->is_SpillCopy() != NULL) ) {  // Bug 4707800, 'n' may be a storeSSL
              if (C->unique() >= 64000) {  // Check when generating nodes
                C->set_root(NULL);
                return 0;
              }
              Node *spill = new MachSpillCopyNode(use,use_rm,def_rm);
              n->set_req(copyidx,spill);
              n->is_SpillCopy()->set_in_RegMask(def_rm);
              // Put the spill just before the copy
              insert_proj( b, insidx++, spill, maxlrg++ );
            }
          }
        }
      }
    }  // End For All Instructions in Block - Non-PHI Pass

    // Check for any live ranges needing 'isolating', or spilling before
    // an upcoming loop.  
    if( SplitLRGsAroundLoops && b->_num_succs == 1 ) {
      Block *l = b->_succs[0];
      if( b->_pre_order < l->_pre_order &&
	  l->_inner_loop && l->_inner_loop->_head == l ) {
	// Here we know 'b' is the loop pre-header
	InnerLoop *il = l->_inner_loop;
	// Check all spilling live ranges for being 'isolated'.
	for( slidx = 0; slidx < spill_cnt; slidx++ ) {
	  // Live range may not yet be def'd
	  if( !Reachblock[slidx] ) continue;
	  uint lidx = lidxs.at(slidx);
	  if( il->_isolate_lrgs.test(lidx) ) {
	    // Reaching def from current loop pre-header?
	    // Then give up isolating this lrg in the loop!
	    if( _cfg._bbs[Reachblock[slidx]->_idx] == b && 
		!Reachblock[slidx]->is_Phi() ) {
	      il->_isolate_lrgs >>= lidx;
	    // Found a live range to isolate
	    } else if( il->_ud_lrgs.test(lidx) ) {
	      // Use/def'd in loop.  See if we can pre-split before the
	      // loop and make progress.
	      if( !UPblock[slidx] ) {// Live range currently down
		//tty->print_cr("Referenced L%d split UP at loop B%d", lidx, l->_pre_order);
                maxlrg = split_DEF( Reachblock[slidx], b, b->end_idx()-1, maxlrg, Reachblock, debug_defs, splits, slidx);
		if (!maxlrg) return 0;
		UPblock[slidx] = 1; // It's up now
	      }
	    } else {		// Else not use/def'd in loop, but must span
	      if( UPblock[slidx] ) {
		//tty->print_cr("Spanning L%d split DOWN at loop B%d", lidx, l->_pre_order);
                maxlrg = split_DEF( Reachblock[slidx], b, b->end_idx()-1, maxlrg, Reachblock, debug_defs, splits, slidx);
		if (!maxlrg) return 0;
		UPblock[slidx] = 0; // It's down now
	      }
	    }
	  }
	}
      }
    }

    // Check if each LRG is live out of this block so as not to propagate
    // beyond the last use of a LRG.
    for( slidx = 0; slidx < spill_cnt; slidx++ ) {
      uint defidx = lidxs.at(slidx);
      IndexSet *liveout = _live->live(b);
      if( !liveout->member(defidx) )
        Reachblock[slidx] = NULL;
      else
        assert(Reachblock[slidx] != NULL,"No reaching definition for liveout value");
    }
#ifndef PRODUCT
    if( TraceSpilling )
      b->dump();
#endif
  }  // End For All Blocks

  //----------PASS 2----------
  // Reset all DEF live range numbers here
  for( insidx = 0; insidx < defs->size(); insidx++ ) {
    // Grab the def
    n1 = defs->at(insidx);
    // Set new lidx for DEF
    MachNode *mach = n1->is_Mach();
    new_lrg(n1, maxlrg++);
  }
  //----------Phi Node Splitting----------
  // Clean up a phi here, and assign a new live range number
  // Cycle through this block's predecessors, collecting Reaches
  // info for each spilled LRG and update edges.
  // Walk the phis list to patch inputs, split phis, and name phis
  for( insidx = 0; insidx < phis->size(); insidx++ ) {
    Node *phi = phis->at(insidx);
    assert(phi->is_Phi() != NULL,"This list must only contain Phi Nodes");
    Block *b = _cfg._bbs[phi->_idx];
    // Grab the live range number
    uint lidx = Find_id(phi);
    uint slidx = lrg2reach[lidx];
    // Update node to lidx map
    new_lrg(phi, maxlrg++);
    // Get PASS1's up/down decision for the block.
    int phi_up = !!UP_entry[slidx]->test(b->_pre_order);

    // Force down if double-spilling live range
    if( lrgs(lidx)._was_spilled1 )
      phi_up = false;

    // When splitting a Phi we an split it normal or "inverted".
    // An inverted split makes the splits target the Phi's UP/DOWN
    // sense inverted; then the Phi is followed by a final def-side
    // split to invert back.  It changes which blocks the spill code
    // goes in.
    float f = -b->_freq;

    // Walk the predecessor blocks and assign the reaching def to the Phi.
    // Split Phi nodes by placing USE side splits wherever the reaching 
    // DEF has the wrong UP/DOWN value.  
    for( uint i = 1; i < b->num_preds(); i++ ) {
      // Get predecessor block pre-order number
      Block *pred = _cfg._bbs[b->pred(i)->_idx];
      pidx = pred->_pre_order;
      // Grab reaching def
      Node *def = Reaches[pidx][slidx];
      assert( def, "must have reaching def" );
      // If input up/down sense and reg-pressure DISagree
      if( def->rematerialize() ) { 
	def = split_Rematerialize( def, pred, pred->end_idx(), maxlrg, splits, slidx, lrg2reach, Reachblock, false );
        if( !def ) return 0;    // Bail out
        f += pred->_freq;
      }
      // Update the Phi's input edge array
      phi->set_req(i,def);
      // Grab the UP/DOWN sense for the input
      u1 = UP[pidx][slidx];
      if( u1 != phi_up ) {
        maxlrg = split_USE(def, b, phi, i, maxlrg, !u1, false, splits,slidx);
	// If it wasn't split bail
	if (!maxlrg) {
	  return 0;
	}
        f += pred->_freq;
      } else {
        f -= pred->_freq;
      }
    }  // End for all inputs to the Phi
#ifndef PRODUCT
    if(false /* PrintOpto && Verbose && f > 0.001. Need new flag here*/) 
      tty->print_cr("gain for inverting: %g",f);
#endif
  }  // End for all Phi Nodes
  // Update _maxlrg to save Union asserts
  _maxlrg = maxlrg;


  //----------PASS 3----------
  // Pass over all Phi's to union the live ranges
  for( insidx = 0; insidx < phis->size(); insidx++ ) {
    Node *phi = phis->at(insidx);
    assert(phi->is_Phi() != NULL,"This list must only contain Phi Nodes");
    // Walk all inputs to Phi and Union input live range with Phi live range
    for( uint i = 1; i < phi->req(); i++ ) {
      // Grab the input node
      Node *n = phi->in(i);
      assert( n, "" );
      uint lidx = Find(n);
      uint pidx = Find(phi);
      if( lidx < pidx )
        Union(n, phi);
      else if( lidx > pidx )
        Union(phi, n);
    }  // End for all inputs to the Phi Node
  }  // End for all Phi Nodes
  // Now union all two address instructions
  for( insidx = 0; insidx < defs->size(); insidx++ ) {
    // Grab the def
    n1 = defs->at(insidx);
    // Set new lidx for DEF & handle 2-addr instructions
    MachNode *mach = n1->is_Mach();
    if( mach && ((twoidx = mach->two_adr()) != 0) ) {
      assert( Find(n1->in(twoidx)) < maxlrg,"Assigning bad live range index");
      // Union the input and output live ranges
      uint lr1 = Find(n1);
      uint lr2 = Find(n1->in(twoidx));
      if( lr1 < lr2 )
        Union(n1, n1->in(twoidx));
      else if( lr1 > lr2 ) 
        Union(n1->in(twoidx), n1);
    }  // End if two address
  }  // End for all defs
  // DEBUG
#ifdef ASSERT
  // Validate all live range index assignments
  for( bidx = 0; bidx < _cfg._num_blocks; bidx++ ) {
    b  = _cfg._blocks[bidx];
    for( insidx = 0; insidx <= b->end_idx(); insidx++ ) {
      Node *n = b->_nodes[insidx];
      uint defidx = Find(n);
      assert(defidx < _maxlrg,"Bad live range index in Split");
      assert(defidx < maxlrg,"Bad live range index in Split");
    }
  }
  // Issue a warning if splitting made no progress
  int noprogress = 0;
  for( slidx = 0; slidx < spill_cnt; slidx++ ) {
    if( PrintOpto && WizardMode && splits.at(slidx) == 0 ) {
      tty->print_cr("Failed to split live range %d", lidxs.at(slidx));
      //BREAKPOINT;
    }
    else {
      noprogress++;
    }
  }
  if(!noprogress) {
    tty->print_cr("Failed to make progress in Split");
    //BREAKPOINT;
  }
#endif
  // Return updated count of live ranges
  return maxlrg;
}


//------------------------------isolate_candidate--------------------------
bool PhaseChaitin::isolate_candidate( InnerLoop *il, Block *bpre, uint lidx ) const {
  if( !il->_ud_lrgs.test(lidx) ) return false; // Not use/def'd in loop
  if( lrgs(lidx)._was_spilled2 ) return false;	// Not already spilled once
  Node *def = lrgs(lidx)._def;
  if( def != (Node*)-1 &&	// Single def in pre-header block?
      _cfg._bbs[def->_idx] == bpre )
    return false;		// Isolating will make no progress
  return true;
}

//------------------------------pre_split_around_loops---------------------
// Pre-split colored live ranges around inner loops that are spilling.
// The input to this routine is a set of live ranges use'd or def'd in
// all inner loops (along with the obvious CFG structures).  The output
// of this routine is set of live ranges to isolate around each inner
// loop.  Loops that have no spilling are ignored.  Loops with spilling
// gather 2 kinds of live ranges - those that span the loop without being
// use'd or def'd and those that are referenced in the loop.
//
// Spanning live ranges may have colored.  In any case, they are forced
// down at the loop entry.  The normal Split action will load them back
// up at their next use.  Referenced live ranges have their non-loop
// parts split off at the loop exits.  Since the spanning live ranges are
// now down, the referenced live range chunks in the loop should get a
// register.
//
void PhaseChaitin::pre_split_around_loops( GrowableArray<uint> &lidxs, uint spill_cnt ) {
  if( !SplitLRGsAroundLoops ) return;

  // Find an inner loop with spilling
  // For all inner loops
  for( InnerLoop *il = _cfg._loops; il; il = il->_next ) {

    // Find loop header block, entry & exit blocks
    Block *bhead = il->_head;
    Block *btail = _cfg._bbs[bhead->head()->in(LoopNode::LoopBackControl)->_idx];
    Block *bpre  = _cfg._bbs[bhead->head()->in(LoopNode::EntryControl   )->_idx];

    // For all spilling live ranges, check to see if its use/def'd in
    // loop.  Don't bother with previously spilled lrg's.
    uint spill_cnt_in_loop[2];  // track int and float spills separately
    spill_cnt_in_loop[0] = spill_cnt_in_loop[1] = 0;
    for( uint slidx = 0; slidx < spill_cnt; slidx++ ) {
      uint lidx = lidxs.at(slidx);
      if( isolate_candidate( il, bpre, lidx ) ) {
	il->_isolate_lrgs.set(lidx);
	spill_cnt_in_loop[lrgs(lidx)._is_float]++;
      }
    }
    // Check to see if loop has spilling of 1st-timers
    if( (spill_cnt_in_loop[0] + spill_cnt_in_loop[1]) == 0) continue;

    // Bail out if the loop is very busy.  Very busy loops will spill
    // no matter what; there's no way to keep a value live in a
    // register across the whole loop.  So don't try, it will cause
    // needless double-spilling.
    if( (int)il->_ud_lrgs.Size() > INTPRESSURE*3 ) {
      il->_isolate_lrgs.Clear();
      continue;			// Loop too busy; don't try.
    }

    // This inner loop has spilling!  Find all live ranges that span
    // the loop (with no use/def), got a color and that color is
    // acceptable to some live range spilling in the loop.  Spilling
    // such live ranges around the loop will free up a register for
    // the live range IN the loop.
    ResourceMark rm;

    // Start with those LRG's live-out of the loop exit block.
    IndexSet *liveout = _live->live(btail);
    IndexSet spanning_lrgs(liveout);
    // Remove the stack pointer as an interesting spanning live range
    uint fp_lrg = n2lidx(((Node*)_cfg._root)->in(1)->in(TypeFunc::FramePtr));
    spanning_lrgs.remove(fp_lrg);

    // Take the live-out LRG's from the exit block and remove any that
    // are use/def'd in the loop body.  Take out live ranges colored
    // to the stack.  What remains spans the loop in a register.
    IndexSetIterator elements(liveout);
    uint lidx;
    while( (lidx = elements.next()) != 0 ) {
      if( il->_ud_lrgs.test(lidx) ||
	  (lrgs(lidx).reg() >= SharedInfo::stack0 &&
	   lrgs(lidx).reg() < LRG::SPILL_REG ) )
	spanning_lrgs.remove(lidx);
    }

    /*
    tty->print("Inner loop spills %d lrgs: ",spill_cnt_in_loop);
    bhead->dump_head( &_cfg._bbs );

    tty->print_cr("-- Spilling lrgs: --");
    for( uint slidx = 0; slidx < spill_cnt; slidx++ ) {
      uint lidx = lidxs.at(slidx);
      if( il->_ud_lrgs.test(lidx) ) {
	tty->print("L%d: ",lidx);
	lrgs(lidx).dump();
      }
    }
    tty->print_cr("-- Spanning lrgs in registers or spilling: --");
    IndexSetIterator slx1(&spanning_lrgs);
    while( (lidx = slx1.next()) != 0 ) {
      tty->print("L%d: ",lidx);
      lrgs(lidx).dump();
    }
    */

    // Isolate enough spanning lrgs to get the spillers registers.  Start
    // with all those that will spill anyways.  

    uint cnt_spans[2];  // count float and int spans separately
    cnt_spans[0] = cnt_spans[1] = 0;
    IndexSetIterator slx2(&spanning_lrgs);
    while(( cnt_spans[0] < spill_cnt_in_loop[0] || cnt_spans[1] < spill_cnt_in_loop[1]) && (lidx = slx2.next()) != 0 ) {
      uint is_float = lrgs(lidx)._is_float;
      if( lrgs(lidx).reg() >= LRG::SPILL_REG  && cnt_spans[is_float] < spill_cnt_in_loop[is_float]) {
	cnt_spans[is_float]++;
	il->_isolate_lrgs.set(lidx);
      }
    }
    /*
    // Since we don't want them
    // spilling-down in hi-pressure areas in the loop, isolate all the
    // spillers; this will force them to spill down BEFORE the loop.
    */
    /*
    tty->print("Lrgs to Isolate: ");
    il->_isolate_lrgs.print();
    tty->cr();
    */

  }

}

