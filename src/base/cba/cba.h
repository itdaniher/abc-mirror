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
    CBA_OBJ_PIO,       // 5:  ioput
    CBA_OBJ_NODE,      // 6:  node
    CBA_OBJ_BOX,       // 7:  box
    CBA_OBJ_LATCH,     // 8:  latch
    CBA_OBJ_CONCAT,    // 9:  concatenation
    CBA_OBJ_UNKNOWN    // 10: unknown
} Cba_ObjType_t; 

// Verilog predefined models
typedef enum { 
    CBA_NODE_NONE = 0, // 0:  unused
    CBA_NODE_C0,       // 1:  constant 0
    CBA_NODE_C1,       // 2:  constant 1
    CBA_NODE_BUF,      // 3:  buffer
    CBA_NODE_INV,      // 4:  inverter
    CBA_NODE_AND,      // 5:  AND
    CBA_NODE_NAND,     // 6:  NAND
    CBA_NODE_OR,       // 7:  OR
    CBA_NODE_NOR,      // 8:  NOR
    CBA_NODE_XOR,      // 9:  XOR
    CBA_NODE_XNOR,     // 10  XNOR
    CBA_NODE_SHARP,    // 11: SHARP
    CBA_NODE_MUX,      // 12: MUX
    CBA_NODE_MAJ,      // 13: MAJ
    CBA_NODE_LUT,      // 14: LUT

    CBA_NODE_RAND,
    CBA_NODE_RNAND,
    CBA_NODE_ROR,
    CBA_NODE_RNOR,
    CBA_NODE_RXOR,
    CBA_NODE_RXNOR,

    CBA_NODE_SEL,
    CBA_NODE_PSEL,
    CBA_NODE_ENC,
    CBA_NODE_PENC,
    CBA_NODE_DEC,

    CBA_NODE_C,
    CBA_NODE_ADD,
    CBA_NODE_SUB,
    CBA_NODE_MUL,
    CBA_NODE_DIV,
    CBA_NODE_MOD,
    CBA_NODE_REM,
    CBA_NODE_POW,
    CBA_NODE_ABS,

    CBA_NODE_LTHEN,
    CBA_NODE_MTHEN,
    CBA_NODE_EQU,
    CBA_NODE_NEQU,

    CBA_NODE_SHIL,
    CBA_NODE_SHIR,
    CBA_NODE_ROTL,
    CBA_NODE_ROTR,

    CBA_NODE_TRI,
    CBA_NODE_RAM,
    CBA_NODE_RAMR,
    CBA_NODE_RAMW,
    CBA_NODE_RAMWC,

    CBA_NODE_LATCH,
    CBA_NODE_LATCHRS,
    CBA_NODE_DFF,
    CBA_NODE_DFFRS,

    CBA_NODE_UNKNOWN   // 50
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
    void *       pMioLib;
    void **      ppGraphs;
};

// network
typedef struct Cba_Ntk_t_ Cba_Ntk_t;
struct Cba_Ntk_t_
{
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
    Vec_Int_t    vFanins;  // fanins          (used by parser to store fanin/fanout/range as NameId)  (node: handle; CO/BI fanin0)
    // attributes
    Vec_Int_t    vInstIds; // instance names  (used by parser to store instance name as NameId)       
    Vec_Int_t    vNameIds; // original names as NameId  
    Vec_Int_t    vRangesL; // ranges as NameId
    Vec_Int_t    vRangesR; // ranges as NameId
};


static inline char *         Cba_ManName( Cba_Man_t * p )                    { return p->pName;                                                                            }
static inline char *         Cba_ManSpec( Cba_Man_t * p )                    { return p->pSpec;                                                                            }
static inline int            Cba_ManNtkNum( Cba_Man_t * p )                  { return Vec_PtrSize(&p->vNtks) - 1;                                                          }
static inline int            Cba_ManNtkId( Cba_Man_t * p, char * pName )     { return Abc_NamStrFind(p->pModels, pName);                                                   }
static inline Cba_Ntk_t *    Cba_ManNtk( Cba_Man_t * p, int i )              { assert( i > 0 ); return (Cba_Ntk_t *)Vec_PtrEntry(&p->vNtks, i);                            }
static inline Cba_Ntk_t *    Cba_ManNtkFind( Cba_Man_t * p, char * pName )   { return Cba_ManNtk( p, Cba_ManNtkId(p, pName) );                                             }
static inline Cba_Ntk_t *    Cba_ManRoot( Cba_Man_t * p )                    { return Cba_ManNtk(p, p->iRoot);                                                             }
static inline Vec_Set_t *    Cba_ManMem( Cba_Man_t * p )                     { return &p->Mem;                                                                             }
static inline int            Cba_ManMemSave( Cba_Man_t * p, int * d, int s ) { return Vec_SetAppend(Cba_ManMem(p), d, s);                                                  }
static inline int *          Cba_ManMemRead( Cba_Man_t * p, int h )          { return h ? (int *)Vec_SetEntry(Cba_ManMem(p), h) : NULL;                                    }

