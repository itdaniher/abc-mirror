/**CFile****************************************************************

  FileName    [cba.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Verilog parser.]

  Synopsis    [External declarations.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - November 29, 2014.]

  Revision    [$Id: cba.h,v 1.00 2014/11/29 00:00:00 alanmi Exp $]

***********************************************************************/

#ifndef ABC__base__cba__cba_h
#define ABC__base__cba__cba_h


////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "aig/gia/gia.h"
#include "misc/extra/extra.h"
#include "misc/util/utilNam.h"
#include "misc/extra/extra.h"
#include "misc/util/utilTruth.h"
#include "misc/vec/vecSet.h"

////////////////////////////////////////////////////////////////////////
///                         PARAMETERS                               ///
////////////////////////////////////////////////////////////////////////

ABC_NAMESPACE_HEADER_START 

////////////////////////////////////////////////////////////////////////
///                         BASIC TYPES                              ///
////////////////////////////////////////////////////////////////////////

// network objects
typedef enum { 
    CBA_OBJ_NONE = 0,  // 0:  unused
    CBA_OBJ_BI,        // 1:  box input
    CBA_OBJ_BO,        // 2:  box output
    CBA_OBJ_PI,        // 3:  input
    CBA_OBJ_PO,        // 4:  output
    CBA_OBJ_PIO,       // 5:  input
    CBA_OBJ_NODE,      // 6:  node
    CBA_OBJ_BOX,       // 7:  box
    CBA_OBJ_LATCH,     // 8:  latch
    CBA_OBJ_UNKNOWN    // 9:  unknown
} Cba_ObjType_t; 

// Verilog predefined models
typedef enum { 
    CBA_NODE_NONE = 0,   // 0:  unused
    CBA_NODE_CONST,      // 1:  constant
    CBA_NODE_BUF,        // 2:  buffer
    CBA_NODE_INV,        // 3:  inverter
    CBA_NODE_AND,        // 4:  AND
    CBA_NODE_NAND,       // 5:  NAND
    CBA_NODE_OR,         // 6:  OR
    CBA_NODE_NOR,        // 7:  NOR
    CBA_NODE_XOR,        // 8:  XOR
    CBA_NODE_XNOR,       // 9  .XNOR
    CBA_NODE_MUX,        // 10: MUX
    CBA_NODE_MAJ,        // 11: MAJ
    CBA_NODE_KNOWN,      // 12: unknown
    CBA_NODE_UNKNOWN     // 13: unknown
} Cba_NodeType_t; 


// design
typedef struct Cba_Man_t_ Cba_Man_t;
struct Cba_Man_t_
{
    // design names
    char *       pName;    // design name
    char *       pSpec;    // spec file name
    Abc_Nam_t *  pNames;   // name manager
    Abc_Nam_t *  pModels;  // model name manager
    Abc_Nam_t *  pFuncs;   // functionality manager
    Cba_Man_t *  pLib;     // library
    // internal data
    Vec_Set_t    Mem;      // memory
    Vec_Ptr_t    vNtks;    // networks
    int          iRoot;    // root network
    Vec_Int_t    vCopies;  // copies
    Vec_Int_t *  vBuf2RootNtk;
    Vec_Int_t *  vBuf2RootObj;
    Vec_Int_t *  vBuf2LeafNtk;
    Vec_Int_t *  vBuf2LeafObj;
};

// network
typedef struct Cba_Ntk_t_ Cba_Ntk_t;
struct Cba_Ntk_t_
{
    char *       pName;    // name
    Cba_Man_t *  pDesign;  // design
    int          Id;       // network ID
    int          iBoxNtk;  // instance network ID
    int          iBoxObj;  // instance object ID
    int          nObjs;    // object counter
    int          iObjStart;// first object in global order
    // interface
    Vec_Int_t    vInouts;  // inouts          (used by parser to store signals as NameId) 
    Vec_Int_t    vInputs;  // inputs          (used by parser to store signals as NameId)
    Vec_Int_t    vOutputs; // outputs         (used by parser to store signals as NameId) 
    Vec_Int_t    vWires;   // wires           (used by parser to store signals as NameId)
    // objects
    Vec_Int_t    vTypes;   // types           (used by parser to store Cba_PrsType_t)
    Vec_Int_t    vFuncs;   // functions       (used by parser to store function)                      (node: function; box: model; CI/CO: index)
    Vec_Int_t    vInstIds; // instance names  (used by parser to store instance name as NameId)       
    Vec_Int_t    vFanins;  // fanins          (used by parser to store fanin/fanout/range as NameId)  (node: handle; CO/BI fanin0)
    // attributes
    Vec_Int_t    vBoxes;   // box objects
    Vec_Int_t    vNameIds; // original names as NameId  
    Vec_Int_t    vRanges;  // ranges as NameId
};


