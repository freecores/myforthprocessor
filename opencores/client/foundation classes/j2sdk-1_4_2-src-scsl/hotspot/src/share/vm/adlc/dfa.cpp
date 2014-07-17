#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)dfa.cpp	1.72 03/01/23 11:12:49 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// DFA.CPP - Method definitions for outputting the matcher DFA from ADLC
#include "adlc.hpp"

//---------------------------Access to internals of class State----------------
static const char *sLeft   = "_kids[0]";
static const char *sRight  = "_kids[1]";

//---------------------------Helper Functions----------------------------------
static void cost_check(FILE *fp, const char *spaces, 
		       const char *arrayIdx, const char *cost, const char *rule) {
  // Check against other match costs, and update cost & rule vectors
  fprintf(fp, "%sif (STATE__NOT_YET_VALID(%s) || _cost[%s] > %s) {\n",  spaces, arrayIdx, arrayIdx, cost);
  fprintf(fp, "%s  _cost[%s] = %s;\n",      spaces, arrayIdx, cost);
  fprintf(fp, "%s  _rule[%s] = %s_rule;\n", spaces, arrayIdx, rule);
  fprintf(fp, "%s  STATE__SET_VALID(%s);\n",       spaces, arrayIdx);
  fprintf(fp, "%s}\n",                      spaces);
}


//---------------------------child_test----------------------------------------
// Example:
//   STATE__VALID_CHILD(_kids[0], FOO) &&  STATE__VALID_CHILD(_kids[1], BAR)
// Macro equivalent to: _kids[0]->valid(FOO) && _kids[1]->valid(BAR)
//
void ArchDesc::child_test(FILE *fp, MatchList &mList) {
  if( mList._lchild )		// If left child, check it
    fprintf(fp, "STATE__VALID_CHILD(_kids[0], %s)", machOperEnum(mList._lchild));
  if( mList._lchild && mList._rchild )      // If both, add the "&&"
    fprintf(fp, " && " );
  if( mList._rchild )		// If right child, check it
    fprintf(fp, "STATE__VALID_CHILD(_kids[1], %s)", machOperEnum(mList._rchild));
}

//---------------------------calc_cost-----------------------------------------
// Example:
//           unsigned int c = _kids[0]->_cost[FOO] + _kids[1]->_cost[BAR] + 5;
//
void ArchDesc::calc_cost(FILE *fp, const char *spaces, MatchList &mList) {
  fprintf(fp, "%sunsigned int c = ", spaces);
  if (mList._lchild )                     // If left child, add it in
    fprintf(fp, "_kids[0]->_cost[%s] + ", machOperEnum(mList._lchild));
  if (mList._rchild)                      // If right child, add it in
    fprintf(fp, "_kids[1]->_cost[%s] + ", machOperEnum(mList._rchild));
  fprintf(fp, "%s;\n", mList.get_cost()); // Add in cost of this rule
}


//---------------------------gen_match-----------------------------------------
void ArchDesc::gen_match(FILE *fp, MatchList &mList, Dict &operands_chained_from) {
  // Open the child-and-predicate-test braces
  fprintf(fp, "    if( ");
  int first_test = 0;

  // Only generate child tests if this is not a leaf node
  if( mList._lchild || mList._rchild ) {
    first_test++;
    child_test(fp, mList);
  }

  // Only generate predicate test if one exists for this match
  if( mList.get_pred() ) {
    if( first_test++ ) fprintf(fp," &&\n");
    fprintf(fp, "        %s", mList.get_pred());
  }

  if( !first_test ) fprintf(fp,"1");

  // End of outer tests
  fprintf(fp," ) {\n");

  // Calculate cost of this match
  calc_cost(fp, "      ", mList);
  // Check against other match costs, and update cost & rule vectors
  cost_check(fp, "      ", machOperEnum(mList._resultStr), "c", mList._opcode);

  // If this is a member of an operand class, update the class cost & rule
  expand_opclass( fp, "      ", "c", mList._resultStr);

  // Check if this rule should be used to generate the chains as well.
  const char *rule = /* set rule to "Invalid" for internal operands */
    strcmp(mList._opcode,mList._resultStr) ? mList._opcode : "Invalid";

  // If this rule produces an operand which has associated chain rules,
  // update the operands with the chain rule + this rule cost & this rule.
  chain_rule(fp, "      ", mList._resultStr, "c", rule, operands_chained_from);

  // Close the child-and-predicate-test braces
  fprintf(fp, "    }\n");

}


