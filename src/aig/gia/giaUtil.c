/**CFile****************************************************************

  FileName    [giaUtil.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Scalable AIG package.]

  Synopsis    [Various utilities.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: giaUtil.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "gia.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////
 
#define NUMBER1  3716960521u
#define NUMBER2  2174103536u

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Creates a sequence or random numbers.]

  Description []
               
  SideEffects []

  SeeAlso     [http://www.codeproject.com/KB/recipes/SimpleRNG.aspx]

***********************************************************************/
unsigned Gia_ManRandom( int fReset )
{
    static unsigned int m_z = NUMBER1;
    static unsigned int m_w = NUMBER2;
    if ( fReset )
    {
        m_z = NUMBER1;
        m_w = NUMBER2;
    }
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;
}


/**Function*************************************************************

  Synopsis    [Creates random info for the primary inputs.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManRandomInfo( Vec_Ptr_t * vInfo, int iInputStart, int iWordStart, int iWordStop )
{
    unsigned * pInfo;
    int i, w;
    Vec_PtrForEachEntryStart( unsigned *, vInfo, pInfo, i, iInputStart )
        for ( w = iWordStart; w < iWordStop; w++ )
            pInfo[w] = Gia_ManRandom(0);
}


/**Function*************************************************************

  Synopsis    [Returns the time stamp.]

  Description [The file should be closed.]

  SideEffects []

  SeeAlso     []

***********************************************************************/
char * Gia_TimeStamp()
{
    static char Buffer[100];
	char * TimeStamp;
	time_t ltime;
    // get the current time
	time( &ltime );
	TimeStamp = asctime( localtime( &ltime ) );
	TimeStamp[ strlen(TimeStamp) - 1 ] = 0;
    strcpy( Buffer, TimeStamp );
    return Buffer;
}