static inline char *         Cba_ManName( Cba_Man_t * p )                    { return p->pName;                                                                            }
static inline int            Cba_ManNtkNum( Cba_Man_t * p )                  { return Vec_PtrSize(&p->vNtks) - 1;                                                          }
static inline int            Cba_ManNtkId( Cba_Man_t * p, char * pName )     { return Abc_NamStrFind(p->pModels, pName);                                                   }
static inline Cba_Ntk_t *    Cba_ManNtk( Cba_Man_t * p, int i )              { assert( i > 0 ); return (Cba_Ntk_t *)Vec_PtrEntry(&p->vNtks, i);                            }
static inline Cba_Ntk_t *    Cba_ManRoot( Cba_Man_t * p )                    { return Cba_ManNtk(p, p->iRoot);                                                             }
static inline Vec_Set_t *    Cba_ManMem( Cba_Man_t * p )                     { return &p->Mem;                                                                             }
static inline int            Cba_ManMemSave( Cba_Man_t * p, int * d, int s ) { return Vec_SetAppend(Cba_ManMem(p), d, s);                                                  }
static inline int *          Cba_ManMemRead( Cba_Man_t * p, int h )          { return h ? (int *)Vec_SetEntry(Cba_ManMem(p), h) : NULL;                                    }

static inline int            Cba_NtkId( Cba_Ntk_t * p )                      { return p->Id;                                                                               }
static inline char *         Cba_NtkName( Cba_Ntk_t * p )                    { return p->pName;                                                                            }
static inline Cba_Man_t *    Cba_NtkMan( Cba_Ntk_t * p )                     { return p->pDesign;                                                                          }
static inline int            Cba_NtkObjNum( Cba_Ntk_t * p )                  { return Vec_IntSize(&p->vFanins);                                                            }
static inline int            Cba_NtkPiNum( Cba_Ntk_t * p )                   { return Vec_IntSize(&p->vInputs);                                                            }
static inline int            Cba_NtkPoNum( Cba_Ntk_t * p )                   { return Vec_IntSize(&p->vOutputs);                                                           }
static inline int            Cba_NtkBoxNum( Cba_Ntk_t * p )                  { return Vec_IntSize(&p->vBoxes);                                                             }
static inline int            Cba_NtkPi( Cba_Ntk_t * p, int i )               { return Vec_IntEntry(&p->vInputs, i);                                                        }
static inline int            Cba_NtkPo( Cba_Ntk_t * p, int i )               { return Vec_IntEntry(&p->vOutputs, i);                                                       }
static inline char *         Cba_NtkStr( Cba_Ntk_t * p, int i )              { return Abc_NamStr(p->pDesign->pNames, i);                                                   }
static inline char *         Cba_NtkModelStr( Cba_Ntk_t * p, int i )         { return Abc_NamStr(p->pDesign->pModels, i);                                                  }
static inline char *         Cba_NtkFuncStr( Cba_Ntk_t * p, int i )          { return Abc_NamStr(p->pDesign->pFuncs, i);                                                   }
static inline Vec_Set_t *    Cba_NtkMem( Cba_Ntk_t * p )                     { return Cba_ManMem(p->pDesign);                                                              }
static inline int            Cba_NtkMemSave( Cba_Ntk_t * p, int * d, int s ) { return Cba_ManMemSave(p->pDesign, d, s);                                                    }
static inline int *          Cba_NtkMemRead( Cba_Ntk_t * p, int h )          { return Cba_ManMemRead(p->pDesign, h);                                                       }
static inline Cba_Ntk_t *    Cba_NtkHost( Cba_Ntk_t * p )                    { return Cba_ManNtk(p->pDesign, p->iBoxNtk);                                                  }
static inline void           Cba_NtkSetHost( Cba_Ntk_t * p, int n, int i )   { p->iBoxNtk = n; p->iBoxObj = i;                                                             }
static inline Vec_Int_t *    Cba_NtkCopies( Cba_Ntk_t * p )                  { return &p->pDesign->vCopies;                                                                }
static inline int            Cba_NtkCopy( Cba_Ntk_t * p, int i )             { return Vec_IntEntry( Cba_NtkCopies(p), p->iObjStart + i );                                  }
static inline void           Cba_NtkSetCopy( Cba_Ntk_t * p, int i, int x )   { Vec_IntWriteEntry( Cba_NtkCopies(p), p->iObjStart + i, x );                                 }

