/**CFile****************************************************************

  FileName    [cbaNtk.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Verilog parser.]

  Synopsis    [Parses several flavors of word-level Verilog.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - November 29, 2014.]

  Revision    [$Id: cbaNtk.c,v 1.00 2014/11/29 00:00:00 alanmi Exp $]

***********************************************************************/

#include "cba.h"

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
void Cba_ManAssignInternNamesNtk( Cba_Ntk_t * p )
{
    int i, NameId;
    int nDigits = Abc_Base10Log( Cba_NtkObjNum(p) );
    Cba_NtkForEachNode( p, i )
    {
        if ( Cba_ObjNameId(p, i) == -1 )
        {
            char Buffer[100];
            sprintf( Buffer, "%s%0*d", "_n_", nDigits, i );
            NameId = Abc_NamStrFindOrAdd( p->pDesign->pNames, Buffer, NULL );
            Vec_IntWriteEntry( &p->vNameIds, i, NameId );
        }
    }
}
void Cba_ManAssignInternNames( Cba_Man_t * p )
{
    Cba_Ntk_t * pNtk; int i;
    Cba_ManForEachNtk( p, pNtk, i )
        Cba_ManAssignInternNamesNtk( pNtk );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Cba_ManObjNum( Cba_Man_t * p )
{
    Cba_Ntk_t * pNtk; 
    int i, Count = 0;
    Cba_ManForEachNtk( p, pNtk, i )
    {
        pNtk->iObjStart = Count;
        Count += Cba_NtkObjNum(pNtk);
    }
    return Count;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
// duplicate PI/PO/boxes
void Cba_ObjDupStart( Cba_Ntk_t * pNew, Cba_Ntk_t * p, int iObj )
{
    if ( Cba_ObjIsPi(p, iObj) )
        Vec_IntWriteEntry( &pNew->vInputs, Cba_ObjFuncId(p, iObj), pNew->nObjs );
    if ( Cba_ObjIsPo(p, iObj) )
        Vec_IntWriteEntry( &pNew->vOutputs, Cba_ObjFuncId(p, iObj), pNew->nObjs );
    Vec_IntWriteEntry( &pNew->vTypes,   pNew->nObjs, Cba_ObjType(p, iObj) );
    Vec_IntWriteEntry( &pNew->vFuncs,   pNew->nObjs, Cba_ObjFuncId(p, iObj) );
    Vec_IntWriteEntry( &pNew->vNameIds, pNew->nObjs, Cba_ObjNameId(p, iObj) );
    if ( Cba_ObjIsBox(p, iObj) )
        Cba_NtkSetHost( Cba_ObjBoxModel(pNew, pNew->nObjs), Cba_NtkId(pNew), pNew->nObjs );
    Cba_NtkSetCopy( p, iObj, pNew->nObjs++ );
}
void Cba_NtkDupStart( Cba_Ntk_t * pNew, Cba_Ntk_t * p )
{
    int i, iObj, iTerm;
    pNew->nObjs = 0;
    Cba_NtkForEachPi( p, iObj, i )
        Cba_ObjDupStart( pNew, p, iObj );
    Cba_NtkForEachPo( p, iObj, i )
        Cba_ObjDupStart( pNew, p, iObj );
    Cba_NtkForEachBox( p, iObj )
    {
        Cba_BoxForEachBi( p, iObj, iTerm, i )
            Cba_ObjDupStart( pNew, p, iTerm );
        Cba_ObjDupStart( pNew, p, iObj );
        Cba_BoxForEachBo( p, iObj, iTerm, i )
            Cba_ObjDupStart( pNew, p, iTerm );
        // connect box outputs to boxes
        Cba_BoxForEachBo( p, iObj, iTerm, i )
            Vec_IntWriteEntry( &pNew->vFanins, Cba_NtkCopy(p, iTerm), Cba_NtkCopy(p, Cba_ObjFanin0(p, iTerm)) );
    }
    assert( Cba_NtkBoxNum(p) == Cba_NtkBoxNum(pNew) );
}
Cba_Man_t * Cba_ManDupStart( Cba_Man_t * p, Vec_Int_t * vNtkSizes )
{
    Cba_Ntk_t * pNtk; int i;
    Cba_Man_t * pNew = Cba_ManClone( p );
    Cba_ManForEachNtk( p, pNtk, i )
        Cba_NtkResize( Cba_ManNtk(pNew, i), vNtkSizes ? Vec_IntEntry(vNtkSizes, i) : Cba_NtkObjNum(pNtk) );
    Vec_IntFill( &p->vCopies, Cba_ManObjNum(p), -1 );
    Cba_ManForEachNtk( p, pNtk, i )
        Cba_NtkDupStart( Cba_ManNtk(pNew, i), pNtk );
    return pNew;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
// duplicate internal nodes
void Cba_NtkDupNodes( Cba_Ntk_t * pNew, Cba_Ntk_t * p, Vec_Int_t * vTemp )
{
    Vec_Int_t * vFanins;
    int i, k, Type, iTerm, iObj;
    // dup nodes
    Cba_NtkForEachNode( p, iObj )
        Cba_ObjDupStart( pNew, p, iObj );
    assert( pNew->nObjs == Cba_NtkObjNum(pNew) ); 
    // connect
    Cba_NtkForEachObjType( p, Type, i )
    {
        if ( Type == CBA_OBJ_PI || Type == CBA_OBJ_BOX || Type == CBA_OBJ_BO )
            continue;
        if ( Type == CBA_OBJ_PO || Type == CBA_OBJ_BI )
        {
            assert( Vec_IntEntry(&pNew->vFanins, Cba_NtkCopy(p, i)) == -1 );
            Vec_IntWriteEntry( &pNew->vFanins, Cba_NtkCopy(p, i), Cba_NtkCopy(p, Cba_ObjFanin0(p, i)) );
            continue;
        }
        assert( Type == CBA_OBJ_NODE );
        Vec_IntClear( vTemp );
        vFanins = Cba_ObjFaninVec( p, i );
        Vec_IntForEachEntry( vFanins, iTerm, k )
            Vec_IntPush( vTemp, Cba_NtkCopy(p, iTerm) );
        Vec_IntWriteEntry( &pNew->vFanins, Cba_NtkCopy(p, i), Cba_ManHandleArray(pNew->pDesign, vTemp) );
    }
}
Cba_Man_t * Cba_ManDup( Cba_Man_t * p )
{
    Cba_Ntk_t * pNtk; int i;
    Vec_Int_t * vTemp = Vec_IntAlloc( 100 );
    Cba_Man_t * pNew = Cba_ManDupStart( p, NULL );
    Cba_ManForEachNtk( p, pNtk, i )
        Cba_NtkDupNodes( Cba_ManNtk(pNew, i), pNtk, vTemp );
    Vec_IntFree( vTemp );
    return pNew;

}


#if 0

/**Function*************************************************************

  Synopsis    [Count the number of objects.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Cba_NtkCountObj_rec( Cba_Ntk_t * p )
{
    int iObj, Count = 0;
    if ( p->nObjsRec >= 0 )
        return p->nObjsRec;
    Cba_NtkForEachBox( p, iObj )
        Count += Cba_BoxIsPrim(p, iObj) ? 1 : Cba_NtkCountObj_rec( Cba_ObjBoxModel(p, iObj) );
    return Count;
}
int Cba_ManCountObj( Cba_Man_t * p )
{
    Cba_Ntk_t * pNtk; int i;
    Cba_ManForEachNtk( p, pNtk, i )
        pNtk->nObjsRec = -1;
    return Cba_NtkCountObj_rec( Cba_ManRoot(p) );
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cba_NtkCollapse_rec( Cba_Ntk_t * pNew, Cba_Ntk_t * p, Vec_Int_t * vSigs )
{
    int i, k, iObj, iTerm;
    // set PI copies
    Cba_NtkCleanCopies( p );
    assert( Vec_IntSize(vSigs) == Cba_NtkPiNum(p) );
    Cba_NtkForEachPi( pRoot, iObj, i )
        Cba_ObjSetCopy( pRoot, iObj, Vec_IntEntry(vSigs, i) );
    // duplicate internal objects
    Cba_ManForEachBox( p, iObj )
        if ( Cba_BoxIsPrim(p, iObj) )
            Cba_BoxDup( pNew, p, iObj );
    // duplicate other modules
    Cba_ManForEachBox( p, iObj )
        if ( !Cba_BoxIsPrim(p, iObj) )
        {
            Vec_IntClear( vSigs );
            Cba_BoxForEachBi( iObj, iTerm, k )
                Vec_IntPush( vSigs, Cba_ObjCopy(p, Cba_ObjFanin(p, iTerm)) );
            Cba_NtkCollapse_rec( pNew, Cba_ObjBoxModel(p, iObj), vSigs );
            Cba_BoxForEachBo( iObj, iTerm, k )
                Cba_ObjAddFanin( pNew, Cba_ObjCopy(p, iObj), Vec_IntEntry(vSigs, k) );
        }
    // connect objects
    Cba_ManForEachObj( p, iObj )
        if ( Cba_ObjType(p, iObj) == CBA_OBJ_BI || Cba_ObjType(p, iObj) == CBA_OBJ_BO )
            Cba_ObjAddFanin( pNew, Cba_ObjCopy(p, iObj), Cba_ObjCopy(p, Cba_ObjFanin(p, iObj)) );
    // collect POs
    Vec_IntClear( vSigs );
    Cba_NtkForEachPi( pRoot, iObj, i )
        Vec_IntPush( vSigs, Cba_ObjCopy(p, Cba_ObjFanin(p, iObj)) );
}
Cba_Man_t * Cba_ManCollapse( Cba_Man_t * p )
{
    Cba_Man_t * pNew = Cba_ManAlloc( NULL, Cba_ManName(p) );
    Cba_Ntk_t * pRootNew = Cba_NtkAlloc( pNew, Cba_NtkName(pRoot) );
    int i, iObj;
    Vec_Int_t * vSigs = Vec_IntAlloc( 1000 );
    Cba_Ntk_t * pRoot = Cba_ManRoot( p );
    Cba_NtkForEachPi( pRoot, iObj, i )
        Vec_IntPush( vSigns, Cba_ObjDup(pRootNew, pRoot, iObj) );
    Cba_NtkCollapse_rec( pRootNew, pRoot, vSigns );
    assert( Vec_IntSize(vSigns) == Cba_NtkPoNum(pRoot) );
    Cba_NtkForEachPo( pRoot, iObj, i )
        Cba_ObjAddFanin( pRootNew, Cba_ObjDup(pRootNew, pRoot, iObj), Vec_IntEntry(vSigns, i) );
    Vec_IntFree( vSigs );
    return pNew;
}

#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

