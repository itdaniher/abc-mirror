/**CFile****************************************************************

  FileName    [ifTune.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [FPGA mapping based on priority cuts.]

  Synopsis    [Library tuning.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - November 21, 2006.]

  Revision    [$Id: ifTune.c,v 1.00 2006/11/21 00:00:00 alanmi Exp $]

***********************************************************************/

#include "if.h"
#include "aig/gia/giaAig.h"
#include "sat/bsat/satStore.h"
#include "sat/cnf/cnf.h"
#include "misc/extra/extra.h"
#include "bool/kit/kit.h"

ABC_NAMESPACE_IMPL_START

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////
 
#define IFN_INS    11
#define IFN_WRD  (IFN_INS > 6 ? 1 << (IFN_INS-6) : 1)
#define IFN_PAR  1024

// network types
typedef enum { 
    IF_DSD_NONE = 0,               // 0:  unknown
    IF_DSD_CONST0,                 // 1:  constant
    IF_DSD_VAR,                    // 2:  variable
    IF_DSD_AND,                    // 3:  AND
    IF_DSD_XOR,                    // 4:  XOR
    IF_DSD_MUX,                    // 5:  MUX
    IF_DSD_PRIME                   // 6:  PRIME
} If_DsdType_t;

// object types
static char * Ifn_Symbs[16] = { 
    NULL,                          // 0:  unknown
    "const",                       // 1:  constant
    "var",                         // 2:  variable
    "()",                          // 3:  AND
    "[]",                          // 4:  XOR
    "<>",                          // 5:  MUX
    "{}"                           // 6:  PRIME
};

typedef struct Ifn_Obj_t_  Ifn_Obj_t;
typedef struct Ifn_Ntk_t_  Ifn_Ntk_t;

struct Ifn_Obj_t_
{
    unsigned               Type    :  3;      // node type
    unsigned               nFanins :  5;      // fanin counter
    unsigned               iFirst  :  8;      // first parameter
    unsigned               Var     : 16;      // current variable
    int                    Fanins[IFN_INS];   // fanin IDs
};
struct Ifn_Ntk_t_ 
{
    // cell structure
    int                    nInps;             // inputs
    int                    nObjs;             // objects
    Ifn_Obj_t              Nodes[2*IFN_INS];  // nodes
    // constraints
    int                    pConstr[IFN_INS];  // constraint pairs
    int                    nConstr;           // number of pairs
    // user data
    int                    nVars;             // variables
    int                    nWords;            // truth table words
    int                    nParsVNum;         // selection parameters per variable
    int                    nParsVIni;         // first selection parameter
    int                    nPars;             // total parameters
    word *                 pTruth;            // user truth table
    // matching procedures
    int                    Values[IFN_PAR];            // variable values
    word                   pTtElems[IFN_INS*IFN_WRD];  // elementary truth tables
    word                   pTtObjs[2*IFN_INS*IFN_WRD]; // object truth tables
};

static inline word *       Ifn_ElemTruth( Ifn_Ntk_t * p, int i ) { return p->pTtElems + i * Abc_TtWordNum(p->nInps); }
static inline word *       Ifn_ObjTruth( Ifn_Ntk_t * p, int i )  { return p->pTtObjs + i * p->nWords;                }
 
