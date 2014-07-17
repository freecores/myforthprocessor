#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)buildOopMap.cpp	1.22 03/02/28 16:49:38 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_buildOopMap.cpp.incl"


// The functions in this file builds OopMaps after all scheduling is done.
//
// OopMaps contain a list of all registers and stack-slots containing oops (so
// they can be updated by GC).  OopMaps also contain a list of derived-pointer
// base-pointer pairs.  When the base is moved, the derived pointer moves to
// follow it.  Finally, any registers holding callee-save values are also
// recorded.  These might contain oops, but only the caller knows.
//
// BuildOopMaps implements a simple forward reaching-defs solution.  At each
// GC point we'll have the reaching-def Nodes.  If the reaching Nodes are
// typed as pointers (no offset), then they are oops.  Pointers+offsets are
// derived pointers, and bases can be found from them.  Finally, we'll also
// track reaching callee-save values.  Note that a copy of a callee-save value
// "kills" it's source, so that only 1 copy of a callee-save value is alive at
// a time.
//
// We run a simple bitvector liveness pass to help trim out dead oops.  Due to
// irreducible loops, we can have a reaching def of an oop that only reaches
// along one path and no way to know if it's valid or not on the other path.
// The bitvectors are quite dense and the liveness pass is fast.
//
// At GC points, we consult this information to build OopMaps.  All reaching
// defs typed as oops are added to the OopMap.  Only 1 instance of a
// callee-save register can be recorded.  For derived pointers, we'll have to
// find and record the register holding the base.
//
// The reaching def's is a simple 1-pass worklist approach.  I tried a clever
// breadth-first approach but it was worse (showed O(n^2) in the
// pick-next-block code).
//
// The relevent data is kept in a struct of arrays (it could just as well be
// an array of structs, but the struct-of-arrays is generally a little more
// efficient).  The arrays are indexed by register number (including
// stack-slots as registers) and so is bounded by 200 to 300 elements in
// practice.  One array will map to a reaching def Node (or NULL for
// conflict/dead).  The other array will map to a callee-saved register or
// OptoReg::Bad for not-callee-saved.


//------------------------------OopFlow----------------------------------------
// Structure to pass around 
struct OopFlow : public ResourceObj {
  short *_callees;              // Array mapping register to callee-saved 
  Node **_defs;                 // array mapping register to reaching def
                                // or NULL if dead/conflict
  // OopFlow structs, when not being actively modified, describe the _end_ of
  // this block.
  Block *_b;                    // Block for this struct
  OopFlow *_next;               // Next free OopFlow

  OopFlow( short *callees, Node **defs ) : _callees(callees), _defs(defs),
    _b(NULL), _next(NULL) { }

  // Given reaching-defs for this block start, compute it for this block end
  void compute_reach( PhaseRegAlloc *regalloc, int max_reg, Dict *safehash );

  // Merge these two OopFlows into the 'this' pointer.
  void merge( OopFlow *flow, int max_reg );

  // Copy a 'flow' over an existing flow
  void clone( OopFlow *flow, int max_size);

  // Make a new OopFlow from scratch
  static OopFlow *make( Arena *A, int max_size );

  // Build an oopmap from the current flow info
  OopMap *build_oop_map( Node *n, int max_reg, PhaseRegAlloc *regalloc, int* live );
};

