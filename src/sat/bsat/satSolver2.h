/**************************************************************************************************
MiniSat -- Copyright (c) 2005, Niklas Sorensson
http://www.cs.chalmers.se/Cs/Research/FormalMethods/MiniSat/

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/
// Modified to compile with MS Visual Studio 6.0 by Alan Mishchenko

#ifndef satSolver2_h
#define satSolver2_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "satVec.h"

ABC_NAMESPACE_HEADER_START

#define USE_FLOAT_ACTIVITY

//=================================================================================================
// Public interface:

struct sat_solver2_t;
typedef struct sat_solver2_t sat_solver2;

extern sat_solver2* sat_solver2_new(void);
extern void         sat_solver2_delete(sat_solver2* s);

extern int          sat_solver2_addclause(sat_solver2* s, lit* begin, lit* end);
extern int          sat_solver2_simplify(sat_solver2* s);
extern int          sat_solver2_solve(sat_solver2* s, lit* begin, lit* end, ABC_INT64_T nConfLimit, ABC_INT64_T nInsLimit, ABC_INT64_T nConfLimitGlobal, ABC_INT64_T nInsLimitGlobal);

extern void         sat_solver2_setnvars(sat_solver2* s,int n);

extern void         Sat_Solver2WriteDimacs( sat_solver2 * p, char * pFileName, lit* assumptionsBegin, lit* assumptionsEnd, int incrementVars );
extern void         Sat_Solver2PrintStats( FILE * pFile, sat_solver2 * p );
extern int *        Sat_Solver2GetModel( sat_solver2 * p, int * pVars, int nVars );
extern void         Sat_Solver2DoubleClauses( sat_solver2 * p, int iVar );

// trace recording
extern void         sat_solver2TraceStart( sat_solver2 * pSat, char * pName );
extern void         sat_solver2TraceStop( sat_solver2 * pSat );
extern void         sat_solver2TraceWrite( sat_solver2 * pSat, int * pBeg, int * pEnd, int fRoot );

// clause storage
extern void         sat_solver2_store_alloc( sat_solver2 * s );
extern void         sat_solver2_store_write( sat_solver2 * s, char * pFileName );
extern void         sat_solver2_store_free( sat_solver2 * s );
extern void         sat_solver2_store_mark_roots( sat_solver2 * s );
extern void         sat_solver2_store_mark_clauses_a( sat_solver2 * s );
extern void *       sat_solver2_store_release( sat_solver2 * s ); 

// global variables
extern int          var_is_partA (sat_solver2* s, int v);
extern void         var_set_partA(sat_solver2* s, int v, int partA);
// clause grouping (these two only work after creating a clause before the solver is called)
extern int          clause_is_partA (sat_solver2* s, int handle);
extern void         clause_set_partA(sat_solver2* s, int handle, int partA);
// other clause functions
extern int          clause_id(sat_solver2* s, int h);
 

//=================================================================================================
// Solver representation:

struct varinfo_t;
typedef struct varinfo_t varinfo;

struct sat_solver2_t
{
    int             size;           // nof variables
    int             cap;            // size of varmaps
    int             qhead;          // Head index of queue.
    int             qtail;          // Tail index of queue.

    int             root_level;     // Level of first proper decision.
    int             simpdb_assigns; // Number of top-level assignments at last 'simplifyDB()'.
    int             simpdb_props;   // Number of propagations before next 'simplifyDB()'.
    double          random_seed;
    double          progress_estimate;
    int             verbosity;      // Verbosity level. 0=silent, 1=some progress report, 2=everything
    int             fNotUseRandom;  // do not allow random decisions with a fixed probability
//  int             fSkipSimplify;  // set to one to skip simplification of the clause database
    int             fProofLogging;  // enable proof-logging

    // clauses
    veci            clauses;        // clause memory
    veci*           wlists;         // watcher lists (for each literal)
    int             hLearntFirst;   // the first learnt clause 
    int             hLearntLast;    // in proof-logging mode, the ID of the final conflict clause (conf_final)

    // activities
#ifdef USE_FLOAT_ACTIVITY
    double          var_inc;        // Amount to bump next variable with.
    double          var_decay;      // INVERSE decay factor for variable activity: stores 1/decay. 
    float           cla_inc;        // Amount to bump next clause with.
    float           cla_decay;      // INVERSE decay factor for clause activity: stores 1/decay.
    double*         activity;       // A heuristic measurement of the activity of a variable.
#else
    int             var_inc;        // Amount to bump next variable with.
    int             cla_inc;        // Amount to bump next clause with.
    unsigned*       activity;       // A heuristic measurement of the activity of a variable.
#endif
    veci            claActs;        // clause activities
    veci            claProofs;      // clause proofs

    // internal state
    varinfo *       vi;             // variable information
    lit*            trail;          // sequence of assignment and implications
    int*            orderpos;       // Index in variable order.
    cla*            reasons;        // reason clauses
    cla*            units;          // unit clauses

    veci            tagged;         // (contains: var)
    veci            stack;          // (contains: var)
    veci            order;          // Variable order. (heap) (contains: var)
    veci            trail_lim;      // Separator indices for different decision levels in 'trail'. (contains: int)
    veci            temp_clause;    // temporary storage for a CNF clause
    veci            model;          // If problem is solved, this vector contains the model (contains: lbool).
    veci            conf_final;     // If problem is unsatisfiable (possibly under assumptions),
                                    // this vector represent the final conflict clause expressed in the assumptions.
    veci            mark_levels;    // temporary storage for labeled levels
    veci            min_lit_order;  // ordering of removable literals
    veci            min_step_order; // ordering of resolution steps
    veci            learnt_live;    // remaining clauses after reduce DB

    // proof logging
    veci            proofs;         // sequence of proof records
    int             iStartChain;    // temporary variable to remember beginning of the current chain in proof logging
    int             nUnits;         // the total number of unit clauses

    // statistics
    stats_t         stats;
    ABC_INT64_T     nConfLimit;     // external limit on the number of conflicts
    ABC_INT64_T     nInsLimit;      // external limit on the number of implications
    int             nRuntimeLimit;  // external limit on runtime
};

typedef struct satset_t satset;
struct satset_t 
{
    unsigned learnt :  1;
    unsigned mark   :  1;
    unsigned partA  :  1;
    unsigned nEnts  : 29;
    int      Id;
    lit      pEnts[0];
};

static inline satset* satset_read   (veci* p, cla h )     { return h ? (satset*)(veci_begin(p) + h) : NULL;                           }
static inline cla     satset_handle (veci* p, satset* c)  { return (cla)((int *)c - veci_begin(p));                                   }
static inline int     satset_check  (veci* p, satset* c)  { return (int*)c > veci_begin(p) && (int*)c < veci_begin(p) + veci_size(p); }
static inline int     satset_size   (int nLits)           { return sizeof(satset)/4 + nLits;                                          }
static inline void    satset_print  (satset * c)          { 
    int i;
    printf( "{ " );
    for ( i = 0; i < (int)c->nEnts; i++ )
        printf( "%d ", (c->pEnts[i] & 1)? -(c->pEnts[i] >> 1) : c->pEnts[i] >> 1 );
    printf( "}\n" );
}

#define satset_foreach_entry( p, c, h, s )            \
    for ( h = s; (h < veci_size(p)) && (((c) = satset_read(p, h)), 1); h += satset_size(c->nEnts) )
#define satset_foreach_entry_vec( pVec, p, c, i )     \
    for ( i = 0; (i < veci_size(pVec)) && ((c) = satset_read(p, veci_begin(pVec)[i])); i++ )
#define satset_foreach_var( p, var, i, start )        \
    for ( i = start; (i < (int)(p)->nEnts) && ((var) = lit_var((p)->pEnts[i])); i++ )

//=================================================================================================
// Public APIs:

static inline int sat_solver2_nvars(sat_solver2* s)
{
    return s->size;
}

static inline int sat_solver2_nclauses(sat_solver2* s)
{
    return (int)s->stats.clauses;
}

static inline int sat_solver2_nconflicts(sat_solver2* s)
{
    return (int)s->stats.conflicts;
}

static inline int sat_solver2_var_value( sat_solver2* s, int v ) 
{
    assert( s->model.ptr != NULL && v < s->size );
    return (int)(s->model.ptr[v] == l_True);
}
static inline int sat_solver2_var_literal( sat_solver2* s, int v ) 
{
    assert( s->model.ptr != NULL && v < s->size );
    return toLitCond( v, s->model.ptr[v] != l_True );
}
static inline void sat_solver2_act_var_clear(sat_solver2* s) 
{
    int i;
    for (i = 0; i < s->size; i++)
        s->activity[i] = 0;//.0;
    s->var_inc = 1.0;
}
static inline void sat_solver2_compress(sat_solver2* s) 
{
    if ( s->qtail != s->qhead )
    {
        int RetValue = sat_solver2_simplify(s);
        assert( RetValue != 0 );
    }
}

static inline int sat_solver2_final(sat_solver2* s, int ** ppArray)
{
    *ppArray = s->conf_final.ptr;
    return s->conf_final.size;
}

static inline int sat_solver2_set_runtime_limit(sat_solver2* s, int Limit)
{
    int nRuntimeLimit = s->nRuntimeLimit;
    s->nRuntimeLimit = Limit;
    return nRuntimeLimit;
}

static inline int sat_solver2_set_random(sat_solver2* s, int fNotUseRandom)
{
    int fNotUseRandomOld = s->fNotUseRandom;
    s->fNotUseRandom = fNotUseRandom;
    return fNotUseRandomOld;
}

extern void Sat_ProofTest( veci * pClauses, veci * pProof, veci * pRoots, int hRoot );

ABC_NAMESPACE_HEADER_END

#endif
