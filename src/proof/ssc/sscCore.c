/**CFile****************************************************************

  FileName    [sscCore.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [SAT sweeping under constraints.]

  Synopsis    [The core procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 29, 2008.]

  Revision    [$Id: sscCore.c,v 1.00 2008/07/29 00:00:00 alanmi Exp $]

***********************************************************************/

#include "sscInt.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [This procedure sets default parameters.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ssc_ManSetDefaultParams( Ssc_Pars_t * p )
{
    memset( p, 0, sizeof(Ssc_Pars_t) );
    p->nWords         =     1;  // the number of simulation words
    p->nBTLimit       =  1000;  // conflict limit at a node
    p->nSatVarMax     =  5000;  // the max number of SAT variables
    p->nCallsRecycle  =   100;  // calls to perform before recycling SAT solver
    p->fVerbose       =     0;  // verbose stats
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ssc_ManStop( Ssc_Man_t * p )
{
    Vec_IntFreeP( &p->vFront );
    Vec_IntFreeP( &p->vFanins );
    Vec_IntFreeP( &p->vPattern );
    Vec_IntFreeP( &p->vDisPairs );
    Vec_IntFreeP( &p->vPivot );
    Vec_IntFreeP( &p->vId2Var );
    Vec_IntFreeP( &p->vVar2Id );
    if ( p->pSat ) sat_solver_delete( p->pSat );
    Gia_ManStopP( &p->pFraig );
    ABC_FREE( p );
}
Ssc_Man_t * Ssc_ManStart( Gia_Man_t * pAig, Gia_Man_t * pCare, Ssc_Pars_t * pPars )
{
    Ssc_Man_t * p;
    p = ABC_CALLOC( Ssc_Man_t, 1 );
    p->pPars  = pPars;
    p->pAig   = pAig;
    p->pCare  = pCare;
    p->pFraig = Gia_ManDup( p->pCare );
    Gia_ManHashStart( p->pFraig );
    Gia_ManInvertPos( p->pFraig );
    Ssc_ManStartSolver( p );
    if ( p->pSat == NULL )
    {
        printf( "Constraints are UNSAT after propagation (likely a bug!).\n" );
        Ssc_ManStop( p );
        return NULL;
    }
//    p->vPivot = Ssc_GiaFindPivotSim( p->pFraig );
//    Vec_IntFreeP( &p->vPivot  );
    p->vPivot = Ssc_ManFindPivotSat( p );
    if ( p->vPivot == NULL )
    {
        printf( "Constraints are UNSAT or conflict limit is too low.\n" );
        Ssc_ManStop( p );
        return NULL;
    }
    sat_solver_bookmark( p->pSat );
    Vec_IntPrint( p->vPivot );
    Gia_ManSetPhasePattern( p->pAig, p->vPivot );
    Gia_ManSetPhasePattern( p->pCare, p->vPivot );
    if ( Gia_ManCheckCoPhase(p->pCare) )
    {
        printf( "Computed reference pattern violates %d constraints (this is a bug!).\n", Gia_ManCheckCoPhase(p->pCare) );
        Ssc_ManStop( p );
        return NULL;
    }
    // other things
    p->vDisPairs = Vec_IntAlloc( 100 );
    p->vPattern = Vec_IntAlloc( 100 );
    p->vFanins = Vec_IntAlloc( 100 );
    p->vFront = Vec_IntAlloc( 100 );
    Ssc_GiaClassesInit( pAig );
    return p;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t * Ssc_PerformSweeping( Gia_Man_t * pAig, Gia_Man_t * pCare, Ssc_Pars_t * pPars )
{
    Ssc_Man_t * p;
    Gia_Man_t * pResult;
    Gia_Obj_t * pObj, * pRepr;
    clock_t clk, clkTotal = clock();
    int i, fCompl, status;
    assert( Gia_ManRegNum(pCare) == 0 );
    assert( Gia_ManCiNum(pAig) == Gia_ManCiNum(pCare) );
    assert( Gia_ManIsNormalized(pAig) );
    assert( Gia_ManIsNormalized(pCare) );
    // reset random numbers
    Gia_ManRandom( 1 );
    // sweeping manager
    p = Ssc_ManStart( pAig, pCare, pPars );
    if ( p == NULL )
        return Gia_ManDup( pAig );
    // perform simulation 
clk = clock();
    if ( Gia_ManAndNum(pCare) == 0 ) // no constraints
    {
        for ( i = 0; i < 16; i++ )
        {
            Ssc_GiaRandomPiPattern( pAig, 10, NULL );
            Ssc_GiaSimRound( pAig );
            Ssc_GiaClassesRefine( pAig );
            Gia_ManEquivPrintClasses( pAig, 0, 0 );
        }
    }
p->timeSimInit += clock() - clk;

    // prepare user's AIG
    Gia_ManFillValue(pAig);
    Gia_ManConst0(pAig)->Value = 0;
    Gia_ManForEachCi( pAig, pObj, i )
        pObj->Value = Gia_Obj2Lit( p->pFraig, Gia_ManCi(p->pFraig, i) );
    // perform sweeping
    Ssc_GiaResetPiPattern( pAig, pPars->nWords );
    Ssc_GiaSavePiPattern( pAig, p->vPivot );
    Gia_ManForEachCand( pAig, pObj, i )
    {
        if ( pAig->iPatsPi == 64 * pPars->nWords )
        {
            Ssc_GiaSimRound( pAig );
            Ssc_GiaClassesRefine( pAig );
            Gia_ManEquivPrintClasses( pAig, 0, 0 );
            Ssc_GiaClassesCheckPairs( pAig, p->vDisPairs );
            Ssc_GiaResetPiPattern( pAig, pPars->nWords );
            Ssc_GiaSavePiPattern( pAig, p->vPivot );
            Vec_IntClear( p->vDisPairs );
        }
        if ( Gia_ObjIsAnd(pObj) )
            pObj->Value = Gia_ManHashAnd( p->pFraig, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj) );
        if ( !Gia_ObjHasRepr(pAig, i) )
            continue;
        pRepr = Gia_ObjReprObj(pAig, i);
        if ( pRepr->Value == pObj->Value )
            continue;
        assert( Abc_Lit2Var(pRepr->Value) != Abc_Lit2Var(pObj->Value) );
        fCompl = pRepr->fPhase ^ pObj->fPhase ^ Abc_LitIsCompl(pRepr->Value) ^ Abc_LitIsCompl(pObj->Value);

        // perform SAT call
        clk = clock();
        p->nSatCalls++;
        status = Ssc_ManCheckEquivalence( p, Abc_Lit2Var(pRepr->Value), Abc_Lit2Var(pObj->Value), fCompl );
        if ( status == l_False )
        {
            p->nSatCallsUnsat++;
            p->timeSatUnsat += clock() - clk;
            pObj->Value = Abc_LitNotCond( pRepr->Value, pRepr->fPhase ^ pObj->fPhase );
        }
        else if ( status == l_True )
        {
            p->nSatCallsSat++;
            p->timeSatSat += clock() - clk;
            Ssc_GiaSavePiPattern( pAig, p->vPattern );
            Vec_IntPush( p->vDisPairs, Gia_ObjRepr(p->pAig, i) );
            Vec_IntPush( p->vDisPairs, i );
        }
        else if ( status == l_Undef )
        {
            p->nSatCallsUndec++;
            p->timeSatUndec += clock() - clk;
        }
        else assert( 0 );
    }

    // remember the resulting AIG
    pResult = Gia_ManEquivReduce( pAig, 1, 0, 0 );
    if ( pResult == NULL )
    {
        printf( "There is no equivalences.\n" );
        pResult = Gia_ManDup( pAig );
    }
    Ssc_ManStop( p );
    // count the number of representatives
    if ( pPars->fVerbose ) 
    {
        Gia_ManEquivPrintClasses( pAig, 0, 0 );
        Abc_Print( 1, "Reduction in AIG nodes:%8d  ->%8d.  ", Gia_ManAndNum(pAig), Gia_ManAndNum(pResult) );
        Abc_PrintTime( 1, "Time", clock() - clkTotal );
    }
    return pResult;
}
Gia_Man_t * Ssc_PerformSweepingConstr( Gia_Man_t * p, Ssc_Pars_t * pPars )
{
    Gia_Man_t * pAig, * pCare, * pResult;
    int i;
    if ( p->nConstrs == 0 )
    {
        pAig = Gia_ManDup( p );
        pCare = Gia_ManStart( Gia_ManCiNum(p) + 2 );
        pCare->pName = Abc_UtilStrsav( "care" );
        for ( i = 0; i < Gia_ManCiNum(p); i++ )
            Gia_ManAppendCi( pCare );
        Gia_ManAppendCo( pCare, 0 );
    }
    else
    {
        Vec_Int_t * vOuts = Vec_IntStartNatural( Gia_ManPoNum(p) );
        pAig = Gia_ManDupCones( p, Vec_IntArray(vOuts), Gia_ManPoNum(p) - p->nConstrs, 0 );
        pCare = Gia_ManDupCones( p, Vec_IntArray(vOuts) + Gia_ManPoNum(p) - p->nConstrs, p->nConstrs, 0 );
        Vec_IntFree( vOuts );
    }
    pAig = Gia_ManDupLevelized( pResult = pAig );
    Gia_ManStop( pResult );
    pResult = Ssc_PerformSweeping( pAig, pCare, pPars );
    Gia_ManStop( pAig );
    Gia_ManStop( pCare );
    return pResult;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