//------------------------------compute_reach----------------------------------
// Given reaching-defs for this block start, compute it for this block end
void OopFlow::compute_reach( PhaseRegAlloc *regalloc, int max_reg, Dict *safehash ) {

  for( uint i=0; i<_b->_nodes.size(); i++ ) {
    Node *n = _b->_nodes[i];

    if( n->jvms() ) {           // Build an OopMap here?
      JVMState *jvms = n->jvms();
      MachNode *mach = n->is_Mach();
      MachSafePointNode *sfn = mach ? mach->is_MachSafePoint() : NULL;
      // no map needed for leaf calls
      if (sfn != NULL && sfn->is_MachCallLeaf()) sfn = NULL;
      if( sfn ) {
        int *live = (int*) (*safehash)[n];
        assert( live, "must find live" );
        n->set_oop_map( build_oop_map(n,max_reg,regalloc, live) );
      }
    }

    // Assign new reaching def's.
    // Note that I padded the _defs and _callees arrays so it's legal
    // to index at _defs[OptoReg::Bad].
    OptoReg::Name lo = regalloc->get_reg_lo(n);
    OptoReg::Name hi = regalloc->get_reg_hi(n);
    _defs[lo] = n;
    _defs[hi] = n;

    // Pass callee-save info around copies
    int idx = n->is_Copy();
    if( idx ) {                 // Copies move callee-save info
      OptoReg::Name old_lo = regalloc->get_reg_lo(n->in(idx));
      OptoReg::Name old_hi = regalloc->get_reg_hi(n->in(idx));
      int tmp_lo = _callees[old_lo];
      int tmp_hi = _callees[old_hi];
      _callees[old_lo] = OptoReg::Bad; // callee-save is moved, dead in old location
      _callees[old_hi] = OptoReg::Bad;
      _callees[lo] = tmp_lo;
      _callees[hi] = tmp_hi;
    } else if( n->is_Phi() ) {  // Phis do not mod callee-saves
      assert( _callees[lo] == _callees[regalloc->get_reg_lo(n->in(1))], "" );
      assert( _callees[hi] == _callees[regalloc->get_reg_hi(n->in(1))], "" );
      assert( _callees[lo] == _callees[regalloc->get_reg_lo(n->in(n->req()-1))], "" );
      assert( _callees[hi] == _callees[regalloc->get_reg_hi(n->in(n->req()-1))], "" );
    } else {
      _callees[lo] = OptoReg::Bad; // No longer holding a callee-save value
      _callees[hi] = OptoReg::Bad;

      // Find base case for callee saves
      if( n->is_Proj() && n->in(0)->is_Start() ) {
        if( lo != OptoReg::Bad && lo < SharedInfo::stack0 &&
            regalloc->_matcher.is_save_on_entry(lo) )
          _callees[lo] = lo;
        if( hi != OptoReg::Bad && hi < SharedInfo::stack0 &&
            regalloc->_matcher.is_save_on_entry(hi) )
          _callees[hi] = hi;
      }
    }
  }
}

//------------------------------merge------------------------------------------
// Merge the given flow into the 'this' flow
void OopFlow::merge( OopFlow *flow, int max_reg ) {
  assert( _b == NULL, "merging into a happy flow" );
  assert( flow->_b, "this flow is still alive" );
  assert( flow != this, "no self flow" );

  // Do the merge.  If there are any differences, drop to 'bottom' which
  // is OptoReg::Bad or NULL depending.
  for( int i=0; i<max_reg; i++ ) {
    // Merge the callee-save's
    if( _callees[i] != flow->_callees[i] )
      _callees[i] = OptoReg::Bad;
    // Merge the reaching defs
    if( _defs[i] != flow->_defs[i] )
      _defs[i] = NULL;
  }

}

//------------------------------clone------------------------------------------
void OopFlow::clone( OopFlow *flow, int max_size ) {
  _b = flow->_b;
  memcpy( _callees, flow->_callees, sizeof(short)*max_size);
  memcpy( _defs   , flow->_defs   , sizeof(Node*)*max_size);
}

//------------------------------make-------------------------------------------
OopFlow *OopFlow::make( Arena *A, int max_size ) {
  short *callees = NEW_ARENA_ARRAY(A,short,max_size+1);
  Node **defs    = NEW_ARENA_ARRAY(A,Node*,max_size+1);
  OopFlow *flow = new (A) OopFlow(callees+1,defs+1);
  assert( &flow->_callees[OptoReg::Bad] == callees, "Ok to index at OptoReg::Bad" );
  assert( &flow->_defs   [OptoReg::Bad] == defs   , "Ok to index at OptoReg::Bad" );
  return flow;
}

//------------------------------bit twiddlers----------------------------------
static int get_live_bit( int *live, int reg ) {
  return live[reg>>LogBitsPerInt] &   (1<<(reg&(BitsPerInt-1))); }