static inline Cba_ObjType_t  Cba_ObjType( Cba_Ntk_t * p, int i )             { return Vec_IntEntry(&p->vTypes, i);                                                         }
static inline int            Cba_ObjFuncId( Cba_Ntk_t * p, int i )           { return Vec_IntEntry(&p->vFuncs, i);                                                         }
static inline int            Cba_ObjInstId( Cba_Ntk_t * p, int i )           { return Vec_IntEntry(&p->vInstIds, i);                                                       }
static inline int            Cba_ObjFaninId( Cba_Ntk_t * p, int i )          { return Vec_IntEntry(&p->vFanins, i);                                                        }
static inline int            Cba_ObjNameId( Cba_Ntk_t * p, int i )           { return Vec_IntEntry(&p->vNameIds, i);                                                       }
static inline int            Cba_ObjRangeId( Cba_Ntk_t * p, int i )          { return Vec_IntEntry(&p->vRanges, i);                                                        }

static inline int            Cba_ObjIsPi( Cba_Ntk_t * p, int i )             { return Cba_ObjType(p, i) == CBA_OBJ_PI;                                                     }
static inline int            Cba_ObjIsPo( Cba_Ntk_t * p, int i )             { return Cba_ObjType(p, i) == CBA_OBJ_PO;                                                     }
static inline int            Cba_ObjIsBi( Cba_Ntk_t * p, int i )             { return Cba_ObjType(p, i) == CBA_OBJ_BI;                                                     }
static inline int            Cba_ObjIsBo( Cba_Ntk_t * p, int i )             { return Cba_ObjType(p, i) == CBA_OBJ_BO;                                                     }
static inline int            Cba_ObjIsNode( Cba_Ntk_t * p, int i )           { return Cba_ObjType(p, i) == CBA_OBJ_NODE;                                                   }
static inline int            Cba_ObjIsBox( Cba_Ntk_t * p, int i )            { return Cba_ObjType(p, i) == CBA_OBJ_BOX;                                                    }
static inline int            Cba_ObjIsCi( Cba_Ntk_t * p, int i )             { return Cba_ObjIsPi(p, i) || Cba_ObjIsBo(p, i);                                              }
static inline int            Cba_ObjIsCo( Cba_Ntk_t * p, int i )             { return Cba_ObjIsPo(p, i) || Cba_ObjIsBi(p, i);                                              }
static inline int            Cba_ObjIsCio( Cba_Ntk_t * p, int i )            { return Cba_ObjIsCi(p, i) || Cba_ObjIsCo(p, i);                                              }
static inline int            Cba_ObjIsBio( Cba_Ntk_t * p, int i )            { return Cba_ObjIsBi(p, i) || Cba_ObjIsBo(p, i);                                              }

