/**CFile****************************************************************

  FileName    [giaMan.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Scalable AIG package.]

  Synopsis    [Package manager.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: giaMan.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "gia.h"
#include "misc/tim/tim.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Creates AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t * Gia_ManStart( int nObjsMax )  
{ 
    Gia_Man_t * p;
    assert( nObjsMax > 0 );
    p = ABC_CALLOC( Gia_Man_t, 1 );
    p->nObjsAlloc = nObjsMax;
    p->pObjs = ABC_CALLOC( Gia_Obj_t, nObjsMax );
    p->pObjs->iDiff0 = p->pObjs->iDiff1 = GIA_NONE;
    p->nObjs = 1;
    p->vCis  = Vec_IntAlloc( nObjsMax / 20 );
    p->vCos  = Vec_IntAlloc( nObjsMax / 20 );
    return p;
}

/**Function*************************************************************

  Synopsis    [Deletes AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManStop( Gia_Man_t * p )  
{
    Gia_ManStaticFanoutStop( p );
    Tim_ManStopP( (Tim_Man_t **)&p->pManTime );
    assert( p->pManTime == NULL );
    Vec_PtrFreeFree( p->vNamesIn );
    Vec_PtrFreeFree( p->vNamesOut );
    Vec_FltFreeP( &p->vTiming );
    Vec_VecFreeP( &p->vClockDoms );
    Vec_IntFreeP( &p->vLutConfigs );
    Vec_IntFreeP( &p->vUserPiIds );
    Vec_IntFreeP( &p->vUserPoIds );
    Vec_IntFreeP( &p->vUserFfIds );
    Vec_IntFreeP( &p->vFlopClasses );
    Vec_IntFreeP( &p->vGateClasses );
    Vec_IntFreeP( &p->vObjClasses );
    Vec_IntFreeP( &p->vLevels );
    Vec_IntFreeP( &p->vTruths );
    Vec_StrFreeP( &p->vTtNums );
    Vec_IntFreeP( &p->vTtNodes );
    Vec_WrdFreeP( &p->vTtMemory );
    Vec_PtrFreeP( &p->vTtInputs );
    Vec_IntFree( p->vCis );
    Vec_IntFree( p->vCos );
    ABC_FREE( p->pTravIds );
    ABC_FREE( p->pPlacement );
    ABC_FREE( p->pSwitching );
    ABC_FREE( p->pCexSeq );
    ABC_FREE( p->pCexComb );
    ABC_FREE( p->pIso );
    ABC_FREE( p->pMapping );
    ABC_FREE( p->pFanData );
    ABC_FREE( p->pReprsOld );
    ABC_FREE( p->pReprs );
    ABC_FREE( p->pNexts );
    ABC_FREE( p->pName );
    ABC_FREE( p->pRefs );
    ABC_FREE( p->pNodeRefs );
    ABC_FREE( p->pHTable );
    ABC_FREE( p->pObjs );
    ABC_FREE( p );
}

/**Function*************************************************************

  Synopsis    [Stops the AIG manager.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManStopP( Gia_Man_t ** p )
{
    if ( *p == NULL )
        return;
    Gia_ManStop( *p );
    *p = NULL;
}

/**Function*************************************************************

  Synopsis    [Prints stats for the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintClasses_old( Gia_Man_t * p )
{
    Gia_Obj_t * pObj;
    int i;
    if ( p->vFlopClasses == NULL )
        return;
    Gia_ManForEachRo( p, pObj, i )
        printf( "%d", Vec_IntEntry(p->vFlopClasses, i) );
    printf( "\n" );

    {
        Gia_Man_t * pTemp;
        pTemp = Gia_ManDupFlopClass( p, 1 );
        Gia_WriteAiger( pTemp, "dom1.aig", 0, 0 );
        Gia_ManStop( pTemp );
        pTemp = Gia_ManDupFlopClass( p, 2 );
        Gia_WriteAiger( pTemp, "dom2.aig", 0, 0 );
        Gia_ManStop( pTemp );
    }
}

/**Function*************************************************************

  Synopsis    [Prints stats for the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintFlopClasses( Gia_Man_t * p )
{
    int Counter0, Counter1;
    if ( p->vFlopClasses == NULL )
        return;
    if ( Vec_IntSize(p->vFlopClasses) != Gia_ManRegNum(p) )
    {
        printf( "Gia_ManPrintFlopClasses(): The number of flop map entries differs from the number of flops.\n" );
        return;
    }
    Counter0 = Vec_IntCountEntry( p->vFlopClasses, 0 );
    Counter1 = Vec_IntCountEntry( p->vFlopClasses, 1 );
    printf( "Flop-level abstraction:  Excluded FFs = %d  Included FFs = %d  (%.2f %%) ", 
        Counter0, Counter1, 100.0*Counter1/(Counter0 + Counter1 + 1) );
    if ( Counter0 + Counter1 < Gia_ManRegNum(p) )
        printf( "and there are other FF classes..." );
    printf( "\n" );
}

/**Function*************************************************************

  Synopsis    [Prints stats for the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintGateClasses( Gia_Man_t * p )
{
    Vec_Int_t * vPis, * vPPis, * vFlops, * vNodes;
    int nTotal;
    if ( p->vGateClasses == NULL )
        return;
    if ( Vec_IntSize(p->vGateClasses) != Gia_ManObjNum(p) )
    {
        printf( "Gia_ManPrintGateClasses(): The number of flop map entries differs from the number of flops.\n" );
        return;
    }
    // create additional arrays
    Gia_ManGlaCollect( p, p->vGateClasses, &vPis, &vPPis, &vFlops, &vNodes );
    nTotal = 1 + Vec_IntSize(vFlops) + Vec_IntSize(vNodes);
    printf( "Gate-level abstraction:  PI = %d  PPI = %d  FF = %d (%.2f %%)  AND = %d (%.2f %%)  Obj = %d (%.2f %%)\n", 
        Vec_IntSize(vPis), Vec_IntSize(vPPis), 
        Vec_IntSize(vFlops), 100.0*Vec_IntSize(vFlops)/(Gia_ManRegNum(p)+1), 
        Vec_IntSize(vNodes), 100.0*Vec_IntSize(vNodes)/(Gia_ManAndNum(p)+1), 
        nTotal,              100.0*nTotal             /(Gia_ManRegNum(p)+Gia_ManAndNum(p)+1) );
    Vec_IntFree( vPis );
    Vec_IntFree( vPPis );
    Vec_IntFree( vFlops );
    Vec_IntFree( vNodes );
}

/**Function*************************************************************

  Synopsis    [Prints stats for the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintObjClasses( Gia_Man_t * p )
{
    Vec_Int_t * vSeens;  // objects seen so far
    Vec_Int_t * vAbs = p->vObjClasses;
    int i, k, Entry, iStart, iStop = -1, nFrames;
    int nObjBits, nObjMask, iObj, iFrame, nWords;
    unsigned * pInfo;
	int * pCountAll, * pCountUni;
    if ( vAbs == NULL )
        return;
    nFrames = Vec_IntEntry( vAbs, 0 );
    assert( Vec_IntEntry(vAbs, nFrames+1) == Vec_IntSize(vAbs) );
    pCountAll = ABC_ALLOC( int, nFrames + 1 );
    pCountUni = ABC_ALLOC( int, nFrames + 1 );
    // start storage for seen objects
    nWords = Abc_BitWordNum( nFrames );
    vSeens = Vec_IntStart( Gia_ManObjNum(p) * nWords );
    // get the bitmasks
    nObjBits = Abc_Base2Log( Gia_ManObjNum(p) );
    nObjMask = (1 << nObjBits) - 1;
    assert( Gia_ManObjNum(p) <= nObjMask );
    // print info about frames
    printf( "Frame   Core   F0   F1   F2   F3 ...\n" );
    for ( i = 0; i < nFrames; i++ )
    {
        iStart = Vec_IntEntry( vAbs, i+1 );
        iStop  = Vec_IntEntry( vAbs, i+2 );
        memset( pCountAll, 0, sizeof(int) * (nFrames + 1) );
        memset( pCountUni, 0, sizeof(int) * (nFrames + 1) );
        Vec_IntForEachEntryStartStop( vAbs, Entry, k, iStart, iStop )
        {
            iObj   = (Entry &  nObjMask);
            iFrame = (Entry >> nObjBits);
            pInfo  = (unsigned *)Vec_IntEntryP( vSeens, nWords * iObj );
            if ( Abc_InfoHasBit(pInfo, iFrame) == 0 )
            {
                Abc_InfoSetBit( pInfo, iFrame );
                pCountUni[iFrame+1]++;
                pCountUni[0]++;
            }
            pCountAll[iFrame+1]++;
            pCountAll[0]++;
        }
        assert( pCountAll[0] == (iStop - iStart) );
//        printf( "%5d%5d  ", pCountAll[0], pCountUni[0] ); 
        printf( "%3d :", i );
        printf( "%7d", pCountAll[0] ); 
        if ( i >= 10 )
        {
            for ( k = 0; k < 4; k++ )
                printf( "%5d", pCountAll[k+1] ); 
            printf( "  ..." );
            for ( k = i-4; k <= i; k++ )
                printf( "%5d", pCountAll[k+1] ); 
        }
        else
        {
            for ( k = 0; k <= i; k++ )
                if ( k <= i )
                    printf( "%5d", pCountAll[k+1] ); 
        }
//        for ( k = 0; k < nFrames; k++ )
//            if ( k <= i )
//                printf( "%5d", pCountAll[k+1] ); 
        printf( "\n" );
    }
    assert( iStop == Vec_IntSize(vAbs) );
    Vec_IntFree( vSeens );
    ABC_FREE( pCountAll );
    ABC_FREE( pCountUni );
}

/**Function*************************************************************

  Synopsis    [Prints stats for the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintPlacement( Gia_Man_t * p )
{
    int i, nFixed = 0, nUndef = 0;
    if ( p->pPlacement == NULL )
        return;
    for ( i = 0; i < Gia_ManObjNum(p); i++ )
    {
        nFixed += p->pPlacement[i].fFixed;
        nUndef += p->pPlacement[i].fUndef;
    }
    printf( "Placement:  Objects = %8d.  Fixed = %8d.  Undef = %8d.\n", Gia_ManObjNum(p), nFixed, nUndef );
}


/**Function*************************************************************

  Synopsis    [Duplicates AIG for unrolling.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintTents_rec( Gia_Man_t * p, Gia_Obj_t * pObj, Vec_Int_t * vObjs )  
{
    if ( Gia_ObjIsTravIdCurrent(p, pObj) )
        return;
    Gia_ObjSetTravIdCurrent(p, pObj);
    Vec_IntPush( vObjs, Gia_ObjId(p, pObj) );
    if ( Gia_ObjIsCi(pObj) )
        return;
    Gia_ManPrintTents_rec( p, Gia_ObjFanin0(pObj), vObjs );
    if ( Gia_ObjIsAnd(pObj) )
        Gia_ManPrintTents_rec( p, Gia_ObjFanin1(pObj), vObjs );
}
void Gia_ManPrintTents( Gia_Man_t * p )  
{
    Vec_Int_t * vObjs;
    Gia_Obj_t * pObj;
    int t, i, iObjId, nSizePrev, nSizeCurr;
    assert( Gia_ManPoNum(p) > 0 );
    vObjs = Vec_IntAlloc( 100 );
    // save constant class
    Gia_ManIncrementTravId( p );
    Gia_ObjSetTravIdCurrent( p, Gia_ManConst0(p) );
    Vec_IntPush( vObjs, 0 );
    // create starting root
    nSizePrev = Vec_IntSize(vObjs);
    Gia_ManForEachPo( p, pObj, i )
        Gia_ManPrintTents_rec( p, pObj, vObjs );
    // build tents
    printf( "Tents:  " );
    for ( t = 1; nSizePrev < Vec_IntSize(vObjs); t++ )
    {
        nSizeCurr = Vec_IntSize(vObjs);
        Vec_IntForEachEntryStartStop( vObjs, iObjId, i, nSizePrev, nSizeCurr )
            if ( Gia_ObjIsRo(p, Gia_ManObj(p, iObjId)) )
                Gia_ManPrintTents_rec( p, Gia_ObjRoToRi(p, Gia_ManObj(p, iObjId)), vObjs );
        printf( "%d=%d  ", t, nSizeCurr - nSizePrev );
        nSizePrev = nSizeCurr;
    }
    printf( " Unused=%d\n", Gia_ManObjNum(p) - Vec_IntSize(vObjs) );
    Vec_IntFree( vObjs );
    // the remaining objects are PIs without fanout
//    Gia_ManForEachObj( p, pObj, i )
//        if ( !Gia_ObjIsTravIdCurrent(p, pObj) )
//            Gia_ObjPrint( p, pObj );
}

/**Function*************************************************************

  Synopsis    [Prints stats for the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintStats( Gia_Man_t * p, int fTents, int fSwitch )
{
    if ( p->pName )
        printf( "%-8s : ", p->pName );
    printf( "i/o =%7d/%7d", Gia_ManPiNum(p), Gia_ManPoNum(p) );
    if ( Gia_ManConstrNum(p) )
        printf( "(c=%d)", Gia_ManConstrNum(p) );
    if ( Gia_ManRegNum(p) )
        printf( "  ff =%7d", Gia_ManRegNum(p) );
    printf( "  and =%8d", Gia_ManAndNum(p) );
    printf( "  lev =%5d", Gia_ManLevelNum(p) ); Vec_IntFreeP( &p->vLevels );
    printf( "  cut =%5d", Gia_ManCrossCut(p) );
//    printf( "  mem =%5.2f MB", 1.0*(sizeof(Gia_Obj_t)*p->nObjs + sizeof(int)*(Vec_IntSize(p->vCis) + Vec_IntSize(p->vCos)))/(1<<20) );
    printf( "  mem =%5.2f MB", 1.0*(sizeof(Gia_Obj_t)*p->nObjsAlloc + sizeof(int)*(Vec_IntCap(p->vCis) + Vec_IntCap(p->vCos)))/(1<<20) );
    if ( Gia_ManHasDangling(p) )
        printf( "  ch =%5d", Gia_ManEquivCountClasses(p) );
    if ( fSwitch )
    {
        if ( p->pSwitching )
            printf( "  power =%7.2f", Gia_ManEvaluateSwitching(p) );
        else
            printf( "  power =%7.2f", Gia_ManComputeSwitching(p, 48, 16, 0) );
    }
//    printf( "obj =%5d  ", Gia_ManObjNum(p) );
    printf( "\n" );

//    Gia_ManSatExperiment( p );
    if ( p->pReprs && p->pNexts )
        Gia_ManEquivPrintClasses( p, 0, 0.0 );
    if ( p->pMapping )
        Gia_ManPrintMappingStats( p );
    if ( p->pPlacement )
        Gia_ManPrintPlacement( p );
    // print register classes
    Gia_ManPrintFlopClasses( p );
    Gia_ManPrintGateClasses( p );
    Gia_ManPrintObjClasses( p );
    if ( fTents )
    {
        int k, Entry, Prev = 1;
        Vec_Int_t * vLimit = Vec_IntAlloc( 1000 );
        Gia_Man_t * pNew = Gia_ManUnrollDup( p, vLimit );
        printf( "Tents:  " );
        Vec_IntForEachEntryStart( vLimit, Entry, k, 1 )
            printf( "%d=%d  ", k, Entry-Prev ), Prev = Entry;
        printf( " Unused=%d.", Gia_ManObjNum(p) - Gia_ManObjNum(pNew) );
        printf( "\n" );
        Vec_IntFree( vLimit );
        Gia_ManStop( pNew );
        Gia_ManPrintTents( p );
    }
}

/**Function*************************************************************

  Synopsis    [Prints stats for the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintStatsShort( Gia_Man_t * p )
{
    printf( "i/o =%7d/%7d  ", Gia_ManPiNum(p), Gia_ManPoNum(p) );
    printf( "ff =%7d  ", Gia_ManRegNum(p) );
    printf( "and =%8d  ", Gia_ManAndNum(p) );
    printf( "lev =%5d  ", Gia_ManLevelNum(p) );
//    printf( "mem =%5.2f MB", 12.0*Gia_ManObjNum(p)/(1<<20) );
    printf( "\n" );
}
 
/**Function*************************************************************

  Synopsis    [Prints stats for the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintMiterStatus( Gia_Man_t * p )
{
    Gia_Obj_t * pObj, * pChild;
    int i, nSat = 0, nUnsat = 0, nUndec = 0, iOut = -1;
    Gia_ManForEachPo( p, pObj, i )
    {
        pChild = Gia_ObjChild0(pObj);
        // check if the output is constant 0
        if ( pChild == Gia_ManConst0(p) )
            nUnsat++;
        // check if the output is constant 1
        else if ( pChild == Gia_ManConst1(p) )
        {
            nSat++;
            if ( iOut == -1 )
                iOut = i;
        }
        // check if the output is a primary input
        else if ( Gia_ObjIsPi(p, Gia_Regular(pChild)) )
        {
            nSat++;
            if ( iOut == -1 )
                iOut = i;
        }
/*
        // check if the output is 1 for the 0000 pattern
        else if ( Gia_Regular(pChild)->fPhase != (unsigned)Gia_IsComplement(pChild) )
        {
            nSat++;
            if ( iOut == -1 )
                iOut = i;
        }
*/
        else
            nUndec++;
    }
    printf( "Outputs = %7d.  Unsat = %7d.  Sat = %7d.  Undec = %7d.\n", 
        Gia_ManPoNum(p), nUnsat, nSat, nUndec );
}

/**Function*************************************************************

  Synopsis    [Prints stats for the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSetRegNum( Gia_Man_t * p, int nRegs )
{
    assert( p->nRegs == 0 );
    p->nRegs = nRegs;
}


/**Function*************************************************************

  Synopsis    [Reports the reduction of the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManReportImprovement( Gia_Man_t * p, Gia_Man_t * pNew )
{
    printf( "REG: Beg = %5d. End = %5d. (R =%5.1f %%)  ",
        Gia_ManRegNum(p), Gia_ManRegNum(pNew), 
        Gia_ManRegNum(p)? 100.0*(Gia_ManRegNum(p)-Gia_ManRegNum(pNew))/Gia_ManRegNum(p) : 0.0 );
    printf( "AND: Beg = %6d. End = %6d. (R =%5.1f %%)",
        Gia_ManAndNum(p), Gia_ManAndNum(pNew), 
        Gia_ManAndNum(p)? 100.0*(Gia_ManAndNum(p)-Gia_ManAndNum(pNew))/Gia_ManAndNum(p) : 0.0 );
    printf( "\n" );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

