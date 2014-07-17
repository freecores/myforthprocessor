#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)domgraph.cpp	1.61 03/02/06 16:57:23 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Portions of code courtesy of Clifford Click

// Optimization - Graph Style

#include "incls/_precompiled.incl"
#include "incls/_domgraph.cpp.incl"

//------------------------------Tarjan-----------------------------------------
// A data structure that holds all the information needed to find dominators.
struct Tarjan {
  Block *_block;		// Basic block for this info

  uint _semi;			// Semi-dominators
  uint _size;			// Used for faster LINK and EVAL 
  Tarjan *_parent;		// Parent in DFS
  Tarjan *_label;		// Used for LINK and EVAL
  Tarjan *_ancestor;		// Used for LINK and EVAL 
  Tarjan *_child;		// Used for faster LINK and EVAL 
  Tarjan *_dom;			// Parent in dominator tree (immediate dom)
  Tarjan *_bucket;		// Set of vertices with given semidominator 

  Tarjan *_dom_child;		// Child in dominator tree
  Tarjan *_dom_next;		// Next in dominator tree

  // Fast union-find work
  void COMPRESS();
  Tarjan *EVAL(void);
  void LINK( Tarjan *w, Tarjan *tarjan0 );

  void setdepth( uint depth );

};
         
//------------------------------Dominator--------------------------------------
// Compute the dominator tree of the CFG.  The CFG must already have been 
// constructed.  This is the Lengauer & Tarjan O(E-alpha(E,V)) algorithm.
void PhaseCFG::Dominators( ) {
  // Pre-grow the blocks array, prior to the ResourceMark kicking in
  _blocks.map(_num_blocks,0);

  ResourceMark rm;
  // Setup mappings from my Graph to Tarjan's stuff and back 
  // Note: Tarjan uses 1-based arrays
  Tarjan *tarjan = NEW_RESOURCE_ARRAY(Tarjan,_num_blocks+1);

  // Tarjan's algorithm, almost verbatim: 
  // Step 1: 
  _rpo_ctr = _num_blocks;
  uint dfsnum = DFS( _broot, tarjan, 1 );
  if( dfsnum-1 != _num_blocks ) {// Check for unreachable loops!
    // If the returned dfsnum does not match the number of blocks, then we
    // must have some unreachable loops.  These can be made at any time by
    // IterGVN.  They are cleaned up by CCP or the loop opts, but the last
    // IterGVN can always make more that are not cleaned up.  Highly unlikely
    // except in ZKM.jar, where endless irreducible loops cause the loop opts
    // to not get run.
    //
    // Having found unreachable loops, we have made a bad RPO _block layout.
    // We can re-run the above DFS pass with the correct number of blocks,
    // and hack the Tarjan algorithm below to be robust in the presence of
    // such dead loops (as was done for the NTarjan code farther below).
    // Since this situation is so unlikely, instead I've decided to bail out.
    // CNC 7/24/2001
    _root = NULL;
    return;
  }
  _blocks._cnt = _num_blocks;

  // Tarjan is using 1-based arrays, so these are some initialize flags
  tarjan[0]._size = tarjan[0]._semi = 0;
  tarjan[0]._label = &tarjan[0];
  
  uint i;
  for( i=_num_blocks; i>=2; i-- ) { // For all vertices in DFS order 
    Tarjan *w = &tarjan[i];	// Get vertex from DFS 

    // Step 2: 
    Node *whead = w->_block->head();
    for( uint j=1; j < whead->req(); j++ ) {
      Block *b = _bbs[whead->in(j)->_idx];
      Tarjan *vx = &tarjan[b->_pre_order];
      Tarjan *u = vx->EVAL();
      if( u->_semi < w->_semi )
	w->_semi = u->_semi;
    } 

    // w is added to a bucket here, and only here.
    // Thus w is in at most one bucket and the sum of all bucket sizes is O(n).
    // Thus bucket can be a linked list.
    // Thus we do not need a small integer name for each Block.
    w->_bucket = tarjan[w->_semi]._bucket;
    tarjan[w->_semi]._bucket = w;

    w->_parent->LINK( w, &tarjan[0] );

    // Step 3: 
    for( Tarjan *vx = w->_parent->_bucket; vx; vx = vx->_bucket ) {
      Tarjan *u = vx->EVAL();
      vx->_dom = (u->_semi < vx->_semi) ? u : w->_parent;
    }
  }

  // Step 4: 
  for( i=2; i <= _num_blocks; i++ ) {
    Tarjan *w = &tarjan[i];
    if( w->_dom != &tarjan[w->_semi] )
      w->_dom = w->_dom->_dom;
    w->_dom_next = w->_dom_child = NULL;  // Initialize for building tree later
  }
  // No immediate dominator for the root
  Tarjan *w = &tarjan[_broot->_pre_order];
  w->_dom = NULL;	
  w->_dom_next = w->_dom_child = NULL;  // Initialize for building tree later

  // Convert the dominator tree array into my kind of graph 
  for( i=1; i<=_num_blocks;i++){// For all Tarjan vertices
    Tarjan *t = &tarjan[i];     // Handy access
    Tarjan *tdom = t->_dom;     // Handy access to immediate dominator 
    if( tdom )	{               // Root has no immediate dominator
      t->_block->_idom = tdom->_block; // Set immediate dominator
      t->_dom_next = tdom->_dom_child; // Make me a sibling of parent's child
      tdom->_dom_child = t;     // Make me a child of my parent
    } else
      t->_block->_idom = NULL;	// Root
  }
  w->setdepth( 1 );		// Set depth in dominator tree

}