/**Function*************************************************************

  Synopsis    [Returns the composite name of the file.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
char * Gia_FileNameGenericAppend( char * pBase, char * pSuffix )
{
    static char Buffer[1000];
    char * pDot;
    strcpy( Buffer, pBase );
    if ( (pDot = strrchr( Buffer, '.' )) )
        *pDot = 0;
    strcat( Buffer, pSuffix );
    if ( (pDot = strrchr( Buffer, '\\' )) || (pDot = strrchr( Buffer, '/' )) )
        return pDot+1;
    return Buffer;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManIncrementTravId( Gia_Man_t * p )                     
{ 
    if ( p->pTravIds == NULL )
    {
        p->nTravIdsAlloc = Gia_ManObjNum(p) + 100; 
        p->pTravIds = ABC_CALLOC( int, p->nTravIdsAlloc ); 
        p->nTravIds = 0;  
    }
    while ( p->nTravIdsAlloc < Gia_ManObjNum(p) )
    {
        p->nTravIdsAlloc *= 2;
        p->pTravIds = ABC_REALLOC( int, p->pTravIds, p->nTravIdsAlloc );
        memset( p->pTravIds + p->nTravIdsAlloc/2, 0, sizeof(int) * p->nTravIdsAlloc/2 );
    }
    p->nTravIds++;                                                    
}

/**Function*************************************************************

  Synopsis    [Sets phases of the internal nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSetMark0( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
        pObj->fMark0 = 1;
}

/**Function*************************************************************

  Synopsis    [Sets phases of the internal nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCleanMark0( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
        pObj->fMark0 = 0;
}

/**Function*************************************************************

  Synopsis    [Sets phases of the internal nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCheckMark0( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
        assert( pObj->fMark0 == 0 );
}

/**Function*************************************************************

  Synopsis    [Sets phases of the internal nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSetMark1( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
        pObj->fMark1 = 1;
}

/**Function*************************************************************

  Synopsis    [Sets phases of the internal nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCleanMark1( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
        pObj->fMark1 = 0;
}

/**Function*************************************************************

  Synopsis    [Sets phases of the internal nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCheckMark1( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
        assert( pObj->fMark1 == 0 );
}

/**Function*************************************************************

  Synopsis    [Cleans the value.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCleanValue( Gia_Man_t * p )  
{
    int i;
    for ( i = 0; i < p->nObjs; i++ )
        p->pObjs[i].Value = 0;
}

/**Function*************************************************************

  Synopsis    [Cleans the value.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManFillValue( Gia_Man_t * p )  
{
    int i;
    for ( i = 0; i < p->nObjs; i++ )
        p->pObjs[i].Value = ~0;
}

/**Function*************************************************************

  Synopsis    [Sets the phase of one object.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ObjSetPhase( Gia_Obj_t * pObj )  
{
    if ( Gia_ObjIsAnd(pObj) )
        pObj->fPhase = (Gia_ObjPhase(Gia_ObjFanin0(pObj)) ^ Gia_ObjFaninC0(pObj)) & 
                       (Gia_ObjPhase(Gia_ObjFanin1(pObj)) ^ Gia_ObjFaninC1(pObj));
    else if ( Gia_ObjIsCo(pObj) )
        pObj->fPhase = (Gia_ObjPhase(Gia_ObjFanin0(pObj)) ^ Gia_ObjFaninC0(pObj));
    else
        pObj->fPhase = 0;
}

/**Function*************************************************************

  Synopsis    [Sets phases of the internal nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSetPhase( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
        Gia_ObjSetPhase( pObj );
}

/**Function*************************************************************

  Synopsis    [Sets phases of the internal nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSetPhase1( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachCi( p, pObj, i )
        pObj->fPhase = 1;
    Gia_ManForEachObj( p, pObj, i )
        if ( !Gia_ObjIsCi(pObj) )
            Gia_ObjSetPhase( pObj );
}

/**Function*************************************************************

  Synopsis    [Sets phases of the internal nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCleanPhase( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
        pObj->fPhase = 0;
}

/**Function*************************************************************

  Synopsis    [Prepares copies for the model.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCleanLevels( Gia_Man_t * p, int Size )
{
    if ( p->vLevels == NULL )
        p->vLevels = Vec_IntAlloc( Size );
    Vec_IntFill( p->vLevels, Size, 0 );
}
/**Function*************************************************************

  Synopsis    [Prepares copies for the model.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCleanTruth( Gia_Man_t * p )
{
    if ( p->vTruths == NULL )
        p->vTruths = Vec_IntAlloc( Gia_ManObjNum(p) );
    Vec_IntFill( p->vTruths, Gia_ManObjNum(p), -1 );
}

/**Function*************************************************************

  Synopsis    [Assigns levels.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManLevelNum( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManCleanLevels( p, Gia_ManObjNum(p) );
    p->nLevels = 0;
    Gia_ManForEachObj( p, pObj, i )
        if ( Gia_ObjIsAnd(pObj) )
            Gia_ObjSetAndLevel( p, pObj );
        else if ( Gia_ObjIsCo(pObj) )
        {
            Gia_ObjSetCoLevel( p, pObj );
            p->nLevels = Abc_MaxInt( p->nLevels, Gia_ObjLevel(p, pObj) );
        }
        else
            Gia_ObjSetLevel( p, pObj, 0 );
    return p->nLevels;
}

/**Function*************************************************************

  Synopsis    [Assigns levels.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSetRefs( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
    {
        pObj->Value = 0;
        if ( Gia_ObjIsAnd(pObj) )
        {
            Gia_ObjFanin0(pObj)->Value++;
            Gia_ObjFanin1(pObj)->Value++;
        }
        else if ( Gia_ObjIsCo(pObj) )
            Gia_ObjFanin0(pObj)->Value++;
    }
}

/**Function*************************************************************

  Synopsis    [Assigns references.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCreateRefs( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj;
    int i;
    assert( p->pRefs == NULL );
    p->pRefs = ABC_CALLOC( int, Gia_ManObjNum(p) );
    Gia_ManForEachObj( p, pObj, i )
    {
        if ( Gia_ObjIsAnd(pObj) )
        {
            Gia_ObjRefFanin0Inc( p, pObj );
            Gia_ObjRefFanin1Inc( p, pObj );
        }
        else if ( Gia_ObjIsCo(pObj) )
            Gia_ObjRefFanin0Inc( p, pObj );
    }
}

/**Function*************************************************************

  Synopsis    [Assigns references.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int * Gia_ManCreateMuxRefs( Gia_Man_t * p )  
{
    Gia_Obj_t * pObj, * pCtrl, * pFan0, * pFan1;
    int i, * pMuxRefs;
    pMuxRefs = ABC_CALLOC( int, Gia_ManObjNum(p) );
    Gia_ManForEachObj( p, pObj, i )
    {
        if ( Gia_ObjRecognizeExor( pObj, &pFan0, &pFan1 ) )
            continue;
        if ( !Gia_ObjIsMuxType(pObj) )
            continue;
        pCtrl = Gia_ObjRecognizeMux( pObj, &pFan0, &pFan1 );
        pMuxRefs[ Gia_ObjId(p, Gia_Regular(pCtrl)) ]++;
    }
    return pMuxRefs;
}

/**Function*************************************************************

  Synopsis    [Computes the maximum frontier size.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManCrossCut( Gia_Man_t * p )
{
    Gia_Obj_t * pObj;
    int i, nCutCur = 0, nCutMax = 0;
    Gia_ManSetRefs( p );
    Gia_ManForEachObj( p, pObj, i )
    {
        if ( pObj->Value )
            nCutCur++;
        if ( nCutMax < nCutCur )
            nCutMax = nCutCur;
        if ( Gia_ObjIsAnd(pObj) )
        {
            if ( --Gia_ObjFanin0(pObj)->Value == 0 )
                nCutCur--;
            if ( --Gia_ObjFanin1(pObj)->Value == 0 )
                nCutCur--;
        }
        else if ( Gia_ObjIsCo(pObj) )
        {
            if ( --Gia_ObjFanin0(pObj)->Value == 0 )
                nCutCur--;
        }
    }
//    Gia_ManForEachObj( p, pObj, i )
//        assert( pObj->Value == 0 );
    return nCutMax;
}

/**Function*************************************************************

  Synopsis    [Makes sure the manager is normalized.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManIsNormalized( Gia_Man_t * p )  
{
    int i, nOffset;
    nOffset = 1;
    for ( i = 0; i < Gia_ManCiNum(p); i++ )
        if ( !Gia_ObjIsCi( Gia_ManObj(p, nOffset+i) ) )
            return 0;
    nOffset = 1 + Gia_ManCiNum(p) + Gia_ManAndNum(p);
    for ( i = 0; i < Gia_ManCoNum(p); i++ )
        if ( !Gia_ObjIsCo( Gia_ManObj(p, nOffset+i) ) )
            return 0;
    return 1;
}


/**Function*************************************************************

  Synopsis    [Collects PO Ids into one array.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_ManCollectPoIds( Gia_Man_t * p )
{
    Vec_Int_t * vStart;
    int Entry, i;
    vStart = Vec_IntAlloc( Gia_ManPoNum(p) );
    Vec_IntForEachEntryStop( p->vCos, Entry, i, Gia_ManPoNum(p) )
        Vec_IntPush( vStart, Entry );
    return vStart;
}


/**Function*************************************************************

  Synopsis    [Returns 1 if the node is the root of MUX or EXOR/NEXOR.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ObjIsMuxType( Gia_Obj_t * pNode )
{
    Gia_Obj_t * pNode0, * pNode1;
    // check that the node is regular
    assert( !Gia_IsComplement(pNode) );
    // if the node is not AND, this is not MUX
    if ( !Gia_ObjIsAnd(pNode) )
        return 0;
    // if the children are not complemented, this is not MUX
    if ( !Gia_ObjFaninC0(pNode) || !Gia_ObjFaninC1(pNode) )
        return 0;
    // get children
    pNode0 = Gia_ObjFanin0(pNode);
    pNode1 = Gia_ObjFanin1(pNode);
    // if the children are not ANDs, this is not MUX
    if ( !Gia_ObjIsAnd(pNode0) || !Gia_ObjIsAnd(pNode1) )
        return 0;
    // otherwise the node is MUX iff it has a pair of equal grandchildren
    return (Gia_ObjFanin0(pNode0) == Gia_ObjFanin0(pNode1) && (Gia_ObjFaninC0(pNode0) ^ Gia_ObjFaninC0(pNode1))) || 
           (Gia_ObjFanin0(pNode0) == Gia_ObjFanin1(pNode1) && (Gia_ObjFaninC0(pNode0) ^ Gia_ObjFaninC1(pNode1))) ||
           (Gia_ObjFanin1(pNode0) == Gia_ObjFanin0(pNode1) && (Gia_ObjFaninC1(pNode0) ^ Gia_ObjFaninC0(pNode1))) ||
           (Gia_ObjFanin1(pNode0) == Gia_ObjFanin1(pNode1) && (Gia_ObjFaninC1(pNode0) ^ Gia_ObjFaninC1(pNode1)));
}


/**Function*************************************************************

  Synopsis    [Recognizes what nodes are inputs of the EXOR.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ObjRecognizeExor( Gia_Obj_t * pObj, Gia_Obj_t ** ppFan0, Gia_Obj_t ** ppFan1 )
{
    Gia_Obj_t * p0, * p1;
    assert( !Gia_IsComplement(pObj) );
    if ( !Gia_ObjIsAnd(pObj) )
        return 0;
    assert( Gia_ObjIsAnd(pObj) );
    p0 = Gia_ObjChild0(pObj);
    p1 = Gia_ObjChild1(pObj);
    if ( !Gia_IsComplement(p0) || !Gia_IsComplement(p1) )
        return 0;
    p0 = Gia_Regular(p0);
    p1 = Gia_Regular(p1);
    if ( !Gia_ObjIsAnd(p0) || !Gia_ObjIsAnd(p1) )
        return 0;
    if ( Gia_ObjFanin0(p0) != Gia_ObjFanin0(p1) || Gia_ObjFanin1(p0) != Gia_ObjFanin1(p1) )
        return 0;
    if ( Gia_ObjFaninC0(p0) == Gia_ObjFaninC0(p1) || Gia_ObjFaninC1(p0) == Gia_ObjFaninC1(p1) )
        return 0;
    *ppFan0 = Gia_ObjChild0(p0);
    *ppFan1 = Gia_ObjChild1(p0);
    return 1;
}

/**Function*************************************************************

  Synopsis    [Recognizes what nodes are control and data inputs of a MUX.]

  Description [If the node is a MUX, returns the control variable C.
  Assigns nodes T and E to be the then and else variables of the MUX. 
  Node C is never complemented. Nodes T and E can be complemented.
  This function also recognizes EXOR/NEXOR gates as MUXes.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Obj_t * Gia_ObjRecognizeMux( Gia_Obj_t * pNode, Gia_Obj_t ** ppNodeT, Gia_Obj_t ** ppNodeE )
{
    Gia_Obj_t * pNode0, * pNode1;
    assert( !Gia_IsComplement(pNode) );
    assert( Gia_ObjIsMuxType(pNode) );
    // get children
    pNode0 = Gia_ObjFanin0(pNode);
    pNode1 = Gia_ObjFanin1(pNode);

    // find the control variable
    if ( Gia_ObjFanin1(pNode0) == Gia_ObjFanin1(pNode1) && (Gia_ObjFaninC1(pNode0) ^ Gia_ObjFaninC1(pNode1)) )
    {
//        if ( FrGia_IsComplement(pNode1->p2) )
        if ( Gia_ObjFaninC1(pNode0) )
        { // pNode2->p2 is positive phase of C
            *ppNodeT = Gia_Not(Gia_ObjChild0(pNode1));//pNode2->p1);
            *ppNodeE = Gia_Not(Gia_ObjChild0(pNode0));//pNode1->p1);
            return Gia_ObjChild1(pNode1);//pNode2->p2;
        }
        else
        { // pNode1->p2 is positive phase of C
            *ppNodeT = Gia_Not(Gia_ObjChild0(pNode0));//pNode1->p1);
            *ppNodeE = Gia_Not(Gia_ObjChild0(pNode1));//pNode2->p1);
            return Gia_ObjChild1(pNode0);//pNode1->p2;
        }
    }
    else if ( Gia_ObjFanin0(pNode0) == Gia_ObjFanin0(pNode1) && (Gia_ObjFaninC0(pNode0) ^ Gia_ObjFaninC0(pNode1)) )
    {
//        if ( FrGia_IsComplement(pNode1->p1) )
        if ( Gia_ObjFaninC0(pNode0) )
        { // pNode2->p1 is positive phase of C
            *ppNodeT = Gia_Not(Gia_ObjChild1(pNode1));//pNode2->p2);
            *ppNodeE = Gia_Not(Gia_ObjChild1(pNode0));//pNode1->p2);
            return Gia_ObjChild0(pNode1);//pNode2->p1;
        }
        else
        { // pNode1->p1 is positive phase of C
            *ppNodeT = Gia_Not(Gia_ObjChild1(pNode0));//pNode1->p2);
            *ppNodeE = Gia_Not(Gia_ObjChild1(pNode1));//pNode2->p2);
            return Gia_ObjChild0(pNode0);//pNode1->p1;
        }
    }
    else if ( Gia_ObjFanin0(pNode0) == Gia_ObjFanin1(pNode1) && (Gia_ObjFaninC0(pNode0) ^ Gia_ObjFaninC1(pNode1)) )
    {
//        if ( FrGia_IsComplement(pNode1->p1) )
        if ( Gia_ObjFaninC0(pNode0) )
        { // pNode2->p2 is positive phase of C
            *ppNodeT = Gia_Not(Gia_ObjChild0(pNode1));//pNode2->p1);
            *ppNodeE = Gia_Not(Gia_ObjChild1(pNode0));//pNode1->p2);
            return Gia_ObjChild1(pNode1);//pNode2->p2;
        }
        else
        { // pNode1->p1 is positive phase of C
            *ppNodeT = Gia_Not(Gia_ObjChild1(pNode0));//pNode1->p2);
            *ppNodeE = Gia_Not(Gia_ObjChild0(pNode1));//pNode2->p1);
            return Gia_ObjChild0(pNode0);//pNode1->p1;
        }
    }
    else if ( Gia_ObjFanin1(pNode0) == Gia_ObjFanin0(pNode1) && (Gia_ObjFaninC1(pNode0) ^ Gia_ObjFaninC0(pNode1)) )
    {
//        if ( FrGia_IsComplement(pNode1->p2) )
        if ( Gia_ObjFaninC1(pNode0) )
        { // pNode2->p1 is positive phase of C
            *ppNodeT = Gia_Not(Gia_ObjChild1(pNode1));//pNode2->p2);
            *ppNodeE = Gia_Not(Gia_ObjChild0(pNode0));//pNode1->p1);
            return Gia_ObjChild0(pNode1);//pNode2->p1;
        }
        else
        { // pNode1->p2 is positive phase of C
            *ppNodeT = Gia_Not(Gia_ObjChild0(pNode0));//pNode1->p1);
            *ppNodeE = Gia_Not(Gia_ObjChild1(pNode1));//pNode2->p2);
            return Gia_ObjChild1(pNode0);//pNode1->p2;
        }
    }
    assert( 0 ); // this is not MUX
    return NULL;
}


/**Function*************************************************************

  Synopsis    [Dereferences the node's MFFC.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_NodeDeref_rec( Gia_Man_t * p, Gia_Obj_t * pNode )
{
    Gia_Obj_t * pFanin;
    int Counter = 0;
    if ( Gia_ObjIsCi(pNode) )
        return 0;
    assert( Gia_ObjIsAnd(pNode) );
    pFanin = Gia_ObjFanin0(pNode);
    assert( Gia_ObjRefs(p, pFanin) > 0 );
    if ( Gia_ObjRefDec(p, pFanin) == 0 )
        Counter += Gia_NodeDeref_rec( p, pFanin );
    pFanin = Gia_ObjFanin1(pNode);
    assert( Gia_ObjRefs(p, pFanin) > 0 );
    if ( Gia_ObjRefDec(p, pFanin) == 0 )
        Counter += Gia_NodeDeref_rec( p, pFanin );
    return Counter + 1;
}

/**Function*************************************************************

  Synopsis    [References the node's MFFC.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_NodeRef_rec( Gia_Man_t * p, Gia_Obj_t * pNode )
{
    Gia_Obj_t * pFanin;
    int Counter = 0;
    if ( Gia_ObjIsCi(pNode) )
        return 0;
    assert( Gia_ObjIsAnd(pNode) );
    pFanin = Gia_ObjFanin0(pNode);
    if ( Gia_ObjRefInc(p, pFanin) == 0 )
        Counter += Gia_NodeRef_rec( p, pFanin );
    pFanin = Gia_ObjFanin1(pNode);
    if ( Gia_ObjRefInc(p, pFanin) == 0 )
        Counter += Gia_NodeRef_rec( p, pFanin );
    return Counter + 1;
}

/**Function*************************************************************

  Synopsis    [Returns the number of internal nodes in the MFFC.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_NodeMffcSize( Gia_Man_t * p, Gia_Obj_t * pNode )
{
    int ConeSize1, ConeSize2;
    assert( !Gia_IsComplement(pNode) );
    assert( Gia_ObjIsCand(pNode) );
    ConeSize1 = Gia_NodeDeref_rec( p, pNode );
    ConeSize2 = Gia_NodeRef_rec( p, pNode );
    assert( ConeSize1 == ConeSize2 );
    assert( ConeSize1 >= 0 );
    return ConeSize1;
}

/**Function*************************************************************

  Synopsis    [Returns 1 if AIG has choices.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManHasChoices( Gia_Man_t * p )
{
    Gia_Obj_t * pObj;
    int i, Counter1 = 0, Counter2 = 0;
    int nFailNoRepr = 0;
    int nFailHaveRepr = 0;
    int nChoiceNodes = 0;
    int nChoices = 0;
    if ( p->pReprs == NULL || p->pNexts == NULL )
        return 0;
    // check if there are any representatives
    Gia_ManForEachObj( p, pObj, i )
    {
        if ( Gia_ObjReprObj( p, Gia_ObjId(p, pObj) ) )
        {
//            printf( "%d ", i );
            Counter1++;
        }
//        if ( Gia_ObjNext( p, Gia_ObjId(p, pObj) ) )
//            Counter2++;
    }
//    printf( "\n" );
    Gia_ManForEachObj( p, pObj, i )
    {
//        if ( Gia_ObjReprObj( p, Gia_ObjId(p, pObj) ) )
//            Counter1++;
        if ( Gia_ObjNext( p, Gia_ObjId(p, pObj) ) )
        {
//            printf( "%d ", i );
            Counter2++;
        }
    }
//    printf( "\n" );
    if ( Counter1 == 0 )
    {
        printf( "Warning: AIG has repr data-strucure but not reprs.\n" );
        return 0;
    }
    printf( "%d nodes have reprs.\n", Counter1 );
    printf( "%d nodes have nexts.\n", Counter2 );
    // check if there are any internal nodes without fanout
    // make sure all nodes without fanout have representatives
    // make sure all nodes with fanout have no representatives
    ABC_FREE( p->pRefs );
    Gia_ManCreateRefs( p );
    Gia_ManForEachAnd( p, pObj, i )
    {
        if ( Gia_ObjRefs(p, pObj) == 0 )
        {
            if ( Gia_ObjReprObj( p, Gia_ObjId(p, pObj) ) == NULL )
                nFailNoRepr++;
            else
                nChoices++;
        }
        else
        {
            if ( Gia_ObjReprObj( p, Gia_ObjId(p, pObj) ) != NULL )
                nFailHaveRepr++;
            if ( Gia_ObjNextObj( p, Gia_ObjId(p, pObj) ) != NULL )
                nChoiceNodes++;
        }
        if ( Gia_ObjReprObj( p, i ) )
            assert( Gia_ObjRepr(p, i) < i );
    }
    if ( nChoices == 0 )
        return 0;
    if ( nFailNoRepr )
    {
        printf( "Gia_ManHasChoices(): Error: %d internal nodes have no fanout and no repr.\n", nFailNoRepr );
//        return 0;
    }
    if ( nFailHaveRepr )
    {
        printf( "Gia_ManHasChoices(): Error: %d internal nodes have both fanout and repr.\n", nFailHaveRepr );
//        return 0;
    }
//    printf( "Gia_ManHasChoices(): AIG has %d choice nodes with %d choices.\n", nChoiceNodes, nChoices );
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManVerifyChoices( Gia_Man_t * p )
{
    Gia_Obj_t * pObj;
    int i, fProb = 0;
    assert( p->pReprs );
    Gia_ManForEachObj( p, pObj, i )
    {
        if ( Gia_ObjIsAnd(pObj) )
        {
            if ( Gia_ObjHasRepr(p, Gia_ObjFaninId0(pObj, i)) )
                printf( "Fanin 0 of AND node %d has a repr.\n", i ), fProb = 1;
            if ( Gia_ObjHasRepr(p, Gia_ObjFaninId1(pObj, i)) )
                printf( "Fanin 1 of AND node %d has a repr.\n", i ), fProb = 1;
        }
        else if ( Gia_ObjIsCo(pObj) )
        {
            if ( Gia_ObjHasRepr(p, Gia_ObjFaninId0(pObj, i)) )
                printf( "Fanin 0 of CO node %d has a repr.\n", i ), fProb = 1;
        }
    }
//    if ( !fProb )
//        printf( "GIA with choices is correct.\n" );
}

/**Function*************************************************************

  Synopsis    [Returns 1 if AIG has dangling nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManHasDangling( Gia_Man_t * p )
{
    Gia_Obj_t * pObj;
    int i, Counter = 0;
    Gia_ManForEachObj( p, pObj, i )
    {
        pObj->fMark0 = 0;
        if ( Gia_ObjIsAnd(pObj) )
        {
            Gia_ObjFanin0(pObj)->fMark0 = 1;
            Gia_ObjFanin1(pObj)->fMark0 = 1;
        }
        else if ( Gia_ObjIsCo(pObj) )
            Gia_ObjFanin0(pObj)->fMark0 = 1;
    }
    Gia_ManForEachAnd( p, pObj, i )
        Counter += !pObj->fMark0;
    Gia_ManCleanMark0( p );
    return Counter;
}

/**Function*************************************************************

  Synopsis    [Returns 1 if AIG has dangling nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManMarkDangling( Gia_Man_t * p )
{
    Gia_Obj_t * pObj;
    int i, Counter = 0;
    Gia_ManForEachObj( p, pObj, i )
    {
        pObj->fMark0 = 0;
        if ( Gia_ObjIsAnd(pObj) )
        {
            Gia_ObjFanin0(pObj)->fMark0 = 1;
            Gia_ObjFanin1(pObj)->fMark0 = 1;
        }
        else if ( Gia_ObjIsCo(pObj) )
            Gia_ObjFanin0(pObj)->fMark0 = 1;
    }
    Gia_ManForEachAnd( p, pObj, i )
        Counter += !pObj->fMark0;
    return Counter;
}

/**Function*************************************************************

  Synopsis    [Returns 1 if AIG has dangling nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_ManGetDangling( Gia_Man_t * p )
{
    Vec_Int_t * vDangles;
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( p, pObj, i )
    {
        pObj->fMark0 = 0;
        if ( Gia_ObjIsAnd(pObj) )
        {
            Gia_ObjFanin0(pObj)->fMark0 = 1;
            Gia_ObjFanin1(pObj)->fMark0 = 1;
        }
        else if ( Gia_ObjIsCo(pObj) )
            Gia_ObjFanin0(pObj)->fMark0 = 1;
    }
    vDangles = Vec_IntAlloc( 100 );
    Gia_ManForEachAnd( p, pObj, i )
        if ( !pObj->fMark0 )
            Vec_IntPush( vDangles, i );
    Gia_ManCleanMark0( p );
    return vDangles;
}
/**Function*************************************************************

  Synopsis    [Verbose printing of the AIG node.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ObjPrint( Gia_Man_t * p, Gia_Obj_t * pObj )
{
    if ( pObj == NULL )
    {
        printf( "Object is NULL." );
        return;
    }
    if ( Gia_IsComplement(pObj) )
    {
        printf( "Compl " );
        pObj = Gia_Not(pObj);
    }
    assert( !Gia_IsComplement(pObj) );
    printf( "Node %4d : ", Gia_ObjId(p, pObj) );
    if ( Gia_ObjIsConst0(pObj) )
        printf( "constant 0" );
    else if ( Gia_ObjIsPi(p, pObj) )
        printf( "PI" );
    else if ( Gia_ObjIsPo(p, pObj) )
        printf( "PO( %4d%s )", Gia_ObjFaninId0p(p, pObj), (Gia_ObjFaninC0(pObj)? "\'" : " ") );
    else if ( Gia_ObjIsCi(pObj) )
        printf( "RO" );
    else if ( Gia_ObjIsCo(pObj) )
        printf( "RI( %4d%s )", Gia_ObjFaninId0p(p, pObj), (Gia_ObjFaninC0(pObj)? "\'" : " ") );
//    else if ( Gia_ObjIsBuf(pObj) )
//        printf( "BUF( %d%s )", Gia_ObjFaninId0p(p, pObj), (Gia_ObjFaninC0(pObj)? "\'" : " ") );
    else
        printf( "AND( %4d%s, %4d%s )", 
            Gia_ObjFaninId0p(p, pObj), (Gia_ObjFaninC0(pObj)? "\'" : " "), 
            Gia_ObjFaninId1p(p, pObj), (Gia_ObjFaninC1(pObj)? "\'" : " ") );
    if ( p->pRefs )
        printf( " (refs = %3d)", Gia_ObjRefs(p, pObj) );
    printf( "\n" );
/*
    if ( p->pRefs )
    {
        Gia_Obj_t * pFanout;
        int i;
        int iFan = -1; // Suppress "might be used uninitialized"
        printf( "\nFanouts:\n" );
        Gia_ObjForEachFanout( p, pObj, pFanout, iFan, i )
        {
            printf( "    " );
            printf( "Node %4d : ", Gia_ObjId(pFanout) );
            if ( Gia_ObjIsPo(pFanout) )
                printf( "PO( %4d%s )", Gia_ObjFanin0(pFanout)->Id, (Gia_ObjFaninC0(pFanout)? "\'" : " ") );
            else if ( Gia_ObjIsBuf(pFanout) )
                printf( "BUF( %d%s )", Gia_ObjFanin0(pFanout)->Id, (Gia_ObjFaninC0(pFanout)? "\'" : " ") );
            else
                printf( "AND( %4d%s, %4d%s )", 
                    Gia_ObjFanin0(pFanout)->Id, (Gia_ObjFaninC0(pFanout)? "\'" : " "), 
                    Gia_ObjFanin1(pFanout)->Id, (Gia_ObjFaninC1(pFanout)? "\'" : " ") );
            printf( "\n" );
        }
        return;
    }
*/
}

