/**CFile****************************************************************

  FileName    [abcNpn.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Network and node package.]

  Synopsis    [Procedures for testing and comparing semi-canonical forms.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: abcNpn.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "misc/extra/extra.h"
#include "misc/vec/vec.h"

#include "bool/kit/kit.h"
#include "bool/lucky/lucky.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////
 
// semi-canonical form types
// 0 - none
// 1 - based on counting 1s in cofactors
// 2 - based on minimum truth table value
// 3 - exact NPN

// data-structure to store a bunch of truth tables
typedef struct Abc_TtStore_t_  Abc_TtStore_t;
struct Abc_TtStore_t_ 
{
    int                nVars;
    int                nWords;
    int                nFuncs;
    word **            pFuncs;
};

extern Abc_TtStore_t * Abc_TtStoreLoad( char * pFileName );
extern void            Abc_TtStoreFree( Abc_TtStore_t * p );
extern void            Abc_TtStoreWrite( char * pFileName, Abc_TtStore_t * p );

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Counts the number of unique truth tables.]

  Description []
               
  SideEffects [] 

  SeeAlso     []

***********************************************************************/
int nWords = 0; // unfortunate global variable
int Abc_TruthCompare( word ** p1, word ** p2 ) { return memcmp(*p1, *p2, sizeof(word) * nWords); }
int Abc_TruthNpnCountUnique( Abc_TtStore_t * p )
{
    int i, k;
    // sort them by value
    nWords = p->nWords;
    assert( nWords > 0 );
    qsort( (void *)p->pFuncs, p->nFuncs, sizeof(word *), (int(*)(const void *,const void *))Abc_TruthCompare );
    // count the number of unqiue functions
    for ( i = k = 1; i < p->nFuncs; i++ )
        if ( memcmp( p->pFuncs[i-1], p->pFuncs[i], sizeof(word) * nWords ) )
            p->pFuncs[k++] = p->pFuncs[i];
    return (p->nFuncs = k);
}

