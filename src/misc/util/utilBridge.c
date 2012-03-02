/**CFile****************************************************************

  FileName    [utilBridge.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName []

  Synopsis    []

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: utilBridge.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "abc_global.h"
#include "src/aig/gia/gia.h"
#include "src/misc/vec/vec.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

extern void Gia_WriteAigerEncodeStr( Vec_Str_t * vStr, unsigned x );

extern unsigned Gia_ReadAigerDecode( unsigned char ** ppPos );


////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Str_t * Gia_ManToBridgeVec( FILE * pFile, Gia_Man_t * p )
{
    Vec_Str_t * vBuffer;
    Gia_Obj_t * pObj;
    int nNodes = 0, i, uLit, uLit0, uLit1; 
    // set the node numbers to be used in the output file
    Gia_ManConst0(p)->Value = nNodes++;
    Gia_ManForEachCi( p, pObj, i )
        pObj->Value = nNodes++;
    Gia_ManForEachAnd( p, pObj, i )
        pObj->Value = nNodes++;

    // write the header "M I L O A" where M = I + L + A
    vBuffer = Vec_StrAlloc( 3*Gia_ManObjNum(p) );
    Vec_StrPrintStr( vBuffer, "aig " );
    Vec_StrPrintNum( vBuffer, Gia_ManCandNum(p) );
    Vec_StrPrintStr( vBuffer, " " );
    Vec_StrPrintNum( vBuffer, Gia_ManPiNum(p) );
    Vec_StrPrintStr( vBuffer, " " );
    Vec_StrPrintNum( vBuffer, Gia_ManRegNum(p) );
    Vec_StrPrintStr( vBuffer, " " );
    Vec_StrPrintNum( vBuffer, Gia_ManPoNum(p) );
    Vec_StrPrintStr( vBuffer, " " );
    Vec_StrPrintNum( vBuffer, Gia_ManAndNum(p) );
    Vec_StrPrintStr( vBuffer, "\n" );

    // write latch drivers
    Gia_ManForEachRi( p, pObj, i )
    {
        uLit = Abc_Var2Lit( Gia_ObjValue(Gia_ObjFanin0(pObj)), Gia_ObjFaninC0(pObj) );
        Vec_StrPrintNum( vBuffer, uLit );
        Vec_StrPrintStr( vBuffer, "\n" );
    }

    // write PO drivers
    Gia_ManForEachPo( p, pObj, i )
    {
        uLit = Abc_Var2Lit( Gia_ObjValue(Gia_ObjFanin0(pObj)), Gia_ObjFaninC0(pObj) );
        Vec_StrPrintNum( vBuffer, uLit );
        Vec_StrPrintStr( vBuffer, "\n" );
    }
    // write the nodes into the buffer
    Gia_ManForEachAnd( p, pObj, i )
    {
        uLit  = Abc_Var2Lit( Gia_ObjValue(pObj), 0 );
        uLit0 = Abc_Var2Lit( Gia_ObjValue(Gia_ObjFanin0(pObj)), Gia_ObjFaninC0(pObj) );
        uLit1 = Abc_Var2Lit( Gia_ObjValue(Gia_ObjFanin1(pObj)), Gia_ObjFaninC1(pObj) );
        assert( uLit0 != uLit1 );
        if ( uLit0 > uLit1 )
        {
            int Temp = uLit0;
            uLit0 = uLit1;
            uLit1 = Temp;
        }
        Gia_WriteAigerEncodeStr( vBuffer, uLit  - uLit1 );
        Gia_WriteAigerEncodeStr( vBuffer, uLit1 - uLit0 );
    }
    Vec_StrPrintStr( vBuffer, "c" );
    return vBuffer;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManToBridge( FILE * pFile, Gia_Man_t * pMan )
{
    Vec_Str_t * vBuffer;
    vBuffer = Gia_ManToBridgeVec( pFile, pMan );
    fwrite( Vec_StrArray(vBuffer), 1, Vec_StrSize(vBuffer), pFile );
    Vec_StrFree( vBuffer );
    return 0;
}



/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t *  Gia_ManFromBridgeReadBody( int Size, unsigned char * pBuffer, Vec_Int_t ** pvInits )
{
    int fHash = 0;
    Vec_Int_t * vLits, * vInits;
    Gia_Man_t * p = NULL;
    unsigned char * pBufferPivot, * pBufferEnd = pBuffer + Size;
    int i, nInputs, nFlops, nGates, nProps;
    unsigned iFan0, iFan1;

    nInputs = Gia_ReadAigerDecode( &pBuffer );
    nFlops  = Gia_ReadAigerDecode( &pBuffer );
    nGates  = Gia_ReadAigerDecode( &pBuffer );

    vLits = Vec_IntAlloc( 1000 );
    Vec_IntPush( vLits, -1 );
    Vec_IntPush( vLits,  1 );

    // start the AIG package
    p = Gia_ManStart( nInputs + nFlops * 2 + nGates + 1 + 1 );
    p->pName = Abc_UtilStrsav( "temp" );

    // create PIs
    for ( i = 0; i < nInputs; i++ )
        Vec_IntPush( vLits, Gia_ManAppendCi( p ) );

    // create flop outputs
    for ( i = 0; i < nFlops; i++ )
        Vec_IntPush( vLits, Gia_ManAppendCi( p ) );

    // create nodes
    if ( fHash )
        Gia_ManHashAlloc( p );
    for ( i = 0; i < nGates; i++ )
    {
        iFan0 = Gia_ReadAigerDecode( &pBuffer );
        iFan1 = Gia_ReadAigerDecode( &pBuffer );
        assert( (iFan0 & 1)==0 );
        iFan0 >>= 1;

        iFan0 = Abc_LitNotCond( Vec_IntEntry(vLits, iFan0 >> 1), iFan0 & 1 );
        iFan1 = Abc_LitNotCond( Vec_IntEntry(vLits, iFan1 >> 1), iFan0 & 1 );
        if ( fHash )
            Vec_IntPush( vLits, Gia_ManHashAnd(p, iFan0, iFan1) );
        else 
            Vec_IntPush( vLits, Gia_ManAppendAnd(p, iFan0, iFan1) );

    }
    if ( fHash )
        Gia_ManHashStop( p );

    // remember where flops begin
    pBufferPivot = pBuffer;
    // stroll through flops
    for ( i = 0; i < nFlops; i++ )
        Gia_ReadAigerDecode( &pBuffer );

    // create POs
    nProps = Gia_ReadAigerDecode( &pBuffer );
    assert( nProps == 1 );
    for ( i = 0; i < nProps; i++ )
    {
        iFan0 = Gia_ReadAigerDecode( &pBuffer );
        iFan0 = Abc_LitNotCond( Vec_IntEntry(vLits, iFan0 >> 1), iFan0 & 1 );
        Gia_ManAppendCo( p, iFan0 );
    }
    // make sure the end of buffer is reached
    assert( pBufferEnd == pBuffer );

    // resetting to flops
    pBuffer = pBufferPivot;
    vInits = Vec_IntAlloc( nFlops );
    for ( i = 0; i < nFlops; i++ )
    {
        iFan0 = Gia_ReadAigerDecode( &pBuffer );
        Vec_IntPush( vInits, iFan0 & 3 ); // 0 = X value; 1 = not used; 2 = false; 3 = true
        iFan0 >>= 2;
        iFan0 = Abc_LitNotCond( Vec_IntEntry(vLits, iFan0 >> 1), iFan0 & 1 );
        Gia_ManAppendCo( p, iFan0 );
    }
    Gia_ManSetRegNum( p, nFlops );
    Vec_IntFree( vLits );

    // remove wholes in the node list
    if ( fHash )
    {
        Gia_Man_t * pTemp;
        p = Gia_ManCleanup( pTemp = p );
        Gia_ManStop( pTemp );
    }

    // return
    if ( pvInits )
        *pvInits = vInits;
    else
        Vec_IntFree( vInits );
    return p;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManFromBridgeReadPackage( FILE * pFile, int * pType, int * pSize, unsigned char ** ppBuffer )
{
    char Temp[24];
    int RetValue;
    RetValue = fread( Temp, 24, 1, pFile );
    if ( RetValue != 1 )
    {
        printf( "Gia_ManFromBridgeReadPackage();  Error 1: Something is wrong!\n" );
        return 0;
    }
    Temp[6] = 0;
    Temp[23]= 0;

    *pType = atoi( Temp );
    *pSize = atoi( Temp + 7 );

    *ppBuffer = ABC_ALLOC( char, *pSize );
    RetValue = fread( *ppBuffer, *pSize, 1, pFile );
    if ( RetValue != 1 && *pSize != 0 )
    {
        ABC_FREE( *ppBuffer );
        printf( "Gia_ManFromBridgeReadPackage();  Error 2: Something is wrong!\n" );
        return 0;
    }
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t * Gia_ManFromBridge( FILE * pFile, Vec_Int_t ** pvInit )
{
    unsigned char * pBuffer;
    int Type, Size, RetValue;
    Gia_Man_t * p = NULL;

    RetValue = Gia_ManFromBridgeReadPackage( pFile, &Type, &Size, &pBuffer );
    ABC_FREE( pBuffer );
    if ( !RetValue )
        return NULL; 

    RetValue = Gia_ManFromBridgeReadPackage( pFile, &Type, &Size, &pBuffer );
    if ( !RetValue )
        return NULL;

    p = Gia_ManFromBridgeReadBody( Size, pBuffer, pvInit ); 
    ABC_FREE( pBuffer );
    if ( p == NULL )
        return NULL;

    RetValue = Gia_ManFromBridgeReadPackage( pFile, &Type, &Size, &pBuffer );
    ABC_FREE( pBuffer );
    if ( !RetValue )
        return NULL;

    return p;
}

/*
    {
        extern void Gia_ManFromBridgeTest( char * pFileName );
        Gia_ManFromBridgeTest( "C:\\_projects\\abc\\_TEST\\bug\\65\\par.dump" );

    }
*/

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManFromBridgeTest( char * pFileName )
{
    Gia_Man_t * p;
    FILE * pFile = fopen( pFileName, "rb" );
    if ( pFile == NULL )
    {
        printf( "Cannot open input file \"%s\".\n", pFileName );
        return;
    }

    p = Gia_ManFromBridge( pFile, NULL );
    fclose ( pFile );

    Gia_ManPrintStats( p, 0, 0 );
//    Gia_WriteAiger( p, "temp.aig", 0, 0 );
    Gia_ManStop( p );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END
