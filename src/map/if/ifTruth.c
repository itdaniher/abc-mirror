/**CFile****************************************************************

  FileName    [ifTruth.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [FPGA mapping based on priority cuts.]

  Synopsis    [Computation of truth tables of the cuts.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - November 21, 2006.]

  Revision    [$Id: ifTruth.c,v 1.00 2006/11/21 00:00:00 alanmi Exp $]

***********************************************************************/

#include "if.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

//#define IF_TRY_NEW

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Sorts the pins in the decreasing order of delays.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void If_CutTruthPermute( word * pTruth, int nLeaves, int nVars, int nWords, float * pDelays, int * pVars )
{
    while ( 1 )
    {
        int i, fChange = 0;
        for ( i = 0; i < nLeaves - 1; i++ )
        {
            if ( pDelays[i] >= pDelays[i+1] )
                continue;
            ABC_SWAP( float, pDelays[i], pDelays[i+1] );
            ABC_SWAP( int, pVars[i], pVars[i+1] );
            if ( pTruth )
                Abc_TtSwapAdjacent( pTruth, nWords, i );
            fChange = 1;
        }
        if ( !fChange )
            return;
    }
}
void If_CutRotatePins( If_Man_t * p, If_Cut_t * pCut )
{
    If_Obj_t * pLeaf;
    float PinDelays[IF_MAX_LUTSIZE];
    int i, truthId;
    assert( !p->pPars->fUseTtPerm );
    If_CutForEachLeaf( p, pCut, pLeaf, i )
        PinDelays[i] = If_ObjCutBest(pLeaf)->Delay;
    if ( p->vTtMem[pCut->nLeaves] == NULL )
    {
        If_CutTruthPermute( NULL, If_CutLeaveNum(pCut), pCut->nLeaves, p->nTruth6Words[pCut->nLeaves], PinDelays, If_CutLeaves(pCut) );
        return;
    }
    Abc_TtCopy( p->puTempW, If_CutTruthWR(p, pCut), p->nTruth6Words[pCut->nLeaves], 0 );
    If_CutTruthPermute( p->puTempW, If_CutLeaveNum(pCut), pCut->nLeaves, p->nTruth6Words[pCut->nLeaves], PinDelays, If_CutLeaves(pCut) );
    truthId        = Vec_MemHashInsert( p->vTtMem[pCut->nLeaves], p->puTempW );
    pCut->iCutFunc = Abc_Var2Lit( truthId, If_CutTruthIsCompl(pCut) );
    assert( (p->puTempW[0] & 1) == 0 );
}