// variable ordering
// - primary inputs            [0;            p->nInps)
// - internal nodes            [p->nInps;     p->nObjs)
// - configuration params      [p->nObjs;     p->nParsVIni)
// - variable selection params [p->nParsVIni; p->pPars)

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Prepare network to check the given function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Ifn_Prepare( Ifn_Ntk_t * p, word * pTruth, int nVars )
{
    int i, fVerbose = 0;
    assert( nVars <= p->nInps );
    p->pTruth = pTruth;
    p->nVars  = nVars;
    p->nWords = Abc_TtWordNum(nVars);
    p->nPars  = p->nObjs;
    for ( i = p->nInps; i < p->nObjs; i++ )
    {
        if ( p->Nodes[i].Type != IF_DSD_PRIME )
            continue;
        p->Nodes[i].iFirst = p->nPars;
        p->nPars += (1 << p->Nodes[i].nFanins);
        if ( fVerbose )
            printf( "Node %d  Start %d  Vars %d\n", i, p->Nodes[i].iFirst, (1 << p->Nodes[i].nFanins) );
    }
    if ( fVerbose )
        printf( "Groups start %d\n", p->nPars );
    p->nParsVIni = p->nPars;
    p->nParsVNum = Abc_Base2Log(nVars);
    p->nPars    += p->nParsVNum * p->nInps;
    assert( p->nPars <= IFN_PAR );
    memset( p->Values, 0xFF, sizeof(int) * p->nPars );
    return p->nPars;
}
void Ifn_NtkPrint( Ifn_Ntk_t * p )
{
    int i, k; 
    if ( p == NULL )
        printf( "String is empty.\n" );
    if ( p == NULL )
        return;
    for ( i = p->nInps; i < p->nObjs; i++ )
    {
        printf( "%c=", 'a'+i );
        printf( "%c", Ifn_Symbs[p->Nodes[i].Type][0] );
        for ( k = 0; k < (int)p->Nodes[i].nFanins; k++ )
            printf( "%c", 'a'+p->Nodes[i].Fanins[k] );
        printf( "%c", Ifn_Symbs[p->Nodes[i].Type][1] );
        printf( ";" );
    }
    printf( "\n" );
}
int Ifn_NtkLutSizeMax( Ifn_Ntk_t * p )
{
    int i, LutSize = 0;
    for ( i = p->nInps; i < p->nObjs; i++ )
        if ( p->Nodes[i].Type == IF_DSD_PRIME )
            LutSize = Abc_MaxInt( LutSize, (int)p->Nodes[i].nFanins );
    return LutSize;
}
int Ifn_NtkInputNum( Ifn_Ntk_t * p )
{
    return p->nInps;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Ifn_ManStrCheck( char * pStr, int * pnInps, int * pnObjs )
{
    int i, Marks[32] = {0}, MaxVar = 0, MaxDef = 0, RetValue = 1;
    for ( i = 0; pStr[i]; i++ )
    {
        if ( pStr[i] == '=' || pStr[i] == ';' || 
             pStr[i] == '(' || pStr[i] == ')' || 
             pStr[i] == '[' || pStr[i] == ']' || 
             pStr[i] == '<' || pStr[i] == '>' || 
             pStr[i] == '{' || pStr[i] == '}' )
            continue;
        if ( pStr[i] >= 'A' && pStr[i] <= 'Z' )
            continue;
        if ( pStr[i] >= 'a' && pStr[i] <= 'z' )
        {
            if ( pStr[i+1] == '=' )
                Marks[pStr[i] - 'a'] = 2, MaxDef = Abc_MaxInt(MaxDef, pStr[i] - 'a');
            continue;
        }
        printf( "String \"%s\" contains unrecognized symbol (%c).\n", pStr, pStr[i] );
        RetValue = 0;
    }
    if ( !RetValue )
        return 0;
    for ( i = 0; pStr[i]; i++ )
    {
        if ( pStr[i] == '=' || pStr[i] == ';' || 
             pStr[i] == '(' || pStr[i] == ')' || 
             pStr[i] == '[' || pStr[i] == ']' || 
             pStr[i] == '<' || pStr[i] == '>' || 
             pStr[i] == '{' || pStr[i] == '}' )
            continue;
        if ( pStr[i] >= 'A' && pStr[i] <= 'Z' )
            continue;
        if ( pStr[i] >= 'a' && pStr[i] <= 'z' )
        {
            if ( pStr[i+1] != '=' && Marks[pStr[i] - 'a'] != 2 )
                Marks[pStr[i] - 'a'] = 1, MaxVar = Abc_MaxInt(MaxVar, pStr[i] - 'a');
            continue;
        }
        printf( "String \"%s\" contains unrecognized symbol (%c).\n", pStr, pStr[i] );
        RetValue = 0;
    }
    if ( !RetValue )
        return 0;
    MaxVar++;
    MaxDef++;
    for ( i = 0; i < MaxDef; i++ )
        if ( Marks[i] == 0 )
            printf( "String \"%s\" has no symbol (%c).\n", pStr, 'a' + i ), RetValue = 0;
    for ( i = 0; i < MaxVar; i++ )
        if ( Marks[i] == 2 )
            printf( "String \"%s\" has definition of input variable (%c).\n", pStr, 'a' + i ), RetValue = 0;
    for ( i = MaxVar; i < MaxDef; i++ )
        if ( Marks[i] == 1 )
            printf( "String \"%s\" has no definition for internal variable (%c).\n", pStr, 'a' + i ), RetValue = 0;
    if ( !RetValue )
        return 0;
    *pnInps = MaxVar;
    *pnObjs = MaxDef;
    return 1;
}
int Ifn_ErrorMessage( const char * format, ...  )
{
    char * pMessage;
    va_list args;
    va_start( args, format );
    pMessage = vnsprintf( format, args );
    va_end( args );
    printf( "%s", pMessage );
    ABC_FREE( pMessage );
    return 0;
}
int Ifn_NtkParseInt( char * pStr, Ifn_Ntk_t * p )
{
    int i, k, n, f, nFans, iFan;
    if ( !Ifn_ManStrCheck(pStr, &p->nInps, &p->nObjs) )
        return 0;
    if ( p->nInps > IFN_INS )
        return Ifn_ErrorMessage( "The number of variables (%d) exceeds predefined limit (%d). Recompile with different value of IFN_INS.\n", p->nInps, IFN_INS );
    assert( p->nInps > 1 && p->nInps < p->nObjs && p->nInps <= IFN_INS && p->nObjs < 2*IFN_INS );
    for ( i = p->nInps; i < p->nObjs; i++ )
    {
        char Next = 0;
        for ( k = 0; pStr[k]; k++ )
            if ( pStr[k] == 'a' + i && pStr[k+1] == '=' )
                break;
        if ( pStr[k] == 0 )
            return Ifn_ErrorMessage( "Cannot find definition of signal %c.\n", 'a' + i );
        if ( pStr[k+2] == '(' )
            p->Nodes[i].Type = IF_DSD_AND, Next = ')';
        else if ( pStr[k+2] == '[' )
            p->Nodes[i].Type = IF_DSD_XOR, Next = ']';
        else if ( pStr[k+2] == '<' )
            p->Nodes[i].Type = IF_DSD_MUX, Next = '>';
        else if ( pStr[k+2] == '{' )
            p->Nodes[i].Type = IF_DSD_PRIME, Next = '}';
        else 
            return Ifn_ErrorMessage( "Cannot find openning operation symbol in the defition of of signal %c.\n", 'a' + i );
        for ( n = k + 3; pStr[n]; n++ )
            if ( pStr[n] == Next )
                break;
        if ( pStr[n] == 0 )
            return Ifn_ErrorMessage( "Cannot find closing operation symbol in the defition of of signal %c.\n", 'a' + i );
        nFans = n - k - 3;
        if ( nFans < 1 || nFans > 8 )
            return Ifn_ErrorMessage( "Cannot find matching operation symbol in the defition of of signal %c.\n", 'a' + i );
        for ( f = 0; f < nFans; f++ )
        {
            iFan = pStr[k + 3 + f] - 'a';
            if ( iFan < 0 || iFan >= i )
                return Ifn_ErrorMessage( "Fanin number %d is signal %d is out of range.\n", f, 'a' + i );
            p->Nodes[i].Fanins[f] = iFan;
        }
        p->Nodes[i].nFanins = nFans;
    }
    // truth tables
    Abc_TtElemInit2( p->pTtElems, p->nInps );
    // parse constraints
    p->nConstr = 0;
    for ( i = 0; i < p->nInps; i++ )
        for ( k = 0; pStr[k]; k++ )
            if ( pStr[k] == 'A' + i && pStr[k-1] == ';' )
            {
                p->pConstr[p->nConstr++] = ((int)(pStr[k] - 'A') << 16) | (int)(pStr[k+1] - 'A');
//                printf( "Added constraint (%c < %c)\n", pStr[k], pStr[k+1] );
            }
//    if ( p->nConstr )
//        printf( "Total constraints = %d\n", p->nConstr );
    return 1;
}
Ifn_Ntk_t * Ifn_NtkParse( char * pStr )
{
    Ifn_Ntk_t * p = ABC_CALLOC( Ifn_Ntk_t, 1 );
    if ( Ifn_NtkParseInt( pStr, p ) )
        return p;
    ABC_FREE( p );
    return NULL;
}

/**Function*************************************************************

  Synopsis    [Derive truth table given the configulation values.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
word * Ifn_NtkDeriveTruth( Ifn_Ntk_t * p, int * pValues )
{
    int i, v, f, iVar, iStart;
    // elementary variables
    for ( i = 0; i < p->nInps; i++ )
    {
        // find variable
        iStart = p->nParsVIni + i * p->nParsVNum;
        for ( v = iVar = 0; v < p->nParsVNum; v++ )
            if ( p->Values[iStart+v] )
                iVar += (1 << v);
        // assign variable
        Abc_TtCopy( Ifn_ObjTruth(p, i), Ifn_ElemTruth(p, iVar), p->nWords, 0 );
    }
    // internal variables
    for ( i = p->nInps; i < p->nObjs; i++ )
    {
        int nFans = p->Nodes[i].nFanins;
        int * pFans = p->Nodes[i].Fanins;
        word * pTruth = Ifn_ObjTruth( p, i );
        if ( p->Nodes[i].Type == IF_DSD_AND )
        {
            Abc_TtFill( pTruth, p->nWords );
            for ( f = 0; f < nFans; f++ )
                Abc_TtAnd( pTruth, pTruth, Ifn_ObjTruth(p, pFans[f]), p->nWords, 0 );
        }
        else if ( p->Nodes[i].Type == IF_DSD_XOR )
        {
            Abc_TtClear( pTruth, p->nWords );
            for ( f = 0; f < nFans; f++ )
                Abc_TtXor( pTruth, pTruth, Ifn_ObjTruth(p, pFans[f]), p->nWords, 0 );
        }
        else if ( p->Nodes[i].Type == IF_DSD_MUX )
        {
            assert( nFans == 3 );
            Abc_TtMux( pTruth, Ifn_ObjTruth(p, pFans[0]), Ifn_ObjTruth(p, pFans[1]), Ifn_ObjTruth(p, pFans[2]), p->nWords );
        }
        else if ( p->Nodes[i].Type == IF_DSD_PRIME )
        {
            int nValues = (1 << nFans);
            word * pTemp = Ifn_ObjTruth(p, p->nObjs);
            Abc_TtClear( pTruth, p->nWords );
            for ( v = 0; v < nValues; v++ )
            {
                if ( pValues[p->Nodes[i].iFirst + v] == 0 )
                    continue;
                Abc_TtFill( pTemp, p->nWords );
                for ( f = 0; f < nFans; f++ )
                    if ( (v >> f) & 1 )
                        Abc_TtAnd( pTemp, pTemp, Ifn_ObjTruth(p, pFans[f]), p->nWords, 0 );
                    else
                        Abc_TtSharp( pTemp, pTemp, Ifn_ObjTruth(p, pFans[f]), p->nWords );
                Abc_TtOr( pTruth, pTruth, pTemp, p->nWords );
            }
        }
        else assert( 0 );
//Dau_DsdPrintFromTruth( pTruth, p->nVars );
    }
    return Ifn_ObjTruth(p, p->nObjs-1);
}

/**Function*************************************************************

  Synopsis    [Compute more or equal]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ifn_TtComparisonConstr( word * pTruth, int nVars, int fMore, int fEqual )
{
    word Cond[4], Equa[4], Temp[4];
    word s_TtElems[8][4] = {
        { ABC_CONST(0xAAAAAAAAAAAAAAAA),ABC_CONST(0xAAAAAAAAAAAAAAAA),ABC_CONST(0xAAAAAAAAAAAAAAAA),ABC_CONST(0xAAAAAAAAAAAAAAAA) },
        { ABC_CONST(0xCCCCCCCCCCCCCCCC),ABC_CONST(0xCCCCCCCCCCCCCCCC),ABC_CONST(0xCCCCCCCCCCCCCCCC),ABC_CONST(0xCCCCCCCCCCCCCCCC) },
        { ABC_CONST(0xF0F0F0F0F0F0F0F0),ABC_CONST(0xF0F0F0F0F0F0F0F0),ABC_CONST(0xF0F0F0F0F0F0F0F0),ABC_CONST(0xF0F0F0F0F0F0F0F0) },
        { ABC_CONST(0xFF00FF00FF00FF00),ABC_CONST(0xFF00FF00FF00FF00),ABC_CONST(0xFF00FF00FF00FF00),ABC_CONST(0xFF00FF00FF00FF00) },
        { ABC_CONST(0xFFFF0000FFFF0000),ABC_CONST(0xFFFF0000FFFF0000),ABC_CONST(0xFFFF0000FFFF0000),ABC_CONST(0xFFFF0000FFFF0000) },
        { ABC_CONST(0xFFFFFFFF00000000),ABC_CONST(0xFFFFFFFF00000000),ABC_CONST(0xFFFFFFFF00000000),ABC_CONST(0xFFFFFFFF00000000) },
        { ABC_CONST(0x0000000000000000),ABC_CONST(0xFFFFFFFFFFFFFFFF),ABC_CONST(0x0000000000000000),ABC_CONST(0xFFFFFFFFFFFFFFFF) },
        { ABC_CONST(0x0000000000000000),ABC_CONST(0x0000000000000000),ABC_CONST(0xFFFFFFFFFFFFFFFF),ABC_CONST(0xFFFFFFFFFFFFFFFF) }
    };
    int i, nWords = Abc_TtWordNum(2*nVars);
    assert( nVars > 0 && nVars <= 4 );
    Abc_TtClear( pTruth, nWords );
    Abc_TtFill( Equa, nWords );
    for ( i = nVars - 1; i >= 0; i-- )
    {
        if ( fMore )
            Abc_TtSharp( Cond, s_TtElems[2*i+1], s_TtElems[2*i+0], nWords );
        else
            Abc_TtSharp( Cond, s_TtElems[2*i+0], s_TtElems[2*i+1], nWords );
        Abc_TtAnd( Temp, Equa, Cond, nWords, 0 );
        Abc_TtOr( pTruth, pTruth, Temp, nWords );
        Abc_TtXor( Temp, s_TtElems[2*i+0], s_TtElems[2*i+1], nWords, 1 );
        Abc_TtAnd( Equa, Equa, Temp, nWords, 0 );
    }
    if ( fEqual )
        Abc_TtNot( pTruth, nWords );
}

/**Function*************************************************************

  Synopsis    [Adds parameter constraints.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Ifn_AddClause( sat_solver * pSat, int * pBeg, int * pEnd )
{
    int fVerbose = 0;
    int RetValue = sat_solver_addclause( pSat, pBeg, pEnd );
    if ( fVerbose )
    {
        for ( ; pBeg < pEnd; pBeg++ )
            printf( "%c%d ", Abc_LitIsCompl(*pBeg) ? '-':'+', Abc_Lit2Var(*pBeg) );
        printf( "\n" );
    }
    return RetValue;
}
void Ifn_NtkAddConstrOne( sat_solver * pSat, Vec_Int_t * vCover, int * pVars, int nVars )
{
    int RetValue, k, c, Cube, Literal, nLits, pLits[IFN_INS];
    Vec_IntForEachEntry( vCover, Cube, c )
    {
        nLits = 0;
        for ( k = 0; k < nVars; k++ )
        {
            Literal = 3 & (Cube >> (k << 1));
            if ( Literal == 1 )      // '0'  -> pos lit
                pLits[nLits++] = Abc_Var2Lit(pVars[k], 0);
            else if ( Literal == 2 ) // '1'  -> neg lit
                pLits[nLits++] = Abc_Var2Lit(pVars[k], 1);
            else if ( Literal != 0 )
                assert( 0 );
        }
        RetValue = Ifn_AddClause( pSat, pLits, pLits + nLits );
        assert( RetValue );
    }
}
void Ifn_NtkAddConstraints( Ifn_Ntk_t * p, sat_solver * pSat )
{
    int fAddConstr = 1;
    Vec_Int_t * vCover = Vec_IntAlloc( 0 );
    word uTruth = Abc_Tt6Stretch( ~Abc_Tt6Mask(p->nVars), p->nParsVNum );
    assert( p->nParsVNum <= 4 );
    if ( uTruth )
    {
        int i, k, pVars[IFN_INS];
        int RetValue = Kit_TruthIsop( (unsigned *)&uTruth, p->nParsVNum, vCover, 0 );
        assert( RetValue == 0 );
//        Dau_DsdPrintFromTruth( &uTruth, p->nParsVNum );
        // add capacity constraints
        for ( i = 0; i < p->nInps; i++ )
        {
            for ( k = 0; k < p->nParsVNum; k++ )
                pVars[k] = p->nParsVIni + i * p->nParsVNum + k;
            Ifn_NtkAddConstrOne( pSat, vCover, pVars, p->nParsVNum );
        }
    }
    // ordering constraints
    if ( fAddConstr && p->nConstr )
    {
        word pTruth[4];
        int i, k, RetValue, pVars[2*IFN_INS];
        int fForceDiff = (p->nVars == p->nInps);
        Ifn_TtComparisonConstr( pTruth, p->nParsVNum, fForceDiff, fForceDiff );
        RetValue = Kit_TruthIsop( (unsigned *)pTruth, 2*p->nParsVNum, vCover, 0 );
        assert( RetValue == 0 );
//        Kit_TruthIsopPrintCover( vCover, 2*p->nParsVNum, 0 );
        for ( i = 0; i < p->nConstr; i++ )
        {
            int iVar1 = p->pConstr[i] >> 16;
            int iVar2 = p->pConstr[i] & 0xFFFF;
            for ( k = 0; k < p->nParsVNum; k++ )
            {
                pVars[2*k+0] = p->nParsVIni + iVar1 * p->nParsVNum + k;
                pVars[2*k+1] = p->nParsVIni + iVar2 * p->nParsVNum + k;
            }
            Ifn_NtkAddConstrOne( pSat, vCover, pVars, 2*p->nParsVNum );
//            printf( "added constraint with %d clauses for %d and %d\n", Vec_IntSize(vCover), iVar1, iVar2 );
        }
    }
    Vec_IntFree( vCover );
}

/**Function*************************************************************

  Synopsis    [Derive clauses given variable assignment.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Ifn_NtkAddClauses( Ifn_Ntk_t * p, int * pValues, sat_solver * pSat )
{
    int i, f, v, nLits, pLits[IFN_INS+2], pLits2[IFN_INS+2];
    // assign new variables
    int nSatVars = sat_solver_nvars(pSat);
    for ( i = 0; i < p->nObjs-1; i++ )
        p->Nodes[i].Var = nSatVars++;
    p->Nodes[p->nObjs-1].Var = 0xFFFF;
    sat_solver_setnvars( pSat, nSatVars );
    // verify variable values
    for ( i = 0; i < p->nVars; i++ )
        assert( pValues[i] != -1 );
    for ( i = p->nVars; i < p->nObjs-1; i++ )
        assert( pValues[i] == -1 );
    assert( pValues[p->nObjs-1] != -1 );
    // internal variables
//printf( "\n" );
    for ( i = 0; i < p->nInps; i++ )
    {
        int iParStart = p->nParsVIni + i * p->nParsVNum;
        for ( v = 0; v < p->nVars; v++ )
        {
            // add output literal
            pLits[0] = Abc_Var2Lit( p->Nodes[i].Var, pValues[v]==0 );
            // add clause literals
            for ( f = 0; f < p->nParsVNum; f++ )
                pLits[f+1] = Abc_Var2Lit( iParStart + f, (v >> f) & 1 ); 
            if ( !Ifn_AddClause( pSat, pLits, pLits+p->nParsVNum+1 ) )
                return 0;
        }
    }
//printf( "\n" );
    for ( i = p->nInps; i < p->nObjs; i++ )
    {
        int nFans = p->Nodes[i].nFanins;
        int * pFans = p->Nodes[i].Fanins;
        if ( p->Nodes[i].Type == IF_DSD_AND )
        {
            nLits = 0;
            pLits[nLits++] = Abc_Var2Lit( p->Nodes[i].Var, 0 );
            for ( f = 0; f < nFans; f++ )
            {
                pLits[nLits++] = Abc_Var2Lit( p->Nodes[pFans[f]].Var, 1 );
                // add small clause
                pLits2[0] = Abc_Var2Lit( p->Nodes[i].Var, 1 );
                pLits2[1] = Abc_Var2Lit( p->Nodes[pFans[f]].Var, 0 );
                if ( !Ifn_AddClause( pSat, pLits2, pLits2 + 2 ) )
                    return 0;
            }
            // add big clause
            if ( !Ifn_AddClause( pSat, pLits, pLits + nLits ) )
                return 0;
        }
        else if ( p->Nodes[i].Type == IF_DSD_XOR )
        {
            int m, nMints = (1 << (nFans+1));
            for ( m = 0; m < nMints; m++ )
            {
                // skip even 
                int Count = 0;
                for ( v = 0; v <= nFans; v++ )
                    Count += ((m >> v) & 1);
                if ( (Count & 1) == 0 )
                    continue;
                // generate minterm
                pLits[0] = Abc_Var2Lit( p->Nodes[i].Var, (m >> nFans) & 1 );
                for ( v = 0; v < nFans; v++ )
                    pLits[v+1] = Abc_Var2Lit( p->Nodes[pFans[v]].Var, (m >> v) & 1 );
                if ( !Ifn_AddClause( pSat, pLits, pLits + nFans + 1 ) )
                    return 0;
            }
        }
        else if ( p->Nodes[i].Type == IF_DSD_MUX )
        {
            pLits[0] = Abc_Var2Lit( p->Nodes[i].Var, 0 );
            pLits[1] = Abc_Var2Lit( p->Nodes[pFans[0]].Var, 1 ); // ctrl
            pLits[2] = Abc_Var2Lit( p->Nodes[pFans[1]].Var, 1 );
            if ( !Ifn_AddClause( pSat, pLits, pLits + 3 ) )
                return 0;

            pLits[0] = Abc_Var2Lit( p->Nodes[i].Var, 1 );
            pLits[1] = Abc_Var2Lit( p->Nodes[pFans[0]].Var, 1 ); // ctrl
            pLits[2] = Abc_Var2Lit( p->Nodes[pFans[1]].Var, 0 );
            if ( !Ifn_AddClause( pSat, pLits, pLits + 3 ) )
                return 0;

            pLits[0] = Abc_Var2Lit( p->Nodes[i].Var, 0 );
            pLits[1] = Abc_Var2Lit( p->Nodes[pFans[0]].Var, 0 ); // ctrl
            pLits[2] = Abc_Var2Lit( p->Nodes[pFans[2]].Var, 1 );
            if ( !Ifn_AddClause( pSat, pLits, pLits + 3 ) )
                return 0;

            pLits[0] = Abc_Var2Lit( p->Nodes[i].Var, 1 );
            pLits[1] = Abc_Var2Lit( p->Nodes[pFans[0]].Var, 0 ); // ctrl
            pLits[2] = Abc_Var2Lit( p->Nodes[pFans[2]].Var, 0 );
            if ( !Ifn_AddClause( pSat, pLits, pLits + 3 ) )
                return 0;
        }
        else if ( p->Nodes[i].Type == IF_DSD_PRIME )
        {
            int nValues = (1 << nFans);
            int iParStart = p->Nodes[i].iFirst;
            for ( v = 0; v < nValues; v++ )
            {
                nLits = 0;
                if ( pValues[i] == -1 )
                {
                    pLits[nLits]  = Abc_Var2Lit( p->Nodes[i].Var, 0 );
                    pLits2[nLits] = Abc_Var2Lit( p->Nodes[i].Var, 1 );
                    nLits++;
                }
                for ( f = 0; f < nFans; f++, nLits++ )
                    pLits[nLits] = pLits2[nLits] = Abc_Var2Lit( p->Nodes[pFans[f]].Var, (v >> f) & 1 ); 
                pLits[nLits]  = Abc_Var2Lit( iParStart + v, 1 ); 
                pLits2[nLits] = Abc_Var2Lit( iParStart + v, 0 ); 
                nLits++;
                if ( pValues[i] != 0 )
                {
                    if ( !Ifn_AddClause( pSat, pLits2, pLits2 + nLits ) )
                        return 0;
                }
                if ( pValues[i] != 1 )
                {
                    if ( !Ifn_AddClause( pSat, pLits,  pLits + nLits ) )
                        return 0;
                }
            }
        }
        else assert( 0 );
//printf( "\n" );
    }
    return 1;
}

/**Function*************************************************************

  Synopsis    [Returns the minterm number for which there is a mismatch.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ifn_SatPrintStatus( sat_solver * p, int Iter, int status, int iMint, int Value, abctime clk )
{
    printf( "Iter = %5d  ",  Iter );
    printf( "Mint = %5d  ",  iMint );
    printf( "Value = %2d  ", Value );
    printf( "Var = %6d  ",   sat_solver_nvars(p) );
    printf( "Cla = %6d  ",   sat_solver_nclauses(p) );
    printf( "Conf = %6d  ",  sat_solver_nconflicts(p) );
    if ( status == l_False )
        printf( "status = unsat" );
    else if ( status == l_True )
        printf( "status = sat  " );
    else 
        printf( "status = undec" );
    Abc_PrintTime( 1, "", clk );
}
void Ifn_SatPrintConfig( Ifn_Ntk_t * p, sat_solver * pSat )
{
    int v, i;
    for ( v = p->nObjs; v < p->nPars; v++ )
    {
        for ( i = p->nInps; i < p->nObjs; i++ )
            if ( p->Nodes[i].Type == IF_DSD_PRIME && (int)p->Nodes[i].iFirst == v )
                break;
        if ( i < p->nObjs )
            printf( " " );
        else if ( v >= p->nParsVIni && (v - p->nParsVIni) % p->nParsVNum == 0 )
            printf( " %d=", (v - p->nParsVIni) / p->nParsVNum );
        printf( "%d", sat_solver_var_value(pSat, v) );
    }
}

int Ifn_NtkMatch( Ifn_Ntk_t * p, word * pTruth, int nVars, int nConfls, int fVerbose, int fVeryVerbose )
{
    word * pTruth1;
    int RetValue = 0;
    int nIterMax = (1<<nVars);
    int i, v, status, iMint = 0;
    abctime clk = Abc_Clock();
//    abctime clkTru = 0, clkSat = 0, clk2;
    sat_solver * pSat = sat_solver_new();
    Ifn_Prepare( p, pTruth, nVars );
    sat_solver_setnvars( pSat, p->nPars );
    Ifn_NtkAddConstraints( p, pSat );
    if ( fVeryVerbose )
        Ifn_SatPrintStatus( pSat, 0, l_True, -1, -1, Abc_Clock() - clk );
    for ( i = 0; i < nIterMax; i++ )
    {
        // get variable assignment
        for ( v = 0; v < p->nObjs; v++ )
            p->Values[v] = v < p->nVars ? (iMint >> v) & 1 :  -1;
        p->Values[p->nObjs-1] = Abc_TtGetBit( pTruth, iMint );
        // derive clauses
        if ( !Ifn_NtkAddClauses( p, p->Values, pSat ) )
            break;
        // find assignment of parameters
//        clk2 = Abc_Clock();
        status = sat_solver_solve( pSat, NULL, NULL, nConfls, 0, 0, 0 );
//        clkSat += Abc_Clock() - clk2;
        if ( fVeryVerbose )
            Ifn_SatPrintStatus( pSat, i+1, status, iMint, p->Values[p->nObjs-1], Abc_Clock() - clk );
        if ( status != l_True )
            break;
        // collect assignment
        for ( v = p->nObjs; v < p->nPars; v++ )
            p->Values[v] = sat_solver_var_value(pSat, v);
        // find truth table
//        clk2 = Abc_Clock();
        pTruth1 = Ifn_NtkDeriveTruth( p, p->Values );
//        clkTru += Abc_Clock() - clk2;
        Abc_TtXor( pTruth1, pTruth1, p->pTruth, p->nWords, 0 );
        // find mismatch if present
        iMint = Abc_TtFindFirstBit( pTruth1, p->nVars );
        if ( iMint == -1 )
        {
            RetValue = 1;
            break;
        }
    }
    assert( i < nIterMax );
    if ( fVerbose )
    {
        printf( "%s  Iter =%4d. Confl = %6d.  ", RetValue ? "yes":"no ", i, sat_solver_nconflicts(pSat) );
        if ( RetValue )
            Ifn_SatPrintConfig( p, pSat );
        printf( "\n" );
    }
    sat_solver_delete( pSat );
//    Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
//    Abc_PrintTime( 1, "Sat", clkSat );
//    Abc_PrintTime( 1, "Tru", clkTru );
    return RetValue;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ifn_NtkRead()
{
    int RetValue;
    int nVars = 9;
//    int nVars = 8;
//    int nVars = 3;
//    word * pTruth = Dau_DsdToTruth( "(abcdefghi)", nVars );
    word * pTruth = Dau_DsdToTruth( "1008{(1008{(ab)cde}f)ghi}", nVars );
//    word * pTruth = Dau_DsdToTruth( "18{(1008{(ab)cde}f)gh}", nVars );
//    word * pTruth = Dau_DsdToTruth( "1008{(1008{[ab]cde}f)ghi}", nVars );
//    word * pTruth = Dau_DsdToTruth( "(abcd)", nVars );
//    word * pTruth = Dau_DsdToTruth( "(abc)", nVars );
//    char * pStr = "e={abc};f={ed};";
//    char * pStr = "d={ab};e={cd};";
//    char * pStr = "j=(ab);k={jcde};l=(kf);m={lghi};";
//    char * pStr = "i={abc};j={ide};k={ifg};l={jkh};";
//    char * pStr = "h={abcde};i={abcdf};j=<ghi>;";
//    char * pStr = "g=<abc>;h=<ade>;i={fgh};";
    char * pStr = "i=<abc>;j=(def);k=[gh];l={ijk};";
    Ifn_Ntk_t * p = Ifn_NtkParse( pStr );
    if ( p == NULL )
        return;
    Ifn_NtkPrint( p );
    Dau_DsdPrintFromTruth( pTruth, nVars );
    // get the given function
    RetValue = Ifn_NtkMatch( p, pTruth, nVars, 0, 1, 1 );
    ABC_FREE( p );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

