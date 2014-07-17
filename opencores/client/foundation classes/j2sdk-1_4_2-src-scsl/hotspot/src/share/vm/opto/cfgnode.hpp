#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cfgnode.hpp	1.95 03/01/23 12:15:33 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Portions of code courtesy of Clifford Click

// Optimization - Graph Style

class Matcher;
class Node;
class   RegionNode;
class   TypeNode;
class     PhiNode;
class   GotoNode;
class   MultiNode;
class     IfNode;
class       FastLockNode;
class       FastUnockNode;
class     PCTableNode;
class       JumpNode;
class       CatchNode;
class   ProjNode;
class     CProjNode;
class       IfTrueNode;
class       IfFalseNode;
class       CatchProjNode;
class     SCMemProjNode;
class PhaseIdealLoop;

//------------------------------RegionNode-------------------------------------
// The class of RegionNodes, which can be mapped to basic blocks in the
// program.  Their inputs point to Control sources.  PhiNodes (described
// below) have an input point to a RegionNode.  Merged data inputs to PhiNodes
// correspond 1-to-1 with RegionNode inputs.  The zero input of a PhiNode is
// the RegionNode, and the zero input of the RegionNode is itself.
class RegionNode : public Node {
public:
  // Node layout (parallels PhiNode):
  enum { Region,		// Generally points to self.
         Control		// Control arcs are [1..len)
  };

  RegionNode( uint required ) : Node(required) { set_req(0,this); }

  Node* is_copy() const {
    const Node* r = _in[Region];
    if (r == NULL)
      return nonnull_req();
    return NULL;  // not a copy!
  }
  PhiNode* has_phi() const;        // returns an arbitrary phi user, or NULL
  PhiNode* has_unique_phi() const; // returns the unique phi user, or NULL
  virtual int Opcode() const;
  virtual int pinned() const { return (const Node *)in(0) == this; }
  virtual const RegionNode *is_Region() const { return this; }
  virtual int is_block_start() const { return (const Node*)in(0) == this; }
  virtual bool  is_CFG   () const { return true; }
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual bool depends_only_on_test() const { return false; }
  virtual const Type *bottom_type() const { return Type::CONTROL; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const RegMask &out_RegMask() const;
  virtual uint ideal_reg() const { return 0; }
};

//------------------------------PhiNode----------------------------------------
// PhiNodes merge values from different Control paths.  Slot 0 points to the
// controlling RegionNode.  Other slots map 1-for-1 with incoming control flow
// paths to the RegionNode.  For speed reasons (to avoid another pass) we 
// can turn PhiNodes into copys in-place by NULL'ing out their RegionNode 
// input in slot 0.
class PhiNode : public TypeNode {
  const TypePtr* const _adr_type; // non-null only for Type::MEMORY nodes.
  // Size is bigger to hold the _adr_type field.
  virtual uint hash() const;    // Check the type
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const { return sizeof(*this); }
public:
  // Node layout (parallels RegionNode):
  enum { Region,		// Control input is the Phi's region.
         Input			// Input values are [1..len)
  };

  PhiNode( Node *r, const Type *t, const TypePtr* at = NULL )
    : TypeNode(t,r->req()), _adr_type(at) { set_req(0, r); verify_adr_type(); }
  // create a new phi with in edges matching r and set (initially) to x
  static PhiNode* make( Node* r, Node* x );
  // extra type arguments override the new phi's bottom_type and adr_type
  static PhiNode* make( Node* r, Node* x, const Type *t, const TypePtr* at = NULL );
  // create a new phi with narrowed memory type
  PhiNode* slice_memory(const TypePtr* adr_type) const;
  // like make(r, x), but does not initialize the in edges to x
  static PhiNode* make_blank( Node* r, Node* x );

  // Accessors
  RegionNode* region() const { Node* r = in(Region); assert(!r || r->is_Region(), ""); return (RegionNode*)r; }

  // Force this Phi to become a plain copy.
  void degrade_to_copy(Node* n);
  Node* is_copy() const {
    // The node is a real phi if _in[0] is a Region node.
    // Otherwise _in[0] must be either NULL or top.
    // If _in[0] is top (a rare transient condition during IGVN)
    // then this node is deemed to be copy of top.
    const Node* r = _in[Region];
    if (r == NULL)
      return _in[Input];
    // (This "req<=1" check is simply a performance hack.)
    if (r->is_top())
      return (Node*)r;
    assert(r->is_Region(), "valid region?");
    return NULL;  // not a copy!
  }

