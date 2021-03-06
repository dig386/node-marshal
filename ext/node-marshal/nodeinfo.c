/*
 * This file contains information about Ruby nodes
 * Supported Ruby versions:
 * - Ruby 1.9.3
 * - Ruby 2.2.1
 * - Ruby 2.3.0
 *
 * Fragments from Ruby source code are used here
 * (mainly from node.c, gc.c)
 *
 * License: BSD-2-Clause
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ruby.h>
#include <ruby/version.h>
#include "nodedump.h"

/* Information about nodes types

   NODE_ARRAY contains an undocumented feature: if an array contains
   more than 1 element the 2nd child of 2nd element will contain
   a reference to the last element of the array (and have NT_NODE
   not NT_LONG type)
   Another case is Hash: every 2nd element of NODE_ARRAY chain
   has NT_NODE type (see nodedump.c for details)

   Such child is ignored by GC because there is a reference to it
   from another place of Ruby AST.

   NODE_LASGN and NODE_DASGN_CURR have special value -1 of the 2nd
   child (that is usually node not a number)
 */
static int nodes_child_info[][4] = 
{
	{NODE_BLOCK,    NT_NODE, NT_NULL, NT_NODE},
	{NODE_IF,       NT_NODE, NT_NODE, NT_NODE},

	{NODE_CASE,     NT_NODE, NT_NODE, NT_NULL},
	{NODE_WHEN,     NT_NODE, NT_NODE, NT_NODE},

	{NODE_OPT_N,    NT_NULL, NT_NODE, NT_LONG}, /* ??? */
	{NODE_WHILE,    NT_NODE, NT_NODE, NT_LONG},
	{NODE_UNTIL,    NT_NODE, NT_NODE, NT_LONG},

	{NODE_ITER,     NT_VALUE, NT_NODE, NT_NODE}, /* ??? */
	{NODE_FOR,      NT_VALUE, NT_NODE, NT_NODE},

	{NODE_BREAK,    NT_NODE, NT_NULL, NT_NULL},
	{NODE_NEXT,     NT_NODE, NT_NULL, NT_NULL},
	{NODE_RETURN,   NT_NODE, NT_NULL, NT_NULL},

	{NODE_REDO,     NT_NULL, NT_NULL, NT_NULL},
	{NODE_RETRY,    NT_NULL, NT_NULL, NT_NULL},

	{NODE_BEGIN,    NT_NULL, NT_NODE, NT_NULL},

	{NODE_RESCUE,   NT_NODE, NT_NODE, NT_NODE},
	{NODE_RESBODY,  NT_NODE, NT_NODE, NT_NODE},
	{NODE_ENSURE,   NT_NODE, NT_NULL, NT_NODE},

	{NODE_AND,      NT_NODE, NT_NODE, NT_NULL},
	{NODE_OR,       NT_NODE, NT_NODE, NT_NULL},

	{NODE_MASGN,    NT_NODE, NT_NODE, NT_NODE},

	{NODE_LASGN,    NT_ID,   NT_NODE, NT_NULL},
	{NODE_DASGN,    NT_ID,   NT_NODE, NT_NULL},
	{NODE_DASGN_CURR, NT_ID,   NT_NODE, NT_NULL},
	{NODE_IASGN,    NT_ID,   NT_NODE, NT_NULL},
	{NODE_CVASGN,   NT_ID,   NT_NODE, NT_NULL},

	{NODE_GASGN,    NT_NULL, NT_NODE, NT_ENTRY},

	{NODE_CDECL,    NT_ID,   NT_NODE, NT_NODE},

	{NODE_OP_ASGN1, NT_NODE, NT_ID,   NT_NODE},
	{NODE_OP_ASGN2, NT_NODE, NT_NODE,   NT_NODE},

	{NODE_OP_ASGN_AND, NT_NODE, NT_NODE, NT_NULL},
	{NODE_OP_ASGN_OR,  NT_NODE, NT_NODE, NT_NULL},

	{NODE_CALL, NT_NODE, NT_ID, NT_NODE},
	{NODE_FCALL, NT_NULL, NT_ID, NT_NODE},
	{NODE_VCALL, NT_NULL, NT_ID, NT_NULL},
#ifdef NODE_QCALL
	{NODE_QCALL, NT_NODE, NT_ID, NT_NODE}, /* &. operator from Ruby 2.3 */
#endif	

	{NODE_SUPER, NT_NULL, NT_NULL, NT_NODE},
	{NODE_ZSUPER, NT_NULL, NT_NULL, NT_NULL},
	{NODE_ARRAY,  NT_NODE, NT_LONG, NT_NODE}, /* 2nd child has undocumented variants (see above) */
	{NODE_VALUES, NT_NODE, NT_LONG, NT_NODE},
	{NODE_ZARRAY, NT_NULL, NT_NULL, NT_NULL},

	{NODE_HASH,   NT_NODE, NT_NULL, NT_NULL},
	{NODE_YIELD,  NT_NODE, NT_NULL, NT_NULL},

	{NODE_LVAR,   NT_ID,   NT_NULL, NT_NULL},
	{NODE_DVAR,   NT_ID,   NT_NULL, NT_NULL},
	{NODE_IVAR,   NT_ID,   NT_NULL, NT_NULL},
	{NODE_CONST,  NT_ID,   NT_NULL, NT_NULL},
	{NODE_CVAR,   NT_ID,   NT_NULL, NT_NULL},

	{NODE_GVAR,   NT_NULL, NT_NULL, NT_ENTRY},

	{NODE_NTH_REF, NT_NULL, NT_LONG, NT_NULL},
	{NODE_BACK_REF, NT_NULL, NT_LONG, NT_LONG},

	{NODE_MATCH,  NT_VALUE, NT_NULL, NT_NULL},
	{NODE_MATCH2, NT_NODE, NT_NODE, NT_NULL},
	{NODE_MATCH3, NT_NODE, NT_NODE, NT_NULL},

	{NODE_LIT,    NT_VALUE, NT_NULL, NT_NULL},
	{NODE_STR,    NT_VALUE, NT_NULL, NT_NULL},
	{NODE_XSTR,   NT_VALUE, NT_NULL, NT_NULL},

	{NODE_DSTR,        NT_VALUE, NT_NULL, NT_NODE},
	{NODE_DXSTR,       NT_VALUE, NT_NULL, NT_NODE},
	{NODE_DREGX,       NT_VALUE, NT_NULL, NT_NODE},
	{NODE_DREGX_ONCE,  NT_VALUE, NT_NULL, NT_NODE},
	{NODE_DSYM,        NT_VALUE, NT_NULL, NT_NODE},

	{NODE_EVSTR, NT_NULL, NT_NODE, NT_NULL},

	{NODE_ARGSCAT, NT_NODE, NT_NODE, NT_NULL},
	{NODE_ARGSPUSH, NT_NODE, NT_NODE, NT_NULL},
	{NODE_SPLAT, NT_NODE, NT_NULL, NT_NULL},
	{NODE_BLOCK_PASS, NT_NODE, NT_NODE, NT_NODE}, /* ??? */

	{NODE_DEFN, NT_NULL, NT_ID,   NT_NODE},
	{NODE_DEFS, NT_NODE, NT_ID, NT_NODE},
	{NODE_ALIAS, NT_NODE, NT_NODE, NT_NULL},
	{NODE_VALIAS, NT_ID, NT_ID, NT_NULL},
	{NODE_UNDEF, NT_NULL, NT_NODE, NT_NULL},

	{NODE_CLASS, NT_NODE, NT_NODE, NT_NODE},
	{NODE_MODULE, NT_NODE, NT_NODE, NT_NULL},
	{NODE_SCLASS, NT_NODE, NT_NODE, NT_NULL},

	{NODE_COLON2, NT_NODE, NT_ID,   NT_NULL},
	{NODE_COLON3, NT_NULL, NT_ID,   NT_NULL},

	{NODE_DOT2,   NT_NODE, NT_NODE, NT_NULL},
	{NODE_DOT3,   NT_NODE, NT_NODE, NT_NULL},
	{NODE_FLIP2,  NT_NODE, NT_NODE, NT_NULL},
	{NODE_FLIP3,  NT_NODE, NT_NODE, NT_NULL},

	{NODE_SELF,    NT_NULL, NT_NULL, NT_NULL},
	{NODE_NIL,     NT_NULL, NT_NULL, NT_NULL},
	{NODE_TRUE,    NT_NULL, NT_NULL, NT_NULL},
	{NODE_FALSE,   NT_NULL, NT_NULL, NT_NULL},
	{NODE_ERRINFO, NT_NULL, NT_NULL, NT_NULL},

	{NODE_DEFINED, NT_NODE, NT_NULL, NT_NULL},

	{NODE_POSTEXE,  NT_NULL, NT_NODE, NT_NULL},
	{NODE_ATTRASGN, NT_NODE, NT_ID,   NT_NODE},
	{NODE_PRELUDE,  NT_NODE, NT_NODE, NT_NULL},

	{NODE_LAMBDA,   NT_NULL, NT_NODE, NT_NULL},

	{NODE_OPT_ARG,  NT_NULL, NT_NODE, NT_NODE},
	{NODE_POSTARG,  NT_NODE, NT_NODE, NT_NULL},

#ifdef USE_RB_ARGS_INFO
	{NODE_ARGS,     NT_NULL,    NT_VALUE,    NT_ARGS}, /* ??? */
	{NODE_KW_ARG,	NT_NULL,    NT_NODE,     NT_NODE},
#else
	{NODE_ARGS,     NT_NODE,    NT_NULL,    NT_NODE}, /* ??? */
#endif
	{NODE_SCOPE,    NT_IDTABLE, NT_NODE,    NT_NODE},

	{NODE_ARGS_AUX, NT_LONG,      NT_LONG,    NT_NODE},

	{-1, 0, 0, 0}
};