//---------------------------expand_opclass------------------------------------
// Chain from one result_type to all other members of its operand class
void ArchDesc::expand_opclass(FILE *fp, const char *indent, const char *cost, 
			      const char *result_type) {
  const Form *form = _globalNames[result_type];
  OperandForm *op = form ? form->is_operand() : NULL;
  if( op && op->_classes.count() > 0 ) {
    // Iterate through all operand classes which include this operand
    op->_classes.reset();
    const char *oclass;
    while( (oclass = op->_classes.iter()) != NULL ) 
      // Check against other match costs, and update cost & rule vectors
      cost_check(fp, indent, machOperEnum(oclass), cost, result_type);
  }
}

#define  COST_BUFFER_SIZE  256
//---------------------------chain_rule----------------------------------------
// Starting at 'operand', check if we know how to automatically generate other results
void ArchDesc::chain_rule(FILE *fp, const char *indent, const char *operand, 
     const char *icost, const char *irule, Dict &operands_chained_from) {
  int         size;                // characters sprintf'd into cost_buffer
  const char *result, *cost, *rule;
  const char *total_cost;
  char        cost_buffer[COST_BUFFER_SIZE];

  // Check if we have already generated chains from this starting point
  if( operands_chained_from[operand] != NULL ) {
    return;
  } else {
    operands_chained_from.Insert( operand, operand);
  }

  ChainList *lst = (ChainList *)_chainRules[operand];
  if (lst) {
    // printf("\nChain from <%s> at cost #%s\n",operand, icost ? icost : "_");
    for(lst->reset(); (lst->iter(result,cost,rule)) == true; ) {
      // Compute the cost for previous match + chain_rule_cost
      if( icost == NULL ) {
        // No initial cost
        total_cost = cost;
      } else {
        if( strcmp("0",cost) == 0 ) {
          // chain for zero cost
          total_cost = icost;
        } else {
          // add cost for chain to current cost
          cost_buffer[0] = 0;
          size = sprintf( cost_buffer, "%s+%s", (icost ? icost : "0"), cost);
          assert( ( size > 0 ) && ( size < COST_BUFFER_SIZE - 1 ),
                 "Cost buffer overflow");
          total_cost = cost_buffer;
        }
      }
      // Do not generate operands that are already available
      if( operands_chained_from[result] != NULL ) {
        continue;
      } else {
        // Check for transitive chain rules
        Form *form = (Form *)_globalNames[rule];
        if ( ! form->is_instruction()) {
          // printf("   result=%s cost=%s rule=%s\n", result, total_cost, rule);
          // Check against other match costs, and update cost & rule vectors
          const char *reduce_rule = strcmp(irule,"Invalid") ? irule : rule;
          cost_check(fp, indent, machOperEnum(result), total_cost, reduce_rule);
          chain_rule(fp, indent, result, total_cost, irule, operands_chained_from);
        } else {
          // printf("   result=%s cost=%s rule=%s\n", result, total_cost, rule);
          // Check against other match costs, and update cost & rule vectors
          cost_check(fp, indent, machOperEnum(result), total_cost, rule);
	  chain_rule(fp, indent, result, total_cost, rule, operands_chained_from);
        }

        // If this is a member of an operand class, update class cost & rule
        expand_opclass( fp, indent, total_cost, result );
      }
    }
  }
}

//---------------------------prune_matchlist-----------------------------------
// Check for duplicate entries in a matchlist, and prune out the higher cost
// entry.
void ArchDesc::prune_matchlist(Dict &minimize, MatchList &mlist) {

}