static inline Cba_Man_t *    Cba_NtkMan( Cba_Ntk_t * p )                     { return p->pDesign;                                                                          }
static inline int            Cba_NtkId( Cba_Ntk_t * p )                      { return p->Id;                                                                               }
static inline char *         Cba_NtkName( Cba_Ntk_t * p )                    { return Abc_NamStr(Cba_NtkMan(p)->pModels, Cba_NtkId(p));                                    }
static inline int            Cba_NtkObjNum( Cba_Ntk_t * p )                  { return Vec_IntSize(&p->vFanins);                                                            }
static inline int            Cba_NtkPioNum( Cba_Ntk_t * p )                  { return Vec_IntSize(&p->vInouts);                                                            }
static inline int            Cba_NtkPiNum( Cba_Ntk_t * p )                   { return Vec_IntSize(&p->vInputs);                                                            }
static inline int            Cba_NtkPoNum( Cba_Ntk_t * p )                   { return Vec_IntSize(&p->vOutputs);                                                           }
static inline int            Cba_NtkNodeNum( Cba_Ntk_t * p )                 { return Vec_IntCountEntry(&p->vTypes, CBA_OBJ_NODE);                                         }
static inline int            Cba_NtkBoxNum( Cba_Ntk_t * p )                  { return Vec_IntCountEntry(&p->vTypes, CBA_OBJ_BOX);                                          }

static inline int            Cba_NtkPio( Cba_Ntk_t * p, int i )              { return Vec_IntEntry(&p->vInouts, i);                                                        }
static inline int            Cba_NtkPi( Cba_Ntk_t * p, int i )               { return Vec_IntEntry(&p->vInputs, i);                                                        }
static inline int            Cba_NtkPo( Cba_Ntk_t * p, int i )               { return Vec_IntEntry(&p->vOutputs, i);                                                       }
static inline char *         Cba_NtkStr( Cba_Ntk_t * p, int i )              { return Abc_NamStr(p->pDesign->pNames, i);                                                   }
static inline char *         Cba_NtkModelStr( Cba_Ntk_t * p, int i )         { return Abc_NamStr(p->pDesign->pModels, i);                                                  }
static inline char *         Cba_NtkFuncStr( Cba_Ntk_t * p, int i )          { return Abc_NamStr(p->pDesign->pFuncs, i);                                                   }
static inline Vec_Set_t *    Cba_NtkMem( Cba_Ntk_t * p )                     { return Cba_ManMem(p->pDesign);                                                              }
static inline int            Cba_NtkMemSave( Cba_Ntk_t * p, int * d, int s ) { return Cba_ManMemSave(p->pDesign, d, s);                                                    }
static inline int *          Cba_NtkMemRead( Cba_Ntk_t * p, int h )          { return Cba_ManMemRead(p->pDesign, h);                                                       }
static inline Cba_Ntk_t *    Cba_NtkHostNtk( Cba_Ntk_t * p )                 { return Cba_ManNtk(p->pDesign, p->iBoxNtk);                                                  }
static inline int            Cba_NtkHostObj( Cba_Ntk_t * p )                 { return p->iBoxObj;                                                                          }
static inline void           Cba_NtkSetHost( Cba_Ntk_t * p, int n, int i )   { p->iBoxNtk = n; p->iBoxObj = i;                                                             }
static inline Vec_Int_t *    Cba_NtkCopies( Cba_Ntk_t * p )                  { return &p->pDesign->vCopies;                                                                }
static inline int            Cba_NtkCopy( Cba_Ntk_t * p, int i )             { return Vec_IntEntry( Cba_NtkCopies(p), p->iObjStart + i );                                  }
static inline void           Cba_NtkSetCopy( Cba_Ntk_t * p, int i, int x )   { Vec_IntWriteEntry( Cba_NtkCopies(p), p->iObjStart + i, x );                                 }
static inline int            Cba_NtkIsWordLevel( Cba_Ntk_t * p )             { return Vec_IntSize(&p->vRangesL) > 0;                                                       }