/*
 * Check the correctness of nodes table from the viewpoint
 * This function is based on Ruby source code (node.c)
 */
void check_nodes_child_info(int pos)
{
	int type = nodes_child_info[pos][0];
	int isval[3] = {0, 0, 0};
	int isval_tbl[3] = {1, 1, 1};
	int i;
	/* Check NODE_LAMBDA position */
	if (strcmp(ruby_node_name(NODE_LAMBDA), "NODE_LAMBDA"))
	{
		rb_raise(rb_eArgError, "Invalid NODE_LAMBDA position");
	}
	/* RUBY 1.9.3 VARIANT */
#if RUBY_API_VERSION_MAJOR == 1
	switch (type)
	{
	case NODE_IF:         /* 1,2,3 */
	case NODE_FOR:
	case NODE_ITER:
	case NODE_WHEN:
	case NODE_MASGN:
	case NODE_RESCUE:
	case NODE_RESBODY:
	case NODE_CLASS:
	case NODE_BLOCK_PASS:
		isval[0] = 1;
		isval[1] = 1;
		isval[2] = 1;
		break;

	case NODE_BLOCK:      /* 1,3 */
	case NODE_OPTBLOCK:
	case NODE_ARRAY:
	case NODE_DSTR:
	case NODE_DXSTR:
	case NODE_DREGX:
	case NODE_DREGX_ONCE:
	case NODE_ENSURE:
	case NODE_CALL:
	case NODE_DEFS:
	case NODE_OP_ASGN1:
	case NODE_ARGS:
		isval[0] = 1;
		isval[2] = 1;
		break;

	case NODE_SUPER:      /* 3 */
	case NODE_FCALL:
	case NODE_DEFN:
	case NODE_ARGS_AUX:
		isval[2] = 1;
		break;

	case NODE_WHILE:      /* 1,2 */
	case NODE_UNTIL:
	case NODE_AND:
	case NODE_OR:
	case NODE_CASE:
	case NODE_SCLASS:
	case NODE_DOT2:
	case NODE_DOT3:
	case NODE_FLIP2:
	case NODE_FLIP3:
	case NODE_MATCH2:
	case NODE_MATCH3:
	case NODE_OP_ASGN_OR:
	case NODE_OP_ASGN_AND:
	case NODE_MODULE:
	case NODE_ALIAS:
//	case NODE_VALIAS:
	case NODE_ARGSCAT:
		isval[0] = 1;
		isval[1] = 1;
		break;

	case NODE_GASGN:      /* 2 */
	case NODE_LASGN:
	case NODE_DASGN:
	case NODE_DASGN_CURR:
	case NODE_IASGN:
	case NODE_IASGN2:
	case NODE_CVASGN:
//	case NODE_COLON3:
	case NODE_OPT_N:
	case NODE_EVSTR:
	case NODE_UNDEF:
	case NODE_POSTEXE:
		isval[1] = 1;
		break;

	case NODE_HASH:       /* 1 */
	case NODE_LIT:
	case NODE_STR:
	case NODE_XSTR:
	case NODE_DEFINED:
	case NODE_MATCH:
	case NODE_RETURN:
	case NODE_BREAK:
	case NODE_NEXT:
	case NODE_YIELD:
	case NODE_COLON2:
	case NODE_SPLAT:
	case NODE_TO_ARY:
		isval[0] = 1;
		break;

	case NODE_SCOPE:      /* 2,3 */
	case NODE_CDECL:
	case NODE_OPT_ARG:
		isval[1] = 1;
		isval[2] = 1;
		break;

	case NODE_ZARRAY:     /* - */
	case NODE_ZSUPER:
	case NODE_VCALL:
	case NODE_GVAR:
	case NODE_LVAR:
	case NODE_DVAR:
	case NODE_IVAR:
	case NODE_CVAR:
	case NODE_NTH_REF:
	case NODE_BACK_REF:
	case NODE_REDO:
	case NODE_RETRY:
	case NODE_SELF:
	case NODE_NIL:
	case NODE_TRUE:
	case NODE_FALSE:
	case NODE_ERRINFO:
	case NODE_BLOCK_ARG:
		break;

	default:
		return;
	}
#elif (RUBY_API_VERSION_MAJOR == 2) && (RUBY_API_VERSION_MINOR == 2)
	/* RUBY 2.2.1 VARIANT */    
	switch (type)
	{
	case NODE_IF:		/* 1,2,3 */
	case NODE_FOR:
	case NODE_ITER:
	case NODE_WHEN:
	case NODE_MASGN:
	case NODE_RESCUE:
	case NODE_RESBODY:
	case NODE_CLASS:
	case NODE_BLOCK_PASS:
		isval[0] = 1;
		isval[1] = 1;
		isval[2] = 1;
		break;

	case NODE_BLOCK:	/* 1,3 */
	case NODE_ARRAY:
	case NODE_DSTR:
	case NODE_DXSTR:
	case NODE_DREGX:
	case NODE_DREGX_ONCE:
	case NODE_ENSURE:
	case NODE_CALL:
	case NODE_DEFS:
	case NODE_OP_ASGN1:
		isval[0] = 1;
		isval[2] = 1;
		break;

	case NODE_SUPER:	/* 3 */
	case NODE_FCALL:
	case NODE_DEFN:
	case NODE_ARGS_AUX:
		isval[2] = 1;
		break;

	case NODE_WHILE:	/* 1,2 */
	case NODE_UNTIL:
	case NODE_AND:
	case NODE_OR:
	case NODE_CASE:
	case NODE_SCLASS:
	case NODE_DOT2:
	case NODE_DOT3:
	case NODE_FLIP2:
	case NODE_FLIP3:
	case NODE_MATCH2:
	case NODE_MATCH3:
	case NODE_OP_ASGN_OR:
	case NODE_OP_ASGN_AND:
	case NODE_MODULE:
	case NODE_ALIAS:
	//case NODE_VALIAS:
	case NODE_ARGSCAT:
		isval[0] = 1;
		isval[1] = 1;
		break;
	
	case NODE_GASGN:	/* 2 */
	case NODE_LASGN:
	case NODE_DASGN:
	case NODE_DASGN_CURR:
	case NODE_IASGN:
	case NODE_IASGN2:
	case NODE_CVASGN:
	//case NODE_COLON3:
	case NODE_OPT_N:
	case NODE_EVSTR:
	case NODE_UNDEF:
	case NODE_POSTEXE:
		isval[1] = 1;
		break;

	case NODE_HASH:	/* 1 */
	case NODE_LIT:
	case NODE_STR:
	case NODE_XSTR:
	case NODE_DEFINED:
	case NODE_MATCH:
	case NODE_RETURN:
	case NODE_BREAK:
	case NODE_NEXT:
	case NODE_YIELD:
	case NODE_COLON2:
	case NODE_SPLAT:
	case NODE_TO_ARY:
		isval[0] = 1;
		break;
		
	case NODE_SCOPE:	/* 2,3 */
	case NODE_CDECL:
	case NODE_OPT_ARG:
		isval[1] = 1;
		isval[2] = 1;
		break;
		
	case NODE_ARGS:	/* custom */
		isval[1] = 1;
		break;

	case NODE_ZARRAY:	/* - */
	case NODE_ZSUPER:
	case NODE_VCALL:
	case NODE_GVAR:
	case NODE_LVAR:
	case NODE_DVAR:
	case NODE_IVAR:
	case NODE_CVAR:
	case NODE_NTH_REF:
	case NODE_BACK_REF:
	case NODE_REDO:
	case NODE_RETRY:
	case NODE_SELF:
	case NODE_NIL:
	case NODE_TRUE:
	case NODE_FALSE:
	case NODE_ERRINFO:
	case NODE_BLOCK_ARG:
		break;
	
	default:		/* unlisted NODE */
		//printf("Warning: unknown node %s in the initial table\n",
		//	ruby_node_name(nodes_child_info[pos][0]));
		return;
	}
#elif (RUBY_API_VERSION_MAJOR == 2) && (RUBY_API_VERSION_MINOR == 3)
	switch (type)
	{
	case NODE_IF:		/* 1,2,3 */
	case NODE_FOR:
	case NODE_ITER:
	case NODE_WHEN:
	case NODE_MASGN:
	case NODE_RESCUE:
	case NODE_RESBODY:
	case NODE_CLASS:
	case NODE_BLOCK_PASS:
		isval[1] = 1;
	/* fall through */
	case NODE_BLOCK:	/* 1,3 */
	case NODE_ARRAY:
	case NODE_DSTR:
	case NODE_DXSTR:
	case NODE_DREGX:
	case NODE_DREGX_ONCE:
	case NODE_ENSURE:
	case NODE_CALL:
	case NODE_DEFS:
	case NODE_OP_ASGN1:
		isval[0] = 1;
	/* fall through */
	case NODE_SUPER:	/* 3 */
	case NODE_FCALL:
	case NODE_DEFN:
	case NODE_ARGS_AUX:
		isval[2] = 1;
		break;

	case NODE_WHILE:	/* 1,2 */
	case NODE_UNTIL:
	case NODE_AND:
	case NODE_OR:
	case NODE_CASE:
	case NODE_SCLASS:
	case NODE_DOT2:
	case NODE_DOT3:
	case NODE_FLIP2:
	case NODE_FLIP3:
	case NODE_MATCH2:
	case NODE_MATCH3:
	case NODE_OP_ASGN_OR:
	case NODE_OP_ASGN_AND:
	case NODE_MODULE:
	case NODE_ALIAS:
	//case NODE_VALIAS:
	case NODE_ARGSCAT:
		isval[0] = 1;
	/* fall through */
	case NODE_GASGN:	/* 2 */
	case NODE_LASGN:
	case NODE_DASGN:
	case NODE_DASGN_CURR:
	case NODE_IASGN:
	case NODE_IASGN2:
	case NODE_CVASGN:
	//case NODE_COLON3:
	case NODE_OPT_N:
	case NODE_EVSTR:
	case NODE_UNDEF:
	case NODE_POSTEXE:
		isval[1] = 1;
		break;

	case NODE_HASH:	/* 1 */
	case NODE_LIT:
	case NODE_STR:
	case NODE_XSTR:
	case NODE_DEFINED:
	case NODE_MATCH:
	case NODE_RETURN:
	case NODE_BREAK:
	case NODE_NEXT:
	case NODE_YIELD:
	case NODE_COLON2:
	case NODE_SPLAT:
	case NODE_TO_ARY:
		isval[0] = 1;
		break;

	case NODE_SCOPE:	/* 2,3 */
	case NODE_CDECL:
	case NODE_OPT_ARG:
		isval[1] = 1;
		isval[2] = 1;
		break;

	case NODE_ARGS:	/* custom */
		isval[1] = 1;
		break;

	case NODE_ZARRAY:	/* - */
	case NODE_ZSUPER:
	case NODE_VCALL:
	case NODE_GVAR:
	case NODE_LVAR:
	case NODE_DVAR:
	case NODE_IVAR:
	case NODE_CVAR:
	case NODE_NTH_REF:
	case NODE_BACK_REF:
	case NODE_REDO:
	case NODE_RETRY:
	case NODE_SELF:
	case NODE_NIL:
	case NODE_TRUE:
	case NODE_FALSE:
	case NODE_ERRINFO:
	case NODE_BLOCK_ARG:
		break;
	/* NODE_ALLOCA EXCLUDED */
	default:		/* unlisted NODE */
		return;
	}
#endif
		
	for (i = 0; i < 3; i++)
		isval_tbl[i] = (nodes_child_info[pos][i+1] == NT_NODE || nodes_child_info[pos][i+1] == NT_VALUE) ? 1 : 0;
	
	if ( (isval[0] - isval_tbl[0] != 0) ||
	     (isval[1] - isval_tbl[1] != 0) ||
	     (isval[2] - isval_tbl[2] != 0) )
	{
		rb_raise(rb_eArgError, "Bad node entry in the initial table (%s): %d%d%d instead of %d%d%d",
			ruby_node_name(nodes_child_info[pos][0]),
			isval[0], isval[1], isval[2], isval_tbl[0], isval_tbl[1], isval_tbl[2]);
	}
	
}