//------------------------------DFS--------------------------------------------
// Perform DFS search.	Setup 'vertex' as DFS to vertex mapping.  Setup      
// 'semi' as vertex to DFS mapping.  Set 'parent' to DFS parent.             
uint PhaseCFG::DFS( Block *b, Tarjan *tarjan, uint pre_order ) {
  Tarjan *t = &tarjan[pre_order]; // Fast local access
  b->_pre_order = pre_order++;	// Flag as visited

  t->_block = b;                // Save actual block

  t->_semi = b->_pre_order;     // Block to DFS map 
  t->_label = t;		// DFS to vertex map 
  t->_ancestor = NULL;		// Fast LINK & EVAL setup 
  t->_child = &tarjan[0];       // Sentenial
  t->_size = 1;
  t->_bucket = NULL;

  // Now walk over all successors in pre-order.  Special handling to do the
  // most frequent target first.  Next stanza of code finds the index into 
  // the b->succs[] array of the most frequent successor.
  uint freq_idx = 0;
  int eidx = b->end_idx();
  Node *n = b->_nodes[eidx];
  MachNode *mach = n->is_Mach();
  int op = mach ? mach->ideal_Opcode() : n->Opcode();
  switch( op ) {
  case Op_CountedLoopEnd:
  case Op_If: {               // Split frequency amongst children
    float prob = mach->is_MachIf()->_prob;
    // Is succ[0] the TRUE branch or the FALSE branch?
    if( b->_nodes[eidx+1]->Opcode() == Op_IfFalse )
      prob = 1.0f - prob;
    freq_idx = prob < 0.5;      // freq=1 for succ[0] < 0.5 prob
    break;
  }
  case Op_Catch:                // Split frequency amongst children
    for( freq_idx = 0; freq_idx < b->_num_succs; freq_idx++ ) 
      if( b->_nodes[eidx+1+freq_idx]->is_CatchProj()->_con == CatchProjNode::fall_through_index )
        break;
    // Handle case of no fall-thru (e.g., check-cast MUST throw an exception)
    if( freq_idx == b->_num_succs ) freq_idx = 0;
    break;
  case Op_Root:
  case Op_Goto:               
  case Op_NeverBranch:
    freq_idx = 0;               // fall thru
    break;
  case Op_TailCall:
  case Op_TailJump:
  case Op_Return:
  case Op_Halt:
  case Op_Rethrow:
    break;                    
  default: 
    ShouldNotReachHere();
  }
  
  // Walk all the remaining targets
  for( uint i = 0; i < b->_num_succs; i++ ) {
    if( i == freq_idx ) continue;
    Block *s = b->_succs[i];
    if( !s->_pre_order ) {      // Check for no-pre-order, not-visited
      tarjan[pre_order]._parent = t; // Save parent in DFS
      pre_order = DFS(s,tarjan,pre_order); // DFS on all children 
    }
  }
  // Now walk the most frequent target last
  Block *s = b->_succs[freq_idx];
  if( !s->_pre_order ) {        // Check for no-pre-order, not-visited
    tarjan[pre_order]._parent = t; // Save parent in DFS
    pre_order = DFS(s,tarjan,pre_order); // DFS on all children 
  }

  // Here I build a reverse post-order in the CFG _blocks array
  _blocks.map(--_rpo_ctr,b);

  return pre_order;
}

//------------------------------COMPRESS---------------------------------------
void Tarjan::COMPRESS()
{
  assert( _ancestor != 0, "" );
  if( _ancestor->_ancestor != 0 ) {
    _ancestor->COMPRESS( );
    if( _ancestor->_label->_semi < _label->_semi )
      _label = _ancestor->_label;
    _ancestor = _ancestor->_ancestor;
  }
}