static void set_live_bit( int *live, int reg ) {
         live[reg>>LogBitsPerInt] |=  (1<<(reg&(BitsPerInt-1))); }
static void clr_live_bit( int *live, int reg ) {
         live[reg>>LogBitsPerInt] &= ~(1<<(reg&(BitsPerInt-1))); }

//------------------------------build_oop_map----------------------------------
// Build an oopmap from the current flow info
OopMap *OopFlow::build_oop_map( Node *n, int max_reg, PhaseRegAlloc *regalloc, int* live ) {
  int framesize = regalloc->_framesize;
  int max_inarg_slot = regalloc->_matcher._new_SP - SharedInfo::stack0;
  debug_only( char *dup_check = NEW_RESOURCE_ARRAY(char,SharedInfo::stack0); 
	      memset(dup_check,0,SharedInfo::stack0) );

  OopMap *omap = new OopMap( framesize,  max_inarg_slot );
  MachCallNode *mcall = n->is_Mach()->is_MachCall();
  MachCallInterpreterNode *mint = mcall ? mcall->is_MachCallInterpreter() : NULL;
  JVMState* jvms = n->jvms();

  // For all registers do...
  for( int reg=0; reg<max_reg; reg++ ) {
    if( get_live_bit(live,reg) == 0 )
      continue;                 // Ignore if not live

    // See if dead (no reaching def).
    Node *def = _defs[reg];     // Get reaching def
    assert( def, "since live better have reaching def" );

    // Classify the reaching def as oop, derived, callee-save, dead, or other
    const Type *t = def->bottom_type();
    if( t->isa_oop_ptr() ) {    // Oop or derived?
      assert( _callees[reg] == OptoReg::Bad, "oop can't be callee save" );
#ifdef _LP64
      // 64-bit pointers record oop-ishness on 2 aligned adjacent registers.
      // Make sure both are record from the same reaching def, but do not
      // put both into the oopmap.
      if( (reg&1) == 1 ) {      // High half of oop-pair?
        assert( _defs[reg-1] == _defs[reg], "both halves from same reaching def" );
        continue;               // Do not record high parts in oopmap
      }
#endif
      if( t->is_ptr()->_offset == 0 ) { // Not derived?
        // Interpreter calls have nothing live.  The current stack crawls
        // cannot handle reaching-but-dead oops in C2I adapters.
        if( mint ) continue;
        if( mcall ) {
          // Outgoing argument GC mask responsibility belongs to the callee, 
          // not the caller.  Inspect the inputs to the call, to see if 
          // this live-range is one of them.
          uint cnt = mcall->tf()->domain()->cnt();
          uint j;
          for( j = TypeFunc::Parms; j < cnt; j++)
            if( mcall->in(j) == def )
              break;            // reaching def is an argument oop
          if( j < cnt )         // arg oops dont go in GC map
            continue;           // Continue on to the next register
        }
        omap->set_oop( OptoReg::Name(reg), framesize, max_inarg_slot );
      } else {                  // Else it's derived.  
        // Find the base of the derived value.
        uint i;
        // Fast, common case, scan
        for( i = jvms->oopoff(); i < n->req(); i+=2 ) 
          if( n->in(i) == def ) break; // Common case
        if( i == n->req() ) {   // Missed, try a more generous scan
          // Scan again, but this time peek through copies
          for( i = jvms->oopoff(); i < n->req(); i+=2 ) {
            Node *m = n->in(i); // Get initial derived value
            while( 1 ) {
              Node *d = def;    // Get initial reaching def
              while( 1 ) {      // Follow copies of reaching def to end
                if( m == d ) goto found; // breaks 3 loops
                int idx = d->is_Copy();
                if( !idx ) break;
                d = d->in(idx);     // Link through copy
              }
              int idx = m->is_Copy();
              if( !idx ) break;
              m = m->in(idx);
            }
          }
         guarantee( 0, "must find derived/base pair" );
        }
      found: ;
        Node *base = n->in(i+1); // Base is other half of pair
        int breg = regalloc->get_reg_lo(base);

        // I record liveness at safepoints BEFORE I make the inputs
        // live.  This is because argument oops are NOT live at a
        // safepoint (or at least they cannot appear in the oopmap).
        // Thus bases of base/derived pairs might not be in the
        // liveness data but they need to appear in the oopmap.
        if( get_live_bit(live,breg) == 0 ) {// Not live?
          // Flag it, so next derived pointer won't re-insert into oopmap
          set_live_bit(live,breg);
          if( breg < reg )      // Already missed our turn?
            omap->set_oop( OptoReg::Name(breg), framesize, max_inarg_slot );
        }
        omap->set_derived_oop( OptoReg::Name(reg), framesize, max_inarg_slot, 
                               OptoReg::Name(breg) );
      }

    } else if( _callees[reg] != OptoReg::Bad ) { // callee-save?
      // It's a callee-save value
      assert( dup_check[_callees[reg]]==0, "trying to callee save same reg twice" );
      debug_only( dup_check[_callees[reg]]=1; )
      omap->set_callee_saved( OptoReg::Name(reg), framesize, max_inarg_slot, OptoReg::Name(_callees[reg]) );

    } else {
      // Other - some reaching non-oop value
      omap->set_value( OptoReg::Name(reg), framesize, max_inarg_slot );
    }

  }

#ifdef ASSERT
  /* Nice, Intel-only assert
  int cnt_callee_saves=0;
  for( int reg2=0; reg2<SharedInfo::stack0; reg2++ ) 
    if( dup_check[reg2] != 0) cnt_callee_saves++;
  assert( cnt_callee_saves==3 || cnt_callee_saves==5, "missed some callee-save" );
  */
#endif
  
  return omap;
}