/**Function*************************************************************

  Synopsis    [Resimulates the counter-example.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManVerifyCex( Gia_Man_t * pAig, Abc_Cex_t * p, int fDualOut )
{
    Gia_Obj_t * pObj, * pObjRi, * pObjRo;
    int RetValue, i, k, iBit = 0;
    Gia_ManCleanMark0(pAig);
    Gia_ManForEachRo( pAig, pObj, i )
        pObj->fMark0 = Abc_InfoHasBit(p->pData, iBit++);
    for ( i = 0; i <= p->iFrame; i++ )
    {
        Gia_ManForEachPi( pAig, pObj, k )
            pObj->fMark0 = Abc_InfoHasBit(p->pData, iBit++);
        Gia_ManForEachAnd( pAig, pObj, k )
            pObj->fMark0 = (Gia_ObjFanin0(pObj)->fMark0 ^ Gia_ObjFaninC0(pObj)) & 
                           (Gia_ObjFanin1(pObj)->fMark0 ^ Gia_ObjFaninC1(pObj));
        Gia_ManForEachCo( pAig, pObj, k )
            pObj->fMark0 = Gia_ObjFanin0(pObj)->fMark0 ^ Gia_ObjFaninC0(pObj);
        if ( i == p->iFrame )
            break;
        Gia_ManForEachRiRo( pAig, pObjRi, pObjRo, k )
        {
            pObjRo->fMark0 = pObjRi->fMark0;
        }
    }
    assert( iBit == p->nBits );
    if ( fDualOut )
        RetValue = Gia_ManPo(pAig, 2*p->iPo)->fMark0 ^ Gia_ManPo(pAig, 2*p->iPo+1)->fMark0;
    else
        RetValue = Gia_ManPo(pAig, p->iPo)->fMark0;
    Gia_ManCleanMark0(pAig);
    return RetValue;
}

/**Function*************************************************************

  Synopsis    [Resimulates the counter-example.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManFindFailedPoCex( Gia_Man_t * pAig, Abc_Cex_t * p, int nOutputs )
{
    Gia_Obj_t * pObj, * pObjRi, * pObjRo;
    int RetValue, i, k, iBit = 0;
    assert( Gia_ManPiNum(pAig) == p->nPis );
    Gia_ManCleanMark0(pAig);
//    Gia_ManForEachRo( pAig, pObj, i )
//       pObj->fMark0 = Abc_InfoHasBit(p->pData, iBit++);
    iBit = p->nRegs;
    for ( i = 0; i <= p->iFrame; i++ )
    {
        Gia_ManForEachPi( pAig, pObj, k )
            pObj->fMark0 = Abc_InfoHasBit(p->pData, iBit++);
        Gia_ManForEachAnd( pAig, pObj, k )
            pObj->fMark0 = (Gia_ObjFanin0(pObj)->fMark0 ^ Gia_ObjFaninC0(pObj)) & 
                           (Gia_ObjFanin1(pObj)->fMark0 ^ Gia_ObjFaninC1(pObj));
        Gia_ManForEachCo( pAig, pObj, k )
            pObj->fMark0 = Gia_ObjFanin0(pObj)->fMark0 ^ Gia_ObjFaninC0(pObj);
        Gia_ManForEachRiRo( pAig, pObjRi, pObjRo, k )
            pObjRo->fMark0 = pObjRi->fMark0;
    }
    assert( iBit == p->nBits );
    // figure out the number of failed output
    RetValue = -1;
//	for ( i = Gia_ManPoNum(pAig) - 1; i >= nOutputs; i-- )
	for ( i = nOutputs; i < Gia_ManPoNum(pAig); i++ )
	{
        if ( Gia_ManPo(pAig, i)->fMark0 )
        {
            RetValue = i;
            break;
        }
	}
    Gia_ManCleanMark0(pAig);
    return RetValue;
}

/**Function*************************************************************

  Synopsis    [Complements the constraint outputs.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManInvertConstraints( Gia_Man_t * pAig )
{
    Gia_Obj_t * pObj;
    int i;
    if ( Gia_ManConstrNum(pAig) == 0 )
        return;
    Gia_ManForEachPo( pAig, pObj, i )
    {
        if ( i >= Gia_ManPoNum(pAig) - Gia_ManConstrNum(pAig) )
            Gia_ObjFlipFaninC0( pObj );
    }
}

/**Function*************************************************************

  Synopsis    [Converting VTA vector to GLA vector.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_VtaConvertToGla( Gia_Man_t * p, Vec_Int_t * vVta )
{
    Gia_Obj_t * pObj;
    Vec_Int_t * vGla;
    int nObjMask, nObjs = Gia_ManObjNum(p);
    int i, Entry, nFrames = Vec_IntEntry( vVta, 0 );
    assert( Vec_IntEntry(vVta, nFrames+1) == Vec_IntSize(vVta) );
    // get the bitmask
    nObjMask = (1 << Abc_Base2Log(nObjs)) - 1;
    assert( nObjs <= nObjMask );
    // go through objects
    vGla = Vec_IntStart( nObjs );
    Vec_IntWriteEntry( vGla, 0, 1 );
    Vec_IntForEachEntryStart( vVta, Entry, i, nFrames+2 )
    {
        pObj = Gia_ManObj( p, (Entry &  nObjMask) );
        assert( Gia_ObjIsRo(p, pObj) || Gia_ObjIsAnd(pObj) || Gia_ObjIsConst0(pObj) );
        Vec_IntWriteEntry( vGla, (Entry &  nObjMask), 1 );
    }
    return vGla;
}

/**Function*************************************************************

  Synopsis    [Converting GLA vector to VTA vector.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_VtaConvertFromGla( Gia_Man_t * p, Vec_Int_t * vGla, int nFrames )
{
    Vec_Int_t * vVta;
    int nObjBits, nObjMask, nObjs = Gia_ManObjNum(p);
    int i, k, j, Entry, Counter, nGlaSize;
    //. get the GLA size
    nGlaSize = Vec_IntSum(vGla);
    // get the bitmask
    nObjBits = Abc_Base2Log(nObjs);
    nObjMask = (1 << Abc_Base2Log(nObjs)) - 1;
    assert( nObjs <= nObjMask );
    // go through objects
    vVta = Vec_IntAlloc( 1000 );
    Vec_IntPush( vVta, nFrames );
    Counter = nFrames + 2;
    for ( i = 0; i <= nFrames; i++, Counter += i * nGlaSize )
        Vec_IntPush( vVta, Counter );
    for ( i = 0; i < nFrames; i++ )
        for ( k = 0; k <= i; k++ )
            Vec_IntForEachEntry( vGla, Entry, j )
                if ( Entry ) 
                    Vec_IntPush( vVta, (k << nObjBits) | j );
    Counter = Vec_IntEntry(vVta, nFrames+1);
    assert( Vec_IntEntry(vVta, nFrames+1) == Vec_IntSize(vVta) );
    return vVta;
}

/**Function*************************************************************

  Synopsis    [Converting GLA vector to FLA vector.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_FlaConvertToGla_rec( Gia_Man_t * p, Gia_Obj_t * pObj, Vec_Int_t * vGla )
{
    if ( Gia_ObjIsTravIdCurrent(p, pObj) )
        return;
    Gia_ObjSetTravIdCurrent(p, pObj);
    Vec_IntWriteEntry( vGla, Gia_ObjId(p, pObj), 1 );
    if ( Gia_ObjIsRo(p, pObj) )
        return;
    assert( Gia_ObjIsAnd(pObj) );
    Gia_FlaConvertToGla_rec( p, Gia_ObjFanin0(pObj), vGla );
    Gia_FlaConvertToGla_rec( p, Gia_ObjFanin1(pObj), vGla );
}

/**Function*************************************************************

  Synopsis    [Converting FLA vector to GLA vector.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_FlaConvertToGla( Gia_Man_t * p, Vec_Int_t * vFla )
{
    Vec_Int_t * vGla;
    Gia_Obj_t * pObj;
    int i;
    // mark const0 and relevant CI objects
    Gia_ManIncrementTravId( p );
    Gia_ObjSetTravIdCurrent(p, Gia_ManConst0(p));
    Gia_ManForEachPi( p, pObj, i )
        Gia_ObjSetTravIdCurrent(p, pObj);
    Gia_ManForEachRo( p, pObj, i )
        if ( !Vec_IntEntry(vFla, i) )
            Gia_ObjSetTravIdCurrent(p, pObj);
    // label all objects reachable from the PO and selected flops
    vGla = Vec_IntStart( Gia_ManObjNum(p) );
    Vec_IntWriteEntry( vGla, 0, 1 );
    Gia_ManForEachPo( p, pObj, i )
        Gia_FlaConvertToGla_rec( p, Gia_ObjFanin0(pObj), vGla );
    Gia_ManForEachRi( p, pObj, i )
        if ( Vec_IntEntry(vFla, i) )
            Gia_FlaConvertToGla_rec( p, Gia_ObjFanin0(pObj), vGla );
    return vGla;
}

/**Function*************************************************************

  Synopsis    [Converting GLA vector to FLA vector.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_GlaConvertToFla( Gia_Man_t * p, Vec_Int_t * vGla )
{
    Vec_Int_t * vFla;
    Gia_Obj_t * pObj;
    int i;
    vFla = Vec_IntStart( Gia_ManRegNum(p) );
    Gia_ManForEachRo( p, pObj, i )
        if ( Vec_IntEntry(vGla, Gia_ObjId(p, pObj)) )
            Vec_IntWriteEntry( vFla, i, 1 );
    return vFla;
}

/**Function*************************************************************

  Synopsis    [Testing the speedup due to grouping POs into batches.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManCollectObjs_rec( Gia_Man_t * p, int iObjId, Vec_Int_t * vObjs, int Limit )
{
    Gia_Obj_t * pObj;
    if ( Vec_IntSize(vObjs) == Limit )
        return;
    if ( Gia_ObjIsTravIdCurrentId(p, iObjId) )
        return;
    Gia_ObjSetTravIdCurrentId(p, iObjId);
    pObj = Gia_ManObj( p, iObjId );
    if ( Gia_ObjIsAnd(pObj) )
    {
        Gia_ManCollectObjs_rec( p, Gia_ObjFaninId0p(p, pObj), vObjs, Limit );
        if ( Vec_IntSize(vObjs) == Limit )
            return;
        Gia_ManCollectObjs_rec( p, Gia_ObjFaninId1p(p, pObj), vObjs, Limit );
        if ( Vec_IntSize(vObjs) == Limit )
            return;
    }
    Vec_IntPush( vObjs, iObjId );
}
unsigned * Gia_ManComputePoTruthTables( Gia_Man_t * p, int nBytesMax )
{
    int nVars = Gia_ManPiNum(p);
    int nTruthWords = Abc_TruthWordNum( nVars );
    int nTruths = nBytesMax / (sizeof(unsigned) * nTruthWords);
    int nTotalNodes = 0, nRounds = 0;
    Vec_Int_t * vObjs;
    Gia_Obj_t * pObj;
    clock_t clk = clock();
    int i;
    printf( "Var = %d. Words = %d. Truths = %d.\n", nVars, nTruthWords, nTruths );
    vObjs = Vec_IntAlloc( nTruths );
    Gia_ManIncrementTravId( p );
    Gia_ManForEachPo( p, pObj, i )
    {
        Gia_ManCollectObjs_rec( p, Gia_ObjFaninId0p(p, pObj), vObjs, nTruths );
        if ( Vec_IntSize(vObjs) == nTruths )
        {
            nRounds++;
//            printf( "%d ", i );
            nTotalNodes += Vec_IntSize( vObjs );
            Vec_IntClear( vObjs );
            Gia_ManIncrementTravId( p );
        }
    }
//    printf( "\n" );
    nTotalNodes += Vec_IntSize( vObjs );
    Vec_IntFree( vObjs );

    printf( "Rounds = %d. Objects = %d. Total = %d.   ", nRounds, Gia_ManObjNum(p), nTotalNodes );
    Abc_PrintTime( 1, "Time", clock() - clk );

    return NULL;
}


/**Function*************************************************************

  Synopsis    [Computing the truth table of one PO.]

  Description [The truth table should be used (or saved into the user's
  storage) before this procedure is called next time!]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ObjComputeTruthTable_rec( Gia_Man_t * p, Gia_Obj_t * pObj )
{
    word * pTruth0, * pTruth1, * pTruth, * pTruthL;
    if ( Gia_ObjIsTravIdCurrent(p, pObj) )
        return pObj->Value;
    Gia_ObjSetTravIdCurrent(p, pObj);
    assert( Gia_ObjIsAnd(pObj) );
    pTruth0 = Vec_WrdArray(p->vTtMemory) + p->nTtWords * Gia_ObjComputeTruthTable_rec( p, Gia_ObjFanin0(pObj) );
    pTruth1 = Vec_WrdArray(p->vTtMemory) + p->nTtWords * Gia_ObjComputeTruthTable_rec( p, Gia_ObjFanin1(pObj) );
    assert( p->nTtWords * p->iTtNum < Vec_WrdSize(p->vTtMemory) );
    pTruth  = Vec_WrdArray(p->vTtMemory) + p->nTtWords * p->iTtNum++;
    pTruthL = Vec_WrdArray(p->vTtMemory) + p->nTtWords * p->iTtNum;
    if ( Gia_ObjFaninC0(pObj) )
    {
        if ( Gia_ObjFaninC1(pObj) )
            while ( pTruth < pTruthL )
                *pTruth++ = ~*pTruth0++ & ~*pTruth1++;
        else
            while ( pTruth < pTruthL )
                *pTruth++ = ~*pTruth0++ &  *pTruth1++;
    }
    else
    {
        if ( Gia_ObjFaninC1(pObj) )
            while ( pTruth < pTruthL )
                *pTruth++ =  *pTruth0++ & ~*pTruth1++;
        else
            while ( pTruth < pTruthL )
                *pTruth++ =  *pTruth0++ &  *pTruth1++;
    }
    return p->iTtNum-1;
}
unsigned * Gia_ObjComputeTruthTable( Gia_Man_t * p, Gia_Obj_t * pObj )
{
    Gia_Obj_t * pTemp;
    word * pTruth;
    int i, k;
    // this procedure works only for primary outputs
    if ( p->vTtMemory == NULL )
    {
        word Truth6[7] = {
            0x0000000000000000,
            0xAAAAAAAAAAAAAAAA,
            0xCCCCCCCCCCCCCCCC,
            0xF0F0F0F0F0F0F0F0,
            0xFF00FF00FF00FF00,
            0xFFFF0000FFFF0000,
            0xFFFFFFFF00000000
        };
        p->nTtVars   = Gia_ManPiNum( p );
        p->nTtWords  = (p->nTtVars <= 6 ? 1 : (1 << (p->nTtVars - 6)));
        p->vTtMemory = Vec_WrdStart( p->nTtWords * 256 );
        for ( i = 0; i < 7; i++ )
            for ( k = 0; k < p->nTtWords; k++ )
                Vec_WrdWriteEntry( p->vTtMemory, i * p->nTtWords + k, Truth6[i] );
    }
    else
    {
        // make sure the number of primary inputs did not change 
        // since the truth table computation storage was prepared
        assert( p->nTtVars == Gia_ManPiNum(p) );
    }
    // mark const and PIs
    Gia_ManIncrementTravId( p );
    Gia_ObjSetTravIdCurrent( p, Gia_ManConst0(p) );
    Gia_ManConst0(p)->Value = 0;
    Gia_ManForEachPi( p, pTemp, i )
    {
        Gia_ObjSetTravIdCurrent( p, pTemp );
        pTemp->Value = i+1;
    }
    p->iTtNum = 7;
    // compute truth table for the fanin node
    if ( Gia_ObjIsCo(pObj) )
    {
        pTruth = Vec_WrdArray(p->vTtMemory) + p->nTtWords * Gia_ObjComputeTruthTable_rec(p, Gia_ObjFanin0(pObj));
        // complement if needed
        if ( Gia_ObjFaninC0(pObj) )
        {
            word * pTemp = pTruth;
            assert( p->nTtWords * p->iTtNum < Vec_WrdSize(p->vTtMemory) );
            pTruth = Vec_WrdArray(p->vTtMemory) + p->nTtWords * p->iTtNum;
            for ( k = 0; k < p->nTtWords; k++ )
                pTruth[k] = ~pTemp[k];
        }
    }
    else
        pTruth = Vec_WrdArray(p->vTtMemory) + p->nTtWords * Gia_ObjComputeTruthTable_rec(p, pObj);
    return (unsigned *)pTruth;
}

/**Function*************************************************************

  Synopsis    [Testing truth table computation.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ObjComputeTruthTableTest( Gia_Man_t * p )
{
    Gia_Obj_t * pObj;
    unsigned * pTruth;
    clock_t clk = clock();
    int i;
    Gia_ManForEachPo( p, pObj, i )
    {
        pTruth = Gia_ObjComputeTruthTable( p, pObj );
//        Extra_PrintHex( stdout, pTruth, Gia_ManPiNum(p) ); printf( "\n" );
    }
    Abc_PrintTime( 1, "Time", clock() - clk );
}


////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