//------------------------------EVAL-------------------------------------------
Tarjan *Tarjan::EVAL() {
  if( !_ancestor ) return _label;
  COMPRESS();
  return (_ancestor->_label->_semi >= _label->_semi) ? _label : _ancestor->_label;
}

//------------------------------LINK-------------------------------------------
void Tarjan::LINK( Tarjan *w, Tarjan *tarjan0 ) {
  Tarjan *s = w;
  while( w->_label->_semi < s->_child->_label->_semi ) {
    if( s->_size + s->_child->_child->_size >= (s->_child->_size << 1) ) {
      s->_child->_ancestor = s;
      s->_child = s->_child->_child;
    } else {
      s->_child->_size = s->_size;
      s = s->_ancestor = s->_child;
    }
  }
  s->_label = w->_label;
  _size += w->_size;
  if( _size < (w->_size << 1) ) {
    Tarjan *tmp = s; s = _child; _child = tmp;
  }
  while( s != tarjan0 ) {
    s->_ancestor = this;
    s = s->_child;
  }
}

//------------------------------setdepth---------------------------------------
void Tarjan::setdepth( uint depth ) {
  _block->_dom_depth = depth;	// Set depth in dominator tree
  if( _dom_child ) _dom_child->setdepth( depth+1 );
  if( _dom_next )  _dom_next ->setdepth( depth   );
}

//*********************** DOMINATORS ON THE SEA OF NODES***********************
//------------------------------NTarjan----------------------------------------
// A data structure that holds all the information needed to find dominators.
struct NTarjan {
  Node *_control;		// Control node associated with this info

  uint _semi;			// Semi-dominators
  uint _size;			// Used for faster LINK and EVAL 
  NTarjan *_parent;		// Parent in DFS
  NTarjan *_label;		// Used for LINK and EVAL
  NTarjan *_ancestor;		// Used for LINK and EVAL 
  NTarjan *_child;		// Used for faster LINK and EVAL 
  NTarjan *_dom;		// Parent in dominator tree (immediate dom)
  NTarjan *_bucket;		// Set of vertices with given semidominator 

  NTarjan *_dom_child;		// Child in dominator tree
  NTarjan *_dom_next;		// Next in dominator tree

  // Perform DFS search.  
  // Setup 'vertex' as DFS to vertex mapping.  
  // Setup 'semi' as vertex to DFS mapping.  
  // Set 'parent' to DFS parent.             
  static int DFS( NTarjan *ntarjan, VectorSet &visited, PhaseIdealLoop *pil, uint *dfsorder );
  void setdepth( int depth, uint idx, uint *dom_depth );

  // Fast union-find work
  void COMPRESS();
  NTarjan *EVAL(void);
  void LINK( NTarjan *w, NTarjan *ntarjan0 );
#ifndef PRODUCT
  void dump(int offset) const;
#endif
};