static inline Cba_ObjType_t  Cba_ObjType( Cba_Ntk_t * p, int i )             { return (Cba_ObjType_t)Vec_IntEntry(&p->vTypes, i);                                          }
static inline int            Cba_ObjFuncId( Cba_Ntk_t * p, int i )           { return Vec_IntEntry(&p->vFuncs, i);                                                         }
static inline int            Cba_ObjFaninId( Cba_Ntk_t * p, int i )          { return Vec_IntEntry(&p->vFanins, i);                                                        }
static inline int            Cba_ObjInstId( Cba_Ntk_t * p, int i )           { return Vec_IntEntry(&p->vInstIds, i);                                                       }
static inline int            Cba_ObjNameId( Cba_Ntk_t * p, int i )           { return Vec_IntEntry(&p->vNameIds, i);                                                       }
static inline int            Cba_ObjRangeLId( Cba_Ntk_t * p, int i )         { return Vec_IntEntry(&p->vRangesL, i);                                                       }
static inline int            Cba_ObjRangeRId( Cba_Ntk_t * p, int i )         { return Vec_IntEntry(&p->vRangesR, i);                                                       }

static inline int            Cba_ObjIsPi( Cba_Ntk_t * p, int i )             { return Cba_ObjType(p, i) == CBA_OBJ_PI;                                                     }
static inline int            Cba_ObjIsPo( Cba_Ntk_t * p, int i )             { return Cba_ObjType(p, i) == CBA_OBJ_PO;                                                     }
static inline int            Cba_ObjIsBi( Cba_Ntk_t * p, int i )             { return Cba_ObjType(p, i) == CBA_OBJ_BI;                                                     }
static inline int            Cba_ObjIsBo( Cba_Ntk_t * p, int i )             { return Cba_ObjType(p, i) == CBA_OBJ_BO;                                                     }
static inline int            Cba_ObjIsNode( Cba_Ntk_t * p, int i )           { return Cba_ObjType(p, i) == CBA_OBJ_NODE;                                                   }
static inline int            Cba_ObjIsBox( Cba_Ntk_t * p, int i )            { return Cba_ObjType(p, i) == CBA_OBJ_BOX;                                                    }
static inline int            Cba_ObjIsConcat( Cba_Ntk_t * p, int i )         { return Cba_ObjType(p, i) == CBA_OBJ_CONCAT;                                                 }
static inline int            Cba_ObjIsCi( Cba_Ntk_t * p, int i )             { return Cba_ObjIsPi(p, i) || Cba_ObjIsBo(p, i);                                              }
static inline int            Cba_ObjIsCo( Cba_Ntk_t * p, int i )             { return Cba_ObjIsPo(p, i) || Cba_ObjIsBi(p, i);                                              }
static inline int            Cba_ObjIsCio( Cba_Ntk_t * p, int i )            { return Cba_ObjIsCi(p, i) || Cba_ObjIsCo(p, i);                                              }
static inline int            Cba_ObjIsBio( Cba_Ntk_t * p, int i )            { return Cba_ObjIsBi(p, i) || Cba_ObjIsBo(p, i);                                              }