static inline int            Cba_ObjFanin0( Cba_Ntk_t * p, int i )           { assert(Cba_ObjIsPo(p, i) || Cba_ObjIsBio(p, i)); return Cba_ObjFuncId(p, i);                }
static inline int *          Cba_ObjFaninArray( Cba_Ntk_t * p, int i )       { assert(Cba_ObjIsNode(p, i)); return Cba_NtkMemRead(p, Cba_ObjFaninId(p, i));                }
static inline int            Cba_ObjFaninNum( Cba_Ntk_t * p, int i )         { return *Cba_ObjFaninArray(p, i);                                                            }
static inline int *          Cba_ObjFanins( Cba_Ntk_t * p, int i )           { return Cba_ObjFaninArray(p, i) + 1;                                                         }
static inline Vec_Int_t *    Cba_ObjFaninVec( Cba_Ntk_t * p, int i )         { static Vec_Int_t V; V.pArray = Cba_ObjFaninArray(p, i); V.nSize = V.nCap = V.pArray ? *V.pArray++ : 0; return &V; }
static inline Vec_Int_t *    Cba_ObjFaninVec2( Cba_Ntk_t * p, int i )        { static Vec_Int_t W; W.pArray = Cba_ObjFaninArray(p, i); W.nSize = W.nCap = W.pArray ? *W.pArray++ : 0; return &W; }
static inline Cba_NodeType_t Cba_ObjNodeType( Cba_Ntk_t * p, int i )         { assert(Cba_ObjIsNode(p, i)); return Cba_ObjFaninId(p, i);                                   }
static inline int            Cba_ObjBoxModelId( Cba_Ntk_t * p, int i )       { assert(Cba_ObjIsBox(p, i)); return Cba_ObjFuncId(p, i);                                     }
static inline Cba_Ntk_t *    Cba_ObjBoxModel( Cba_Ntk_t * p, int i )         { assert(Cba_ObjIsBox(p, i)); return Cba_ManNtk(p->pDesign, Cba_ObjBoxModelId(p, i));         }
static inline int            Cba_ObjBoxBi( Cba_Ntk_t * p, int b, int i )     { assert(Cba_ObjIsBox(p, i)); return b - Cba_NtkPiNum(Cba_ObjBoxModel(p, b)) + i;             }
static inline int            Cba_ObjBoxBo( Cba_Ntk_t * p, int b, int i )     { assert(Cba_ObjIsBox(p, i)); return b + 1 + i;                                               }
static inline int            Cba_ObjBiModelId( Cba_Ntk_t * p, int i )        { assert(Cba_ObjIsBi(p, i)); while (!Cba_ObjIsBox(p, i)) i++; return Cba_ObjBoxModelId(p, i); }
static inline int            Cba_ObjBoModelId( Cba_Ntk_t * p, int i )        { assert(Cba_ObjIsBo(p, i)); return Cba_ObjBoxModelId(p, Cba_ObjFanin0(p, i));                }
static inline Cba_Ntk_t *    Cba_ObjBiModel( Cba_Ntk_t * p, int i )          { return Cba_ManNtk( p->pDesign, Cba_ObjBiModelId(p, i) );                                    }
static inline Cba_Ntk_t *    Cba_ObjBoModel( Cba_Ntk_t * p, int i )          { return Cba_ManNtk( p->pDesign, Cba_ObjBoModelId(p, i) );                                    }
static inline int            Cba_ObjCioIndex( Cba_Ntk_t * p, int i )         { assert(Cba_ObjIsCio(p, i) || Cba_ObjIsBio(p, i)); return Cba_ObjFuncId(p, i);               }

static inline char *         Cba_ObjFuncStr( Cba_Ntk_t * p, int i )          { return Cba_NtkStr(p, Cba_ObjFuncId(p, i));                                                  }
static inline char *         Cba_ObjInstStr( Cba_Ntk_t * p, int i )          { return Cba_NtkStr(p, Cba_ObjInstId(p, i));                                                  }
static inline char *         Cba_ObjNameStr( Cba_Ntk_t * p, int i )          { return Cba_NtkStr(p, Cba_ObjNameId(p, i));                                                  }
static inline char *         Cba_ObjRangeStr( Cba_Ntk_t * p, int i )         { return Cba_NtkStr(p, Cba_ObjRangeId(p, i));                                                 }


////////////////////////////////////////////////////////////////////////
///                      MACRO DEFINITIONS                           ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                             ITERATORS                            ///
////////////////////////////////////////////////////////////////////////