/*
 * Converts nodes_child_info 2D array of int into nodes_ctbl 1D array of int
 *
 * nodes_child_info is similar to hash by the structure: each 1D subarray
 * has the next structure:
 *   {NODE_ID, NT_CHILD1_TYPE, NT_CHILD2_TYPE, NT_CHILD3_TYPE}
 */
void init_nodes_table(int *nodes_ctbl, int num_of_entries)
{
	int pos, i;
	/* Check the input array using information from Ruby source code */
	for (pos = 0; nodes_child_info[pos][0] != -1; pos++)
	{
		check_nodes_child_info(pos);
	}
	/* Initialize output array by NT_UNKNOWN (if node is not defined
	   in the input table the types of its childs are unknown) */	
	for (i = 0; i < num_of_entries * 3; i++)
	{
		nodes_ctbl[i] = NT_UNKNOWN;
	}
	/* Fill output array */
	for (pos = 0; nodes_child_info[pos][0] != -1; pos++)
	{
		int index = nodes_child_info[pos][0], offset;
		if (index < 0 || index > num_of_entries)
		{
			rb_raise(rb_eArgError, "NODE ID %d is out or nodes_ctbl array boundaries", index);
		}
		offset = index * 3;
		nodes_ctbl[offset++] = nodes_child_info[pos][1];
		nodes_ctbl[offset++] = nodes_child_info[pos][2];
		nodes_ctbl[offset++] = nodes_child_info[pos][3];
	}
}