//---------------------------buildDFA------------------------------------------
// DFA is a large switch with case statements for each ideal opcode encountered
// in any match rule in the ad file.  Each case has a series of if's to handle
// the match or fail decisions.  The matches test the cost function of that
// rule, and prune any cases which are higher cost for the same reduction.
// In order to generate the DFA we walk the table of ideal opcode/MatchList
// pairs generated by the ADLC front end to build the contents of the case
// statements (a series of if statements).
void ArchDesc::buildDFA(FILE* fp) {
  int i;
  // Remember operands that are the starting points for chain rules.
  // Prevent cycles by checking if we have already generated chain.
  Dict operands_chained_from(cmpstr, hashstr, Form::arena);

  // Hash inputs to match rules so that final DFA contains only one entry for
  // each match pattern which is the low cost entry.
  Dict minimize(cmpstr, hashstr, Form::arena);

  // Output the start of the DFA method into the output file
  
  fprintf(fp, "\n");
  fprintf(fp, "//------------------------- Source -----------------------------------------\n");
  // Do not put random source code into the DFA.
  // If there are constants which need sharing, put them in "source_hpp" forms.
  // _source.output(fp);
  fprintf(fp, "\n");
  fprintf(fp, "//------------------------- Attributes -------------------------------------\n");
  _attributes.output(fp);
  fprintf(fp, "\n");
  fprintf(fp, "//------------------------- DFA --------------------------------------------\n");
  
  fprintf(fp, 
"// DFA is a large switch with case statements for each ideal opcode encountered\n"
"// in any match rule in the ad file.  Each case has a series of if's to handle\n"
"// the match or fail decisions.  The matches test the cost function of that\n"
"// rule, and prune any cases which are higher cost for the same reduction.\n"
"// In order to generate the DFA we walk the table of ideal opcode/MatchList\n"
"// pairs generated by the ADLC front end to build the contents of the case\n"
"// statements (a series of if statements).\n"
);
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  if (_dfa_small) {
    // Now build the individual routines just like the switch entries in large version
    // Iterate over the table of MatchLists, start at first valid opcode of 1
    for (i = 1; i < _last_opcode; i++) {
      if (_mlistab[i] == NULL) continue;
      // Generate the routine header statement for this opcode
      fprintf(fp, "void  State::_sub_Op_%s(const Node *n){\n", NodeClassNames[i]);
      // Generate body. Shared for both inline and out-of-line version
      gen_dfa_state_body(fp, minimize, operands_chained_from, i);
      // End of routine
      fprintf(fp, "}\n");
    }      
  }
  fprintf(fp, "bool State::DFA");
  fprintf(fp, "(int opcode, const Node *n) {\n");
  fprintf(fp, "  switch(opcode) {\n");

  // Iterate over the table of MatchLists, start at first valid opcode of 1
  for (i = 1; i < _last_opcode; i++) {
    if (_mlistab[i] == NULL) continue;
    // Generate the case statement for this opcode
    if (_dfa_small) {
      fprintf(fp, "  case Op_%s: { _sub_Op_%s(n);\n", NodeClassNames[i], NodeClassNames[i]);
    } else {
      fprintf(fp, "  case Op_%s: {\n", NodeClassNames[i]);
      // Walk the list, compacting it
      gen_dfa_state_body(fp, minimize, operands_chained_from, i);
    }
    // Print the "break"
    fprintf(fp, "    break;\n");
    fprintf(fp, "  }\n");
  }

  // Generate the default case for switch(opcode)
  fprintf(fp, "  \n");
  fprintf(fp, "  default:\n");
  fprintf(fp, "    tty->print(\"Default case invoked for: \\n\");\n");
  fprintf(fp, "    tty->print(\"   opcode  = %cd, \\\"%cs\\\"\\n\", opcode, NodeClassNames[opcode]);\n", '%', '%');
  fprintf(fp, "    return false;\n");
  fprintf(fp, "  }\n");

  // Return status, indicating a successful match.
  fprintf(fp, "  return true;\n");
  // Generate the closing brace for method Matcher::DFA
  fprintf(fp, "}\n");
}


class dfa_shared_preds {
  enum { count = 2 };

  static bool        _found[count];
  static const char* _type [count];
  static const char* _var  [count];
  static const char* _pred [count];

  static void check_index(int index) { assert( 0 <= index && index < count, "Invalid index"); }

  // Confirm that this is a separate sub-expression.
  // Only need to catch common cases like " ... && shared ..." 
  // and avoid hazardous ones like "...->shared"
  static bool valid_loc(char *pred, char *shared) {
    // start of predicate is valid
    if( shared == pred ) return true;

    // Check previous character and recurse if needed
    char *prev = shared - 1;
    char c  = *prev;
    switch( c ) {
    case ' ':
      return dfa_shared_preds::valid_loc(pred, prev);
    case '!':
    case '(':
    case '<':
    case '=':
      return true;
    case '|':
      if( prev != pred && *(prev-1) == '|' ) return true;
    case '&':
      if( prev != pred && *(prev-1) == '&' ) return true;
    default:
      return false;
    }

    return false;
  }

public:

  static bool        found(int index){ check_index(index); return _found[index]; }
  static void    set_found(int index, bool val) { check_index(index); _found[index] = val; }
  static void  reset_found() { 
    for( int i = 0; i < count; ++i ) { _found[i] = false; }
  };