//------------------------------do_liveness------------------------------------
// Compute backwards liveness on registers
static void do_liveness( PhaseRegAlloc *regalloc, PhaseCFG *cfg, Block_List *worklist, int max_reg_ints, Arena *A, Dict *safehash ) {
  int *live = NEW_ARENA_ARRAY(A, int, (cfg->_num_blocks+1) * max_reg_ints);
  int *tmp_live = &live[cfg->_num_blocks * max_reg_ints];
  memset( live, 0, cfg->_num_blocks * (max_reg_ints<<LogBytesPerInt) );
  // Push preds onto worklist
  for( uint i=1; i<cfg->C->root()->req(); i++ )
    worklist->push(cfg->_bbs[cfg->C->root()->in(i)->_idx]);

  // ZKM.jar includes tiny infinite loops which are unreached from below.
  // If we missed any blocks, we'll retry here after pushing all missed
  // blocks on the worklist.  Normally this outer loop never trips more
  // than once.
  while( 1 ) {

    while( worklist->size() ) { // Standard worklist algorithm
      Block *b = worklist->rpop();
      
      // Copy first successor into my tmp_live space
      int s0num = b->_succs[0]->_pre_order;
      int *t = &live[s0num*max_reg_ints];
      for( int i=0; i<max_reg_ints; i++ )
        tmp_live[i] = t[i];
      
      // OR in the remaining live registers
      for( uint j=1; j<b->_num_succs; j++ ) {
        uint sjnum = b->_succs[j]->_pre_order;
        int *t = &live[sjnum*max_reg_ints];
        for( int i=0; i<max_reg_ints; i++ )
          tmp_live[i] |= t[i];
      }
      
      // Now walk tmp_live up the block backwards, computing live
      for( int k=b->_nodes.size()-1; k>=0; k-- ) {
        Node *n = b->_nodes[k];
        // KILL def'd bits
        int lo = regalloc->get_reg_lo(n);
        int hi = regalloc->get_reg_hi(n);
        if( lo != OptoReg::Bad ) clr_live_bit(tmp_live,lo);
        if( hi != OptoReg::Bad ) clr_live_bit(tmp_live,hi);

        // Native wrappers handlize oops in stack slots.
        // Such slots have no obvious uses, but are live everywhere.
        MachNode *m = n->is_Mach();
        if( m && m->ideal_Opcode() == Op_Box ) {
          int lo = regalloc->get_reg_lo(n->in(1));
          int hi = regalloc->get_reg_hi(n->in(1));
          if( lo != OptoReg::Bad ) {
            for( uint i=0; i<cfg->_num_blocks; i++ )
              set_live_bit(&live[i*max_reg_ints],lo);
          }
          if( hi != OptoReg::Bad ) {
            for( uint i=0; i<cfg->_num_blocks; i++ )
              set_live_bit(&live[i*max_reg_ints],hi);
          }
        }

        // GEN use'd bits
        for( uint l=1; l<n->req(); l++ ) {
          Node *m = n->in(l);
          if( m ) {
            int lo = regalloc->get_reg_lo(m);
            int hi = regalloc->get_reg_hi(m);
            if( lo != OptoReg::Bad ) set_live_bit(tmp_live,lo);
            if( hi != OptoReg::Bad ) set_live_bit(tmp_live,hi);
          }
        }

        if( n->jvms() ) {       // Record liveness at safepoint

          // This placement of this stanza means inputs to calls are
          // considered live at the callsite's OopMap.  Argument oops are
          // hence live, but NOT included in the oopmap.  See cutout in
          // build_oop_map.  Debug oops are live (and in OopMap).
          int *n_live = NEW_ARENA_ARRAY(A, int, max_reg_ints);
          for( int l=0; l<max_reg_ints; l++ )
            n_live[l] = tmp_live[l];
          safehash->Insert(n,n_live);
        }

      }
      
      // Now at block top, see if we have any changes.  If so, propagate
      // to prior blocks.
      int *old_live = &live[b->_pre_order*max_reg_ints];
      int l;
      for( l=0; l<max_reg_ints; l++ )
        if( tmp_live[l] != old_live[l] )
          break;
      if( l<max_reg_ints ) {     // Change!
        // Copy in new value
        for( l=0; l<max_reg_ints; l++ )
          old_live[l] = tmp_live[l];
        // Push preds onto worklist
        for( l=1; l<(int)b->num_preds(); l++ )
          worklist->push(cfg->_bbs[b->pred(l)->_idx]);
      }
    }
    
    // Scan for any missing safepoints.  Happens to infinite loops
    // ala ZKM.jar
    uint i;
    for( i=1; i<cfg->_num_blocks; i++ ) {
      Block *b = cfg->_blocks[i];
      uint j;
      for( j=1; j<b->_nodes.size(); j++ )
        if( b->_nodes[j]->jvms() &&
            (*safehash)[b->_nodes[j]] == NULL )
           break;
      if( j<b->_nodes.size() ) break;
    }
    if( i == cfg->_num_blocks )
      break;                    // Got 'em all
    if( PrintOpto && Verbose )
      tty->print_cr("retripping live calc");
    // Force the issue (expensively): recheck everybody
    for( i=1; i<cfg->_num_blocks; i++ )
      worklist->push(cfg->_blocks[i]);
  }

}

