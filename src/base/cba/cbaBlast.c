/**CFile****************************************************************

  FileName    [cbaBlast.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Verilog parser.]

  Synopsis    [Parses several flavors of word-level Verilog.]

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
    int i;
    if ( p->pMioLib == NULL )
        return;
    assert( p->ppGraphs == NULL );
    p->ppGraphs = ABC_ALLOC( Dec_Graph_t *, Abc_NamObjNumMax(p->pFuncs) );
    p->ppGraphs[0] = NULL;
    for ( i = 1; i < Abc_NamObjNumMax(p->pFuncs); i++ )
    {
        char * pGateName = Abc_NamStr( p->pFuncs, i );
        Mio_Gate_t * pGate = Mio_LibraryReadGateByName( (Mio_Library_t *)p->pMioLib, pGateName, NULL );
        char * pSop = Mio_GateReadSop( pGate );
        p->ppGraphs[i] = Dec_Factor( pSop );
    }
}
void Cba_ManUndoGates( Cba_Man_t * p )
{
    int i;
    if ( p->pMioLib == NULL )
        return;
    for ( i = 1; i < Abc_NamObjNumMax(p->pFuncs); i++ )
        Dec_GraphFree( p->ppGraphs[i] );
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
        Vec_IntEntry(p->vBuf2LeafNtk, Abc_Lit2Var(iIdLit)) == iLNtk && Vec_IntEntry(p->vBuf2RootNtk, Abc_Lit2Var(iIdLit)) == iRNtk )
        return Abc_LitNotCond( Vec_IntEntry(pNew->vBarBufs, Abc_Lit2Var(iIdLit)), Abc_LitIsCompl(iRes) ^ Abc_LitIsCompl(iIdLit) );
    Vec_IntPush( p->vBuf2LeafNtk, iLNtk );
    Vec_IntPush( p->vBuf2LeafObj, iLObj );
    Vec_IntPush( p->vBuf2RootNtk, iRNtk );
    Vec_IntPush( p->vBuf2RootObj, iRObj );
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
    int iRes = Cba_NtkCopy( p, i );
    if ( iRes >= 0 )
        return iRes;
    if ( Cba_ObjIsCo(p, i) )
        iRes = Cba_ManExtract_rec( pNew, p, Cba_ObjFanin0(p, i), fBuffers, vMap );
    else if ( Cba_ObjIsBo(p, i) )
    {
        Cba_Ntk_t * pBox = Cba_ObjBoModel( p, i );
        int iObj = Cba_NtkPo( pBox, Cba_ObjCioIndex(p, i) );
        iRes = Cba_ManExtract_rec( pNew, pBox, iObj, fBuffers, vMap );
        if ( fBuffers )
            iRes = Cba_ManAddBarbuf( pNew, iRes, p->pDesign, Cba_NtkId(p), i, Cba_NtkId(pBox), iObj, vMap );
    }
    else if ( Cba_ObjIsPi(p, i) )
    {
        Cba_Ntk_t * pHost = Cba_NtkHostNtk( p );
        int iObj = Cba_ObjBoxBi( pHost, Cba_NtkHostObj(p), Cba_ObjCioIndex(p, i) );
        iRes = Cba_ManExtract_rec( pNew, pHost, iObj, fBuffers, vMap );
        if ( fBuffers )
            iRes = Cba_ManAddBarbuf( pNew, iRes, p->pDesign, Cba_NtkId(p), i, Cba_NtkId(pHost), iObj, vMap );
    }
    else if ( Cba_ObjIsNode(p, i) && p->pDesign->ppGraphs )
    {
        extern int Gia_ManFactorGraph( Gia_Man_t * p, Dec_Graph_t * pFForm, Vec_Int_t * vLeaves );
        Dec_Graph_t * pGraph = (Dec_Graph_t *)p->pDesign->ppGraphs[Cba_ObjFuncId(p, i)];
        int k, pLits[32], * pFanins = Cba_ObjFaninArray(p, i);
        Vec_Int_t Leaves = { pFanins[0], pFanins[0], pLits };
        assert( pFanins[0] < 32 );
        for ( k = 0; k < pFanins[0]; k++ )
            pLits[k] = Cba_ManExtract_rec( pNew, p, pFanins[k+1], fBuffers, vMap );
        return Gia_ManFactorGraph( pNew, pGraph, &Leaves );
    }
    else if ( Cba_ObjIsNode(p, i) )
    {
        int * pFanins = Cba_ObjFaninArray(p, i);
        int k, pLits[3], Type = Cba_ObjNodeType(p, i);
        assert( pFanins[0] <= 3 );
        if ( pFanins[0] == 0 )
        {
            if ( Type == CBA_NODE_C0 )
                iRes = 0;
            else if ( Type == CBA_NODE_C1 )
                iRes = 1;
            else assert( 0 );
        }
        else if ( pFanins[0] == 1 )
        {
            if ( Type == CBA_NODE_BUF )
                iRes = Cba_ManExtract_rec( pNew, p, pFanins[1], fBuffers, vMap );
            else if ( Type == CBA_NODE_INV )
                iRes = Abc_LitNot( Cba_ManExtract_rec( pNew, p, pFanins[1], fBuffers, vMap ) );
            else assert( 0 );
        }
        else
        {
            for ( k = 0; k < pFanins[0]; k++ )
                pLits[k] = Cba_ManExtract_rec( pNew, p, pFanins[k+1], fBuffers, vMap );
            if ( Type == CBA_NODE_AND )
                iRes = Gia_ManHashAnd( pNew, pLits[0], pLits[1] );
            else if ( Type == CBA_NODE_OR )
                iRes = Gia_ManHashOr( pNew, pLits[0], pLits[1] );
            else if ( Type == CBA_NODE_NOR )
                iRes = Abc_LitNot( Gia_ManHashOr( pNew, pLits[0], pLits[1] ) );
            else if ( Type == CBA_NODE_XOR )
                iRes = Gia_ManHashXor( pNew, pLits[0], pLits[1] );
            else if ( Type == CBA_NODE_XNOR )
                iRes = Abc_LitNot( Gia_ManHashXor( pNew, pLits[0], pLits[1] ) );
            else if ( Type == CBA_NODE_SHARP )
                iRes = Gia_ManHashAnd( pNew, pLits[0], Abc_LitNot(pLits[1]) );
            else assert( 0 );
        }
    }
    else assert( 0 );
    Cba_NtkSetCopy( p, i, iRes );
    return iRes;
}
Gia_Man_t * Cba_ManExtract( Cba_Man_t * p, int fBuffers, int fVerbose )
{
    Cba_Ntk_t * pRoot = Cba_ManRoot(p);
    Gia_Man_t * pNew, * pTemp; 
    Vec_Int_t * vMap = NULL;
    int i, iObj;
    Vec_IntFreeP( &p->vBuf2LeafNtk );
    Vec_IntFreeP( &p->vBuf2LeafObj );
    Vec_IntFreeP( &p->vBuf2RootNtk );
    Vec_IntFreeP( &p->vBuf2RootObj );
    p->vBuf2LeafNtk = Vec_IntAlloc( 1000 );
    p->vBuf2LeafObj = Vec_IntAlloc( 1000 );
    p->vBuf2RootNtk = Vec_IntAlloc( 1000 );
    p->vBuf2RootObj = Vec_IntAlloc( 1000 );

    // start the manager
    pNew = Gia_ManStart( Cba_ManObjNum(p) );
    pNew->pName = Abc_UtilStrsav(p->pName);
    pNew->pSpec = Abc_UtilStrsav(p->pSpec);
    Vec_IntFill( &p->vCopies, Cba_ManObjNum(p), -1 );

    // primary inputs
    Cba_NtkForEachPi( pRoot, iObj, i )
        Cba_NtkSetCopy( pRoot, iObj, Gia_ManAppendCi(pNew) );

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
        Gia_ManAppendCo( pNew, Cba_NtkCopy(pRoot, iObj) );
    assert( Vec_IntSize(p->vBuf2LeafNtk) == pNew->nBufs );

    // cleanup
    pNew = Gia_ManCleanup( pTemp = pNew );
    Gia_ManStop( pTemp );
    //Gia_ManPrintStats( pNew, NULL );
    return pNew;
}

/**Function*************************************************************

  Synopsis    [Returns the number of objects in each network.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Cba_ManCountGia( Cba_Man_t * p, Gia_Man_t * pGia, int fAlwaysAdd )
{
    Cba_Ntk_t * pNtk;
    Gia_Obj_t * pObj; 
    int i, iBox, Count = 0;
    Vec_Int_t * vNtkSizes  = Vec_IntStart( Cba_ManNtkNum(p) + 1 );
    Vec_Int_t * vDrivenCos = Vec_IntStart( Cba_ManNtkNum(p) + 1 );
    assert( Vec_IntSize(p->vBuf2LeafNtk) == Gia_ManBufNum(pGia) );
    // assing for each GIA node, the network it belongs to and count nodes for all networks
    Gia_ManConst0(pGia)->Value = 1;
    Gia_ManForEachPi( pGia, pObj, i )
        pObj->Value = 1;
    Gia_ManForEachAnd( pGia, pObj, i )
    {
        if ( Gia_ObjIsBuf(pObj) )
        {
            if ( Gia_ObjIsAnd(Gia_ObjFanin0(pObj)) )
                Vec_IntAddToEntry( vDrivenCos, Gia_ObjFanin0(pObj)->Value, 1 );
            pObj->Value = Vec_IntEntry( p->vBuf2LeafNtk, Count++ );
        }
        else
        {
            pObj->Value = Gia_ObjFanin0(pObj)->Value;
            assert( pObj->Value == Gia_ObjFanin1(pObj)->Value );
            Vec_IntAddToEntry( vNtkSizes, pObj->Value, 1 );
        }
    }
    assert( Count == Gia_ManBufNum(pGia) );
    Gia_ManForEachPo( pGia, pObj, i )
    {
        assert( Gia_ObjFanin0(pObj)->Value == 1 );
        pObj->Value = Gia_ObjFanin0(pObj)->Value;
        if ( Gia_ObjIsAnd(Gia_ObjFanin0(pObj)) )
            Vec_IntAddToEntry( vDrivenCos, pObj->Value, 1 );
    }
    // for each network, count the total number of COs
    Cba_ManForEachNtk( p, pNtk, i )
    {
        Count = Cba_NtkPiNum(pNtk) + 2 * Cba_NtkPoNum(pNtk) - (fAlwaysAdd ? 0 : Vec_IntEntry(vDrivenCos, i));
        Cba_NtkForEachBox( pNtk, iBox )
            Count += Cba_ObjBoxSize(pNtk, iBox) + Cba_ObjBoxBiNum(pNtk, iBox);
        Vec_IntAddToEntry( vNtkSizes, i, Count );
    }
    Vec_IntFree( vDrivenCos );
    return vNtkSizes;
}
void Cba_ManRemapBarbufs( Cba_Man_t * pNew, Cba_Man_t * p )
{
    Cba_Ntk_t * pNtk;  int Entry, i;
    assert( p->vBuf2RootNtk != NULL );
    assert( pNew->vBuf2RootNtk == NULL );
    pNew->vBuf2RootNtk = Vec_IntDup( p->vBuf2RootNtk );
    pNew->vBuf2RootObj = Vec_IntDup( p->vBuf2RootObj );
    pNew->vBuf2LeafNtk = Vec_IntDup( p->vBuf2LeafNtk );
    pNew->vBuf2LeafObj = Vec_IntDup( p->vBuf2LeafObj );
    Vec_IntForEachEntry( p->vBuf2LeafObj, Entry, i )
    {
        pNtk = Cba_ManNtk( p, Vec_IntEntry(p->vBuf2LeafNtk, i) );
        Vec_IntWriteEntry( pNew->vBuf2LeafObj, i, Cba_NtkCopy(pNtk, Entry) );
    }
    Vec_IntForEachEntry( p->vBuf2RootObj, Entry, i )
    {
        pNtk = Cba_ManNtk( p, Vec_IntEntry(p->vBuf2RootNtk, i) );
        Vec_IntWriteEntry( pNew->vBuf2RootObj, i, Cba_NtkCopy(pNtk, Entry) );
    }
}
void Cba_NtkCreateAndConnectBuffer( Gia_Man_t * pGia, Gia_Obj_t * pObj, Cba_Ntk_t * p, int iTerm )
{
    Vec_IntWriteEntry( &p->vTypes, p->nObjs, CBA_OBJ_NODE );
    if ( pGia && Gia_ObjFaninId0p(pGia, pObj) > 0 )
    {
        Vec_IntWriteEntry( &p->vFuncs,  p->nObjs, Gia_ObjFaninC0(pObj) ? CBA_NODE_INV : CBA_NODE_BUF );
        Vec_IntWriteEntry( &p->vFanins, p->nObjs, Cba_ManHandleBuffer(p->pDesign, Gia_ObjFanin0(pObj)->Value) );
    }
    else
    {
        Vec_IntWriteEntry( &p->vFuncs,  p->nObjs, pGia && Gia_ObjFaninC0(pObj) ? CBA_NODE_C1 : CBA_NODE_C0 );
        Vec_IntWriteEntry( &p->vFanins, p->nObjs, Cba_ManHandleBuffer(p->pDesign, -1) );
    }
    Vec_IntWriteEntry( &p->vNameIds, p->nObjs, Cba_ObjNameId(p, iTerm) );
    Vec_IntWriteEntry( &p->vFanins, iTerm, p->nObjs++ );
}
void Cba_NtkInsertGia( Cba_Man_t * p, Gia_Man_t * pGia )
{
    Cba_Ntk_t * pNtk, * pRoot = Cba_ManRoot( p );
    Vec_Int_t * vTemp = Vec_IntAlloc( 100 );
    int i, j, k, iBox, iTerm, Count = 0;
    Gia_Obj_t * pObj;

    Gia_ManConst0(pGia)->Value = ~0;
    Gia_ManForEachPi( pGia, pObj, i )
        pObj->Value = Cba_NtkPi( pRoot, i );
    Gia_ManForEachAnd( pGia, pObj, i )
    {
        if ( Gia_ObjIsBuf(pObj) )
        {
            pNtk = Cba_ManNtk( p, Vec_IntEntry(p->vBuf2RootNtk, Count) );
            iTerm = Vec_IntEntry( p->vBuf2RootObj, Count );
            assert( Cba_ObjIsCo(pNtk, iTerm) );
            Cba_NtkCreateAndConnectBuffer( pGia, pObj, pNtk, iTerm );
            // prepare leaf
            pObj->Value = Vec_IntEntry( p->vBuf2LeafObj, Count++ );
        }
        else
        {
            int iLit0 = Gia_ObjFanin0(pObj)->Value;
            int iLit1 = Gia_ObjFanin1(pObj)->Value;
            Cba_NodeType_t Type;
            pNtk = Cba_ManNtk( p, pObj->Value );
            if ( Gia_ObjFaninC0(pObj) && Gia_ObjFaninC1(pObj) )
                Type = CBA_NODE_NOR;
            else if ( Gia_ObjFaninC1(pObj) )
                Type = CBA_NODE_SHARP;
            else if ( Gia_ObjFaninC0(pObj) )
            {
                Type = CBA_NODE_SHARP;
                ABC_SWAP( int, iLit0, iLit1 );
            }
            else 
                Type = CBA_NODE_AND;
            Vec_IntFillTwo( vTemp, 2, iLit0, iLit1 );
            Vec_IntWriteEntry( &pNtk->vTypes,  pNtk->nObjs, CBA_OBJ_NODE );
            Vec_IntWriteEntry( &pNtk->vFuncs,  pNtk->nObjs, Type );
            Vec_IntWriteEntry( &pNtk->vFanins, pNtk->nObjs, Cba_ManHandleArray(p, vTemp) );
            pObj->Value = pNtk->nObjs++;
        }
    }
    assert( Count == Gia_ManBufNum(pGia) );
    Vec_IntFree( vTemp );

    // create constant 0 drivers for COs without barbufs
    Cba_ManForEachNtk( p, pNtk, i )
    {
        Cba_NtkForEachBox( pNtk, iBox )
            Cba_BoxForEachBi( pNtk, iBox, iTerm, j )
                if ( Cba_ObjFanin0(pNtk, iTerm) == -1 )
                    Cba_NtkCreateAndConnectBuffer( NULL, NULL, pNtk, iTerm );
        Cba_NtkForEachPo( pNtk, iTerm, k )
            if ( pNtk != pRoot && Cba_ObjFanin0(pNtk, iTerm) == -1 )
                Cba_NtkCreateAndConnectBuffer( NULL, NULL, pNtk, iTerm );
    }
    // create node and connect POs
    Gia_ManForEachPo( pGia, pObj, i )
        Cba_NtkCreateAndConnectBuffer( pGia, pObj, pRoot, Cba_NtkPo(pRoot, i) );
    Cba_ManForEachNtk( p, pNtk, i )
        assert( Cba_NtkObjNum(pNtk) == pNtk->nObjs );
}
Cba_Man_t * Cba_ManInsertGia( Cba_Man_t * p, Gia_Man_t * pGia )
{
    Vec_Int_t * vNtkSizes = Cba_ManCountGia( p, pGia, 1 );
    Cba_Man_t * pNew = Cba_ManDupStart( p, vNtkSizes );
    Cba_ManRemapBarbufs( pNew, p );
    Cba_NtkInsertGia( pNew, pGia );
    Vec_IntFree( vNtkSizes );
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
    Cba_ManAssignInternNames( pNew );
    return pNew;
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_NodeIsSeriousGate( Abc_Obj_t * p )
{
    return Abc_ObjIsNode(p) && (Abc_ObjFaninNum(p) > 0);
}
Vec_Int_t * Cba_ManCountAbc( Cba_Man_t * p, Abc_Ntk_t * pNtk, int fAlwaysAdd )
{
    Cba_Ntk_t * pCbaNtk;
    Abc_Obj_t * pObj, * pFanin; 
    int i, k, iBox, Count = 0;
    Vec_Int_t * vNtkSizes  = Vec_IntStart( Cba_ManNtkNum(p) + 1 );
    Vec_Int_t * vDrivenCos = Vec_IntStart( Cba_ManNtkNum(p) + 1 );
    assert( Vec_IntSize(p->vBuf2LeafNtk) == pNtk->nBarBufs2 );
    // assing for each GIA node, the network it belongs to and count nodes for all networks
    Abc_NtkForEachPi( pNtk, pObj, i )
        pObj->iTemp = 1;
    Abc_NtkForEachNode( pNtk, pObj, i )
    {
        if ( Abc_ObjIsBarBuf(pObj) )
        {
            if ( Abc_NodeIsSeriousGate(Abc_ObjFanin0(pObj)) )
                Vec_IntAddToEntry( vDrivenCos, Abc_ObjFanin0(pObj)->iTemp, 1 );
            pObj->iTemp = Vec_IntEntry( p->vBuf2LeafNtk, Count++ );
        }
        else if ( Abc_NodeIsSeriousGate(pObj) )
        {
            pObj->iTemp = Abc_ObjFanin0(pObj)->iTemp;
            Abc_ObjForEachFanin( pObj, pFanin, k )
                assert( pObj->iTemp == pFanin->iTemp );
            Vec_IntAddToEntry( vNtkSizes, pObj->iTemp, 1 );
        }
    }
    assert( Count == pNtk->nBarBufs2 );
    Abc_NtkForEachPo( pNtk, pObj, i )
    {
        assert( Abc_ObjFanin0(pObj)->iTemp == 1 );
        pObj->iTemp = Abc_ObjFanin0(pObj)->iTemp;
        if ( Abc_NodeIsSeriousGate(Abc_ObjFanin0(pObj)) )
            Vec_IntAddToEntry( vDrivenCos, pObj->iTemp, 1 );
    }
    // for each network, count the total number of COs
    Cba_ManForEachNtk( p, pCbaNtk, i )
    {
        Count = Cba_NtkPiNum(pCbaNtk) + 2 * Cba_NtkPoNum(pCbaNtk) - (fAlwaysAdd ? 0 : Vec_IntEntry(vDrivenCos, i));
        Cba_NtkForEachBox( pCbaNtk, iBox )
            Count += Cba_ObjBoxSize(pCbaNtk, iBox) + Cba_ObjBoxBiNum(pCbaNtk, iBox);
        Vec_IntAddToEntry( vNtkSizes, i, Count );
    }
    Vec_IntFree( vDrivenCos );
    return vNtkSizes;
}
void Cba_NtkCreateOrConnectFanin( Abc_Ntk_t * pNtk, Abc_Obj_t * pFanin, Cba_Ntk_t * p, int iTerm )
{
    if ( pNtk && Abc_NodeIsSeriousGate(pFanin) )
    {
        Vec_IntWriteEntry( &p->vNameIds, pFanin->iTemp, Cba_ObjNameId(p, iTerm) );
        Vec_IntWriteEntry( &p->vFanins,  iTerm,         pFanin->iTemp );
    }
    else
    {
        assert( !pFanin || Abc_NodeIsConst0(pFanin) || Abc_NodeIsConst1(pFanin) );
        Vec_IntWriteEntry( &p->vTypes,   p->nObjs, CBA_OBJ_NODE );
        Vec_IntWriteEntry( &p->vFuncs,   p->nObjs, pNtk && Abc_NodeIsConst1(pFanin) ? 2 : 1 ); // assuming elem gates are added first
        Vec_IntWriteEntry( &p->vFanins,  p->nObjs, Cba_ManHandleBuffer(p->pDesign, -1) );
        Vec_IntWriteEntry( &p->vNameIds, p->nObjs, Cba_ObjNameId(p, iTerm) );
        Vec_IntWriteEntry( &p->vFanins,  iTerm,    p->nObjs++ );
    }
}
void Cba_NtkPrepareLibrary( Cba_Man_t * p, Mio_Library_t * pLib )
{
    Mio_Gate_t * pGate;
    Mio_Gate_t * pGate0 = Mio_LibraryReadConst0( pLib );
    Mio_Gate_t * pGate1 = Mio_LibraryReadConst1( pLib );
    assert( Abc_NamObjNumMax(p->pFuncs) == 1 );
    Abc_NamStrFindOrAdd( p->pFuncs, Mio_GateReadName(pGate0), NULL );
    Abc_NamStrFindOrAdd( p->pFuncs, Mio_GateReadName(pGate1), NULL );
    assert( Abc_NamObjNumMax(p->pFuncs) == 3 );
    Mio_LibraryForEachGate( pLib, pGate )
        if ( pGate != pGate0 && pGate != pGate1 )
            Abc_NamStrFindOrAdd( p->pFuncs, Mio_GateReadName(pGate), NULL );
    assert( Abc_NamObjNumMax(p->pFuncs) > 1 );
}
void Cba_NtkInsertNtk( Cba_Man_t * p, Abc_Ntk_t * pNtk )
{
    Cba_Ntk_t * pCbaNtk, * pRoot = Cba_ManRoot( p );
    Vec_Int_t * vTemp = Vec_IntAlloc( 100 );
    int i, j, k, iBox, iTerm, Count = 0;
    Abc_Obj_t * pObj, * pFanin;
    assert( Abc_NtkHasMapping(pNtk) );
    Cba_NtkPrepareLibrary( p, (Mio_Library_t *)pNtk->pManFunc );

    Abc_NtkForEachPi( pNtk, pObj, i )
        pObj->iTemp = Cba_NtkPi( pRoot, i );
    Abc_NtkForEachNode( pNtk, pObj, i )
    {
        if ( Abc_ObjIsBarBuf(pObj) )
        {
            pCbaNtk = Cba_ManNtk( p, Vec_IntEntry(p->vBuf2RootNtk, Count) );
            iTerm = Vec_IntEntry( p->vBuf2RootObj, Count );
            assert( Cba_ObjIsCo(pCbaNtk, iTerm) );
            Cba_NtkCreateOrConnectFanin( pNtk, Abc_ObjFanin0(pObj), pCbaNtk, iTerm );
            // prepare leaf
            pObj->iTemp = Vec_IntEntry( p->vBuf2LeafObj, Count++ );
        }
        else if ( Abc_NodeIsSeriousGate(pObj) )
        {
            Vec_IntClear( vTemp );
            Abc_ObjForEachFanin( pObj, pFanin, k )
                Vec_IntPush( vTemp, pFanin->iTemp );
            pCbaNtk = Cba_ManNtk( p, pObj->iTemp );
            Vec_IntWriteEntry( &pCbaNtk->vTypes,  pCbaNtk->nObjs, CBA_OBJ_NODE );
            Vec_IntWriteEntry( &pCbaNtk->vFuncs,  pCbaNtk->nObjs, Abc_NamStrFind(p->pFuncs, Mio_GateReadName((Mio_Gate_t *)pObj->pData)) );
            Vec_IntWriteEntry( &pCbaNtk->vFanins, pCbaNtk->nObjs, Cba_ManHandleArray(p, vTemp) );
            pObj->iTemp = pCbaNtk->nObjs++;
        }
    }
    assert( Count == pNtk->nBarBufs2 );
    Vec_IntFree( vTemp );

    // create constant 0 drivers for COs without barbufs
    Cba_ManForEachNtk( p, pCbaNtk, i )
    {
        Cba_NtkForEachBox( pCbaNtk, iBox )
            Cba_BoxForEachBi( pCbaNtk, iBox, iTerm, j )
                if ( Cba_ObjFanin0(pCbaNtk, iTerm) == -1 )
                    Cba_NtkCreateOrConnectFanin( NULL, NULL, pCbaNtk, iTerm );
        Cba_NtkForEachPo( pCbaNtk, iTerm, k )
            if ( pCbaNtk != pRoot && Cba_ObjFanin0(pCbaNtk, iTerm) == -1 )
                Cba_NtkCreateOrConnectFanin( NULL, NULL, pCbaNtk, iTerm );
    }
    // create node and connect POs
    Abc_NtkForEachPo( pNtk, pObj, i )
        Cba_NtkCreateOrConnectFanin( pNtk, Abc_ObjFanin0(pObj), pRoot, Cba_NtkPo(pRoot, i) );
    Cba_ManForEachNtk( p, pCbaNtk, i )
        assert( Cba_NtkObjNum(pCbaNtk) == pCbaNtk->nObjs );
}
void * Cba_ManInsertAbc( Cba_Man_t * p, void * pAbc )
{
    Abc_Ntk_t * pNtk = (Abc_Ntk_t *)pAbc;
    Vec_Int_t * vNtkSizes = Cba_ManCountAbc( p, pNtk, 0 );
    Cba_Man_t * pNew = Cba_ManDupStart( p, vNtkSizes );
    Cba_ManRemapBarbufs( pNew, p );
    Cba_NtkInsertNtk( pNew, pNtk );
    Vec_IntFree( vNtkSizes );
    return pNew;
}



////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