//------------------------------Dominator--------------------------------------
// Compute the dominator tree of the sea of nodes.  This version walks all CFG
// nodes (using the is_CFG() call) and places them in a dominator tree.  Thus,
// it needs a count of the CFG nodes for the mapping table. This is the
// Lengauer & Tarjan O(E-alpha(E,V)) algorithm.
void PhaseIdealLoop::Dominators( ) {
  ResourceMark rm;
  // Setup mappings from my Graph to Tarjan's stuff and back 
  // Note: Tarjan uses 1-based arrays
  NTarjan *ntarjan = NEW_RESOURCE_ARRAY(NTarjan,C->unique()+1);
  // Initialize _control field for fast reference
  int i;
  for( i= C->unique()-1; i>=0; i-- ) 
    ntarjan[i]._control = NULL;
 
  // Store the DFS order for the main loop
  uint *dfsorder = NEW_RESOURCE_ARRAY(uint,C->unique()+1);
  memset(dfsorder, max_uint, (C->unique()+1) * sizeof(uint));

  // Tarjan's algorithm, almost verbatim: 
  // Step 1:
  VectorSet visited(Thread::current()->resource_area());
  int dfsnum = NTarjan::DFS( ntarjan, visited, this, dfsorder);

  // Tarjan is using 1-based arrays, so these are some initialize flags
  ntarjan[0]._size = ntarjan[0]._semi = 0;
  ntarjan[0]._label = &ntarjan[0];
  
  for( i = dfsnum-1; i>1; i-- ) {        // For all nodes in reverse DFS order
    NTarjan *w = &ntarjan[i];            // Get Node from DFS 
    assert(w->_control != NULL,"bad DFS walk");

    // Step 2: 
    Node *whead = w->_control;
    for( uint j=0; j < whead->req(); j++ ) { // For each predecessor
      if( whead->in(j) == NULL || !whead->in(j)->is_CFG() )
	continue;                            // Only process control nodes
      uint b = dfsorder[whead->in(j)->_idx];
      if(b == max_uint) continue;
      NTarjan *vx = &ntarjan[b];
      NTarjan *u = vx->EVAL();
      if( u->_semi < w->_semi )
	w->_semi = u->_semi;
    } 

    // w is added to a bucket here, and only here.
    // Thus w is in at most one bucket and the sum of all bucket sizes is O(n).
    // Thus bucket can be a linked list.
    w->_bucket = ntarjan[w->_semi]._bucket;
    ntarjan[w->_semi]._bucket = w;

    w->_parent->LINK( w, &ntarjan[0] );

    // Step 3: 
    for( NTarjan *vx = w->_parent->_bucket; vx; vx = vx->_bucket ) {
      NTarjan *u = vx->EVAL();
      vx->_dom = (u->_semi < vx->_semi) ? u : w->_parent;
    }

    // Cleanup any unreachable loops now.  Unreachable loops are loops that
    // flow into the main graph (and hence into ROOT) but are not reachable
    // from above.  Such code is dead, but requires a global pass to detect
    // it; this global pass was the 'build_loop_tree' pass run just prior.
    if( whead->is_Region() ) {
      for( uint i = 1; i < whead->req(); i++ ) {
        if (!has_node(whead->in(i))) {
          // Kill dead input path
          assert( !visited.test(whead->in(i)->_idx), 
                  "input with no loop must be dead" );
          _igvn.hash_delete(whead);
	  whead->del_req(i);
          _igvn._worklist.push(whead);
	  for (DUIterator_Fast jmax, j = whead->fast_outs(jmax); j < jmax; j++) {
	    Node* p = whead->fast_out(j);
	    if( p->is_Phi() ) {
              _igvn.hash_delete(p);
	      p->del_req(i);
              _igvn._worklist.push(p);
	    }
	  }
          i--;                  // Rerun same iteration
        } // End of if dead input path
      } // End of for all input paths
    } // End if if whead is a Region
  } // End of for all Nodes in reverse DFS order

  // Step 4: 
  for( i=2; i < dfsnum; i++ ) {	// DFS order
    NTarjan *w = &ntarjan[i];
    assert(w->_control != NULL,"Bad DFS walk");
    if( w->_dom != &ntarjan[w->_semi] )
      w->_dom = w->_dom->_dom;
    w->_dom_next = w->_dom_child = NULL;  // Initialize for building tree later
  }
  // No immediate dominator for the root
  NTarjan *w = &ntarjan[dfsorder[C->root()->_idx]];
  w->_dom = NULL;
  w->_parent = NULL;
  w->_dom_next = w->_dom_child = NULL;  // Initialize for building tree later

  // Convert the dominator tree array into my kind of graph 
  for( i=1; i<dfsnum; i++ ) {	       // For all Tarjan vertices
    NTarjan *t = &ntarjan[i];          // Handy access
    assert(t->_control != NULL,"Bad DFS walk");
    NTarjan *tdom = t->_dom;           // Handy access to immediate dominator 
    if( tdom )	{                      // Root has no immediate dominator
      _idom[t->_control->_idx] = tdom->_control; // Set immediate dominator
      t->_dom_next = tdom->_dom_child; // Make me a sibling of parent's child
      tdom->_dom_child = t;            // Make me a child of my parent
    } else
      _idom[C->root()->_idx] = NULL; // Root
  }
  w->setdepth( 1, w->_control->_idx, _dom_depth );// Set depth in dominator tree
  // Pick up the 'top' node as well
  _idom     [C->top()->_idx] = C->root();
  _dom_depth[C->top()->_idx] = 1;

  // Debug Print of Dominator tree
  if( PrintDominators ) {
#ifndef PRODUCT
    w->dump(0);
#endif
  }
}