/**Function*************************************************************

  Synopsis    [Truth table computation.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int If_CutComputeTruth( If_Man_t * p, If_Cut_t * pCut, If_Cut_t * pCut0, If_Cut_t * pCut1, int fCompl0, int fCompl1 )
{
    int fCompl, truthId, nLeavesNew, RetValue = 0;
    int nWords      = Abc_TtWordNum( pCut->nLeaves );
    word * pTruth0s = Vec_MemReadEntry( p->vTtMem[pCut0->nLeaves], Abc_Lit2Var(pCut0->iCutFunc) );
    word * pTruth1s = Vec_MemReadEntry( p->vTtMem[pCut1->nLeaves], Abc_Lit2Var(pCut1->iCutFunc) );
    word * pTruth0  = (word *)p->puTemp[0];
    word * pTruth1  = (word *)p->puTemp[1];
    word * pTruth   = (word *)p->puTemp[2];
    Abc_TtCopy( pTruth0, pTruth0s, nWords, fCompl0 ^ pCut0->fCompl ^ Abc_LitIsCompl(pCut0->iCutFunc) );
    Abc_TtCopy( pTruth1, pTruth1s, nWords, fCompl1 ^ pCut1->fCompl ^ Abc_LitIsCompl(pCut1->iCutFunc) );
    Abc_TtStretch6( pTruth0, pCut0->nLeaves, pCut->nLeaves );
    Abc_TtStretch6( pTruth1, pCut1->nLeaves, pCut->nLeaves );
    Abc_TtStretch( pTruth0, pCut->nLeaves, pCut0->pLeaves, pCut0->nLeaves, pCut->pLeaves, pCut->nLeaves );
    Abc_TtStretch( pTruth1, pCut->nLeaves, pCut1->pLeaves, pCut1->nLeaves, pCut->pLeaves, pCut->nLeaves );
    fCompl         = (pTruth0[0] & pTruth1[0] & 1);
    Abc_TtAnd( pTruth, pTruth0, pTruth1, nWords, fCompl );
    if ( p->pPars->fCutMin )
    {
        nLeavesNew = Abc_TtMinBase( pTruth, pCut->pLeaves, pCut->nLeaves, pCut->nLeaves );
        if ( nLeavesNew < If_CutLeaveNum(pCut) )
        {
            pCut->nLeaves = nLeavesNew;
            pCut->uSign   = If_ObjCutSignCompute( pCut );
            RetValue      = 1;
        }
    }
    truthId        = Vec_MemHashInsert( p->vTtMem[pCut->nLeaves], pTruth );
    pCut->iCutFunc = Abc_Var2Lit( truthId, fCompl );
    assert( (pTruth[0] & 1) == 0 );
#ifdef IF_TRY_NEW
    {
        word pCopy[1024];
        char pCanonPerm[16];
        memcpy( pCopy, If_CutTruthW(pCut), sizeof(word) * nWords );
        Abc_TtCanonicize( pCopy, pCut->nLeaves, pCanonPerm );
    }
#endif
    return RetValue;
}

/**Function*************************************************************

  Synopsis    [Truth table computation.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int If_CutComputeTruthPerm_int( If_Man_t * p, If_Cut_t * pCut, If_Cut_t * pCut0, If_Cut_t * pCut1, int iCutFunc0, int iCutFunc1 )
{
    int fVerbose = 0;
    int pPerm[IF_MAX_LUTSIZE];
    char pCanonPerm[IF_MAX_LUTSIZE];
    int v, Place, fCompl, truthId, nLeavesNew, uCanonPhase, RetValue = 0;
    int nWords      = Abc_TtWordNum( pCut->nLeaves );
    word * pTruth0s = Vec_MemReadEntry( p->vTtMem[pCut0->nLeaves], Abc_Lit2Var(iCutFunc0) );
    word * pTruth1s = Vec_MemReadEntry( p->vTtMem[pCut1->nLeaves], Abc_Lit2Var(iCutFunc1) );
    word * pTruth0  = (word *)p->puTemp[0];
    word * pTruth1  = (word *)p->puTemp[1];
    word * pTruth   = (word *)p->puTemp[2];
    assert( pCut0->iCutDsd >= 0 );
    assert( pCut1->iCutDsd >= 0 );
    Abc_TtCopy( pTruth0, pTruth0s, nWords, Abc_LitIsCompl(iCutFunc0) );
    Abc_TtCopy( pTruth1, pTruth1s, nWords, Abc_LitIsCompl(iCutFunc1) );
    Abc_TtStretch6( pTruth0, pCut0->nLeaves, pCut->nLeaves );
    Abc_TtStretch6( pTruth1, pCut1->nLeaves, pCut->nLeaves );

if ( fVerbose )
{
//Kit_DsdPrintFromTruth( pTruth0, pCut0->nLeaves ); printf( "\n" );
//Kit_DsdPrintFromTruth( pTruth1, pCut1->nLeaves ); printf( "\n" );
}
    // create literals
    for ( v = 0; v < (int)pCut0->nLeaves; v++ )
        pCut->pLeaves[v] = Abc_Var2Lit( pCut0->pLeaves[v], If_CutLeafBit(pCut0, v) );
    for ( v = 0; v < (int)pCut1->nLeaves; v++ )
        if ( p->pPerm[1][v] >= (int)pCut0->nLeaves )
            pCut->pLeaves[p->pPerm[1][v]] = Abc_Var2Lit( pCut1->pLeaves[v], If_CutLeafBit(pCut1, v) );
        else if ( If_CutLeafBit(pCut0, p->pPerm[1][v]) != If_CutLeafBit(pCut1, v) )
            Abc_TtFlip( pTruth1, nWords, v );  
    // permute variables
    for ( v = (int)pCut1->nLeaves; v < (int)pCut->nLeaves; v++ )
        p->pPerm[1][v] = -1;
    for ( v = 0; v < (int)pCut1->nLeaves; v++ )
    {
        Place = p->pPerm[1][v];
        if ( Place == v || Place == -1 )
            continue;
        Abc_TtSwapVars( pTruth1, pCut->nLeaves, v, Place );
        p->pPerm[1][v] = p->pPerm[1][Place];
        p->pPerm[1][Place] = Place;
        v--;
    }

if ( fVerbose )
{
//Kit_DsdPrintFromTruth( pTruth0, pCut0->nLeaves ); printf( "\n" );
//Kit_DsdPrintFromTruth( pTruth1, pCut->nLeaves ); printf( "\n" );
}

    // perform operation
    Abc_TtAnd( pTruth, pTruth0, pTruth1, nWords, 0 );
    // minimize support
    if ( p->pPars->fCutMin )
    {
        nLeavesNew = Abc_TtMinBase( pTruth, pCut->pLeaves, pCut->nLeaves, pCut->nLeaves );
        if ( nLeavesNew < If_CutLeaveNum(pCut) )
        {
            pCut->nLeaves = nLeavesNew;
            RetValue      = 1;
        }
    }
    // compute canonical form
    uCanonPhase = Abc_TtCanonicize( pTruth, pCut->nLeaves, pCanonPerm );
    for ( v = 0; v < (int)pCut->nLeaves; v++ )
        pPerm[v] = Abc_LitNotCond( pCut->pLeaves[(int)pCanonPerm[v]], ((uCanonPhase>>v)&1) );
    pCut->iCutDsd = 0;
    for ( v = 0; v < (int)pCut->nLeaves; v++ )
    {
        pCut->pLeaves[v] = Abc_Lit2Var(pPerm[v]);
        if ( Abc_LitIsCompl(pPerm[v]) )
            pCut->iCutDsd |= (1 << v);
    }
    // create signature after lowering literals
    if ( RetValue )
        pCut->uSign = If_ObjCutSignCompute( pCut );
    else
        assert( pCut->uSign == If_ObjCutSignCompute( pCut ) );

    // hash function
    fCompl         = ((uCanonPhase >> pCut->nLeaves) & 1);
    truthId        = Vec_MemHashInsert( p->vTtMem[pCut->nLeaves], pTruth );
    pCut->iCutFunc = Abc_Var2Lit( truthId, fCompl );

if ( fVerbose )
{
//Kit_DsdPrintFromTruth( pTruth, pCut->nLeaves ); printf( "\n" );
//If_CutPrint( pCut0 );
//If_CutPrint( pCut1 );
//If_CutPrint( pCut );
//printf( "%d\n\n", pCut->iCutFunc );
}

    return RetValue;
}
int If_CutComputeTruthPerm( If_Man_t * p, If_Cut_t * pCut, If_Cut_t * pCut0, If_Cut_t * pCut1, int iCutFunc0, int iCutFunc1 )
{
    int i, k, Num, nEntriesOld, RetValue;
//    if ( pCut0->nLeaves + pCut1->nLeaves > pCut->nLeaves || iCutFunc0 < 2 || iCutFunc1 < 2 )
        return If_CutComputeTruthPerm_int( p, pCut, pCut0, pCut1, iCutFunc0, iCutFunc1 );
    assert( pCut0->nLeaves + pCut1->nLeaves == pCut->nLeaves );
    nEntriesOld = Hash_IntManEntryNum(p->vPairHash);
    Num = Hash_Int2ManInsert( p->vPairHash, (iCutFunc0 << 5)|pCut0->nLeaves, (iCutFunc1 << 5)|pCut1->nLeaves, -1 );
    assert( Num > 0 );
    if ( nEntriesOld == Hash_IntManEntryNum(p->vPairHash) )
    {
        int v, pPerm[IF_MAX_LUTSIZE];
        char * pCanonPerm = Vec_StrEntryP( p->vPairPerms, Num * pCut->nLimit );
        pCut->iCutFunc = Vec_IntEntry( p->vPairRes, Num );
        // move complements from the fanin cuts
        for ( v = 0; v < (int)pCut->nLeaves; v++ )
            if ( v < (int)pCut0->nLeaves )
            {
                assert( pCut->pLeaves[v] == pCut0->pLeaves[v] );
                pCut->pLeaves[v] = Abc_Var2Lit( pCut->pLeaves[v], If_CutLeafBit(pCut0, v) );
            }
            else
            {
                assert( pCut->pLeaves[v] == pCut1->pLeaves[v-(int)pCut0->nLeaves] );
                pCut->pLeaves[v] = Abc_Var2Lit( pCut->pLeaves[v], If_CutLeafBit(pCut1, v-(int)pCut0->nLeaves) );
            }
        // reorder the cut
        for ( v = 0; v < (int)pCut->nLeaves; v++ )
        {
            assert( pCanonPerm[v] >= 0 );
            pPerm[v] = Abc_LitNotCond( pCut->pLeaves[Abc_Lit2Var((int)pCanonPerm[v])], Abc_LitIsCompl((int)pCanonPerm[v]) );
        }
        // generate the result
        pCut->iCutDsd = 0;
        for ( v = 0; v < (int)pCut->nLeaves; v++ )
        {
            pCut->pLeaves[v] = Abc_Lit2Var(pPerm[v]);
            if ( Abc_LitIsCompl(pPerm[v]) )
                pCut->iCutDsd |= (1 << v);
        }
//        printf( "Found: %d(%d) %d(%d) -> %d(%d)\n", iCutFunc0, pCut0->nLeaves, iCutFunc1, pCut0->nLeaves, pCut->iCutFunc, pCut->nLeaves );
        return 0;
    }
    RetValue = If_CutComputeTruthPerm_int( p, pCut, pCut0, pCut1, iCutFunc0, iCutFunc1 );
    assert( RetValue == 0 );
//    printf( "Added: %d(%d) %d(%d) -> %d(%d)\n", iCutFunc0, pCut0->nLeaves, iCutFunc1, pCut0->nLeaves, pCut->iCutFunc, pCut->nLeaves );
    // save the result
    assert( Num == Vec_IntSize(p->vPairRes) );
    Vec_IntPush( p->vPairRes, pCut->iCutFunc );
    // save the permutation
    assert( Num * (int)pCut->nLimit == Vec_StrSize(p->vPairPerms) );
    for ( i = 0; i < (int)pCut0->nLeaves; i++ )
        for ( k = 0; k < (int)pCut->nLeaves; k++ )
            if ( pCut0->pLeaves[i] == pCut->pLeaves[k] )
                { Vec_StrPush( p->vPairPerms, (char)Abc_Var2Lit(k, If_CutLeafBit(pCut0, i) != If_CutLeafBit(pCut, k)) ); break; }
    for ( i = 0; i < (int)pCut1->nLeaves; i++ )
        for ( k = 0; k < (int)pCut->nLeaves; k++ )
            if ( pCut1->pLeaves[i] == pCut->pLeaves[k] )
                { Vec_StrPush( p->vPairPerms, (char)Abc_Var2Lit(k, If_CutLeafBit(pCut1, i) != If_CutLeafBit(pCut, k)) ); break; }
    for ( i = (int)pCut0->nLeaves + (int)pCut1->nLeaves; i < (int)pCut->nLimit; i++ )
        Vec_StrPush( p->vPairPerms, (char)-1 );
    return 0;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