//------------------------------BuildOopMaps-----------------------------------
// Collect GC mask info - where are all the OOPs?
void Compile::BuildOopMaps() {
  TracePhase t3("bldOopMaps", &_t_buildOopMaps, TimeCompiler);
  // Can't resource-mark because I need to leave all those OopMaps around,
  // or else I need to resource-mark some arena other than the default.
  // ResourceMark rm;              // Reclaim all OopFlows when done
  int max_reg = _regalloc->_max_reg; // Current array extent

  Arena *A = Thread::current()->resource_area();
  Block_List worklist;          // Worklist of pending blocks
  
  int max_reg_ints = round_to(max_reg, BitsPerInt)>>LogBitsPerInt;
  Dict *safehash = NULL;        // Used for assert only
  // Compute a backwards liveness per register.  Needs a bitarray of
  // #blocks x (#registers, rounded up to ints)
  safehash = new Dict(cmpkey,hashkey,A);
  do_liveness( _regalloc, _cfg, &worklist, max_reg_ints, A, safehash );
  OopFlow *free_list = NULL;    // Free, unused

  // Array mapping blocks to completed oopflows
  OopFlow **flows = NEW_ARENA_ARRAY(A, OopFlow*, _cfg->_num_blocks);
  memset( flows, 0, _cfg->_num_blocks*sizeof(OopFlow*) );


  // Do the first block 'by hand' to prime the worklist
  Block *entry = _cfg->_blocks[1];
  OopFlow *rootflow = OopFlow::make(A,max_reg);
  // Initialize to 'bottom' (not 'top')
  memset( rootflow->_callees, OptoReg::Bad, max_reg*sizeof(short) );
  memset( rootflow->_defs   ,            0, max_reg*sizeof(Node*) );
  flows[entry->_pre_order] = rootflow;

  // Do the first block 'by hand' to prime the worklist
  rootflow->_b = entry;
  rootflow->compute_reach( _regalloc, max_reg, safehash );
  for( uint i=0; i<entry->_num_succs; i++ )
    worklist.push(entry->_succs[i]);

  // Now worklist contains blocks which have some, but perhaps not all,
  // predecessors visited.
  while( worklist.size() ) {
    // Scan for a block with all predecessors visited, or any randoms slob
    // otherwise.  All-preds-visited order allows me to recycle OopFlow
    // structures rapidly and cut down on the memory footprint.
    // Note: not all predecessors might be visited yet (must happen for
    // irreducible loops).  This is OK, since every live value must have the
    // SAME reaching def for the block, so any reaching def is OK.  
    uint i;

    Block *b = worklist.pop(); 
    // Ignore root block
    if( b == _cfg->_broot ) continue;
    // Block is already done?  Happens if block has several predecessors,
    // he can get on the worklist more than once.
    if( flows[b->_pre_order] ) continue;

    // If this block has a visited predecessor AND that predecessor has this
    // last block as his only undone child, we can move the OopFlow from the
    // pred to this block.  Otherwise we have to grab a new OopFlow.
    OopFlow *flow = NULL;       // Flag for finding optimized flow
    Block *pred = (Block*)0xdeadbeef;
    uint j;
    // Scan this block's preds to find a done predecessor
    for( j=1; j<b->num_preds(); j++ ) {
      Block *p = _cfg->_bbs[b->pred(j)->_idx];
      OopFlow *p_flow = flows[p->_pre_order];
      if( p_flow ) {            // Predecessor is done
        assert( p_flow->_b == p, "cross check" );
        pred = p;               // Record some predecessor
        // If all successors of p are done except for 'b', then we can carry
        // p_flow forward to 'b' without copying, otherwise we have to draw
        // from the free_list and clone data.
        uint k;
        for( k=0; k<p->_num_succs; k++ )
          if( !flows[p->_succs[k]->_pre_order] &&
              p->_succs[k] != b )
            break;

        // Either carry-forward the now-unused OopFlow for b's use
        // or draw a new one from the free list
        if( k==p->_num_succs ) {
          flow = p_flow;
          break;                // Found an ideal pred, use him
        }
      }
    }

    if( flow ) {
      // We have an OopFlow that's the last-use of a predecessor.
      // Carry it forward.
    } else {                    // Draw a new OopFlow from the freelist
      if( !free_list )
        free_list = OopFlow::make(A,max_reg);
      flow = free_list;
      assert( flow->_b == NULL, "oopFlow is not free" );
      free_list = flow->_next;
      flow->_next = NULL;

      // Copy/clone over the data
      flow->clone(flows[pred->_pre_order], max_reg);
    }

    // Mark flow for block.  Blocks can only be flowed over once,
    // because after the first time they are guarded from entering
    // this code again.
    assert( flow->_b == pred, "have some prior flow" );
    flow->_b = NULL;

    // Now push flow forward
    flows[b->_pre_order] = flow;// Mark flow for this block
    flow->_b = b;               
    flow->compute_reach( _regalloc, max_reg, safehash );

    // Now push children onto worklist
    for( i=0; i<b->_num_succs; i++ )
      worklist.push(b->_succs[i]);

  }
}