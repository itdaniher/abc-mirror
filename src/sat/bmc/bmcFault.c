/**CFile****************************************************************

  FileName    [bmcFault.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [SAT-based bounded model checking.]

  Synopsis    [Checking for functional faults.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: bmcFault.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "bmc.h"
#include "sat/cnf/cnf.h"
#include "sat/bsat/satStore.h"
#include "aig/gia/giaAig.h"

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
static inline Cnf_Dat_t * Cnf_DeriveGiaRemapped( Gia_Man_t * p )
{
    Cnf_Dat_t * pCnf;
    Aig_Man_t * pAig = Gia_ManToAigSimple( p );
    pAig->nRegs = 0;
    pCnf = Cnf_Derive( pAig, Aig_ManCoNum(pAig) );
    Aig_ManStop( pAig );
    return pCnf;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Cnf_DataLiftGia( Cnf_Dat_t * p, Gia_Man_t * pGia, int nVarsPlus )
{
    Gia_Obj_t * pObj;
    int v;
    Gia_ManForEachObj( pGia, pObj, v )
        if ( p->pVarNums[Gia_ObjId(pGia, pObj)] >= 0 )
            p->pVarNums[Gia_ObjId(pGia, pObj)] += nVarsPlus;
    for ( v = 0; v < p->nLiterals; v++ )
        p->pClauses[0][v] += 2*nVarsPlus;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t * Gia_ManFaultUnfold( Gia_Man_t * p, int fUseMuxes, int fComplVars )
{
    Gia_Man_t * pNew, * pTemp;
    Gia_Obj_t * pObj;
    int i, iCtrl, iThis;
    pNew = Gia_ManStart( (2 + 3 * fUseMuxes) * Gia_ManObjNum(p) );
    pNew->pName = Abc_UtilStrsav( p->pName );
    Gia_ManHashAlloc( pNew );
    Gia_ManConst0(p)->Value = 0;
    // add first timeframe
    Gia_ManForEachRo( p, pObj, i )
        pObj->Value = Gia_ManAppendCi( pNew );
    Gia_ManForEachPi( p, pObj, i )
        pObj->Value = Gia_ManAppendCi( pNew );
    Gia_ManForEachAnd( p, pObj, i )
        pObj->Value = Gia_ManHashAnd( pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj) );
    Gia_ManForEachCo( p, pObj, i )
        pObj->Value = Gia_ObjFanin0Copy(pObj);
    // add second timeframe
    Gia_ManForEachRo( p, pObj, i )
        pObj->Value = Gia_ObjRoToRi(p, pObj)->Value;
    Gia_ManForEachPi( p, pObj, i )
        pObj->Value = Gia_ManAppendCi( pNew );
    Gia_ManForEachAnd( p, pObj, i )
    {
        iCtrl = Abc_LitNotCond( Gia_ManAppendCi(pNew), fComplVars );
        iThis = Gia_ManHashAnd( pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj) );
        if ( fUseMuxes )
            pObj->Value = Gia_ManHashMux( pNew, iCtrl, pObj->Value, iThis );
        else
            pObj->Value = iThis;
    }
    Gia_ManForEachCo( p, pObj, i )
        pObj->Value = Gia_ManAppendCo( pNew, Gia_ObjFanin0Copy(pObj) );
    pNew = Gia_ManCleanup( pTemp = pNew );
    Gia_ManStop( pTemp );
    assert( Gia_ManPiNum(pNew) == Gia_ManRegNum(p) + 2 * Gia_ManPiNum(p) + Gia_ManAndNum(p) );
    return pNew;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t * Gia_ManStuckAtUnfold( Gia_Man_t * p, int fUseFaults, int fComplVars )
{
    Gia_Man_t * pNew, * pTemp;
    Gia_Obj_t * pObj;
    int i, iCtrl0, iCtrl1;
    pNew = Gia_ManStart( 3 * Gia_ManObjNum(p) );
    pNew->pName = Abc_UtilStrsav( p->pName );
    Gia_ManHashAlloc( pNew );
    Gia_ManConst0(p)->Value = 0;
    Gia_ManForEachCi( p, pObj, i )
        pObj->Value = Gia_ManAppendCi( pNew );
    Gia_ManForEachAnd( p, pObj, i )
    {
        pObj->Value = Gia_ManHashAnd( pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj) );
        iCtrl0 = Abc_LitNotCond( Gia_ManAppendCi(pNew), fComplVars );
        iCtrl1 = Abc_LitNotCond( Gia_ManAppendCi(pNew), fComplVars );
        if ( fUseFaults )
        {
            pObj->Value = Gia_ManHashAnd( pNew, Abc_LitNot(iCtrl0), pObj->Value );
            pObj->Value = Gia_ManHashOr( pNew, iCtrl1, pObj->Value );
        }
    }
    Gia_ManForEachCo( p, pObj, i )
        pObj->Value = Gia_ManAppendCo( pNew, Gia_ObjFanin0Copy(pObj) );
    pNew = Gia_ManCleanup( pTemp = pNew );
    Gia_ManStop( pTemp );
    assert( Gia_ManPiNum(pNew) == Gia_ManCiNum(p) + 2 * Gia_ManAndNum(p) );
    return pNew;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t * Gia_ManFlipUnfold( Gia_Man_t * p, int fUseFaults, int fComplVars )
{
    Gia_Man_t * pNew, * pTemp;
    Gia_Obj_t * pObj;
    int i, iCtrl0;
    pNew = Gia_ManStart( 3 * Gia_ManObjNum(p) );
    pNew->pName = Abc_UtilStrsav( p->pName );
    Gia_ManHashAlloc( pNew );
    Gia_ManConst0(p)->Value = 0;
    Gia_ManForEachCi( p, pObj, i )
        pObj->Value = Gia_ManAppendCi( pNew );
    Gia_ManForEachAnd( p, pObj, i )
    {
        pObj->Value = Gia_ManHashAnd( pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj) );
        iCtrl0 = Abc_LitNotCond( Gia_ManAppendCi(pNew), fComplVars );
        if ( fUseFaults )
            pObj->Value = Gia_ManHashXor( pNew, iCtrl0, pObj->Value );
    }
    Gia_ManForEachCo( p, pObj, i )
        pObj->Value = Gia_ManAppendCo( pNew, Gia_ObjFanin0Copy(pObj) );
    pNew = Gia_ManCleanup( pTemp = pNew );
    Gia_ManStop( pTemp );
    assert( Gia_ManPiNum(pNew) == Gia_ManCiNum(p) + Gia_ManAndNum(p) );
    return pNew;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t * Gia_ManFaultCofactor( Gia_Man_t * p, Vec_Int_t * vValues )
{
    Gia_Man_t * pNew, * pTemp;
    Gia_Obj_t * pObj;
    int i;
    pNew = Gia_ManStart( Gia_ManObjNum(p) );
    pNew->pName = Abc_UtilStrsav( p->pName );
    Gia_ManHashAlloc( pNew );
    Gia_ManConst0(p)->Value = 0;
    Gia_ManForEachPi( p, pObj, i )
    {
        pObj->Value = Gia_ManAppendCi( pNew );
        if ( i < Vec_IntSize(vValues) )
            pObj->Value = Vec_IntEntry( vValues, i );
    }
    Gia_ManForEachAnd( p, pObj, i )
        pObj->Value = Gia_ManHashAnd( pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj) );
    Gia_ManForEachCo( p, pObj, i )
        pObj->Value = Gia_ManAppendCo( pNew, Gia_ObjFanin0Copy(pObj) );
    pNew = Gia_ManCleanup( pTemp = pNew );
    Gia_ManStop( pTemp );
    assert( Gia_ManPiNum(pNew) == Gia_ManPiNum(p) );
    return pNew;

}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManDumpTests( Vec_Int_t * vTests, int nIter, char * pFileName )
{
    FILE * pFile = fopen( pFileName, "wb" );
    int i, k, v, nVars = Vec_IntSize(vTests) / nIter;
    assert( Vec_IntSize(vTests) % nIter == 0 );
    for ( v = i = 0; i < nIter; i++, fprintf(pFile, "\n") )
        for ( k = 0; k < nVars; k++ )
            fprintf( pFile, "%d", Vec_IntEntry(vTests, v++) );
    fclose( pFile );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManPrintResults( Gia_Man_t * p, sat_solver * pSat, int nIter, abctime clk )
{
    FILE * pTable = fopen( "fault_stats.txt", "a+" );

    fprintf( pTable, "%s ", Gia_ManName(p) );
    fprintf( pTable, "%d ", Gia_ManPiNum(p) );
    fprintf( pTable, "%d ", Gia_ManPoNum(p) );
    fprintf( pTable, "%d ", Gia_ManRegNum(p) );
    fprintf( pTable, "%d ", Gia_ManAndNum(p) );

    fprintf( pTable, "%d ", sat_solver_nvars(pSat) );
    fprintf( pTable, "%d ", sat_solver_nclauses(pSat) );
    fprintf( pTable, "%d ", sat_solver_nconflicts(pSat) );

    fprintf( pTable, "%d ", nIter );
    fprintf( pTable, "%.2f", 1.0*clk/CLOCKS_PER_SEC );
    fprintf( pTable, "\n" );
    fclose( pTable );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManFaultAddOne( Gia_Man_t * pM, Cnf_Dat_t * pCnf, sat_solver * pSat, Vec_Int_t * vLits, int nFuncVars )
{
    Gia_Man_t * pC;
    Cnf_Dat_t * pCnf2;
    Gia_Obj_t * pObj;
    int i, Lit;
    // derive the cofactor
    pC = Gia_ManFaultCofactor( pM, vLits );
    // derive new CNF
    pCnf2 = Cnf_DeriveGiaRemapped( pC );
    Cnf_DataLiftGia( pCnf2, pC, sat_solver_nvars(pSat) );
    // add timeframe clauses
    for ( i = 0; i < pCnf2->nClauses; i++ )
        if ( !sat_solver_addclause( pSat, pCnf2->pClauses[i], pCnf2->pClauses[i+1] ) )
            assert( 0 );
    // add constraint clauses
    Gia_ManForEachPo( pC, pObj, i )
    {
        Lit = Abc_Var2Lit( pCnf2->pVarNums[Gia_ObjId(pC, pObj)], 1 );
        if ( !sat_solver_addclause( pSat, &Lit, &Lit+1 ) )
            assert( 0 );
    }
    // add connection clauses
    Gia_ManForEachPi( pM, pObj, i )
        if ( i >= nFuncVars )
            sat_solver_add_buffer( pSat, pCnf->pVarNums[Gia_ObjId(pM, pObj)], pCnf2->pVarNums[Gia_ObjId(pC, Gia_ManPi(pC, i))], 0 );
    Cnf_DataFree( pCnf2 );
    Gia_ManStop( pC );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_ManGetTestPatterns( char * pFileName )
{
    FILE * pFile = fopen( pFileName, "rb" );
    Vec_Int_t * vTests; int c;
    if ( pFile == NULL )
    {
        printf( "Cannot open input file \"%s\".\n", pFileName );
        return NULL;
    }
    vTests = Vec_IntAlloc( 10000 );
    while ( (c = fgetc(pFile)) != EOF )
    {
        if ( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
            continue;
        if ( c != '0' && c != '1' )
        {
            printf( "Wring symbol (%c) in the input file.\n", c );
            Vec_IntFreeP( &vTests );
            break;
        }
        Vec_IntPush( vTests, c - '0' );
    }
    fclose( pFile );
    return vTests;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManFaultTest( Gia_Man_t * p, char * pFileName, int Algo, int fComplVars, int nTimeOut, int fDump, int fVerbose )
{
    int nIterMax = 1000000;
    int i, Iter, status, nFuncVars = -1;
    abctime clkSat = 0, clkTotal = Abc_Clock();
    Vec_Int_t * vLits, * vTests;
    Gia_Man_t * p0, * p1, * pM;
    Gia_Obj_t * pObj;
    Cnf_Dat_t * pCnf;
    sat_solver * pSat;

    // select algorithm
    if ( Algo == 1 )
        nFuncVars = Gia_ManRegNum(p) + 2 * Gia_ManPiNum(p);
    else if ( Algo == 2 )
        nFuncVars = Gia_ManCiNum(p);
    else if ( Algo == 3 )
        nFuncVars = Gia_ManCiNum(p);
    else
    {
        printf( "Unregnized algorithm (%d).\n", Algo );
        return;
    }

    // collect test patterns from file
    if ( pFileName )
        vTests = Gia_ManGetTestPatterns( pFileName );
    else
        vTests = Vec_IntAlloc( 10000 );
    if ( vTests == NULL )
        return;
    if ( Vec_IntSize(vTests) % nFuncVars != 0 )
    {
        printf( "The number of symbols in the input patterns (%d) does not divide evenly on the number of test variables (%d).\n", Vec_IntSize(vTests), nFuncVars );
        Vec_IntFree( vTests );
        return;
    }

    // select algorithm
    if ( Algo == 1 )
    {
        assert( Gia_ManRegNum(p) > 0 );
        p0 = Gia_ManFaultUnfold( p, 0, fComplVars );
        p1 = Gia_ManFaultUnfold( p, 1, fComplVars );
    }
    else if ( Algo == 2 )
    {
        p0 = Gia_ManStuckAtUnfold( p, 0, fComplVars );
        p1 = Gia_ManStuckAtUnfold( p, 1, fComplVars );
    }
    else if ( Algo == 3 )
    {
        p0 = Gia_ManFlipUnfold( p, 0, fComplVars );
        p1 = Gia_ManFlipUnfold( p, 1, fComplVars );
    }
    else
    {
        printf( "Unregnized algorithm (%d).\n", Algo );
        return;
    }

    // create miter
    pM = Gia_ManMiter( p0, p1, 0, 0, 0, 0, 0 );
    pCnf = Cnf_DeriveGiaRemapped( pM );
    Gia_ManStop( p0 );
    Gia_ManStop( p1 );

    // start the SAT solver
    pSat = sat_solver_new();
    sat_solver_setnvars( pSat, pCnf->nVars );
    sat_solver_set_runtime_limit( pSat, nTimeOut ? nTimeOut * CLOCKS_PER_SEC + Abc_Clock(): 0 );
    // add timeframe clauses
    for ( i = 0; i < pCnf->nClauses; i++ )
        if ( !sat_solver_addclause( pSat, pCnf->pClauses[i], pCnf->pClauses[i+1] ) )
            assert( 0 );

    // add one large OR clause
    vLits = Vec_IntAlloc( Gia_ManCoNum(p) );
    Gia_ManForEachCo( pM, pObj, i )
        Vec_IntPush( vLits, Abc_Var2Lit(pCnf->pVarNums[Gia_ObjId(pM, pObj)], 0) );
    sat_solver_addclause( pSat, Vec_IntArray(vLits), Vec_IntArray(vLits) + Vec_IntSize(vLits) );

    // add available test-patterns
    if ( Vec_IntSize(vTests) > 0 )
    {
        int nTests = Vec_IntSize(vTests) / nFuncVars;
        assert( Vec_IntSize(vTests) % nFuncVars == 0 );
        printf( "Reading %d pre-computed test patterns from file \"%s\".\n", Vec_IntSize(vTests) / nFuncVars, pFileName );
        for ( Iter = 0; Iter < nTests; Iter++ )
        {
            abctime clk = Abc_Clock();
            status = sat_solver_solve( pSat, NULL, NULL, (ABC_INT64_T)0, (ABC_INT64_T)0, (ABC_INT64_T)0, (ABC_INT64_T)0 );
            clkSat += Abc_Clock() - clk;
            // get pattern
            Vec_IntClear( vLits );
            for ( i = 0; i < nFuncVars; i++ )
                Vec_IntPush( vLits, Vec_IntEntry(vTests, Iter*nFuncVars + i) );
            Gia_ManFaultAddOne( pM, pCnf, pSat, vLits, nFuncVars );
            if ( fVerbose )
            {
                printf( "Iter%6d : ",       Iter );
                printf( "Var =%10d  ",      sat_solver_nvars(pSat) );
                printf( "Clause =%10d  ",   sat_solver_nclauses(pSat) );
                printf( "Conflict =%10d  ", sat_solver_nconflicts(pSat) );
                //Abc_PrintTime( 1, "Time", clkSat );
                ABC_PRTr( "Solver time", clkSat );
            }
        }
    }

    // iterate through the test vectors
    for ( Iter = Vec_IntSize(vTests) / nFuncVars; Iter < nIterMax; Iter++ )
    {
        abctime clk = Abc_Clock();
        status = sat_solver_solve( pSat, NULL, NULL, (ABC_INT64_T)0, (ABC_INT64_T)0, (ABC_INT64_T)0, (ABC_INT64_T)0 );
        clkSat += Abc_Clock() - clk;
        if ( fVerbose )
        {
            printf( "Iter%6d : ",       Iter );
            printf( "Var =%10d  ",      sat_solver_nvars(pSat) );
            printf( "Clause =%10d  ",   sat_solver_nclauses(pSat) );
            printf( "Conflict =%10d  ", sat_solver_nconflicts(pSat) );
            //Abc_PrintTime( 1, "Time", clkSat );
            ABC_PRTr( "Solver time", clkSat );
        }
        if ( status == l_Undef )
        {
            if ( fVerbose )
                printf( "\n" );
            printf( "Timeout reached after %d seconds and %d iterations.  ", nTimeOut, Iter );
            break;
        }
        if ( status == l_False )
        {
            if ( fVerbose )
                printf( "\n" );
            printf( "The problem is UNSAT after %d iterations.  ", Iter );
            break;
        }
        assert( status == l_True );
        // collect SAT assignment
        Vec_IntClear( vLits );
        Gia_ManForEachPi( pM, pObj, i )
            if ( i < nFuncVars )
                Vec_IntPush( vLits, sat_solver_var_value(pSat, pCnf->pVarNums[Gia_ObjId(pM, pObj)]) );
        Vec_IntAppend( vTests, vLits );
        // add constraint
        Gia_ManFaultAddOne( pM, pCnf, pSat, vLits, nFuncVars );
    }
    // print results
//    if ( status == l_False )
//        Gia_ManPrintResults( p, pSat, Iter, Abc_Clock() - clkTotal );
    // cleanup
    Vec_IntFree( vLits );
    sat_solver_delete( pSat );
    Cnf_DataFree( pCnf );
    Gia_ManStop( pM );
    Abc_PrintTime( 1, "Total runtime", Abc_Clock() - clkTotal );
    // dump the test suite
    if ( fDump )
    {
        char * pFileName = "tests.txt";
        Gia_ManDumpTests( vTests, Iter, pFileName );
        printf( "Dumping %d computed test patterns into file \"%s\".\n", Vec_IntSize(vTests) / nFuncVars, pFileName );
    }
    Vec_IntFree( vTests );
}



////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