#define Cba_ManForEachNtk( p, pNtk, i )                                   \
    for ( i = 1; (i <= Cba_ManNtkNum(p)) && (((pNtk) = Cba_ManNtk(p, i)), 1); i++ ) 

#define Cba_NtkForEachPi( p, iObj, i )                                    \
    for ( i = 0; (i < Cba_NtkPiNum(p))  && (((iObj) = Vec_IntEntry(&p->vInputs, i)), 1); i++ ) 
#define Cba_NtkForEachPo( p, iObj, i )                                    \
    for ( i = 0; (i < Cba_NtkPoNum(p))  && (((iObj) = Vec_IntEntry(&p->vOutputs, i)), 1); i++ ) 

#define Cba_NtkForEachObjType( p, Type, i )                               \
    for ( i = 0; (i < Cba_NtkObjNum(p))  && (((Type) = Cba_ObjType(p, i)), 1); i++ ) 
#define Cba_NtkForEachObjTypeFuncFanins( p, Type, Func, vFanins, i )      \
    for ( i = 0; (i < Cba_NtkObjNum(p))  && (((Type) = Cba_ObjType(p, i)), 1)  && (((Func) = Cba_ObjFuncId(p, i)), 1)  && (((vFanins) = Cba_ObjFaninVec(p, i)), 1); i++ ) 

#define Cba_NtkForEachBox( p, iObj, i )                                   \
    for ( i = 0; (i < Cba_NtkBoxNum(p))  && (((iObj) = Vec_IntEntry(&p->vBoxes, i)), 1); i++ ) 
#define Cba_NtkForEachNode( p, i )                                        \
    for ( i = 0; (i < Cba_NtkObjNum(p)); i++ ) if ( Cba_ObjType(p, i) != CBA_OBJ_NODE ) {} else

#define Cba_NtkForEachCi( p, i )                                          \
    for ( i = 0; (i < Cba_NtkObjNum(p)); i++ ) if ( Cba_ObjType(p, i) != CBA_OBJ_PI && Cba_ObjType(p, i) != CBA_OBJ_BO ) {} else
#define Cba_NtkForEachCo( p, i )                                          \
    for ( i = 0; (i < Cba_NtkObjNum(p)); i++ ) if ( Cba_ObjType(p, i) != CBA_OBJ_PO && Cba_ObjType(p, i) != CBA_OBJ_BI ) {} else

////////////////////////////////////////////////////////////////////////
///                    FUNCTION DECLARATIONS                         ///
////////////////////////////////////////////////////////////////////////

// copy contents to the vector
static inline int Cba_ManHandleArray( Cba_Man_t * p, Vec_Int_t * vFrom )
{
    int h = Vec_SetFetchH( Cba_ManMem(p), sizeof(int) * (Vec_IntSize(vFrom) + 1) );
    int * pArray = (int *)Vec_SetEntry( Cba_ManMem(p), h );
    pArray[0] = Vec_IntSize(vFrom);
    memcpy( pArray+1, Vec_IntArray(vFrom), sizeof(int) * Vec_IntSize(vFrom) );
    Vec_IntClear( vFrom );
    return h;
}
static inline int Cba_ManHandleBuffer( Cba_Man_t * p, int iFanin )
{
    int h = Vec_SetFetchH( Cba_ManMem(p), sizeof(int) * 2 );
    int * pArray = (int *)Vec_SetEntry( Cba_ManMem(p), h );
    pArray[0] = 1;
    pArray[1] = iFanin;
    return h;
}
static inline void Cba_ManSetupArray( Cba_Man_t * p, Vec_Int_t * vTo, Vec_Int_t * vFrom )
{
    if ( Vec_IntSize(vFrom) == 0 )
        return;
    vTo->nSize = vTo->nCap = Vec_IntSize(vFrom);
    vTo->pArray = (int *)Vec_SetFetch( Cba_ManMem(p), sizeof(int) * Vec_IntSize(vFrom) );
    memcpy( Vec_IntArray(vTo), Vec_IntArray(vFrom), sizeof(int) * Vec_IntSize(vFrom) );
    Vec_IntClear( vFrom );
}
static inline void Cba_ManFetchArray( Cba_Man_t * p, Vec_Int_t * vTo, int nSize )
{
    if ( nSize == 0 )
        return;
    vTo->nSize = vTo->nCap = nSize;
    vTo->pArray = (int *)Vec_SetFetch( Cba_ManMem(p), sizeof(int) * nSize );
    memset( Vec_IntArray(vTo), 0xff, sizeof(int) * nSize );
}

