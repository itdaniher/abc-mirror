/**CFile****************************************************************

  FileName    [vecSet.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [SAT solvers.]

  Synopsis    [Multi-page dynamic array.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: vecSet.h,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/
 
#ifndef ABC__sat__bsat__vecSet_h
#define ABC__sat__bsat__vecSet_h


////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include <stdio.h>

ABC_NAMESPACE_HEADER_START


////////////////////////////////////////////////////////////////////////
///                         PARAMETERS                               ///
////////////////////////////////////////////////////////////////////////

#define VEC_SET_PAGE        20
#define VEC_SET_MASK   0xFFFFF

////////////////////////////////////////////////////////////////////////
///                         BASIC TYPES                              ///
////////////////////////////////////////////////////////////////////////

// data-structure for logging entries
// memory is allocated in 2^VEC_SET_PAGE word-sized pages
// the first 'word' of each page is used storing additional data
// the first 'int' of additional data stores the word limit
// the second 'int' of the additional data stores the shadow word limit

typedef struct Vec_Set_t_ Vec_Set_t;
struct Vec_Set_t_ 
{
    int                nEntries;     // entry count
    int                iPage;        // current page
    int                iPageS;       // shadow page
    int                nPagesAlloc;  // page count allocated
    word **            pPages;       // page pointers
};

////////////////////////////////////////////////////////////////////////
///                      MACRO DEFINITIONS                           ///
////////////////////////////////////////////////////////////////////////

static inline int     Vec_SetHandPage( int h )                   { return h >> VEC_SET_PAGE;      }
static inline int     Vec_SetHandShift( int h )                  { return h & VEC_SET_MASK;       }
static inline int     Vec_SetWordNum( int nSize )                { return (nSize + 1) >> 1;       }

static inline word *  Vec_SetEntry( Vec_Set_t * p, int h )       { return p->pPages[Vec_SetHandPage(h)] + Vec_SetHandShift(h);     }
static inline int     Vec_SetEntryNum( Vec_Set_t * p )           { return p->nEntries;            }
static inline void    Vec_SetWriteEntryNum( Vec_Set_t * p, int i){ p->nEntries = i;               }

static inline int     Vec_SetLimit( word * p )                   { return ((int*)p)[0];           }
static inline int     Vec_SetLimitS( word * p )                  { return ((int*)p)[1];           }

static inline int     Vec_SetIncLimit( word * p, int nWords )    { return ((int*)p)[0] += nWords; }
static inline int     Vec_SetIncLimitS( word * p, int nWords )   { return ((int*)p)[1] += nWords; }

static inline void    Vec_SetWriteLimit( word * p, int nWords )  { ((int*)p)[0] = nWords;         }
static inline void    Vec_SetWriteLimitS( word * p, int nWords ) { ((int*)p)[1] = nWords;         }

static inline int     Vec_SetHandCurrent( Vec_Set_t * p )        { return (p->iPage << VEC_SET_PAGE)  + Vec_SetLimit(p->pPages[p->iPage]);   }
static inline int     Vec_SetHandCurrentS( Vec_Set_t * p )       { return (p->iPageS << VEC_SET_PAGE) + Vec_SetLimitS(p->pPages[p->iPageS]); }

static inline int     Vec_SetHandMemory( Vec_Set_t * p, int h )  { return Vec_SetHandPage(h) * (1 << (VEC_SET_PAGE+3)) + Vec_SetHandShift(h) * 8;  }
static inline int     Vec_SetMemory( Vec_Set_t * p )             { return Vec_SetHandMemory(p, Vec_SetHandCurrent(p));             }
static inline int     Vec_SetMemoryS( Vec_Set_t * p )            { return Vec_SetHandMemory(p, Vec_SetHandCurrentS(p));            }
static inline int     Vec_SetMemoryAll( Vec_Set_t * p )          { return (p->iPage+1) * (1 << (VEC_SET_PAGE+3));                                  }

// Type is the Set type
// pVec is vector of set
// nSize should be given by the user
// pSet is the pointer to the set
// p (page) and s (shift) are variables used here
#define Vec_SetForEachEntry( Type, pVec, nSize, pSet, p, s )   \
    for ( p = 0; p <= pVec->iPage; p++ )                       \
        for ( s = 1; s < Vec_SetLimit(pVec->pPages[p]) && ((pSet) = (Type)(pVec->pPages[p] + (s))); s += nSize )

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Allocating vector.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Vec_SetAlloc_( Vec_Set_t * p )
{
    memset( p, 0, sizeof(Vec_Set_t) );
    p->nPagesAlloc  = 256;
    p->pPages       = ABC_CALLOC( word *, p->nPagesAlloc );
    p->pPages[0]    = ABC_ALLOC( word, (1 << VEC_SET_PAGE) );
    p->pPages[0][0] = ~0;
    Vec_SetWriteLimit( p->pPages[0], 1 );
}
static inline Vec_Set_t * Vec_SetAlloc()
{
    Vec_Set_t * p;
    p = ABC_CALLOC( Vec_Set_t, 1 );
    Vec_SetAlloc_( p );
    return p;
}

/**Function*************************************************************

  Synopsis    [Resetting vector.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Vec_SetRestart( Vec_Set_t * p )
{
    p->nEntries     = 0;
    p->iPage        = 0;
    p->iPageS       = 0;
    p->pPages[0][0] = ~0;
    Vec_SetWriteLimit( p->pPages[0], 1 );
}

/**Function*************************************************************

  Synopsis    [Freeing vector.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Vec_SetFree_( Vec_Set_t * p )
{
    int i;
    for ( i = 0; i < p->nPagesAlloc; i++ )
        ABC_FREE( p->pPages[i] );
    ABC_FREE( p->pPages );
}
static inline void Vec_SetFree( Vec_Set_t * p )
{
    Vec_SetFree_( p );
    ABC_FREE( p );
}

/**Function*************************************************************

  Synopsis    [Appending entries to vector.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Vec_SetAppend( Vec_Set_t * p, int * pArray, int nSize )
{
    int nWords = Vec_SetWordNum( nSize );
    assert( nWords < (1 << VEC_SET_PAGE) );
    p->nEntries++;
    if ( Vec_SetLimit( p->pPages[p->iPage] ) + nWords > (1 << VEC_SET_PAGE) )
    {
        if ( ++p->iPage == p->nPagesAlloc )
        {
            p->pPages = ABC_REALLOC( word *, p->pPages, p->nPagesAlloc * 2 );
            memset( p->pPages + p->nPagesAlloc, 0, sizeof(word *) * p->nPagesAlloc );
            p->nPagesAlloc *= 2;
        }
        if ( p->pPages[p->iPage] == NULL )
            p->pPages[p->iPage] = ABC_ALLOC( word, (1 << VEC_SET_PAGE) );
        p->pPages[p->iPage][0] = ~0;
        Vec_SetWriteLimit( p->pPages[p->iPage], 1 );
    }
    if ( pArray )
        memmove( p->pPages[p->iPage] + Vec_SetLimit(p->pPages[p->iPage]), pArray, sizeof(int) * nSize );
    Vec_SetIncLimit( p->pPages[p->iPage], nWords );
    return Vec_SetHandCurrent(p) - nWords;
}
static inline int Vec_SetAppendS( Vec_Set_t * p, int nSize )
{
    int nWords = Vec_SetWordNum( nSize );
    assert( nWords < (1 << VEC_SET_PAGE) );
    if ( Vec_SetLimitS( p->pPages[p->iPageS] ) + nWords > (1 << VEC_SET_PAGE) )
        Vec_SetWriteLimitS( p->pPages[++p->iPageS], 1 );
    Vec_SetIncLimitS( p->pPages[p->iPageS], nWords );
    return Vec_SetHandCurrentS(p) - nWords;
}

/**Function*************************************************************

  Synopsis    [Shrinking vector size.]

  Description []
               
  SideEffects [This procedure does not update the number of entries.]

  SeeAlso     []

***********************************************************************/
static inline void Vec_SetShrink( Vec_Set_t * p, int h )
{
    assert( h <= Vec_SetHandCurrent(p) );
    p->iPage = Vec_SetHandPage(h);
    Vec_SetWriteLimit( p->pPages[p->iPage], Vec_SetHandShift(h) );
}
static inline void Vec_SetShrinkS( Vec_Set_t * p, int h ) 
{ 
    assert( h <= Vec_SetHandCurrent(p) );
    p->iPageS = Vec_SetHandPage(h); 
    Vec_SetWriteLimitS( p->pPages[p->iPageS], Vec_SetHandShift(h) );  
}

static inline void Vec_SetShrinkLimits( Vec_Set_t * p )
{
    int i;
    for ( i = 0; i <= p->iPage; i++ )
        Vec_SetWriteLimit( p->pPages[i], Vec_SetLimitS(p->pPages[i]) );
}


ABC_NAMESPACE_HEADER_END

#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