static inline int            Cba_ObjFanin0( Cba_Ntk_t * p, int i )           { assert(Cba_ObjIsPo(p, i) || Cba_ObjIsBio(p, i)); return Cba_ObjFaninId(p, i);               }
static inline int *          Cba_ObjFaninArray( Cba_Ntk_t * p, int i )       { assert(Cba_ObjType(p, i) >= CBA_OBJ_NODE); return Cba_NtkMemRead(p, Cba_ObjFaninId(p, i));  }
static inline int            Cba_ObjFaninNum( Cba_Ntk_t * p, int i )         { return *Cba_ObjFaninArray(p, i);                                                            }
static inline int *          Cba_ObjFanins( Cba_Ntk_t * p, int i )           { return Cba_ObjFaninArray(p, i) + 1;                                                         }
static inline Vec_Int_t *    Cba_ObjFaninVec( Cba_Ntk_t * p, int i )         { static Vec_Int_t V; V.pArray = Cba_ObjFaninArray(p, i); V.nSize = V.nCap = V.pArray ? *V.pArray++ : 0; return &V; }
static inline Vec_Int_t *    Cba_ObjFaninVec2( Cba_Ntk_t * p, int i )        { static Vec_Int_t W; W.pArray = Cba_ObjFaninArray(p, i); W.nSize = W.nCap = W.pArray ? *W.pArray++ : 0; return &W; }
static inline Cba_NodeType_t Cba_ObjNodeType( Cba_Ntk_t * p, int i )         { assert(Cba_ObjIsNode(p, i)); return (Cba_NodeType_t)Cba_ObjFuncId(p, i);                    }
static inline int            Cba_ObjBoxModelId( Cba_Ntk_t * p, int i )       { assert(Cba_ObjIsBox(p, i)); return Cba_ObjFuncId(p, i);                                     }
static inline Cba_Ntk_t *    Cba_ObjBoxModel( Cba_Ntk_t * p, int i )         { assert(Cba_ObjIsBox(p, i)); return Cba_ManNtk(p->pDesign, Cba_ObjBoxModelId(p, i));         }
static inline int            Cba_ObjBoxSize( Cba_Ntk_t * p, int i )          { return 1 + Cba_NtkPiNum(Cba_ObjBoxModel(p, i)) + Cba_NtkPoNum(Cba_ObjBoxModel(p, i));       }

static inline int            Cba_ObjBoxBiNum( Cba_Ntk_t * p, int i )         { assert(Cba_ObjIsBox(p, i)); return Cba_NtkPiNum(Cba_ObjBoxModel(p, i));                     }
static inline int            Cba_ObjBoxBoNum( Cba_Ntk_t * p, int i )         { assert(Cba_ObjIsBox(p, i)); return Cba_NtkPoNum(Cba_ObjBoxModel(p, i));                     }
static inline int            Cba_ObjBoxBi( Cba_Ntk_t * p, int b, int i )     { assert(Cba_ObjIsBox(p, b)); return b - Cba_ObjBoxBiNum(p, b) + i;                           }
static inline int            Cba_ObjBoxBo( Cba_Ntk_t * p, int b, int i )     { assert(Cba_ObjIsBox(p, b)); return b + 1 + i;                                               }
static inline int            Cba_ObjBiModelId( Cba_Ntk_t * p, int i )        { assert(Cba_ObjIsBi(p, i)); while (!Cba_ObjIsBox(p, i)) i++; return Cba_ObjBoxModelId(p, i); }
static inline int            Cba_ObjBoModelId( Cba_Ntk_t * p, int i )        { assert(Cba_ObjIsBo(p, i)); return Cba_ObjBoxModelId(p, Cba_ObjFanin0(p, i));                }
static inline Cba_Ntk_t *    Cba_ObjBiModel( Cba_Ntk_t * p, int i )          { return Cba_ManNtk( p->pDesign, Cba_ObjBiModelId(p, i) );                                    }
static inline Cba_Ntk_t *    Cba_ObjBoModel( Cba_Ntk_t * p, int i )          { return Cba_ManNtk( p->pDesign, Cba_ObjBoModelId(p, i) );                                    }
static inline int            Cba_ObjCioIndex( Cba_Ntk_t * p, int i )         { assert(Cba_ObjIsCio(p, i) || Cba_ObjIsBio(p, i)); return Cba_ObjFuncId(p, i);               }

static inline char *         Cba_ObjFuncStr( Cba_Ntk_t * p, int i )          { return Cba_NtkStr(p, Cba_ObjFuncId(p, i));                                                  }
static inline char *         Cba_ObjInstStr( Cba_Ntk_t * p, int i )          { return Cba_NtkStr(p, Cba_ObjInstId(p, i));                                                  }
static inline char *         Cba_ObjNameStr( Cba_Ntk_t * p, int i )          { return Cba_NtkStr(p, Cba_ObjNameId(p, i));                                                  }


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

#define Cba_NtkForEachBox( p, i )                                         \
    for ( i = 0; (i < Cba_NtkObjNum(p)); i++ ) if ( Cba_ObjType(p, i) != CBA_OBJ_BOX ) {} else
