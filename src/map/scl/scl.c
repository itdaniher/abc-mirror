/**CFile****************************************************************

  FileName    [scl.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Standard-cell library representation.]

  Synopsis    [Relevant command handlers.]

  Author      [Alan Mishchenko, Niklas Een]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - August 24, 2012.]

  Revision    [$Id: scl.c,v 1.0 2012/08/24 00:00:00 alanmi Exp $]

***********************************************************************/

#include "sclSize.h"
#include "base/main/mainInt.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

static int Scl_CommandRead    ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandWrite   ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandPrintScl( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandDumpGen ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandPrintGS ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandStime   ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandTopo    ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandBuffer  ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandBufSize ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandUnBuffer( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandMinsize ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandMaxsize ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandUpsize  ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandDnsize  ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandBsize   ( Abc_Frame_t * pAbc, int argc, char **argv );
static int Scl_CommandPrintBuf( Abc_Frame_t * pAbc, int argc, char **argv );

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Scl_Init( Abc_Frame_t * pAbc )
{
    Cmd_CommandAdd( pAbc, "SCL mapping",  "read_scl",    Scl_CommandRead,     0 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "write_scl",   Scl_CommandWrite,    0 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "print_scl",   Scl_CommandPrintScl, 0 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "dump_genlib", Scl_CommandDumpGen,  0 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "print_gs",    Scl_CommandPrintGS,  0 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "stime",       Scl_CommandStime,    0 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "topo",        Scl_CommandTopo,     1 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "buffer",      Scl_CommandBuffer,   1 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "bufsize",     Scl_CommandBufSize,  1 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "unbuffer",    Scl_CommandUnBuffer, 1 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "minsize",     Scl_CommandMinsize,  1 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "maxsize",     Scl_CommandMaxsize,  1 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "upsize",      Scl_CommandUpsize,   1 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "dnsize",      Scl_CommandDnsize,   1 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "bsize",       Scl_CommandBsize,    1 ); 
    Cmd_CommandAdd( pAbc, "SCL mapping",  "print_buf",   Scl_CommandPrintBuf, 0 ); 
}
void Scl_End( Abc_Frame_t * pAbc )
{
    Abc_SclLoad( NULL, (SC_Lib **)&pAbc->pLibScl );
}



/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandRead( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    char * pFileName;
    FILE * pFile;
    int c, fVerbose = 0;

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "vh" ) ) != EOF )
    {
        switch ( c )
        {
            case 'v':
                fVerbose ^= 1;
                break;
            case 'h':
                goto usage;
            default:
                goto usage;
        }
    }
    if ( argc != globalUtilOptind + 1 )
        goto usage;

    // get the input file name
    pFileName = argv[globalUtilOptind];
    if ( (pFile = fopen( pFileName, "rb" )) == NULL )
    {
        fprintf( pAbc->Err, "Cannot open input file \"%s\". \n", pFileName );
        return 1;
    }
    fclose( pFile );

    // read new library
    Abc_SclLoad( pFileName, (SC_Lib **)&pAbc->pLibScl );
    if ( fVerbose )
        Abc_SclWriteText( "scl_out.txt", (SC_Lib *)pAbc->pLibScl );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: read_scl [-vh] <file>\n" );
    fprintf( pAbc->Err, "\t         reads Liberty library from file\n" );
    fprintf( pAbc->Err, "\t-v     : toggle writing the result into file \"scl_out.txt\" [default = %s]\n", fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h     : prints the command summary\n" );
    fprintf( pAbc->Err, "\t<file> : the name of a file to read\n" );
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandWrite( Abc_Frame_t * pAbc, int argc, char **argv )
{
    FILE * pFile;
    char * pFileName;
    int c;

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
        switch ( c )
        {
            case 'h':
                goto usage;
            default:
                goto usage;
        }
    }
    if ( argc != globalUtilOptind + 1 )
        goto usage;
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }
    // get the input file name
    pFileName = argv[globalUtilOptind];
    if ( (pFile = fopen( pFileName, "wb" )) == NULL )
    {
        fprintf( pAbc->Err, "Cannot open output file \"%s\". \n", pFileName );
        return 1;
    }
    fclose( pFile );

    // save current library
    Abc_SclSave( pFileName, (SC_Lib *)pAbc->pLibScl );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: write_scl [-h] <file>\n" );
    fprintf( pAbc->Err, "\t         write Liberty library into file\n" );
    fprintf( pAbc->Err, "\t-h     : print the help massage\n" );
    fprintf( pAbc->Err, "\t<file> : the name of the file to write\n" );
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandPrintScl( Abc_Frame_t * pAbc, int argc, char **argv )
{
    float Slew = 200;
    float Gain = 100;
    int c;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "SGh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'S':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-S\" should be followed by a floating point number.\n" );
                goto usage;
            }
            Slew = (float)atof(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( Slew <= 0.0 )
                goto usage;
            break;
        case 'G':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-G\" should be followed by a floating point number.\n" );
                goto usage;
            }
            Gain = (float)atof(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( Gain <= 0.0 )
                goto usage;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }

    // save current library
    Abc_SclPrintCells( (SC_Lib *)pAbc->pLibScl, Slew, Gain );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: print_scl [-SG float] [-h]\n" );
    fprintf( pAbc->Err, "\t           prints statistics of Liberty library\n" );
    fprintf( pAbc->Err, "\t-S float : the slew parameter used to generate the library [default = %.2f]\n", Slew );
    fprintf( pAbc->Err, "\t-G float : the gain parameter used to generate the library [default = %.2f]\n", Gain );
    fprintf( pAbc->Err, "\t-h       : print the help massage\n" );
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandDumpGen( Abc_Frame_t * pAbc, int argc, char **argv )
{
    char * pFileName = NULL;
    float Slew = 200;
    float Gain = 100;
    int nGatesMin = 4;
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "SGMvh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'S':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-S\" should be followed by a floating point number.\n" );
                goto usage;
            }
            Slew = (float)atof(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( Slew <= 0.0 )
                goto usage;
            break;
        case 'G':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-G\" should be followed by a floating point number.\n" );
                goto usage;
            }
            Gain = (float)atof(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( Gain <= 0.0 )
                goto usage;
            break;
        case 'M':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-M\" should be followed by a positive integer.\n" );
                goto usage;
            }
            nGatesMin = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( nGatesMin < 0 ) 
                goto usage;
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        goto usage;
    }
    if ( argc == globalUtilOptind + 1 )
        pFileName = argv[globalUtilOptind];
    Abc_SclDumpGenlib( pFileName, (SC_Lib *)pAbc->pLibScl, Slew, Gain, nGatesMin );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: dump_genlib [-SG float] [-M num] [-vh] <file>\n" );
    fprintf( pAbc->Err, "\t           writes GENLIB file for SCL library\n" );
    fprintf( pAbc->Err, "\t-S float : the slew parameter used to generate the library [default = %.2f]\n", Slew );
    fprintf( pAbc->Err, "\t-G float : the gain parameter used to generate the library [default = %.2f]\n", Gain );
    fprintf( pAbc->Err, "\t-M num   : skip gate classes whose size is less than this [default = %d]\n", nGatesMin );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    fprintf( pAbc->Err, "\t<file>   : optional GENLIB file name\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandPrintGS( Abc_Frame_t * pAbc, int argc, char **argv )
{
    int c;

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
        switch ( c )
        {
            case 'h':
                goto usage;
            default:
                goto usage;
        }
    }
    if ( Abc_FrameReadNtk(pAbc) == NULL )
    {
        fprintf( pAbc->Err, "There is no current network.\n" );
        return 1;
    }
    if ( !Abc_NtkHasMapping(Abc_FrameReadNtk(pAbc)) )
    {
        fprintf( pAbc->Err, "The current network is not mapped.\n" );
        return 1;
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }

    // save current library
    Abc_SclPrintGateSizes( (SC_Lib *)pAbc->pLibScl, Abc_FrameReadNtk(pAbc) );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: print_gs [-h]\n" );
    fprintf( pAbc->Err, "\t         prints gate sizes in the current mapping\n" );
    fprintf( pAbc->Err, "\t-h     : print the help massage\n" );
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandStime( Abc_Frame_t * pAbc, int argc, char **argv )
{
    int c;
    int fShowAll      = 0;
    int fUseWireLoads = 1;
    int fPrintPath    = 0;
    int fDumpStats    = 0;
    int nTreeCRatio   = 0;

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "Xcapdh" ) ) != EOF )
    {
        switch ( c )
        {
            case 'X':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-X\" should be followed by a positive integer.\n" );
                    goto usage;
                }
                nTreeCRatio = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( nTreeCRatio < 0 ) 
                    goto usage;
                break;
            case 'c':
                fUseWireLoads ^= 1;
                break;
            case 'a':
                fShowAll ^= 1;
                break;
            case 'p':
                fPrintPath ^= 1;
                break;
            case 'd':
                fDumpStats ^= 1;
                break;
            case 'h':
                goto usage;
            default:
                goto usage;
        }
    }

    if ( Abc_FrameReadNtk(pAbc) == NULL )
    {
        fprintf( pAbc->Err, "There is no current network.\n" );
        return 1;
    }
    if ( !Abc_NtkHasMapping(Abc_FrameReadNtk(pAbc)) )
    {
        fprintf( pAbc->Err, "The current network is not mapped.\n" );
        return 1;
    }
    if ( !Abc_SclCheckNtk(Abc_FrameReadNtk(pAbc), 0) )
    {
        fprintf( pAbc->Err, "The current network is not in a topo order (run \"topo\").\n" );
        return 1;
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }

    Abc_SclTimePerform( (SC_Lib *)pAbc->pLibScl, Abc_FrameReadNtk(pAbc), nTreeCRatio, fUseWireLoads, fShowAll, fPrintPath, fDumpStats );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: stime [-X num] [-capdth]\n" );
    fprintf( pAbc->Err, "\t         performs STA using Liberty library\n" );
    fprintf( pAbc->Err, "\t-X     : min Cout/Cave ratio for tree estimations [default = %d]\n", nTreeCRatio );
    fprintf( pAbc->Err, "\t-c     : toggle using wire-loads if specified [default = %s]\n", fUseWireLoads? "yes": "no" );
    fprintf( pAbc->Err, "\t-a     : display timing information for all nodes [default = %s]\n", fShowAll? "yes": "no" );
    fprintf( pAbc->Err, "\t-p     : display timing information for critical path [default = %s]\n", fPrintPath? "yes": "no" );
    fprintf( pAbc->Err, "\t-d     : toggle dumping statistics into a file [default = %s]\n", fDumpStats? "yes": "no" );
    fprintf( pAbc->Err, "\t-h     : print the help massage\n" );
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandTopo( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    Abc_Ntk_t * pNtkRes;
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "vh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( pNtk == NULL )
    {
        Abc_Print( -1, "Empty network.\n" );
        return 1;
    }
    if ( !Abc_NtkIsLogic(pNtk) )
    {
        Abc_Print( -1, "This command can only be applied to a logic network.\n" );
        return 1;
    }

    // modify the current network
    pNtkRes = Abc_NtkDupDfs( pNtk );
    if ( pNtkRes == NULL )
    {
        Abc_Print( -1, "The command has failed.\n" );
        return 1;
    }
    // replace the current network
    Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: topo [-vh]\n" );
    fprintf( pAbc->Err, "\t           rearranges nodes to be in a topological order\n" );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    return 1;
} 

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandBuffer( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    Abc_Ntk_t * pNtkRes;
    int FanMin, FanMax, FanMaxR, fAddInvs, fUseInvs, fBufPis;
    int c, fVerbose;
    int fOldAlgo = 0;
    FanMin   =  6;
    FanMax   = 14;
    FanMaxR  =  0;
    fAddInvs =  0;
    fUseInvs =  0;
    fBufPis  =  0;
    fVerbose =  0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "NMRaixpvh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'N':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-N\" should be followed by a positive integer.\n" );
                goto usage;
            }
            FanMin = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( FanMin < 0 ) 
                goto usage;
            break;
        case 'M':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-M\" should be followed by a positive integer.\n" );
                goto usage;
            }
            FanMax = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( FanMax < 0 ) 
                goto usage;
            break;
        case 'R':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-R\" should be followed by a positive integer.\n" );
                goto usage;
            }
            FanMaxR = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( FanMaxR < 0 ) 
                goto usage;
            break;
        case 'a':
            fOldAlgo ^= 1;
            break;
        case 'i':
            fAddInvs ^= 1;
            break;
        case 'x':
            fUseInvs ^= 1;
            break;
        case 'p':
            fBufPis ^= 1;
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( pNtk == NULL )
    {
        Abc_Print( -1, "Empty network.\n" );
        return 1;
    }
    if ( !Abc_NtkIsLogic(pNtk) )
    {
        Abc_Print( -1, "This command can only be applied to a logic network.\n" );
        return 1;
    }
    if ( fAddInvs && pNtk->vPhases == NULL )
    {
        Abc_Print( -1, "Fanin phase information is not avaiable.\n" );
        return 1;
    }

    // modify the current network
    if ( fAddInvs )
        pNtkRes = Abc_SclBufferPhase( pNtk, fVerbose );
    else if ( fOldAlgo )
        pNtkRes = Abc_SclPerformBuffering( pNtk, FanMaxR, FanMax, fUseInvs, fVerbose );
    else
        pNtkRes = Abc_SclBufPerform( pNtk, FanMin, FanMax, fBufPis, fVerbose );
    if ( pNtkRes == NULL )
    {
        Abc_Print( -1, "The command has failed.\n" );
        return 1;
    }
    // replace the current network
    Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: buffer [-NMR num] [-aixpvh]\n" );
    fprintf( pAbc->Err, "\t           performs buffering of the mapped network\n" );
    fprintf( pAbc->Err, "\t-N <num> : the min fanout considered by the algorithm [default = %d]\n", FanMin );
    fprintf( pAbc->Err, "\t-M <num> : the max allowed fanout count of node/buffer [default = %d]\n", FanMax );
    fprintf( pAbc->Err, "\t-R <num> : the max allowed fanout count of root node [default = %d]\n", FanMaxR );
    fprintf( pAbc->Err, "\t-a       : toggle using old algorithm [default = %s]\n", fOldAlgo? "yes": "no" );
    fprintf( pAbc->Err, "\t-i       : toggle adding interters instead of buffering [default = %s]\n", fAddInvs? "yes": "no" );
    fprintf( pAbc->Err, "\t-x       : toggle using interters instead of buffers [default = %s]\n", fUseInvs? "yes": "no" );
    fprintf( pAbc->Err, "\t-p       : toggle buffering primary inputs [default = %s]\n", fBufPis? "yes": "no" );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    return 1;
} 

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandBufSize( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    SC_BusPars Pars, * pPars = &Pars;
    Abc_Ntk_t * pNtkRes, * pNtk = Abc_FrameReadNtk(pAbc);
    int c;
    memset( pPars, 0, sizeof(SC_BusPars) );
    pPars->GainRatio     =  150;
    pPars->Slew          =  300;
    pPars->nDegree       =    4;
    pPars->fSizeOnly     =    0;
    pPars->fAddBufs      =    0;
    pPars->fBufPis       =    0;
    pPars->fUseWireLoads =    1;
    pPars->fVerbose      =    0;
    pPars->fVeryVerbose  =    0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "GSNsbpcvwh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'G':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-G\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->GainRatio = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->GainRatio < 0 ) 
                goto usage;
            break;
        case 'S':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-S\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->Slew = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->Slew < 0 ) 
                goto usage;
            break;
        case 'N':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-N\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->nDegree = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->nDegree < 0 ) 
                goto usage;
            break;
        case 's':
            pPars->fSizeOnly ^= 1;
            break;
        case 'b':
            pPars->fAddBufs ^= 1;
            break;
        case 'p':
            pPars->fBufPis ^= 1;
            break;
        case 'c':
            pPars->fUseWireLoads ^= 1;
            break;
        case 'v':
            pPars->fVerbose ^= 1;
            break;
        case 'w':
            pPars->fVeryVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( pNtk == NULL )
    {
        Abc_Print( -1, "Empty network.\n" );
        return 1;
    }
    if ( !Abc_NtkIsLogic(pNtk) )
    {
        Abc_Print( -1, "This command can only be applied to a logic network.\n" );
        return 1;
    }
    if ( !pPars->fSizeOnly && !pPars->fAddBufs && pNtk->vPhases == NULL )
    {
        Abc_Print( -1, "Fanin phase information is not avaiable.\n" );
        return 1;
    }
    // modify the current network
    pNtkRes = Abc_SclBufSizePerform( pNtk, (SC_Lib *)pAbc->pLibScl, pPars );
    if ( pNtkRes == NULL )
    {
        Abc_Print( -1, "The command has failed.\n" );
        return 1;
    }
    // replace the current network
    Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: bufsize [-GSM num] [-sbpcvwh]\n" );
    fprintf( pAbc->Err, "\t           performs buffering and sizing and mapped network\n" );
    fprintf( pAbc->Err, "\t-G <num> : target gain percentage [default = %d]\n", pPars->GainRatio );
    fprintf( pAbc->Err, "\t-S <num> : target slew in pisoseconds [default = %d]\n", pPars->Slew );
    fprintf( pAbc->Err, "\t-M <num> : the maximum fanout degree [default = %d]\n", pPars->nDegree );
    fprintf( pAbc->Err, "\t-s       : toggle performing only sizing [default = %s]\n", pPars->fSizeOnly? "yes": "no" );
    fprintf( pAbc->Err, "\t-b       : toggle using buffers instead of inverters [default = %s]\n", pPars->fAddBufs? "yes": "no" );
    fprintf( pAbc->Err, "\t-p       : toggle buffering primary inputs [default = %s]\n", pPars->fBufPis? "yes": "no" );
    fprintf( pAbc->Err, "\t-c       : toggle using wire-loads if specified [default = %s]\n", pPars->fUseWireLoads? "yes": "no" );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", pPars->fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-w       : toggle printing more verbose information [default = %s]\n", pPars->fVeryVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    return 1;
} 

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandUnBuffer( Abc_Frame_t * pAbc, int argc, char **argv )
{
    Abc_Ntk_t * pNtkRes, * pNtk = Abc_FrameReadNtk(pAbc);
    int c, fRemInv = 0, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "ivh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'i':
            fRemInv ^= 1;
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( pNtk == NULL )
    {
        fprintf( pAbc->Err, "There is no current network.\n" );
        return 1;
    }
    if ( !Abc_NtkIsLogic(pNtk) )
    {
        fprintf( pAbc->Err, "The current network is not a logic network.\n" );
        return 1;
    }
    if ( fRemInv )
        pNtkRes = Abc_SclUnBufferPhase( pNtk, fVerbose );
    else
        pNtkRes = Abc_SclUnBufferPerform( pNtk, fVerbose );
    if ( pNtkRes == NULL )
    {
        Abc_Print( -1, "The command has failed.\n" );
        return 1;
    }
    Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: unbuffer [-ivh]\n" );
    fprintf( pAbc->Err, "\t           collapses buffer/inverter trees\n" );
    fprintf( pAbc->Err, "\t-i       : toggle removing interters [default = %s]\n", fRemInv? "yes": "no" );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandMinsize( Abc_Frame_t * pAbc, int argc, char **argv )
{
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "vh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( Abc_FrameReadNtk(pAbc) == NULL )
    {
        fprintf( pAbc->Err, "There is no current network.\n" );
        return 1;
    }
    if ( !Abc_NtkHasMapping(Abc_FrameReadNtk(pAbc)) )
    {
        fprintf( pAbc->Err, "The current network is not mapped.\n" );
        return 1;
    }
    if ( !Abc_SclCheckNtk(Abc_FrameReadNtk(pAbc), 0) )
    {
        fprintf( pAbc->Err, "The current network is not in a topo order (run \"topo\").\n" );
        return 1;
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }

    Abc_SclMinsizePerform( (SC_Lib *)pAbc->pLibScl, pNtk, 0, fVerbose );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: minsize [-vh]\n" );
    fprintf( pAbc->Err, "\t           downsizes all gates to their minimum size\n" );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandMaxsize( Abc_Frame_t * pAbc, int argc, char **argv )
{
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "vh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( Abc_FrameReadNtk(pAbc) == NULL )
    {
        fprintf( pAbc->Err, "There is no current network.\n" );
        return 1;
    }
    if ( !Abc_NtkHasMapping(Abc_FrameReadNtk(pAbc)) )
    {
        fprintf( pAbc->Err, "The current network is not mapped.\n" );
        return 1;
    }
    if ( !Abc_SclCheckNtk(Abc_FrameReadNtk(pAbc), 0) )
    {
        fprintf( pAbc->Err, "The current network is not in a topo order (run \"topo\").\n" );
        return 1;
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }

    Abc_SclMinsizePerform( (SC_Lib *)pAbc->pLibScl, pNtk, 1, fVerbose );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: maxsize [-vh]\n" );
    fprintf( pAbc->Err, "\t           upsizes all gates to their maximum size\n" );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandUpsize( Abc_Frame_t * pAbc, int argc, char **argv )
{
    SC_SizePars Pars, * pPars = &Pars;
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    int c;
    memset( pPars, 0, sizeof(SC_SizePars) );
    pPars->nIters        = 1000;
    pPars->nIterNoChange =   50;
    pPars->Window        =    1;
    pPars->Ratio         =   10;
    pPars->Notches       = 1000;
    pPars->DelayUser     =    0;
    pPars->DelayGap      =    0;
    pPars->TimeOut       =    0;
    pPars->BuffTreeEst   =    0;
    pPars->BypassFreq    =    0;
    pPars->fUseDept      =    1;
    pPars->fUseWireLoads =    1;
    pPars->fDumpStats    =    0;
    pPars->fVerbose      =    0;
    pPars->fVeryVerbose  =    0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "IJWRNDGTXBcsdvwh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'I':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-I\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->nIters = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->nIters < 0 ) 
                goto usage;
            break;
        case 'J':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-J\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->nIterNoChange = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->nIterNoChange < 0 ) 
                goto usage;
            break;
        case 'W':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-W\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->Window = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->Window < 0 ) 
                goto usage;
            break;
        case 'R':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-R\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->Ratio = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->Ratio < 0 ) 
                goto usage;
            break;
        case 'N':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-N\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->Notches = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->Notches < 0 ) 
                goto usage;
            break;
        case 'D':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-D\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->DelayUser = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->DelayUser < 0 ) 
                goto usage;
            break;
        case 'G':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-G\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->DelayGap = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            break;
        case 'T':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-T\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->TimeOut = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->TimeOut < 0 ) 
                goto usage;
            break;
        case 'X':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-X\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->BuffTreeEst = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->BuffTreeEst < 0 ) 
                goto usage;
            break;
        case 'B':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-B\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->BypassFreq = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->BypassFreq < 0 ) 
                goto usage;
            break;
        case 'c':
            pPars->fUseWireLoads ^= 1;
            break;
        case 's':
            pPars->fUseDept ^= 1;
            break;
        case 'd':
            pPars->fDumpStats ^= 1;
            break;
        case 'v':
            pPars->fVerbose ^= 1;
            break;
        case 'w':
            pPars->fVeryVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( Abc_FrameReadNtk(pAbc) == NULL )
    {
        fprintf( pAbc->Err, "There is no current network.\n" );
        return 1;
    }
    if ( !Abc_NtkHasMapping(Abc_FrameReadNtk(pAbc)) )
    {
        fprintf( pAbc->Err, "The current network is not mapped.\n" );
        return 1;
    }
    if ( !Abc_SclCheckNtk(Abc_FrameReadNtk(pAbc), 0) )
    {
        fprintf( pAbc->Err, "The current network is not in a topo order (run \"topo\").\n" );
        return 1;
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }

    Abc_SclUpsizePerform( (SC_Lib *)pAbc->pLibScl, pNtk, pPars );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: upsize [-IJWRNDGTXB num] [-csdvwh]\n" );
    fprintf( pAbc->Err, "\t           selectively increases gate sizes on the critical path\n" );
    fprintf( pAbc->Err, "\t-I <num> : the number of upsizing iterations to perform [default = %d]\n", pPars->nIters );
    fprintf( pAbc->Err, "\t-J <num> : the number of iterations without improvement to stop [default = %d]\n", pPars->nIterNoChange );
    fprintf( pAbc->Err, "\t-W <num> : delay window (in percent) of near-critical COs [default = %d]\n", pPars->Window );
    fprintf( pAbc->Err, "\t-R <num> : ratio of critical nodes (in percent) to update [default = %d]\n", pPars->Ratio );
    fprintf( pAbc->Err, "\t-N <num> : limit on discrete upsizing steps at a node [default = %d]\n", pPars->Notches );
    fprintf( pAbc->Err, "\t-D <num> : delay target set by the user, in picoseconds [default = %d]\n", pPars->DelayUser );
    fprintf( pAbc->Err, "\t-G <num> : delay gap during updating, in picoseconds [default = %d]\n", pPars->DelayGap );
    fprintf( pAbc->Err, "\t-T <num> : approximate timeout in seconds [default = %d]\n", pPars->TimeOut );
    fprintf( pAbc->Err, "\t-X <num> : ratio for buffer tree estimation [default = %d]\n", pPars->BuffTreeEst );
    fprintf( pAbc->Err, "\t-B <num> : frequency of bypass transforms [default = %d]\n", pPars->BypassFreq );
    fprintf( pAbc->Err, "\t-c       : toggle using wire-loads if specified [default = %s]\n", pPars->fUseWireLoads? "yes": "no" );
    fprintf( pAbc->Err, "\t-s       : toggle using slack based on departure times [default = %s]\n", pPars->fUseDept? "yes": "no" );
    fprintf( pAbc->Err, "\t-d       : toggle dumping statistics into a file [default = %s]\n", pPars->fDumpStats? "yes": "no" );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", pPars->fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-w       : toggle printing more verbose information [default = %s]\n", pPars->fVeryVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandDnsize( Abc_Frame_t * pAbc, int argc, char **argv )
{
    SC_SizePars Pars, * pPars = &Pars;
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    int c;
    memset( pPars, 0, sizeof(SC_SizePars) );
    pPars->nIters        =    5;
    pPars->nIterNoChange =   50;
    pPars->Notches       = 1000;
    pPars->DelayUser     =    0;
    pPars->DelayGap      = 1000;
    pPars->TimeOut       =    0;
    pPars->BuffTreeEst   =    0;
    pPars->fUseDept      =    1;
    pPars->fUseWireLoads =    1;
    pPars->fDumpStats    =    0;
    pPars->fVerbose      =    0;
    pPars->fVeryVerbose  =    0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "IJNDGTXcsdvwh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'I':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-I\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->nIters = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->nIters < 0 ) 
                goto usage;
            break;
        case 'J':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-J\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->nIterNoChange = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->nIterNoChange < 0 ) 
                goto usage;
            break;
        case 'N':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-N\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->Notches = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->Notches < 0 ) 
                goto usage;
            break;
        case 'D':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-D\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->DelayUser = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->DelayUser < 0 ) 
                goto usage;
            break;
        case 'G':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-G\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->DelayGap = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            break;
        case 'T':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-T\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->TimeOut = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->TimeOut < 0 ) 
                goto usage;
            break;
        case 'X':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-X\" should be followed by a positive integer.\n" );
                goto usage;
            }
            pPars->BuffTreeEst = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( pPars->BuffTreeEst < 0 ) 
                goto usage;
            break;
        case 'c':
            pPars->fUseWireLoads ^= 1;
            break;
        case 's':
            pPars->fUseDept ^= 1;
            break;
        case 'd':
            pPars->fDumpStats ^= 1;
            break;
        case 'v':
            pPars->fVerbose ^= 1;
            break;
        case 'w':
            pPars->fVeryVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( Abc_FrameReadNtk(pAbc) == NULL )
    {
        fprintf( pAbc->Err, "There is no current network.\n" );
        return 1;
    }
    if ( !Abc_NtkHasMapping(Abc_FrameReadNtk(pAbc)) )
    {
        fprintf( pAbc->Err, "The current network is not mapped.\n" );
        return 1;
    }
    if ( !Abc_SclCheckNtk(Abc_FrameReadNtk(pAbc), 0) )
    {
        fprintf( pAbc->Err, "The current network is not in a topo order (run \"topo\").\n" );
        return 1;
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }

    Abc_SclDnsizePerform( (SC_Lib *)pAbc->pLibScl, pNtk, pPars );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: dnsize [-IJNDGTX num] [-csdvwh]\n" );
    fprintf( pAbc->Err, "\t           selectively decreases gate sizes while maintaining delay\n" );
    fprintf( pAbc->Err, "\t-I <num> : the number of upsizing iterations to perform [default = %d]\n", pPars->nIters );
    fprintf( pAbc->Err, "\t-J <num> : the number of iterations without improvement to stop [default = %d]\n", pPars->nIterNoChange );
    fprintf( pAbc->Err, "\t-N <num> : limit on discrete upsizing steps at a node [default = %d]\n", pPars->Notches );
    fprintf( pAbc->Err, "\t-D <num> : delay target set by the user, in picoseconds [default = %d]\n", pPars->DelayUser );
    fprintf( pAbc->Err, "\t-G <num> : delay gap during updating, in picoseconds [default = %d]\n", pPars->DelayGap );
    fprintf( pAbc->Err, "\t-T <num> : approximate timeout in seconds [default = %d]\n", pPars->TimeOut );
    fprintf( pAbc->Err, "\t-X <num> : ratio for buffer tree estimation [default = %d]\n", pPars->BuffTreeEst );
    fprintf( pAbc->Err, "\t-c       : toggle using wire-loads if specified [default = %s]\n", pPars->fUseWireLoads? "yes": "no" );
    fprintf( pAbc->Err, "\t-s       : toggle using slack based on departure times [default = %s]\n", pPars->fUseDept? "yes": "no" );
    fprintf( pAbc->Err, "\t-d       : toggle dumping statistics into a file [default = %s]\n", pPars->fDumpStats? "yes": "no" );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", pPars->fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-w       : toggle printing more verbose information [default = %s]\n", pPars->fVeryVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandBsize( Abc_Frame_t * pAbc, int argc, char **argv )
{
    extern Abc_Ntk_t * Abc_SclBuffSizeStep( SC_Lib * pLib, Abc_Ntk_t * pNtk, int nTreeCRatio, int fUseWireLoads );
    Abc_Ntk_t * pNtkRes;
    int c;
    int fUseWireLoads = 1;
    int nTreeCRatio   = 0;

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "Xch" ) ) != EOF )
    {
        switch ( c )
        {
            case 'X':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-X\" should be followed by a positive integer.\n" );
                    goto usage;
                }
                nTreeCRatio = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( nTreeCRatio < 0 ) 
                    goto usage;
                break;
            case 'c':
                fUseWireLoads ^= 1;
                break;
           case 'h':
                goto usage;
            default:
                goto usage;
        }
    }

    if ( Abc_FrameReadNtk(pAbc) == NULL )
    {
        fprintf( pAbc->Err, "There is no current network.\n" );
        return 1;
    }
    if ( !Abc_NtkHasMapping(Abc_FrameReadNtk(pAbc)) )
    {
        fprintf( pAbc->Err, "The current network is not mapped.\n" );
        return 1;
    }
    if ( !Abc_SclCheckNtk(Abc_FrameReadNtk(pAbc), 0) )
    {
        fprintf( pAbc->Err, "The current network is not in a topo order (run \"topo\").\n" );
        return 1;
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }
    if ( Abc_FrameReadNtk(pAbc)->vPhases == 0 )
    {
        fprintf( pAbc->Err, "There is no phases available.\n" );
        return 1;
    }
    pNtkRes = Abc_SclBuffSizeStep( (SC_Lib *)pAbc->pLibScl, Abc_FrameReadNtk(pAbc), nTreeCRatio, fUseWireLoads );
    if ( pNtkRes == NULL )
    {
        Abc_Print( -1, "The command has failed.\n" );
        return 1;
    }
    Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: bsize [-X num] [-ch]\n" );
    fprintf( pAbc->Err, "\t         performs STA using Liberty library\n" );
    fprintf( pAbc->Err, "\t-X     : min Cout/Cave ratio for tree estimations [default = %d]\n", nTreeCRatio );
    fprintf( pAbc->Err, "\t-c     : toggle using wire-loads if specified [default = %s]\n", fUseWireLoads? "yes": "no" );
    fprintf( pAbc->Err, "\t-h     : print the help massage\n" );
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Scl_CommandPrintBuf( Abc_Frame_t * pAbc, int argc, char **argv )
{
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "vh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( Abc_FrameReadNtk(pAbc) == NULL )
    {
        fprintf( pAbc->Err, "There is no current network.\n" );
        return 1;
    }
    if ( !Abc_NtkHasMapping(Abc_FrameReadNtk(pAbc)) )
    {
        fprintf( pAbc->Err, "The current network is not mapped.\n" );
        return 1;
    }
    if ( !Abc_SclCheckNtk(Abc_FrameReadNtk(pAbc), 0) )
    {
        fprintf( pAbc->Err, "The current network is not in a topo order (run \"topo\").\n" );
        return 1;
    }
    if ( pAbc->pLibScl == NULL )
    {
        fprintf( pAbc->Err, "There is no Liberty library available.\n" );
        return 1;
    }

    Abc_SclPrintBuffers( (SC_Lib *)pAbc->pLibScl, pNtk, fVerbose );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: print_buf [-vh]\n" );
    fprintf( pAbc->Err, "\t           prints buffers trees of the current design\n" );
    fprintf( pAbc->Err, "\t-v       : toggle printing verbose information [default = %s]\n", fVerbose? "yes": "no" );
    fprintf( pAbc->Err, "\t-h       : print the command usage\n");
    return 1;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