//------------------------------DFS--------------------------------------------
// Perform DFS search.	Setup 'vertex' as DFS to vertex mapping.  Setup      
// 'semi' as vertex to DFS mapping.  Set 'parent' to DFS parent.             
int NTarjan::DFS( NTarjan *ntarjan, VectorSet &visited, PhaseIdealLoop *pil, uint *dfsorder) {

  Node_List dfstack;
  // Allocate stack of size C->unique()/8 to avoid frequent realloc
  dfstack.map( (pil->C->unique() >> 3), NULL);
  Node *b = pil->C->root();
  int dfsnum = 1;
  dfsorder[b->_idx] = dfsnum; // Cache parent's dfsnum for a later use
  dfstack.push(b);

  while ( dfstack.size() != 0 ) {
    b = dfstack.pop();
    if( !visited.test_set(b->_idx) ) { // Test node and flag it as visited
      NTarjan *w = &ntarjan[dfsnum];
      // Only fully process control nodes
      w->_control = b;                 // Save actual node
      // Use parent's cached dfsnum to identify "Parent in DFS"
      w->_parent = &ntarjan[dfsorder[b->_idx]];
      dfsorder[b->_idx] = dfsnum;      // Save DFS order info
      w->_semi = dfsnum;               // Node to DFS map 
      w->_label = w;                   // DFS to vertex map 
      w->_ancestor = NULL;             // Fast LINK & EVAL setup 
      w->_child = &ntarjan[0];         // Sentinal
      w->_size = 1;
      w->_bucket = NULL;

      // Need DEF-USE info for this pass
      for ( int i = b->outcnt(); i-- > 0; ) { // Put on stack backwards
        Node* s = b->raw_out(i);       // Get a use
        // CFG nodes only and not dead stuff 
        if( s->is_CFG() && pil->has_node(s) && !visited.test(s->_idx) ) {
          dfsorder[s->_idx] = dfsnum;  // Cache parent's dfsnum for a later use
          dfstack.push(s);
        }
      }
      dfsnum++;  // update after parent's dfsnum has been cached.
    }
  }

  return dfsnum;
}

//------------------------------COMPRESS---------------------------------------
void NTarjan::COMPRESS()
{
  assert( _ancestor != 0, "" );
  if( _ancestor->_ancestor != 0 ) {
    _ancestor->COMPRESS( );
    if( _ancestor->_label->_semi < _label->_semi )
      _label = _ancestor->_label;
    _ancestor = _ancestor->_ancestor;
  }
}

//------------------------------EVAL-------------------------------------------
NTarjan *NTarjan::EVAL() {
  if( !_ancestor ) return _label;
  COMPRESS();
  return (_ancestor->_label->_semi >= _label->_semi) ? _label : _ancestor->_label;
}

//------------------------------LINK-------------------------------------------
void NTarjan::LINK( NTarjan *w, NTarjan *ntarjan0 ) {
  NTarjan *s = w;
  while( w->_label->_semi < s->_child->_label->_semi ) {
    if( s->_size + s->_child->_child->_size >= (s->_child->_size << 1) ) {
      s->_child->_ancestor = s;
      s->_child = s->_child->_child;
    } else {
      s->_child->_size = s->_size;
      s = s->_ancestor = s->_child;
    }
  }
  s->_label = w->_label;
  _size += w->_size;
  if( _size < (w->_size << 1) ) {
    NTarjan *tmp = s; s = _child; _child = tmp;
  }
  while( s != ntarjan0 ) {
    s->_ancestor = this;
    s = s->_child;
  }
}

//------------------------------setdepth---------------------------------------
void NTarjan::setdepth( int depth, uint idx , uint *dom_depth ) {
  dom_depth[idx] = depth;	// Set depth in dominator tree
  if( _dom_child ) _dom_child->setdepth(depth+1, _dom_child->_control->_idx, dom_depth);
  if( _dom_next  ) _dom_next ->setdepth(depth,   _dom_next ->_control->_idx, dom_depth);
}

//------------------------------dump-------------------------------------------
#ifndef PRODUCT
void NTarjan::dump(int offset) const {
  // Dump the data from this node
  int i;
  for(i = offset; i >0; i--)  // Use indenting for tree structure
    tty->print("  ");
  tty->print("Dominator Node: ");
  _control->dump();		  // Control node for this dom node
  tty->print("\n");
  for(i = offset; i >0; i--)      // Use indenting for tree structure
    tty->print("  ");
  tty->print("semi:%d, size:%d\n",_semi, _size);
  for(i = offset; i >0; i--)      // Use indenting for tree structure
    tty->print("  ");
  tty->print("DFS Parent: ");
  if(_parent != NULL)
    _parent->_control->dump();    // Parent in DFS
  tty->print("\n");
  for(i = offset; i >0; i--)      // Use indenting for tree structure
    tty->print("  ");
  tty->print("Dom Parent: ");
  if(_dom != NULL)
    _dom->_control->dump();       // Parent in Dominator Tree
  tty->print("\n");

  // Recurse over remaining tree
  if( _dom_child ) _dom_child->dump(offset+2);   // Children in dominator tree
  if( _dom_next  ) _dom_next ->dump(offset  );   // Siblings in dominator tree

}
#endif