#define Cba_NtkForEachNode( p, i )                                        \
    for ( i = 0; (i < Cba_NtkObjNum(p)); i++ ) if ( Cba_ObjType(p, i) != CBA_OBJ_NODE ) {} else

#define Cba_NtkForEachCi( p, i )                                          \
    for ( i = 0; (i < Cba_NtkObjNum(p)); i++ ) if ( Cba_ObjType(p, i) != CBA_OBJ_PI && Cba_ObjType(p, i) != CBA_OBJ_BO ) {} else
#define Cba_NtkForEachCo( p, i )                                          \
    for ( i = 0; (i < Cba_NtkObjNum(p)); i++ ) if ( Cba_ObjType(p, i) != CBA_OBJ_PO && Cba_ObjType(p, i) != CBA_OBJ_BI ) {} else

#define Cba_BoxForEachBi( p, iBox, iTerm, i )                             \
    for ( iTerm = iBox - Cba_ObjBoxBiNum(p, iBox), i = 0; iTerm < iBox; iTerm++, i++ )
#define Cba_BoxForEachBo( p, iBox, iTerm, i )                             \
    for ( iTerm = iBox + 1, i = 0; iTerm < iBox + 1 + Cba_ObjBoxBoNum(p, iBox); iTerm++, i++ )

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
    pArray[0] = (iFanin >= 0) ? 1 : 0;
    pArray[1] = iFanin;
    return h;
}
static inline void Cba_ManAllocArray( Cba_Man_t * p, Vec_Int_t * vTo, int nSize )
{
    if ( nSize == 0 )
        return;
    vTo->nSize  = 0;
    vTo->nCap   = nSize;
    vTo->pArray = (int *)Vec_SetFetch( Cba_ManMem(p), sizeof(int) * nSize );
}
static inline void Cba_ManFetchArray( Cba_Man_t * p, Vec_Int_t * vTo, int nSize )
{
    if ( nSize == 0 )
        return;
    vTo->nSize  = vTo->nCap = nSize;
    vTo->pArray = (int *)Vec_SetFetch( Cba_ManMem(p), sizeof(int) * nSize );
    memset( Vec_IntArray(vTo), 0xff, sizeof(int) * nSize );
}
static inline void Cba_ManSetupArray( Cba_Man_t * p, Vec_Int_t * vTo, Vec_Int_t * vFrom )
{
    if ( Vec_IntSize(vFrom) == 0 )
        return;
    vTo->nSize  = vTo->nCap = Vec_IntSize(vFrom);
    vTo->pArray = (int *)Vec_SetFetch( Cba_ManMem(p), sizeof(int) * Vec_IntSize(vFrom) );
    memcpy( Vec_IntArray(vTo), Vec_IntArray(vFrom), sizeof(int) * Vec_IntSize(vFrom) );
    Vec_IntClear( vFrom );
}