  static const char* type(int index) { check_index(index); return _type[index]; }
  static const char* var (int index) { check_index(index); return _var [index];  }
  static const char* pred(int index) { check_index(index); return _pred[index]; }

  // Check each predicate in the MatchList for common sub-expressions
  static void cse_matchlist(MatchList *matchList) {
    for( MatchList *mList = matchList; mList != NULL; mList = mList->get_next() ) {
      Predicate* predicate = mList->get_pred_obj();
      char*      pred      = mList->get_pred();
      if( pred != NULL ) {
        for(int index = 0; index < count; ++index ) {
          const char *shared_pred      = dfa_shared_preds::pred(index);
          const char *shared_pred_var  = dfa_shared_preds::var(index);
          bool result = dfa_shared_preds::cse_predicate(predicate, shared_pred, shared_pred_var);
          if( result ) dfa_shared_preds::set_found(index, true);
        }
      }
    }
  }
  
  // If the Predicate contains a common sub-expression, replace the Predicate's 
  // string with one that uses the variable name.
  static bool cse_predicate(Predicate* predicate, const char *shared_pred, const char *shared_pred_var) {
    bool result = false;
    char *pred = predicate->_pred;
    if( pred != NULL ) {
      char *new_pred = pred;
      for( char *shared_pred_loc = strstr(new_pred, shared_pred);
      shared_pred_loc != NULL && dfa_shared_preds::valid_loc(new_pred,shared_pred_loc);
      shared_pred_loc = strstr(new_pred, shared_pred) ) {
        // Do not modify the original predicate string, it is shared
        if( new_pred == pred ) {
          new_pred = strdup(pred);
          shared_pred_loc = strstr(new_pred, shared_pred);
        }
        // Replace shared_pred with variable name
        strncpy(shared_pred_loc, shared_pred_var, strlen(shared_pred_var));
      }
      // Install new predicate
      if( new_pred != pred ) { 
        predicate->_pred = new_pred;
        result = true;
      }
    }
    return result;
  }

  // Output the hoisted common sub-expression if we found it in predicates
  static void generate_cse(FILE *fp) {
    for(int j = 0; j < count; ++j ) {
      if( dfa_shared_preds::found(j) ) {
        const char *shared_pred_type = dfa_shared_preds::type(j);
        const char *shared_pred_var  = dfa_shared_preds::var(j);
        const char *shared_pred      = dfa_shared_preds::pred(j);
        fprintf(fp, "    %s %s = %s;\n", shared_pred_type, shared_pred_var, shared_pred);
      }
    }
  }
};
// shared predicates, _var and _pred entry should be the same length
bool         dfa_shared_preds::_found[dfa_shared_preds::count] = { false, false };
const char*  dfa_shared_preds::_type[dfa_shared_preds::count]  = { "int", "bool" };
const char*  dfa_shared_preds::_var [dfa_shared_preds::count]  = { "_n_get_int__", "Compile__current____select_24_bit_instr__" };
const char*  dfa_shared_preds::_pred[dfa_shared_preds::count]  = { "n->get_int()", "Compile::current()->select_24_bit_instr()" };


void ArchDesc::gen_dfa_state_body(FILE* fp, Dict &minimize, Dict &operands_chained_from, int i) {  
  // Walk the list, compacting it
  MatchList* mList = _mlistab[i];
  do {
    // Hash each entry using inputs as key and pointer as data.
    // If there is already an entry, keep the one with lower cost, and
    // remove the other one from the list.
    prune_matchlist(minimize, *mList);
    // Iterate
    mList = mList->get_next();
  } while(mList != NULL);

  // Hoist previously specified common sub-expressions out of predicates
  dfa_shared_preds::reset_found();
  dfa_shared_preds::cse_matchlist(_mlistab[i]);
  dfa_shared_preds::generate_cse(fp);

  mList = _mlistab[i];

  // Walk the list again, generating code
  do {
    // Each match can generate its own chains
    operands_chained_from.Clear();
    gen_match(fp, *mList, operands_chained_from);
    mList = mList->get_next();
  } while(mList != NULL);
  // Fill in any chain rules which add instructions
  // These can generate their own chains as well.
  operands_chained_from.Clear();  // 
  chain_rule(fp, "   ", (char *)NodeClassNames[i], 0, "Invalid",
	     operands_chained_from);      
}



