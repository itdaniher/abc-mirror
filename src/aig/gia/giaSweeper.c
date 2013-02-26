/**CFile****************************************************************

  FileName    [giaSweeper.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Scalable AIG package.]

  Synopsis    [Incremental SAT sweeper.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: giaSweeper.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "gia.h"
#include "sat/bsat/satSolver.h"

ABC_NAMESPACE_IMPL_START

/*

SAT sweeping/equivalence checking requires the following steps:
- Creating probes
  These APIs should be called for all internal points in the logic, which may be used as
      - nodes representing conditions to be used as constraints
      - nodes representing functions to be equivalence checked
      - nodes representing functions needed by the user at the end of SAT sweeping
  Creating new probe using Gia_ManCreateProbe():  int Gia_ManCreateProbe( Gia_Man_t * p, int iLit );
  Find existing probe using Gia_ManFindProbe():   int Gia_ManFindProbe( Gia_Man_t * p, int iLit );
      If probe with this literal (iLit) exists, this procedure increments its reference counter and returns it.
      If probe with this literal does not exist, it creates new probe and sets is reference counter to 1.
  Dereference probe using Gia_ManDerefProbe():    void Gia_ManDerefProbe( Gia_Man_t * p, int ProbeId );
      Dereferences probe with the given ID. If ref counter become 0, recycles the logic cone of the given probe.
      Recycling of probe IDs can be added later.
  Comments:
      - a probe is identified by its 0-based ID, which is returned by above procedures
- Adding/removing conditions on the current path by calling Gia_ManCondPush() and Gia_ManCondPop()
      extern void Gia_ManCondPush( Gia_Man_t * p, int ProbeId );
      extern void Gia_ManCondPop( Gia_Man_t * p );
- Performing equivalence checking by calling Gia_ManCheckConst() and Gia_ManCheckEquiv()
      extern int Gia_ManCheckConst( Gia_Man_t * p, int ProbeId, int Const1 ); // Const1 can be 0 or 1
      extern int Gia_ManCheckEquiv( Gia_Man_t * p, int ProbeId1, int ProbeId2 );
      (resource limits, such as the number of conflicts, will be controllable by dedicated GIA APIs)
- Extracting the resulting AIG to be returned to the user by calling Gia_ManExtractUserLogic()
      extern Gia_Man_t * Gia_ManExtractUserLogic( Gia_Man_t * p, int * ProbeIds, int nProbeIds );

*/

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

typedef struct Swp_Man_t_ Swp_Man_t;
struct Swp_Man_t_
{
    Gia_Man_t *    pGia;
    int            nBTLimit;
    Vec_Int_t *    vProbes;     // probes
    Vec_Int_t *    vProbRefs;   // probe reference counters
    Vec_Int_t *    vLit2Prob;   // mapping of literal into its probe
//    Vec_Int_t *    vFreeProb;   // recycled probe IDs
    Vec_Int_t *    vCondProbes; // conditions as probes
    Vec_Int_t *    vCondLits;   // conditions as literals
    // equivalence checking
    Vec_Int_t *    vId2Lit;     // mapping of Obj IDs into SAT literal
    Vec_Int_t *    vFront;      // temporary frontier
    Vec_Int_t *    vFanins;     // temporary fanins
    sat_solver *   pSat;        // SAT solver 
    int            nSatVars;    // counter of SAT variables
    // statistics
    int            nSatCalls;
    int            nSatCallsSat;
    int            nSatCallsUnsat;
    int            nSatFails;
    int            nSatProofs;
    clock_t        timeSat;
    clock_t        timeSatSat;
    clock_t        timeSatUnsat;
    clock_t        timeSatUndec;
};

static inline int Swp_ManObj2Lit( Gia_Man_t * p, int Id ) 
{ 
    return Vec_IntGetEntry( ((Swp_Man_t *)p->pData)->vId2Lit, Id );
}
static inline void Swp_ManSetObj2Lit( Gia_Man_t * p, int Id, int Lit ) 
{ 
    Vec_IntSetEntry( ((Swp_Man_t *)p->pData)->vId2Lit, Id, Lit );
}
static inline int Swp_ManLit2Lit( Gia_Man_t * p, int Lit ) 
{ 
    return Abc_Lit2LitL( Vec_IntArray(((Swp_Man_t *)p->pData)->vId2Lit), Lit );
}

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////
    