// constructors desctructors
static inline Cba_Ntk_t * Cba_NtkAlloc( Cba_Man_t * p, char * pName )
{
    int iModelId = Abc_NamStrFindOrAdd( p->pModels, pName, NULL );
    Cba_Ntk_t * pNtk = (Cba_Ntk_t *)Vec_SetFetch( Cba_ManMem(p), sizeof(Cba_Ntk_t) );
    memset( pNtk, 0, sizeof(Cba_Ntk_t) );
    pNtk->pDesign = p;
    pNtk->Id      = Vec_PtrSize(&p->vNtks);        
    Vec_PtrPush( &p->vNtks, pNtk );
    assert( iModelId <= pNtk->Id );
    if ( iModelId < pNtk->Id )
        printf( "Model with name %s already exists.\n", pName );
    return pNtk;
}
static inline void Cba_NtkResize( Cba_Ntk_t * p, int nObjs, int fRange )
{
    assert( Vec_IntSize(&p->vTypes) == 0 );
    Cba_ManFetchArray( p->pDesign, &p->vTypes,   nObjs );
    Cba_ManFetchArray( p->pDesign, &p->vFuncs,   nObjs );
    Cba_ManFetchArray( p->pDesign, &p->vFanins,  nObjs );
    Cba_ManFetchArray( p->pDesign, &p->vNameIds, nObjs );
    if ( !fRange ) return;
    Cba_ManFetchArray( p->pDesign, &p->vRangesL, nObjs );
    Cba_ManFetchArray( p->pDesign, &p->vRangesR, nObjs );
}
static inline Cba_Man_t * Cba_ManAlloc( Cba_Man_t * pOld, char * pFileName )
{
    Cba_Man_t * p = ABC_CALLOC( Cba_Man_t, 1 );
    p->pName    = pOld ? Abc_UtilStrsav( Cba_ManName(pOld) ) : Extra_FileDesignName( pFileName );
    p->pSpec    = pOld ? Abc_UtilStrsav( Cba_ManSpec(pOld) ) : Abc_UtilStrsav( pFileName );
    p->pNames   = pOld ? Abc_NamRef( pOld->pNames )  : Abc_NamStart( 1000, 24 );
    p->pModels  = pOld ? Abc_NamRef( pOld->pModels ) : Abc_NamStart( 1000, 24 );
    p->pFuncs   = pOld ? Abc_NamRef( pOld->pFuncs )  : Abc_NamStart( 1000, 24 );
    p->iRoot    = 1;
    Vec_SetAlloc_( &p->Mem, 18 );
    Vec_PtrPush( &p->vNtks, NULL );
    return p;
}
static inline Cba_Man_t * Cba_ManClone( Cba_Man_t * pOld )
{
    Cba_Ntk_t * pNtk, * pNtkNew; int i;
    Cba_Man_t * p = Cba_ManAlloc( pOld, NULL );
    Cba_ManForEachNtk( pOld, pNtk, i )
    {
        pNtkNew = Cba_NtkAlloc( p, Cba_NtkName(pNtk) );
        Cba_ManFetchArray( p, &pNtkNew->vInputs,  Cba_NtkPiNum(pNtk) );
        Cba_ManFetchArray( p, &pNtkNew->vOutputs, Cba_NtkPoNum(pNtk) );
    }
    assert( Cba_ManNtkNum(p) == Cba_ManNtkNum(pOld) );
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
    Abc_NamDeref( p->pNames );
    Abc_NamDeref( p->pModels );
    Abc_NamDeref( p->pFuncs );
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

static inline void Cba_NtkPrintStats( Cba_Ntk_t * p )
{
    printf( "%-32s ",      Cba_NtkName(p) );
    printf( "pi =%5d  ",   Cba_NtkPiNum(p) );
    printf( "pi =%5d  ",   Cba_NtkPoNum(p) );
    printf( "box =%6d  ",  Cba_NtkBoxNum(p) );
    printf( "node =%6d  ", Cba_NtkNodeNum(p) );
    printf( "obj =%6d  ",  Cba_NtkObjNum(p) );
    printf( "\n" );
}
static inline void Cba_ManPrintStats( Cba_Man_t * p, int fVerbose )
{
    Cba_Ntk_t * pNtk; int i;
    Cba_ManForEachNtk( p, pNtk, i )
    {
        printf( "Module %4d : ", i );
        Cba_NtkPrintStats( pNtk );
    }
}
   
static inline Cba_NodeType_t Ptr_SopToType( char * pSop )
{
    if ( !strcmp(pSop, " 1\n") )         return CBA_NODE_C0;
    if ( !strcmp(pSop, " 0\n") )         return CBA_NODE_C1;
    if ( !strcmp(pSop, "1 1\n") )        return CBA_NODE_BUF;
    if ( !strcmp(pSop, "0 1\n") )        return CBA_NODE_INV;
    if ( !strcmp(pSop, "11 1\n") )       return CBA_NODE_AND;
    if ( !strcmp(pSop, "00 1\n") )       return CBA_NODE_NOR;
    if ( !strcmp(pSop, "00 0\n") )       return CBA_NODE_OR;
    if ( !strcmp(pSop, "-1 1\n1- 1\n") ) return CBA_NODE_OR;
    if ( !strcmp(pSop, "1- 1\n-1 1\n") ) return CBA_NODE_OR;
    if ( !strcmp(pSop, "01 1\n10 1\n") ) return CBA_NODE_XOR;
    if ( !strcmp(pSop, "10 1\n01 1\n") ) return CBA_NODE_XOR;
    if ( !strcmp(pSop, "11 1\n00 1\n") ) return CBA_NODE_XNOR;
    if ( !strcmp(pSop, "00 1\n11 1\n") ) return CBA_NODE_XNOR;
    if ( !strcmp(pSop, "10 1\n") )       return CBA_NODE_SHARP;
    assert( 0 );
    return CBA_NODE_NONE;
}
static inline char * Ptr_TypeToName( Cba_NodeType_t Type )
{
    if ( Type == CBA_NODE_C0 )    return "const0";
    if ( Type == CBA_NODE_C1 )    return "const1";
    if ( Type == CBA_NODE_BUF )   return "buf";
    if ( Type == CBA_NODE_INV )   return "not";
    if ( Type == CBA_NODE_AND )   return "and";
    if ( Type == CBA_NODE_NAND )  return "nand";
    if ( Type == CBA_NODE_OR )    return "or";
    if ( Type == CBA_NODE_NOR )   return "nor";
    if ( Type == CBA_NODE_XOR )   return "xor";
    if ( Type == CBA_NODE_XNOR )  return "xnor";
    if ( Type == CBA_NODE_MUX )   return "mux";
    if ( Type == CBA_NODE_MAJ )   return "maj";
    if ( Type == CBA_NODE_SHARP ) return "sharp";
    assert( 0 );
    return "???";
}
static inline char * Ptr_TypeToSop( Cba_NodeType_t Type )
{
    if ( Type == CBA_NODE_C0 )    return " 0\n";
    if ( Type == CBA_NODE_C1 )    return " 1\n";
    if ( Type == CBA_NODE_BUF )   return "1 1\n";
    if ( Type == CBA_NODE_INV )   return "0 1\n";
    if ( Type == CBA_NODE_AND )   return "11 1\n";
    if ( Type == CBA_NODE_NAND )  return "11 0\n";
    if ( Type == CBA_NODE_OR )    return "00 0\n";
    if ( Type == CBA_NODE_NOR )   return "00 1\n";
    if ( Type == CBA_NODE_XOR )   return "01 1\n10 1\n";
    if ( Type == CBA_NODE_XNOR )  return "00 1\n11 1\n";
    if ( Type == CBA_NODE_SHARP ) return "10 1\n";
    if ( Type == CBA_NODE_MUX )   return "11- 1\n0-1 1\n";
    if ( Type == CBA_NODE_MAJ )   return "11- 1\n1-1 1\n-11 1\n";
    assert( 0 );
    return "???";
}


/*=== cbaBlast.c =========================================================*/
extern Gia_Man_t * Cba_ManExtract( Cba_Man_t * p, int fBuffers, int fVerbose );
extern Cba_Man_t * Cba_ManInsertGia( Cba_Man_t * p, Gia_Man_t * pGia );
extern void *      Cba_ManInsertAbc( Cba_Man_t * p, void * pAbc );
/*=== cbaBuild.c =========================================================*/
extern Cba_Man_t * Cba_ManBuild( Cba_Man_t * p );
/*=== cbaReadBlif.c =========================================================*/
extern Cba_Man_t * Cba_PrsReadBlif( char * pFileName );
/*=== cbaReadVer.c ==========================================================*/
extern Cba_Man_t * Cba_PrsReadVerilog( char * pFileName, int fBinary );
/*=== cbaWriteBlif.c ========================================================*/
extern void        Cba_PrsWriteBlif( char * pFileName, Cba_Man_t * p );
extern void        Cba_ManWriteBlif( char * pFileName, Cba_Man_t * p );
/*=== cbaWriteVer.c =========================================================*/
extern void        Cba_PrsWriteVerilog( char * pFileName, Cba_Man_t * p );
extern void        Cba_ManWriteVerilog( char * pFileName, Cba_Man_t * p );
/*=== cbaNtk.c =========================================================*/
extern void        Cba_ManAssignInternNames( Cba_Man_t * p );
extern int         Cba_NtkNodeNum( Cba_Ntk_t * p );
extern int         Cba_ManObjNum( Cba_Man_t * p );
extern Cba_Man_t * Cba_ManDupStart( Cba_Man_t * p, Vec_Int_t * vNtkSizes );
extern Cba_Man_t * Cba_ManDup( Cba_Man_t * p );
/*=== cbaSimple.c =========================================================*/
extern Cba_Man_t * Cba_PrsReadPtr( Vec_Ptr_t * vDes );
extern void        Ptr_ManFreeDes( Vec_Ptr_t * vDes );


ABC_NAMESPACE_HEADER_END

#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