// constructors desctructors
static inline Cba_Ntk_t * Cba_NtkAlloc( Cba_Man_t * p, char * pName )
{
    Cba_Ntk_t * pNtk = Vec_SetFetch( Cba_ManMem(p), sizeof(Cba_Ntk_t) );
    memset( pNtk, 0, sizeof(Cba_Ntk_t) );
    pNtk->pDesign = p;
    pNtk->pName   = Vec_SetStrsav( Cba_ManMem(p), pName );
    pNtk->Id      = Vec_PtrSize(&p->vNtks);
    Vec_PtrPush( &p->vNtks, pNtk );
    return pNtk;
}
static inline Cba_Man_t * Cba_ManAlloc( char * pFileName )
{
    Cba_Man_t * p = ABC_CALLOC( Cba_Man_t, 1 );
    p->pName    = Extra_FileDesignName( pFileName );
    p->pSpec    = Abc_UtilStrsav( pFileName );
    p->pNames   = Abc_NamStart( 1000, 20 );
    p->pModels  = Abc_NamStart( 1000, 20 );
    p->pFuncs   = Abc_NamStart( 1000, 20 );
    Vec_SetAlloc_( &p->Mem, 20 );
    Vec_PtrPush( &p->vNtks, NULL );
    return p;
}
static inline void Cba_ManFree( Cba_Man_t * p )
{
    Vec_IntFreeP( &p->vBuf2LeafNtk );
    Vec_IntFreeP( &p->vBuf2LeafObj );
    Vec_IntFreeP( &p->vBuf2RootNtk );
    Vec_IntFreeP( &p->vBuf2RootObj );
    ABC_FREE( p->vCopies.pArray );
    ABC_FREE( p->vNtks.pArray );
    Vec_SetFree_( &p->Mem );
    Abc_NamStop( p->pNames );
    Abc_NamStop( p->pModels );
    Abc_NamStop( p->pFuncs );
    ABC_FREE( p->pName );
    ABC_FREE( p->pSpec );
    ABC_FREE( p );
}
static inline int Cba_ManMemory( Cba_Man_t * p )
{
    int nMem = sizeof(Cba_Man_t);
    nMem += Abc_NamMemUsed(p->pNames);
    nMem += Abc_NamMemUsed(p->pModels);
    nMem += Abc_NamMemUsed(p->pFuncs);
    nMem += Vec_SetMemoryAll(&p->Mem);
    nMem += (int)Vec_PtrMemory(&p->vNtks);
    return nMem;
}


/*=== cbaBuild.c =========================================================*/
extern Cba_Man_t * Cba_ManBuild( Cba_Man_t * p );
/*=== cbaReadBlif.c =========================================================*/
extern Cba_Man_t * Cba_PrsReadBlif( char * pFileName );
/*=== cbaWriteBlif.c ========================================================*/
extern void        Cba_PrsWriteBlif( char * pFileName, Cba_Man_t * pDes );
/*=== cbaReadVer.c ==========================================================*/
extern Cba_Man_t * Cba_PrsReadVerilog( char * pFileName );
/*=== cbaWriteVer.c =========================================================*/
extern void        Cba_PrsWriteVerilog( char * pFileName, Cba_Man_t * pDes );
/*=== cbaNtk.c =========================================================*/
extern void        Cba_ManAssignInternNames( Cba_Man_t * p );
extern int         Cba_NtkNodeNum( Cba_Ntk_t * p );
extern int         Cba_ManObjNum( Cba_Man_t * p );
extern void        Cba_ManSetNtkBoxes( Cba_Man_t * p );
extern Cba_Man_t * Cba_ManDupStart( Cba_Man_t * p, Vec_Int_t * vObjCounts );
extern Cba_Man_t * Cba_ManDup( Cba_Man_t * p );



ABC_NAMESPACE_HEADER_END

#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