/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline Swp_Man_t * Swp_ManStart( Gia_Man_t * pGia )
{
    Swp_Man_t * p;
    p = ABC_CALLOC( Swp_Man_t, 1 );
    p->pGia        = pGia;
    p->nBTLimit    = 1000;
    p->vProbes     = Vec_IntAlloc( 100 );
    p->vProbRefs   = Vec_IntAlloc( 100 );
    p->vLit2Prob   = Vec_IntStartFull( 10000 );
    p->vCondProbes = Vec_IntAlloc( 100 );
    p->vCondLits   = Vec_IntAlloc( 100 );
    p->vFront      = Vec_IntAlloc( 100 );
    p->vFanins     = Vec_IntAlloc( 100 );
    p->pSat        = sat_solver_new();
    p->nSatVars    = 1;
    Swp_ManSetObj2Lit( pGia, 0, Abc_Var2Lit(p->nSatVars++, 0) );
    pGia->pData    = p;
    return p;
}
static inline void Swp_ManStop( Gia_Man_t * pGia )
{
    Swp_Man_t * p = (Swp_Man_t *)pGia->pData;
    Vec_IntFree( p->vFront );
    Vec_IntFree( p->vFanins );
    Vec_IntFree( p->vProbes );
    Vec_IntFree( p->vProbRefs );
    Vec_IntFree( p->vLit2Prob );
    Vec_IntFree( p->vCondProbes );
    Vec_IntFree( p->vCondLits );
    ABC_FREE( p );
    pGia->pData = NULL;
}
    