/**Function*************************************************************

  Synopsis    [Prints out one NPN transform.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_TruthNpnPrint( char * pCanonPerm, unsigned uCanonPhase, int nVars )
{
    int i;
    printf( "   %c = ( ", Abc_InfoHasBit(&uCanonPhase, nVars) ? 'Z':'z' );
    for ( i = 0; i < nVars; i++ )
        printf( "%c%s", pCanonPerm[i] + ('A'-'a') * Abc_InfoHasBit(&uCanonPhase, pCanonPerm[i]-'a'), i == nVars-1 ? "":"," );
    printf( " )  " );
}

/**Function*************************************************************

  Synopsis    [Apply decomposition to the truth table.]

  Description [Returns the number of AIG nodes.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_TruthNpnPerform( Abc_TtStore_t * p, int NpnType, int fVerbose )
{
    unsigned pAux[2048];
    char pCanonPerm[32];
	unsigned uCanonPhase=0;
    clock_t clk = clock();
    int i;

    char * pAlgoName = NULL;
    if ( NpnType == 0 )
        pAlgoName = "uniqifying       ";
    else if ( NpnType == 1 )
        pAlgoName = "exact NPN        ";
    else if ( NpnType == 2 )
        pAlgoName = "counting 1s      ";
    else if ( NpnType == 3 )
        pAlgoName = "minimizing TT    ";
    else if ( NpnType == 4 )
        pAlgoName = "hybrid NPN       ";
    else if ( NpnType == 5 )
        pAlgoName = "Jake's hybrid NPN";

    assert( p->nVars <= 16 );
    if ( pAlgoName )
        printf( "Applying %-10s to %8d func%s of %2d vars...  ",  
            pAlgoName, p->nFuncs, (p->nFuncs == 1 ? "":"s"), p->nVars );
    if ( fVerbose )
        printf( "\n" );

    if ( NpnType == 0 )
    {
        for ( i = 0; i < p->nFuncs; i++ )
        {
            if ( fVerbose )
                printf( "%7d : ", i );
            if ( fVerbose )
                Extra_PrintHex( stdout, (unsigned *)p->pFuncs[i], p->nVars ), printf( "\n" );
        }
    }
    else if ( NpnType == 1 )
    {
        int * pComp = Extra_GreyCodeSchedule( p->nVars );
        int * pPerm = Extra_PermSchedule( p->nVars );
        if ( p->nVars == 6 )
        {
            for ( i = 0; i < p->nFuncs; i++ )
            {
                if ( fVerbose )
                    printf( "%7d : ", i );
                *((word *)p->pFuncs[i]) = Extra_Truth6MinimumExact( *((word *)p->pFuncs[i]), pComp, pPerm );
                if ( fVerbose )
                    Extra_PrintHex( stdout, (unsigned *)p->pFuncs[i], p->nVars ), printf( "\n" );
            }
        }
        else
            printf( "This feature only works for 6-variable functions.\n" );
        ABC_FREE( pComp );
        ABC_FREE( pPerm );
    }
    else if ( NpnType == 2 )
    {
        for ( i = 0; i < p->nFuncs; i++ )
        {
            if ( fVerbose )
                printf( "%7d : ", i );
            resetPCanonPermArray(pCanonPerm, p->nVars);
            uCanonPhase = Kit_TruthSemiCanonicize( (unsigned *)p->pFuncs[i], pAux, p->nVars, pCanonPerm );
            if ( fVerbose )
                Extra_PrintHex( stdout, (unsigned *)p->pFuncs[i], p->nVars ), Abc_TruthNpnPrint(pCanonPerm, uCanonPhase, p->nVars), printf( "\n" );
        }
    }
    else if ( NpnType == 3 )
    {
        for ( i = 0; i < p->nFuncs; i++ )
        {
            if ( fVerbose )
                printf( "%7d : ", i );
            resetPCanonPermArray(pCanonPerm, p->nVars);
            uCanonPhase = Kit_TruthSemiCanonicize_new( (unsigned *)p->pFuncs[i], pAux, p->nVars, pCanonPerm );
            if ( fVerbose )
                Extra_PrintHex( stdout, (unsigned *)p->pFuncs[i], p->nVars ), Abc_TruthNpnPrint(pCanonPerm, uCanonPhase, p->nVars), printf( "\n" );
        }
    }
    else if ( NpnType == 4 )
    {
        if ( p->nVars == 6 )
        {
            for ( i = 0; i < p->nFuncs; i++ )
            {
                if ( fVerbose )
                    printf( "%7d : ", i );
                resetPCanonPermArray(pCanonPerm, p->nVars);
                Kit_TruthSemiCanonicize( (unsigned *)p->pFuncs[i], pAux, p->nVars, pCanonPerm );
                *((word *)p->pFuncs[i]) = Extra_Truth6MinimumHeuristic( *((word *)p->pFuncs[i]) );
                if ( fVerbose )
                    Extra_PrintHex( stdout, (unsigned *)p->pFuncs[i], p->nVars ), printf( "\n" );
            }
        }
        else
            printf( "This feature only works for 6-variable functions.\n" );
    }
    else if ( NpnType == 5 )
    {
        for ( i = 0; i < p->nFuncs; i++ )
        {
            if ( fVerbose )
                printf( "%7d : ", i );
            resetPCanonPermArray(pCanonPerm, p->nVars);
            uCanonPhase = luckyCanonicizer_final_fast( p->pFuncs[i], p->nVars, pCanonPerm );
            if ( fVerbose )
                Extra_PrintHex( stdout, (unsigned *)p->pFuncs[i], p->nVars ), Abc_TruthNpnPrint(pCanonPerm, uCanonPhase, p->nVars), printf( "\n" );
        }
    }
    else assert( 0 );

    clk = clock() - clk;
    printf( "Classes =%9d  ", Abc_TruthNpnCountUnique(p) );
    Abc_PrintTime( 1, "Time", clk );
}

/**Function*************************************************************

  Synopsis    [Apply decomposition to truth tables.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_TruthNpnTest( char * pFileName, int NpnType, int fVerbose )
{
    Abc_TtStore_t * p;
    char * pFileNameOut;

    // read info from file
    p = Abc_TtStoreLoad( pFileName );
    if ( p == NULL )
        return;

    // consider functions from the file
    Abc_TruthNpnPerform( p, NpnType, fVerbose );

    // write the result
    pFileNameOut = Extra_FileNameGenericAppend( pFileName, "_out.txt" );
    Abc_TtStoreWrite( pFileNameOut, p );
    if ( fVerbose )
        printf( "The resulting functions are written into file \"%s\".\n", pFileNameOut );

    // delete data-structure
    Abc_TtStoreFree( p );
//    printf( "Finished computing canonical forms for functions from file \"%s\".\n", pFileName );
}


/**Function*************************************************************

  Synopsis    [Testbench for decomposition algorithms.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Abc_NpnTest( char * pFileName, int NpnType, int fVerbose )
{
    if ( fVerbose )
        printf( "Using truth tables from file \"%s\"...\n", pFileName );
    if ( NpnType >= 0 && NpnType <= 5 )
        Abc_TruthNpnTest( pFileName, NpnType, fVerbose );
    else
        printf( "Unknown canonical form value (%d).\n", NpnType );
    fflush( stdout );
    return 0;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

