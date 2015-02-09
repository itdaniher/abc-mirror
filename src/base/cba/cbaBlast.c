/**CFile****************************************************************

  FileName    [cbaBlast.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Hierarchical word-level netlist.]

  Synopsis    [Bit-blasting of the netlist.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - November 29, 2014.]

  Revision    [$Id: cbaBlast.c,v 1.00 2014/11/29 00:00:00 alanmi Exp $]

***********************************************************************/

#include "cba.h"
#include "base/abc/abc.h"
#include "map/mio/mio.h"
#include "bool/dec/dec.h"

ABC_NAMESPACE_IMPL_START

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cba_ManPrepareGates( Cba_Man_t * p )
{
    Dec_Graph_t ** ppGraphs; int i;
    if ( p->pMioLib == NULL )
        return;
    ppGraphs = ABC_CALLOC( Dec_Graph_t *, Abc_NamObjNumMax(p->pMods) );
    for ( i = 1; i < Abc_NamObjNumMax(p->pMods); i++ )
    {
        char * pGateName = Abc_NamStr( p->pMods, i );
        Mio_Gate_t * pGate = Mio_LibraryReadGateByName( (Mio_Library_t *)p->pMioLib, pGateName, NULL );
        if ( pGate != NULL )
            ppGraphs[i] = Dec_Factor( Mio_GateReadSop(pGate) );
    }
    assert( p->ppGraphs == NULL );
    p->ppGraphs = (void **)ppGraphs;
}
void Cba_ManUndoGates( Cba_Man_t * p )
{
    int i;
    if ( p->pMioLib == NULL )
        return;
    for ( i = 1; i < Abc_NamObjNumMax(p->pMods); i++ )
        if ( p->ppGraphs[i] )
            Dec_GraphFree( (Dec_Graph_t *)p->ppGraphs[i] );
    ABC_FREE( p->ppGraphs );
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Cba_ManAddBarbuf( Gia_Man_t * pNew, int iRes, Cba_Man_t * p, int iLNtk, int iLObj, int iRNtk, int iRObj, Vec_Int_t * vMap )
{
    int iBufLit, iIdLit;
    if ( iRes == 0 || iRes == 1 )
        return iRes;
    assert( iRes > 0 );
    if ( vMap && Abc_Lit2Var(iRes) < Vec_IntSize(vMap) && (iIdLit = Vec_IntEntry(vMap, Abc_Lit2Var(iRes))) >= 0 && 
        Vec_IntEntry(&p->vBuf2LeafNtk, Abc_Lit2Var(iIdLit)) == iLNtk && Vec_IntEntry(&p->vBuf2RootNtk, Abc_Lit2Var(iIdLit)) == iRNtk )
        return Abc_LitNotCond( Vec_IntEntry(pNew->vBarBufs, Abc_Lit2Var(iIdLit)), Abc_LitIsCompl(iRes) ^ Abc_LitIsCompl(iIdLit) );
    Vec_IntPush( &p->vBuf2LeafNtk, iLNtk );
    Vec_IntPush( &p->vBuf2LeafObj, iLObj );
    Vec_IntPush( &p->vBuf2RootNtk, iRNtk );
    Vec_IntPush( &p->vBuf2RootObj, iRObj );
    iBufLit = Gia_ManAppendBuf( pNew, iRes );
    if ( vMap )
    {
        Vec_IntSetEntryFull( vMap, Abc_Lit2Var(iRes), Abc_Var2Lit(Vec_IntSize(pNew->vBarBufs), Abc_LitIsCompl(iRes)) );
        Vec_IntPush( pNew->vBarBufs, iBufLit );
    }
    return iBufLit;
}
int Cba_ManExtract_rec( Gia_Man_t * pNew, Cba_Ntk_t * p, int i, int fBuffers, Vec_Int_t * vMap )
{
    int iRes = Cba_ObjCopy( p, i );
    if ( iRes >= 0 )
        return iRes;
    if ( Cba_ObjIsCo(p, i) )
        iRes = Cba_ManExtract_rec( pNew, p, Cba_ObjFanin(p, i), fBuffers, vMap );
    else if ( Cba_ObjIsPi(p, i) )
    {
        Cba_Ntk_t * pHost = Cba_NtkHostNtk( p );
        int iObj = Cba_BoxBi( pHost, Cba_NtkHostObj(p), Cba_ObjIndex(p, i) );
        iRes = Cba_ManExtract_rec( pNew, pHost, iObj, fBuffers, vMap );
        if ( fBuffers )
            iRes = Cba_ManAddBarbuf( pNew, iRes, p->pDesign, Cba_NtkId(p), i, Cba_NtkId(pHost), iObj, vMap );
    }
    else if ( Cba_ObjIsBo(p, i) )
    {
        int iBox = Cba_BoxBoBox(p, i);
        if ( Cba_ObjIsBoxUser(p, iBox) ) // user box
        {
            Cba_Ntk_t * pBox = Cba_BoxBoNtk( p, i );
            int iObj = Cba_NtkPo( pBox, Cba_ObjIndex(p, i) );
            iRes = Cba_ManExtract_rec( pNew, pBox, iObj, fBuffers, vMap );
            if ( fBuffers )
                iRes = Cba_ManAddBarbuf( pNew, iRes, p->pDesign, Cba_NtkId(p), i, Cba_NtkId(pBox), iObj, vMap );
        }
        else // primitive
        {
            int iFanin, nLits, pLits[16];
            assert( Cba_ObjIsBoxPrim(p, iBox) );
            Cba_BoxForEachFanin( p, iBox, iFanin, nLits )
                pLits[nLits] = Cba_ManExtract_rec( pNew, p, iFanin, fBuffers, vMap );
            assert( nLits <= 16 );
            if ( p->pDesign->ppGraphs ) // mapped gate
            {
                extern int Gia_ManFactorGraph( Gia_Man_t * p, Dec_Graph_t * pFForm, Vec_Int_t * vLeaves );
                Dec_Graph_t * pGraph = (Dec_Graph_t *)p->pDesign->ppGraphs[Cba_BoxNtkId(p, iBox)];
                Vec_Int_t Leaves = { nLits, nLits, pLits };
                assert( pGraph != NULL );
                return Gia_ManFactorGraph( pNew, pGraph, &Leaves );
            }
            else
            {
                Cba_ObjType_t Type = Cba_ObjType(p, iBox);
                if ( nLits == 0 )
                {
                    if ( Type == CBA_BOX_C0 )
                        iRes = 0;
                    else if ( Type == CBA_BOX_C1 )
                        iRes = 1;
                    else assert( 0 );
                }
                else if ( nLits == 1 )
                {
                    if ( Type == CBA_BOX_BUF )
                        iRes = pLits[0];
                    else if ( Type == CBA_BOX_INV )
                        iRes = Abc_LitNot( pLits[0] );
                    else assert( 0 );
                }
                else
                {
                    assert( nLits == 2 );
                    if ( Type == CBA_BOX_AND )
                        iRes = Gia_ManHashAnd( pNew, pLits[0], pLits[1] );
                    else if ( Type == CBA_BOX_OR )
                        iRes = Gia_ManHashOr( pNew, pLits[0], pLits[1] );
                    else if ( Type == CBA_BOX_NOR )
                        iRes = Abc_LitNot( Gia_ManHashOr( pNew, pLits[0], pLits[1] ) );
                    else if ( Type == CBA_BOX_XOR )
                        iRes = Gia_ManHashXor( pNew, pLits[0], pLits[1] );
                    else if ( Type == CBA_BOX_XNOR )
                        iRes = Abc_LitNot( Gia_ManHashXor( pNew, pLits[0], pLits[1] ) );
                    else if ( Type == CBA_BOX_SHARP )
                        iRes = Gia_ManHashAnd( pNew, pLits[0], Abc_LitNot(pLits[1]) );
                    else assert( 0 );
                }
                //printf("%d input\n", nLits );
            }
        }
    }
    else assert( 0 );
    Cba_ObjSetCopy( p, i, iRes );
    return iRes;
}
Gia_Man_t * Cba_ManExtract( Cba_Man_t * p, int fBuffers, int fVerbose )
{
    Cba_Ntk_t * pNtk, * pRoot = Cba_ManRoot(p);
    Gia_Man_t * pNew, * pTemp; 
    Vec_Int_t * vMap = NULL;
    int i, iObj;

    Vec_IntClear( &p->vBuf2LeafNtk );
    Vec_IntClear( &p->vBuf2LeafObj );
    Vec_IntClear( &p->vBuf2RootNtk );
    Vec_IntClear( &p->vBuf2RootObj );

    Cba_ManForEachNtk( p, pNtk, i )
    {
        Cba_NtkDeriveIndex( pNtk );
        Cba_NtkStartCopies( pNtk );
    }

    // start the manager
    pNew = Gia_ManStart( Cba_ManNodeNum(p) );
    pNew->pName = Abc_UtilStrsav(p->pName);
    pNew->pSpec = Abc_UtilStrsav(p->pSpec);

    // primary inputs
    Cba_NtkForEachPi( pRoot, iObj, i )
        Cba_ObjSetCopy( pRoot, iObj, Gia_ManAppendCi(pNew) );

    // internal nodes
    Gia_ManHashAlloc( pNew );
    pNew->vBarBufs = Vec_IntAlloc( 10000 );
    vMap = Vec_IntStartFull( 10000 );
    Cba_ManPrepareGates( p );
    Cba_NtkForEachPo( pRoot, iObj, i )
        Cba_ManExtract_rec( pNew, pRoot, iObj, fBuffers, vMap );
    Cba_ManUndoGates( p );
    Vec_IntFreeP( &vMap );
    Gia_ManHashStop( pNew );

    // primary outputs
    Cba_NtkForEachPo( pRoot, iObj, i )
        Gia_ManAppendCo( pNew, Cba_ObjCopy(pRoot, iObj) );
    assert( Vec_IntSize(&p->vBuf2LeafNtk) == pNew->nBufs );

    // cleanup
    pNew = Gia_ManCleanup( pTemp = pNew );
    Gia_ManStop( pTemp );
    //Gia_ManPrintStats( pNew, NULL );
    return pNew;
}

/**Function*************************************************************

  Synopsis    [Mark each GIA node with the network it belongs to.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cba_ManMarkNodesGia( Cba_Man_t * p, Gia_Man_t * pGia )
{
    Gia_Obj_t * pObj; int i, Count = 0;
    assert( Vec_IntSize(&p->vBuf2LeafNtk) == Gia_ManBufNum(pGia) );
    Gia_ManConst0(pGia)->Value = 0;
    Gia_ManForEachPi( pGia, pObj, i )
        pObj->Value = 0;
    Gia_ManForEachAnd( pGia, pObj, i )
    {
        if ( Gia_ObjIsBuf(pObj) )
            pObj->Value = Vec_IntEntry( &p->vBuf2LeafNtk, Count++ );
        else
        {
            pObj->Value = Gia_ObjFanin0(pObj)->Value;
            assert( pObj->Value == Gia_ObjFanin1(pObj)->Value );
        }
    }
    assert( Count == Gia_ManBufNum(pGia) );
    Gia_ManForEachPo( pGia, pObj, i )
    {
        assert( Gia_ObjFanin0(pObj)->Value == 0 );
        pObj->Value = 0;
    }
}
void Cba_ManRemapBarbufs( Cba_Man_t * pNew, Cba_Man_t * p )
{
    Cba_Ntk_t * pNtk;  int Entry, i;
    assert( Vec_IntSize(&p->vBuf2RootNtk) );
    assert( !Vec_IntSize(&pNew->vBuf2RootNtk) );
    Vec_IntAppend( &pNew->vBuf2RootNtk, &p->vBuf2RootNtk );
    Vec_IntAppend( &pNew->vBuf2RootObj, &p->vBuf2RootObj );
    Vec_IntAppend( &pNew->vBuf2LeafNtk, &p->vBuf2LeafNtk );
    Vec_IntAppend( &pNew->vBuf2LeafObj, &p->vBuf2LeafObj );
    Vec_IntForEachEntry( &p->vBuf2LeafObj, Entry, i )
    {
        pNtk = Cba_ManNtk( p, Vec_IntEntry(&p->vBuf2LeafNtk, i) );
        Vec_IntWriteEntry( &pNew->vBuf2LeafObj, i, Cba_ObjCopy(pNtk, Entry) );
    }
    Vec_IntForEachEntry( &p->vBuf2RootObj, Entry, i )
    {
        pNtk = Cba_ManNtk( p, Vec_IntEntry(&p->vBuf2RootNtk, i) );
        Vec_IntWriteEntry( &pNew->vBuf2RootObj, i, Cba_ObjCopy(pNtk, Entry) );
    }
}
void Cba_NtkCreateAndConnectBuffer( Gia_Man_t * pGia, Gia_Obj_t * pObj, Cba_Ntk_t * p, int iTerm )
{
    int iObj;
    if ( pGia && Gia_ObjFaninId0p(pGia, pObj) > 0 )
    {
        assert( Cba_ObjName(p, Gia_ObjFanin0(pObj)->Value) != Cba_ObjName(p, iTerm) ); // not a feedthrough
        iObj = Cba_ObjAlloc( p, CBA_OBJ_BI, Gia_ObjFanin0(pObj)->Value );
        Cba_ObjSetName( p, iObj, Cba_ObjName(p, Gia_ObjFanin0(pObj)->Value) );
        Cba_ObjAlloc( p, Gia_ObjFaninC0(pObj) ? CBA_BOX_INV : CBA_BOX_BUF, -1 );
    }
    else
    {
        Cba_ObjAlloc( p, pGia && Gia_ObjFaninC0(pObj) ? CBA_BOX_C1 : CBA_BOX_C0, -1 );
    }
    iObj = Cba_ObjAlloc( p, CBA_OBJ_BO, -1 );
    Cba_ObjSetName( p, iObj, Cba_ObjName(p, iTerm) );
    Cba_ObjSetFanin( p, iTerm, iObj );
}
void Cba_NtkInsertGia( Cba_Man_t * p, Gia_Man_t * pGia )
{
    Cba_Ntk_t * pNtk, * pRoot = Cba_ManRoot( p );
    int i, j, k, iBox, iTerm, Count = 0;
    Gia_Obj_t * pObj;

    Gia_ManConst0(pGia)->Value = ~0;
    Gia_ManForEachPi( pGia, pObj, i )
        pObj->Value = Cba_NtkPi( pRoot, i );
    Gia_ManForEachAnd( pGia, pObj, i )
    {
        if ( Gia_ObjIsBuf(pObj) )
        {
            pNtk = Cba_ManNtk( p, Vec_IntEntry(&p->vBuf2RootNtk, Count) );
            iTerm = Vec_IntEntry( &p->vBuf2RootObj, Count );
            assert( Cba_ObjIsCo(pNtk, iTerm) );
            if ( Cba_ObjFanin(pNtk, iTerm) == -1 ) // not a feedthrough
                Cba_NtkCreateAndConnectBuffer( pGia, pObj, pNtk, iTerm );
            // prepare leaf
            pObj->Value = Vec_IntEntry( &p->vBuf2LeafObj, Count++ );
        }
        else
        {
            int iLit0 = Gia_ObjFanin0(pObj)->Value;
            int iLit1 = Gia_ObjFanin1(pObj)->Value;
            Cba_ObjType_t Type;
            pNtk = Cba_ManNtk( p, pObj->Value );
            if ( Gia_ObjFaninC0(pObj) && Gia_ObjFaninC1(pObj) )
                Type = CBA_BOX_NOR;
            else if ( Gia_ObjFaninC1(pObj) )
                Type = CBA_BOX_SHARP;
            else if ( Gia_ObjFaninC0(pObj) )
            {
                Type = CBA_BOX_SHARP;
                ABC_SWAP( int, iLit0, iLit1 );
            }
            else 
                Type = CBA_BOX_AND;
            // create box
            iTerm = Cba_ObjAlloc( pNtk, CBA_OBJ_BI, iLit1 );
            Cba_ObjSetName( pNtk, iTerm, Cba_ObjName(pNtk, iLit1) );
            iTerm = Cba_ObjAlloc( pNtk, CBA_OBJ_BI, iLit0 );
            Cba_ObjSetName( pNtk, iTerm, Cba_ObjName(pNtk, iLit0) );
            Cba_ObjAlloc( pNtk, Type, -1 );
            pObj->Value = Cba_ObjAlloc( pNtk, CBA_OBJ_BO, -1 );
        }
    }
    assert( Count == Gia_ManBufNum(pGia) );

    // create constant 0 drivers for COs without barbufs
    Cba_ManForEachNtk( p, pNtk, i )
    {
        Cba_NtkForEachBox( pNtk, iBox )
            Cba_BoxForEachBi( pNtk, iBox, iTerm, j )
                if ( Cba_ObjFanin(pNtk, iTerm) == -1 )
                    Cba_NtkCreateAndConnectBuffer( NULL, NULL, pNtk, iTerm );
        Cba_NtkForEachPo( pNtk, iTerm, k )
            if ( pNtk != pRoot && Cba_ObjFanin(pNtk, iTerm) == -1 )
                Cba_NtkCreateAndConnectBuffer( NULL, NULL, pNtk, iTerm );
    }
    // create node and connect POs
    Gia_ManForEachPo( pGia, pObj, i )
        if ( Cba_ObjFanin(pRoot, Cba_NtkPo(pRoot, i)) == -1 ) // not a feedthrough
            Cba_NtkCreateAndConnectBuffer( pGia, pObj, pRoot, Cba_NtkPo(pRoot, i) );
}
Cba_Man_t * Cba_ManInsertGia( Cba_Man_t * p, Gia_Man_t * pGia )
{
    Cba_Man_t * pNew = Cba_ManDupUserBoxes( p );
    Cba_ManMarkNodesGia( p, pGia );
    Cba_ManRemapBarbufs( pNew, p );
    Cba_NtkInsertGia( pNew, pGia );
    return pNew;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Cba_Man_t * Cba_ManBlastTest( Cba_Man_t * p )
{
    Gia_Man_t * pGia = Cba_ManExtract( p, 1, 0 );
    Cba_Man_t * pNew = Cba_ManInsertGia( p, pGia );
    Gia_ManStop( pGia );
    return pNew;
}

/**Function*************************************************************

  Synopsis    [Mark each GIA node with the network it belongs to.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_NodeIsSeriousGate( Abc_Obj_t * p )
{
    return Abc_ObjIsNode(p) && Abc_ObjFaninNum(p) > 0 && !Abc_ObjIsBarBuf(p);
}
void Cba_ManMarkNodesAbc( Cba_Man_t * p, Abc_Ntk_t * pNtk )
{
    Abc_Obj_t * pObj, * pFanin; int i, k, Count = 0;
    assert( Vec_IntSize(&p->vBuf2LeafNtk) == pNtk->nBarBufs2 );
    Abc_NtkForEachPi( pNtk, pObj, i )
        pObj->iTemp = 0;
    Abc_NtkForEachNode( pNtk, pObj, i )
    {
        if ( Abc_ObjIsBarBuf(pObj) )
            pObj->iTemp = Vec_IntEntry( &p->vBuf2LeafNtk, Count++ );
        else if ( Abc_NodeIsSeriousGate(pObj) )
        {
            pObj->iTemp = Abc_ObjFanin0(pObj)->iTemp;
            Abc_ObjForEachFanin( pObj, pFanin, k )
                assert( pObj->iTemp == pFanin->iTemp );
        }
    }
    Abc_NtkForEachPo( pNtk, pObj, i )
    {
        if ( !Abc_NodeIsSeriousGate(Abc_ObjFanin0(pObj)) )
            continue;
        assert( Abc_ObjFanin0(pObj)->iTemp == 0 );
        pObj->iTemp = Abc_ObjFanin0(pObj)->iTemp;
    }
    assert( Count == pNtk->nBarBufs2 );
}
void Cba_NtkCreateOrConnectFanin( Abc_Obj_t * pFanin, Cba_Ntk_t * p, int iTerm )
{
    int iObj;
    if ( pFanin && Abc_NodeIsSeriousGate(pFanin) && Cba_ObjName(p, pFanin->iTemp) == -1 ) // gate without name
    {
        iObj = pFanin->iTemp;
    }
    else if ( pFanin && (Abc_ObjIsPi(pFanin) || Abc_ObjIsBarBuf(pFanin) || Abc_NodeIsSeriousGate(pFanin)) ) // PI/BO or gate with name
    {
        assert( Cba_ObjName(p, pFanin->iTemp) != Cba_ObjName(p, iTerm) ); // not a feedthrough
        iObj = Cba_ObjAlloc( p, CBA_OBJ_BI, pFanin->iTemp );
        Cba_ObjSetName( p, iObj, Cba_ObjName(p, pFanin->iTemp) );
        Cba_ObjAlloc( p, CBA_BOX_GATE, p->pDesign->ElemGates[2] ); // buffer
        iObj = Cba_ObjAlloc( p, CBA_OBJ_BO, -1 );
    }
    else
    {
        assert( !pFanin || Abc_NodeIsConst0(pFanin) || Abc_NodeIsConst1(pFanin) );
        Cba_ObjAlloc( p, CBA_BOX_GATE, p->pDesign->ElemGates[(pFanin && Abc_NodeIsConst1(pFanin))] ); // const 0/1
        iObj = Cba_ObjAlloc( p, CBA_OBJ_BO, -1 );
    }
    Cba_ObjSetName( p, iObj, Cba_ObjName(p, iTerm) );
    Cba_ObjSetFanin( p, iTerm, iObj );
}
void Cba_NtkPrepareLibrary( Cba_Man_t * p, Mio_Library_t * pLib )
{
    Mio_Gate_t * pGate;
    Mio_Gate_t * pGate0 = Mio_LibraryReadConst0( pLib );
    Mio_Gate_t * pGate1 = Mio_LibraryReadConst1( pLib );
    Mio_Gate_t * pGate2 = Mio_LibraryReadBuf( pLib );
    if ( !pGate0 || !pGate1 || !pGate2 )
    {
        printf( "The library does not have one of the elementary gates.\n" );
        return;
    }
    p->ElemGates[0] = Abc_NamStrFindOrAdd( p->pMods, Mio_GateReadName(pGate0), NULL );
    p->ElemGates[1] = Abc_NamStrFindOrAdd( p->pMods, Mio_GateReadName(pGate1), NULL );
    p->ElemGates[2] = Abc_NamStrFindOrAdd( p->pMods, Mio_GateReadName(pGate2), NULL );
    Mio_LibraryForEachGate( pLib, pGate )
        if ( pGate != pGate0 && pGate != pGate1 && pGate != pGate2 )
            Abc_NamStrFindOrAdd( p->pMods, Mio_GateReadName(pGate), NULL );
    assert( Abc_NamObjNumMax(p->pMods) > 1 );
}
void Cba_NtkInsertNtk( Cba_Man_t * p, Abc_Ntk_t * pNtk )
{
    Cba_Ntk_t * pCbaNtk, * pRoot = Cba_ManRoot( p );
    int i, j, k, iBox, iTerm, Count = 0;
    Abc_Obj_t * pObj;
    assert( Abc_NtkHasMapping(pNtk) );
    Cba_NtkPrepareLibrary( p, (Mio_Library_t *)pNtk->pManFunc );
    p->pMioLib = pNtk->pManFunc;

    Abc_NtkForEachPi( pNtk, pObj, i )
        pObj->iTemp = Cba_NtkPi( pRoot, i );
    Abc_NtkForEachNode( pNtk, pObj, i )
    {
        if ( Abc_ObjIsBarBuf(pObj) )
        {
            pCbaNtk = Cba_ManNtk( p, Vec_IntEntry(&p->vBuf2RootNtk, Count) );
            iTerm = Vec_IntEntry( &p->vBuf2RootObj, Count );
            assert( Cba_ObjIsCo(pCbaNtk, iTerm) );
            if ( Cba_ObjFanin(pCbaNtk, iTerm) == -1 ) // not a feedthrough
                Cba_NtkCreateOrConnectFanin( Abc_ObjFanin0(pObj), pCbaNtk, iTerm );
            // prepare leaf
            pObj->iTemp = Vec_IntEntry( &p->vBuf2LeafObj, Count++ );
        }
        else if ( Abc_NodeIsSeriousGate(pObj) )
        {
            pCbaNtk = Cba_ManNtk( p, pObj->iTemp );
            for ( k = Abc_ObjFaninNum(pObj)-1; k >= 0; k-- )
            {
                iTerm = Cba_ObjAlloc( pCbaNtk, CBA_OBJ_BI, Abc_ObjFanin(pObj, k)->iTemp );
                Cba_ObjSetName( pCbaNtk, iTerm, Cba_ObjName(pCbaNtk, Abc_ObjFanin(pObj, k)->iTemp) );
            }
            Cba_ObjAlloc( pCbaNtk, CBA_BOX_GATE, Abc_NamStrFind(p->pMods, Mio_GateReadName((Mio_Gate_t *)pObj->pData)) );
            pObj->iTemp = Cba_ObjAlloc( pCbaNtk, CBA_OBJ_BO, -1 );
        }
    }
    assert( Count == pNtk->nBarBufs2 );

    // create constant 0 drivers for COs without barbufs
    Cba_ManForEachNtk( p, pCbaNtk, i )
    {
        Cba_NtkForEachBox( pCbaNtk, iBox )
            Cba_BoxForEachBi( pCbaNtk, iBox, iTerm, j )
                if ( Cba_ObjFanin(pCbaNtk, iTerm) == -1 )
                    Cba_NtkCreateOrConnectFanin( NULL, pCbaNtk, iTerm );
        Cba_NtkForEachPo( pCbaNtk, iTerm, k )
            if ( pCbaNtk != pRoot && Cba_ObjFanin(pCbaNtk, iTerm) == -1 )
                Cba_NtkCreateOrConnectFanin( NULL, pCbaNtk, iTerm );
    }
    // create node and connect POs
    Abc_NtkForEachPo( pNtk, pObj, i )
        if ( Cba_ObjFanin(pRoot, Cba_NtkPo(pRoot, i)) == -1 ) // not a feedthrough
            Cba_NtkCreateOrConnectFanin( Abc_ObjFanin0(pObj), pRoot, Cba_NtkPo(pRoot, i) );
}
void * Cba_ManInsertAbc( Cba_Man_t * p, void * pAbc )
{
    Abc_Ntk_t * pNtk = (Abc_Ntk_t *)pAbc;
    Cba_Man_t * pNew = Cba_ManDupUserBoxes( p );
    Cba_ManMarkNodesAbc( p, pNtk );
    Cba_ManRemapBarbufs( pNew, p );
    Cba_NtkInsertNtk( pNew, pNtk );
    return pNew;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