/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t * Gia_ManStartSweeper()
{
    Gia_Man_t * pGia;
    pGia = Gia_ManStart( 10000 );
    Gia_ManHashStart( pGia );
    Swp_ManStart( pGia );
    pGia->fSweeper = 1;
    return pGia;
}
void Gia_ManStopSweeper( Gia_Man_t * pGia )
{
    pGia->fSweeper = 0;
    Swp_ManStop( pGia );
    Gia_ManHashStop( pGia );
    Gia_ManStop( pGia );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
// returns literal of the probe
int Gia_ManProbeLit( Gia_Man_t * p, int ProbeId )
{
    Swp_Man_t * pSwp = (Swp_Man_t *)p->pData;
    return Vec_IntEntry( pSwp->vProbes, ProbeId );
}
// create new probe
int Gia_ManCreateProbe( Gia_Man_t * p, int iLit )
{
    Swp_Man_t * pSwp = (Swp_Man_t *)p->pData;
    int ProbeId = Vec_IntSize(pSwp->vProbes);
    Vec_IntPush( pSwp->vProbes, iLit );
    Vec_IntPush( pSwp->vProbRefs, 1 );
    Vec_IntSetEntryFull( pSwp->vLit2Prob, iLit, ProbeId );
    return ProbeId;
}
// if probe with this literal (iLit) exists, this procedure increments its reference counter and returns it.
// if probe with this literal does not exist, it creates new probe and sets is reference counter to 1.
int Gia_ManFindProbe( Gia_Man_t * p, int iLit )
{
    Swp_Man_t * pSwp = (Swp_Man_t *)p->pData;
    if ( iLit >= Vec_IntSize(pSwp->vLit2Prob) || Vec_IntEntry(pSwp->vLit2Prob, iLit) >= 0 )
        return Vec_IntEntry(pSwp->vLit2Prob, iLit);
    return Gia_ManCreateProbe( p, iLit );
}
// dereferences the probe
void Gia_ManDerefProbe( Gia_Man_t * p, int ProbeId )
{
    Swp_Man_t * pSwp = (Swp_Man_t *)p->pData;
    assert( Vec_IntEntry(pSwp->vProbRefs, ProbeId) > 0 );
    Vec_IntAddToEntry( pSwp->vProbRefs, ProbeId, -1 );
    if ( Vec_IntEntry(pSwp->vProbRefs, ProbeId) == 0 )
    {
        int iLit = Gia_ManProbeLit( p, ProbeId );
        Vec_IntWriteEntry( pSwp->vLit2Prob, iLit, -1 );
    }
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCondPush( Gia_Man_t * p, int ProbeId )
{
    Swp_Man_t * pSwp = (Swp_Man_t *)p->pData;
    Vec_IntPush( pSwp->vCondProbes, ProbeId );
    Vec_IntPush( pSwp->vCondLits, Gia_ManProbeLit(p, ProbeId) );
}
int Gia_ManCondPop( Gia_Man_t * p )
{
    Swp_Man_t * pSwp = (Swp_Man_t *)p->pData;
    int ProbId = Vec_IntPop( pSwp->vCondProbes );
    Vec_IntPop( pSwp->vCondLits );
    return ProbId;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManExtract_rec( Gia_Man_t * p, Gia_Obj_t * pObj, Vec_Int_t * vObjIds )
{
    if ( !Gia_ObjIsAnd(pObj) )
        return;
    if ( Gia_ObjIsTravIdCurrent(p, pObj) )
        return;
    Gia_ObjSetTravIdCurrent(p, pObj);
    Gia_ManExtract_rec( p, Gia_ObjFanin0(pObj), vObjIds );
    Gia_ManExtract_rec( p, Gia_ObjFanin1(pObj), vObjIds );
    Vec_IntPush( vObjIds, Gia_ObjId(p, pObj) );
}
Gia_Man_t * Gia_ManExtractUserLogic( Gia_Man_t * p, int * pProbeIds, int nProbeIds )
{
    Gia_Man_t * pNew;
    Gia_Obj_t * pObj;
    Vec_Int_t * vObjIds;
    int i;
    vObjIds = Vec_IntAlloc( 1000 );
    Gia_ManIncrementTravId( p );
    for ( i = 0; i < nProbeIds; i++ )
    {
        pObj = Gia_Lit2Obj( p, Gia_ManProbeLit(p, pProbeIds[i]) );
        Gia_ManExtract_rec( p, Gia_Regular(pObj), vObjIds );
    }
    // create new manager
    pNew = Gia_ManStart( Gia_ManObjNum(p) );
    pNew->pName = Abc_UtilStrsav( p->pName );
    pNew->pSpec = Abc_UtilStrsav( p->pSpec );
    Gia_ManConst0(p)->Value = 0;
    Gia_ManForEachPi( p, pObj, i )
        pObj->Value = Gia_ManAppendCi(pNew);
    Gia_ManForEachObjVec( vObjIds, p, pObj, i )
        pObj->Value = Gia_ManAppendAnd( pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj) );
    Vec_IntFree( vObjIds );
    for ( i = 0; i < nProbeIds; i++ )
    {
        pObj = Gia_Lit2Obj( p, Gia_ManProbeLit(p, pProbeIds[i]) );
        Gia_ManAppendCo( pNew, Gia_ObjFanin0Copy(Gia_Regular(pObj)) ^ Gia_IsComplement(pObj) );
    }
    return pNew;
}


//#if 0

/**Function*************************************************************

  Synopsis    [Addes clauses to the solver.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManAddClausesMux( Gia_Man_t * pGia, Gia_Obj_t * pNode )
{
    Swp_Man_t * p = (Swp_Man_t *)pGia->pData;
    Gia_Obj_t * pNodeI, * pNodeT, * pNodeE;
    int pLits[4], LitF, LitI, LitT, LitE, RetValue;
    assert( !Gia_IsComplement( pNode ) );
    assert( Gia_ObjIsMuxType( pNode ) );
    // get nodes (I = if, T = then, E = else)
    pNodeI = Gia_ObjRecognizeMux( pNode, &pNodeT, &pNodeE );
    // get the Litiable numbers
    LitF = Swp_ManLit2Lit( pGia, Gia_Obj2Lit(pGia,pNode) );
    LitI = Swp_ManLit2Lit( pGia, Gia_Obj2Lit(pGia,pNodeI) );
    LitT = Swp_ManLit2Lit( pGia, Gia_Obj2Lit(pGia,pNodeT) );
    LitE = Swp_ManLit2Lit( pGia, Gia_Obj2Lit(pGia,pNodeE) );

    // f = ITE(i, t, e)

    // i' + t' + f
    // i' + t  + f'
    // i  + e' + f
    // i  + e  + f'

    // create four clauses
    pLits[0] = Abc_LitNotCond(LitI, 1);
    pLits[1] = Abc_LitNotCond(LitT, 1);
    pLits[2] = Abc_LitNotCond(LitF, 0);
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
    pLits[0] = Abc_LitNotCond(LitI, 1);
    pLits[1] = Abc_LitNotCond(LitT, 0);
    pLits[2] = Abc_LitNotCond(LitF, 1);
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
    pLits[0] = Abc_LitNotCond(LitI, 0);
    pLits[1] = Abc_LitNotCond(LitE, 1);
    pLits[2] = Abc_LitNotCond(LitF, 0);
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
    pLits[0] = Abc_LitNotCond(LitI, 0);
    pLits[1] = Abc_LitNotCond(LitE, 0);
    pLits[2] = Abc_LitNotCond(LitF, 1);
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );

    // two additional clauses
    // t' & e' -> f'
    // t  & e  -> f

    // t  + e   + f'
    // t' + e'  + f

    if ( LitT == LitE )
    {
//        assert( fCompT == !fCompE );
        return;
    }

    pLits[0] = Abc_LitNotCond(LitT, 0);
    pLits[1] = Abc_LitNotCond(LitE, 0);
    pLits[2] = Abc_LitNotCond(LitF, 1);
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
    pLits[0] = Abc_LitNotCond(LitT, 1);
    pLits[1] = Abc_LitNotCond(LitE, 1);
    pLits[2] = Abc_LitNotCond(LitF, 0);
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
}

/**Function*************************************************************

  Synopsis    [Addes clauses to the solver.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManAddClausesSuper( Gia_Man_t * pGia, Gia_Obj_t * pNode, Vec_Int_t * vSuper )
{
    Swp_Man_t * p = (Swp_Man_t *)pGia->pData;
    int i, RetValue, Lit, LitNode, pLits[2];
    assert( !Gia_IsComplement(pNode) );
    assert( Gia_ObjIsAnd( pNode ) );
    // suppose AND-gate is A & B = C
    // add !A => !C   or   A + !C
    LitNode = Swp_ManObj2Lit( pGia, Gia_ObjId(pGia, pNode) );
    Vec_IntForEachEntry( vSuper, Lit, i )
    {
        pLits[0] = Abc_LitNot( LitNode );
        pLits[1] = Swp_ManLit2Lit( pGia, Lit );
        Vec_IntWriteEntry( vSuper, i, Abc_LitNot(pLits[1]) );
        RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 2 );
        assert( RetValue );
    }
    // add A & B => C   or   !A + !B + C
    Vec_IntPush( vSuper, LitNode );
    RetValue = sat_solver_addclause( p->pSat, Vec_IntArray(vSuper), Vec_IntArray(vSuper) + Vec_IntSize(vSuper) );
    assert( RetValue );
    (void) RetValue;
}


/**Function*************************************************************

  Synopsis    [Collects the supergate.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCollectSuper_rec( Gia_Man_t * p, Gia_Obj_t * pObj, Vec_Int_t * vSuper )
{
    // stop at complements, shared, PIs, and MUXes
    if ( Gia_IsComplement(pObj) || pObj->fMark0 || Gia_ObjIsCi(pObj) || Gia_ObjIsMuxType(pObj) )
    {
        Vec_IntPushUnique( vSuper, Gia_ObjId(p, pObj) );
        return;
    }
    Gia_ManCollectSuper_rec( p, Gia_ObjChild0(pObj), vSuper );
    Gia_ManCollectSuper_rec( p, Gia_ObjChild1(pObj), vSuper );
}
void Gia_ManCollectSuper( Gia_Man_t * p, Gia_Obj_t * pObj, Vec_Int_t * vSuper )
{
    assert( !Gia_IsComplement(pObj) );
    assert( Gia_ObjIsAnd(pObj) );
    Vec_IntClear( vSuper );
    Gia_ManCollectSuper_rec( p, Gia_ObjChild0(pObj), vSuper );
    Gia_ManCollectSuper_rec( p, Gia_ObjChild1(pObj), vSuper );
}

/**Function*************************************************************

  Synopsis    [Updates the solver clause database.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManObjAddToFrontier( Gia_Man_t * pGia, int Id, Vec_Int_t * vFront )
{
    Swp_Man_t * p = (Swp_Man_t *)pGia->pData;
    Gia_Obj_t * pObj;
    if ( Id == 0 )
        return;
    if ( Swp_ManObj2Lit(pGia, Id) )
        return;
    pObj = Gia_ManObj( pGia, Id );
    Swp_ManSetObj2Lit( pGia, Id, Abc_Var2Lit(p->nSatVars++, pObj->fPhase) );
    sat_solver_setnvars( p->pSat, p->nSatVars + 100 );
    if ( Gia_ObjIsAnd(pObj) )
        Vec_IntPush( vFront, Id );
}
void Gia_ManCnfNodeAddToSolver( Gia_Man_t * p, int NodeId )
{
    Vec_Int_t * vFront, * vFanins;
    Gia_Obj_t * pNode;
    int i, k, Id;
    // quit if CNF is ready
    if ( NodeId == 0 )
        return;
    if ( Swp_ManObj2Lit(p, NodeId) )
        return;
    // start the frontier
    vFront = ((Swp_Man_t *)p->pData)->vFront;
    Vec_IntClear( vFront );
    Gia_ManObjAddToFrontier( p, NodeId, vFront );
    // explore nodes in the frontier
    Gia_ManForEachObjVec( vFront, p, pNode, i )
    {
        vFanins = ((Swp_Man_t *)p->pData)->vFanins;
        // create the supergate
        assert( Swp_ManObj2Lit(p,Gia_ObjId(p, pNode)) );
        if ( Gia_ObjIsMuxType(pNode) )
        {
            Vec_IntClear( vFanins );
            Vec_IntPushUnique( vFanins, Gia_ObjFaninId0p( p, Gia_ObjFanin0(pNode) ) );
            Vec_IntPushUnique( vFanins, Gia_ObjFaninId0p( p, Gia_ObjFanin1(pNode) ) );
            Vec_IntPushUnique( vFanins, Gia_ObjFaninId1p( p, Gia_ObjFanin0(pNode) ) );
            Vec_IntPushUnique( vFanins, Gia_ObjFaninId1p( p, Gia_ObjFanin1(pNode) ) );
            Vec_IntForEachEntry( vFanins, Id, k )
                Gia_ManObjAddToFrontier( p, Id, vFront );
            Gia_ManAddClausesMux( p, pNode );
        }
        else
        {
            Gia_ManCollectSuper( p, pNode, vFanins );
            Vec_IntForEachEntry( vFanins, Id, k )
                Gia_ManObjAddToFrontier( p, Id, vFront );
            Gia_ManAddClausesSuper( p, pNode, vFanins );
        }
        assert( Vec_IntSize(vFanins) > 1 );
    }
}


/**Function*************************************************************

  Synopsis    [Runs equivalence test for the two nodes.]

  Description [Both nodes should be regular and different from each other.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManNodesAreEquiv( Gia_Man_t * pGia, int iOld, int iNew )
{
    Swp_Man_t * p = (Swp_Man_t *)pGia->pData;
    Gia_Obj_t * pOld = Gia_Lit2Obj( pGia, iOld );
    Gia_Obj_t * pNew = Gia_Lit2Obj( pGia, iNew );
    int Lit, RetValue, RetValue1;
    clock_t clk;
    p->nSatCalls++;

    // sanity checks
    assert( pOld != pNew );
    assert( p->pSat != NULL );

    // if the nodes do not have SAT variables, allocate them
    Gia_ManCnfNodeAddToSolver( pGia, Abc_Lit2Var(iOld) );
    Gia_ManCnfNodeAddToSolver( pGia, Abc_Lit2Var(iNew) );
    sat_solver_compress( p->pSat );

    // solve under assumptions
    // A = 1; B = 0     OR     A = 1; B = 1 
    Lit = Swp_ManLit2Lit( pGia, iOld );
    Vec_IntPush( p->vCondLits, Lit );

    Lit = Swp_ManLit2Lit( pGia, iNew );
    Vec_IntPush( p->vCondLits, Abc_LitNot(Lit) );

clk = clock();
    RetValue1 = sat_solver_solve( p->pSat, Vec_IntArray(p->vCondLits), Vec_IntArray(p->vCondLits) + Vec_IntSize(p->vCondLits), 
        (ABC_INT64_T)p->nBTLimit, (ABC_INT64_T)0, (ABC_INT64_T)0, (ABC_INT64_T)0 );
p->timeSat += clock() - clk;
    if ( RetValue1 == l_False )
    {
        int * pLits = Vec_IntArray(p->vCondLits) + Vec_IntSize(p->vCondLits) - 2;
        pLits[0] = Abc_LitNot( pLits[0] );
        pLits[1] = Abc_LitNot( pLits[1] );
        RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 2 );
        assert( RetValue );
p->timeSatUnsat += clock() - clk;
        p->nSatCallsUnsat++;
        Vec_IntShrink( p->vCondLits, Vec_IntSize(p->vCondLits) - 2 );
    }
    else if ( RetValue1 == l_True )
    {
p->timeSatSat += clock() - clk;
        p->nSatCallsSat++;
        Vec_IntShrink( p->vCondLits, Vec_IntSize(p->vCondLits) - 2 );
        return 0;
    }
    else // if ( RetValue1 == l_Undef )
    {
p->timeSatUndec += clock() - clk;
        p->nSatFails++;
        Vec_IntShrink( p->vCondLits, Vec_IntSize(p->vCondLits) - 2 );
        return -1;
    }

    // if the old node was constant 0, we already know the answer
    if ( Gia_ManIsConstLit(iNew) )
    {
        p->nSatProofs++;
        return 1;
    }

    // solve under assumptions
    // A = 0; B = 1     OR     A = 0; B = 0 
    Lit = Swp_ManLit2Lit( pGia, iOld );
    Vec_IntPush( p->vCondLits, Abc_LitNot(Lit) );

    Lit = Swp_ManLit2Lit( pGia, iNew );
    Vec_IntPush( p->vCondLits, Lit );

clk = clock();
    RetValue1 = sat_solver_solve( p->pSat, Vec_IntArray(p->vCondLits), Vec_IntArray(p->vCondLits) + Vec_IntSize(p->vCondLits), 
        (ABC_INT64_T)p->nBTLimit, (ABC_INT64_T)0, (ABC_INT64_T)0, (ABC_INT64_T)0 );
p->timeSat += clock() - clk;
    if ( RetValue1 == l_False )
    {
        int * pLits = Vec_IntArray(p->vCondLits) + Vec_IntSize(p->vCondLits) - 2;
        pLits[0] = Abc_LitNot( pLits[0] );
        pLits[1] = Abc_LitNot( pLits[1] );
        RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 2 );
        assert( RetValue );
p->timeSatUnsat += clock() - clk;
        p->nSatCallsUnsat++;
        Vec_IntShrink( p->vCondLits, Vec_IntSize(p->vCondLits) - 2 );
    }
    else if ( RetValue1 == l_True )
    {
p->timeSatSat += clock() - clk;
        p->nSatCallsSat++;
        Vec_IntShrink( p->vCondLits, Vec_IntSize(p->vCondLits) - 2 );
        return 0;
    }
    else // if ( RetValue1 == l_Undef )
    {
p->timeSatUndec += clock() - clk;
        p->nSatFails++;
        Vec_IntShrink( p->vCondLits, Vec_IntSize(p->vCondLits) - 2 );
        return -1;
    }
    // return SAT proof
    p->nSatProofs++;
    return 1;
}

//#endif

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManCheckConst( Gia_Man_t * p, int ProbeId, int Const1 )
{
    return -1;
}

int Gia_ManCheckEquiv( Gia_Man_t * p, int ProbeId1, int ProbeId2 )
{
    return -1;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

ABC_NAMESPACE_IMPL_END