  virtual int Opcode() const;
  virtual PhiNode *is_Phi () { return this; }
  virtual int pinned() const { return in(0) != 0; }
  virtual const TypePtr *adr_type() const { verify_adr_type(true); return _adr_type; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const RegMask &out_RegMask() const;
  virtual const RegMask &in_RegMask(uint) const;
  #ifdef ASSERT
  void verify_adr_type(VectorSet& visited, const TypePtr* at) const;
  void verify_adr_type(bool recursive = false) const;
  #else
  void verify_adr_type(bool recursive = false) const {}
  #endif
};

//------------------------------GotoNode---------------------------------------
// GotoNodes perform direct branches.
class GotoNode : public Node {
public:
  GotoNode( Node *control ) : Node(control) {}
  virtual int Opcode() const;
  virtual int pinned() const { return 1; }
  virtual bool  is_CFG() const { return true; }
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual const Node *is_block_proj() const { return this; }
  virtual bool depends_only_on_test() const { return false; }
  virtual const Type *bottom_type() const { return Type::CONTROL; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual const RegMask &out_RegMask() const;
  virtual uint  is_Goto() const { return 1; }
  virtual uint ideal_reg() const { return 0; }
};

//------------------------------CProjNode--------------------------------------
// control projection for node that produces multiple control-flow paths
class CProjNode : public ProjNode {
public:
  CProjNode( Node *ctrl, uint idx ) : ProjNode(ctrl,idx) {}
  virtual int Opcode() const;
  virtual bool  is_CFG() const { return true; }
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual bool depends_only_on_test() const { return false; }
  virtual const Node *is_block_proj() const { return in(0); }
  virtual const RegMask &out_RegMask() const;
  virtual uint ideal_reg() const { return 0; }
};

//------------------------------IfNode-----------------------------------------
// Output selected Control, based on a boolean test
class IfNode : public MultiNode {
  // Size is bigger to hold the probability field.  However, _prob does not
  // change the semantics so it does not appear in the hash & cmp functions.
  virtual uint size_of() const { return sizeof(*this); }
public:
  float _prob;                  // Probability of true path being taken.
  float _fcnt;                  // Frequency counter
  IfNode( Node *control, Node *bool, float p, float fcnt ) : MultiNode(2), _prob(p), _fcnt(fcnt) { set_req(0,control); set_req(1,bool); }
  virtual int Opcode() const;
  virtual int pinned() const { return 1; }
  virtual const Type *bottom_type() const { return TypeTuple::IFBOTH; }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const RegMask &out_RegMask() const;
  virtual uint ideal_reg() const { return NotAMachineReg; }
  virtual IfNode *is_If() { return this; }
  void dominated_by( Node *prev_dom, PhaseIterGVN *igvn );

#ifndef PRODUCT
  virtual void dump_spec() const;
#endif
};

class IfTrueNode : public CProjNode {
public:
  IfTrueNode( IfNode *ifnode ) : CProjNode(ifnode,1) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );  
};

class IfFalseNode : public CProjNode {
public:
  IfFalseNode( IfNode *ifnode ) : CProjNode(ifnode,0) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );  
};


//------------------------------PCTableNode------------------------------------
// Build an indirect branch table.  Given a control and a table index,
// control is passed to the Projection matching the table index.  Used to
// implement switch statements and exception-handling capabilities.
// Undefined behavior if passed-in index is not inside the table.
class PCTableNode : public MultiNode {
  virtual uint hash() const;    // Target count; table size
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const { return sizeof(*this); }

public:
  const uint _size;             // Number of targets

  PCTableNode( Node *ctrl, Node *idx, uint size ) : MultiNode(2), _size(size) {
    set_req(0, ctrl);
    set_req(1, idx);
  }
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *bottom_type() const;
  virtual int pinned() const { return 1; }
  virtual uint ideal_reg() const { return NotAMachineReg; }
  virtual const PCTableNode *is_PCTable() const { return this; }
};

//------------------------------JumpNode---------------------------------------
// Indirect branch.  Uses PCTable above to implement a switch statement.
// It emits as a table load and local branch.  
class JumpNode : public PCTableNode {
public:
  JumpNode( Node *ctrl, Node *idx, uint size) : PCTableNode(ctrl,idx,size){}
  virtual int Opcode() const;
};

//------------------------------CatchNode--------------------------------------
// Helper node to fork exceptions.  "Catch" catches any exceptions thrown by
// a just-prior call.  Looks like a PCTableNode but emits no code - just the
// table.  The table lookup and branch is implemented by RethrowNode.
class CatchNode : public PCTableNode {
public:
  CatchNode( Node *ctrl, Node *idx, uint size ) : PCTableNode(ctrl,idx,size){};
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const CatchNode *is_Catch() const { return this; }
};

// CatchProjNode controls which exception handler is targetted after a call.
// It is passed in the bci of the target handler, or no_handler_bci in case
// the projection doesn't lead to an exception handler.
class CatchProjNode : public CProjNode {
  virtual uint hash() const;
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const { return sizeof(*this); }

private:
  const int _handler_bci;

public:
  enum {
    fall_through_index =  0,      // the fall through projection index
    catch_all_index    =  1,      // the projection index for catch-alls
    no_handler_bci     = -1       // the bci for fall through or catch-all projs
  };

  CatchProjNode(Node* catchnode, uint proj_no, int handler_bci);
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual const Type *bottom_type() const { return Type::CONTROL; }
  virtual const CatchProjNode *is_CatchProj() const { return this; }
  int  handler_bci() const        { return _handler_bci; }
  bool is_handler_proj() const    { return _handler_bci >= 0; }
#ifndef PRODUCT
  virtual void dump_spec() const;
#endif
};


//---------------------------------CreateExNode--------------------------------
// Helper node to create the exception coming back from a call
class CreateExNode : public TypeNode {
public:
  CreateExNode(const Type* t, Node* control, Node* i_o) : TypeNode(t, 2) {
    set_req(0, control);
    set_req(1, i_o);
  }
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual int pinned() const { return 1; }
  uint match_edge(uint idx) const { return 0; }
  virtual uint ideal_reg() const { return Op_RegP; }
  virtual const CreateExNode *is_CreateEx() const { return this; }
};

//------------------------------NeverBranchNode-------------------------------
// The never-taken branch.  Used to give the appearance of exiting infinite
// loops to those algorithms that like all paths to be reachable.  Encodes
// empty.
class NeverBranchNode : public MultiNode {
public:
  NeverBranchNode( Node *ctrl ) : MultiNode(1) { set_req(0,ctrl); }
  virtual int Opcode() const;
  virtual int pinned() const { return 1; };
  virtual const Type *bottom_type() const { return TypeTuple::IFBOTH; }

  virtual void emit(CodeBuffer &cbuf, PhaseRegAlloc *ra_) const { }
  virtual uint size(PhaseRegAlloc *ra_) const { return 0; }
#ifndef PRODUCT
  virtual void format( PhaseRegAlloc * ) const;
#endif
};

