/**CFile****************************************************************

  FileName    [wlc.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Verilog parser.]

  Synopsis    [External declarations.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - August 22, 2014.]

  Revision    [$Id: wlc.h,v 1.00 2014/09/12 00:00:00 alanmi Exp $]

***********************************************************************/

#ifndef ABC__base__wlc__wlc_h
#define ABC__base__wlc__wlc_h


////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "aig/gia/gia.h"
#include "misc/extra/extra.h"
#include "misc/util/utilNam.h"
#include "misc/mem/mem.h"
#include "misc/extra/extra.h"
#include "misc/util/utilTruth.h"
#include "base/main/mainInt.h"

////////////////////////////////////////////////////////////////////////
///                         PARAMETERS                               ///
////////////////////////////////////////////////////////////////////////

ABC_NAMESPACE_HEADER_START 

// object types
typedef enum { 
    WLC_OBJ_NONE = 0,      // 00: unknown
    WLC_OBJ_PI,            // 01: primary input 
    WLC_OBJ_PO,            // 02: primary output (unused)
    WLC_OBJ_FO,            // 03: flop output
    WLC_OBJ_FI,            // 04: flop input (unused)
    WLC_OBJ_FF,            // 05: flop (unused)
    WLC_OBJ_CONST,         // 06: constant
    WLC_OBJ_BUF,           // 07: buffer
    WLC_OBJ_MUX,           // 08: multiplexer
    WLC_OBJ_SHIFT_R,       // 09: shift right
    WLC_OBJ_SHIFT_RA,      // 10: shift right (arithmetic)
    WLC_OBJ_SHIFT_L,       // 11: shift left
    WLC_OBJ_SHIFT_LA,      // 12: shift left (arithmetic)
    WLC_OBJ_ROTATE_R,      // 13: rotate right
    WLC_OBJ_ROTATE_L,      // 14: rotate left
    WLC_OBJ_BIT_NOT,       // 15: bitwise NOT
    WLC_OBJ_BIT_AND,       // 16: bitwise AND
    WLC_OBJ_BIT_OR,        // 17: bitwise OR
    WLC_OBJ_BIT_XOR,       // 18: bitwise XOR
    WLC_OBJ_BIT_SELECT,    // 19: bit selection
    WLC_OBJ_BIT_CONCAT,    // 20: bit concatenation
    WLC_OBJ_BIT_ZEROPAD,   // 21: zero padding
    WLC_OBJ_BIT_SIGNEXT,   // 22: sign extension
    WLC_OBJ_LOGIC_NOT,     // 23: logic NOT
    WLC_OBJ_LOGIC_AND,     // 24: logic AND
    WLC_OBJ_LOGIC_OR,      // 25: logic OR
    WLC_OBJ_LOGIC_XOR,     // 26: logic XOR
    WLC_OBJ_COMP_EQU,      // 27: compare equal
    WLC_OBJ_COMP_NOTEQU,   // 28: compare not equal
    WLC_OBJ_COMP_LESS,     // 29: compare less
    WLC_OBJ_COMP_MORE,     // 30: compare more
    WLC_OBJ_COMP_LESSEQU,  // 31: compare less or equal
    WLC_OBJ_COMP_MOREEQU,  // 32: compare more or equal
    WLC_OBJ_REDUCT_AND,    // 33: reduction AND
    WLC_OBJ_REDUCT_OR,     // 34: reduction OR
    WLC_OBJ_REDUCT_XOR,    // 35: reduction XOR
    WLC_OBJ_ARI_ADD,       // 36: arithmetic addition
    WLC_OBJ_ARI_SUB,       // 37: arithmetic subtraction
    WLC_OBJ_ARI_MULTI,     // 38: arithmetic multiplier
    WLC_OBJ_ARI_DIVIDE,    // 39: arithmetic division
    WLC_OBJ_ARI_MODULUS,   // 40: arithmetic modulus
    WLC_OBJ_ARI_POWER,     // 41: arithmetic power
    WLC_OBJ_ARI_MINUS,     // 42: arithmetic minus
    WLC_OBJ_TABLE,         // 43: bit table
    WLC_OBJ_NUMBER         // 44: unused
} Wlc_ObjType_t;


// Unlike AIG managers and logic networks in ABC, this network treats POs and FIs 
// as attributes of internal nodes and *not* as separate types of objects.


////////////////////////////////////////////////////////////////////////
///                         BASIC TYPES                              ///
////////////////////////////////////////////////////////////////////////

typedef struct Wlc_Obj_t_  Wlc_Obj_t;
struct Wlc_Obj_t_ // 16 bytes
{
    unsigned               Type    :  6;       // node type
    unsigned               Signed  :  1;       // signed
    unsigned               Mark    :  1;       // user mark
    unsigned               fIsPo   :  1;       // this is PO
    unsigned               fIsFi   :  1;       // this is FI
    unsigned               nFanins : 22;       // fanin count
    unsigned               End     : 16;       // range end
    unsigned               Beg     : 16;       // range begin
    union { int            Fanins[2];          // fanin IDs
            int *          pFanins[1]; };
};

typedef struct Wlc_Ntk_t_  Wlc_Ntk_t;
struct Wlc_Ntk_t_ 
{
    char *                 pName;              // model name
    Vec_Int_t              vPis;               // primary inputs 
    Vec_Int_t              vPos;               // primary outputs
    Vec_Int_t              vCis;               // combinational inputs
    Vec_Int_t              vCos;               // combinational outputs
    Vec_Int_t              vFfs;               // flops
    Vec_Int_t *            vInits;             // initial values
    char *                 pInits;             // initial values
    int                    nObjs[WLC_OBJ_NUMBER]; // counter of objects of each type
    int                    nAnds[WLC_OBJ_NUMBER]; // counter of AND gates after blasting
    // memory for objects
    Wlc_Obj_t *            pObjs;
    int                    iObj;
    int                    nObjsAlloc;
    Mem_Flex_t *           pMemFanin;
    Mem_Flex_t *           pMemTable;
    Vec_Ptr_t *            vTables;
    // object names
    Abc_Nam_t *            pManName;           // object names
    Vec_Int_t              vNameIds;           // object name IDs
    Vec_Int_t              vValues;            // value objects
    // object attributes
    int                    nTravIds;           // counter of traversal IDs
    Vec_Int_t              vTravIds;           // trav IDs of the objects
    Vec_Int_t              vCopies;            // object first bits
};

static inline int          Wlc_NtkObjNum( Wlc_Ntk_t * p )                           { return p->iObj - 1;                                                      }
static inline int          Wlc_NtkObjNumMax( Wlc_Ntk_t * p )                        { return p->iObj;                                                          }
static inline int          Wlc_NtkPiNum( Wlc_Ntk_t * p )                            { return Vec_IntSize(&p->vPis);                                            }
static inline int          Wlc_NtkPoNum( Wlc_Ntk_t * p )                            { return Vec_IntSize(&p->vPos);                                            }
static inline int          Wlc_NtkCiNum( Wlc_Ntk_t * p )                            { return Vec_IntSize(&p->vCis);                                            }
static inline int          Wlc_NtkCoNum( Wlc_Ntk_t * p )                            { return Vec_IntSize(&p->vCos);                                            }
static inline int          Wlc_NtkFfNum( Wlc_Ntk_t * p )                            { return Vec_IntSize(&p->vCis) - Vec_IntSize(&p->vPis);                    }

static inline Wlc_Obj_t *  Wlc_NtkObj( Wlc_Ntk_t * p, int Id )                      { assert(Id > 0 && Id < p->nObjsAlloc); return p->pObjs + Id;              }
static inline Wlc_Obj_t *  Wlc_NtkPi( Wlc_Ntk_t * p, int i )                        { return Wlc_NtkObj( p, Vec_IntEntry(&p->vPis, i) );                       }
static inline Wlc_Obj_t *  Wlc_NtkPo( Wlc_Ntk_t * p, int i )                        { return Wlc_NtkObj( p, Vec_IntEntry(&p->vPos, i) );                       }
static inline Wlc_Obj_t *  Wlc_NtkCi( Wlc_Ntk_t * p, int i )                        { return Wlc_NtkObj( p, Vec_IntEntry(&p->vCis, i) );                       }
static inline Wlc_Obj_t *  Wlc_NtkCo( Wlc_Ntk_t * p, int i )                        { return Wlc_NtkObj( p, Vec_IntEntry(&p->vCos, i) );                       }
static inline Wlc_Obj_t *  Wlc_NtkFf( Wlc_Ntk_t * p, int i )                        { return Wlc_NtkObj( p, Vec_IntEntry(&p->vFfs, i) );                       }

static inline int          Wlc_ObjIsPi( Wlc_Obj_t * p )                             { return p->Type == WLC_OBJ_PI;                                            }
static inline int          Wlc_ObjIsPo( Wlc_Obj_t * p )                             { return p->fIsPo;                                                         }
static inline int          Wlc_ObjIsCi( Wlc_Obj_t * p )                             { return p->Type == WLC_OBJ_PI || p->Type == WLC_OBJ_FO;                   }
static inline int          Wlc_ObjIsCo( Wlc_Obj_t * p )                             { return p->fIsPo || p->fIsFi;                                             }

static inline int          Wlc_ObjId( Wlc_Ntk_t * p, Wlc_Obj_t * pObj )             { return pObj - p->pObjs;                                                  }
static inline int          Wlc_ObjCiId( Wlc_Obj_t * p )                             { assert( Wlc_ObjIsCi(p) ); return p->Fanins[1];                           }
static inline int          Wlc_ObjFaninNum( Wlc_Obj_t * p )                         { return p->nFanins;                                                       }
static inline int          Wlc_ObjHasArray( Wlc_Obj_t * p )                         { return p->nFanins > 2 || p->Type == WLC_OBJ_CONST;                       }
static inline int *        Wlc_ObjFanins( Wlc_Obj_t * p )                           { return Wlc_ObjHasArray(p) ? p->pFanins[0] : p->Fanins;                   }
static inline int          Wlc_ObjFaninId( Wlc_Obj_t * p, int i )                   { return Wlc_ObjFanins(p)[i];                                              }
static inline int          Wlc_ObjFaninId0( Wlc_Obj_t * p )                         { return Wlc_ObjFanins(p)[0];                                              }
static inline int          Wlc_ObjFaninId1( Wlc_Obj_t * p )                         { return Wlc_ObjFanins(p)[1];                                              }
static inline int          Wlc_ObjFaninId2( Wlc_Obj_t * p )                         { return Wlc_ObjFanins(p)[2];                                              }
static inline Wlc_Obj_t *  Wlc_ObjFanin( Wlc_Ntk_t * p, Wlc_Obj_t * pObj, int i )   { return Wlc_NtkObj( p, Wlc_ObjFaninId(pObj, i) );                         }
static inline Wlc_Obj_t *  Wlc_ObjFanin0( Wlc_Ntk_t * p, Wlc_Obj_t * pObj )         { return Wlc_NtkObj( p, Wlc_ObjFaninId(pObj, 0) );                         }
static inline Wlc_Obj_t *  Wlc_ObjFanin1( Wlc_Ntk_t * p, Wlc_Obj_t * pObj )         { return Wlc_NtkObj( p, Wlc_ObjFaninId(pObj, 1) );                         }
static inline Wlc_Obj_t *  Wlc_ObjFanin2( Wlc_Ntk_t * p, Wlc_Obj_t * pObj )         { return Wlc_NtkObj( p, Wlc_ObjFaninId(pObj, 2) );                         }

static inline int          Wlc_ObjRange( Wlc_Obj_t * p )                            { return p->End - p->Beg + 1;                                              }
static inline int          Wlc_ObjRangeEnd( Wlc_Obj_t * p )                         { assert(p->Type == WLC_OBJ_BIT_SELECT); return p->Fanins[1] >> 16;        }
static inline int          Wlc_ObjRangeBeg( Wlc_Obj_t * p )                         { assert(p->Type == WLC_OBJ_BIT_SELECT); return p->Fanins[1] & 0xFFFF;     }
static inline int          Wlc_ObjIsSigned( Wlc_Obj_t * p )                         { return p->Signed;                                                        }
static inline int          Wlc_ObjIsSignedFanin01( Wlc_Ntk_t * p, Wlc_Obj_t * pObj ){ return Wlc_ObjFanin0(p, pObj)->Signed && Wlc_ObjFanin1(p, pObj)->Signed; }
static inline int          Wlc_ObjSign( Wlc_Obj_t * p )                             { return Abc_Var2Lit( Wlc_ObjRange(p), Wlc_ObjIsSigned(p) );               }
static inline int *        Wlc_ObjConstValue( Wlc_Obj_t * p )                       { assert(p->Type == WLC_OBJ_CONST);      return Wlc_ObjFanins(p);          }
static inline int          Wlc_ObjTableId( Wlc_Obj_t * p )                          { assert(p->Type == WLC_OBJ_TABLE);      return p->Fanins[1];              }
static inline word *       Wlc_ObjTable( Wlc_Ntk_t * p, Wlc_Obj_t * pObj )          { return (word *)Vec_PtrEntry( p->vTables, Wlc_ObjTableId(pObj) );         }

static inline void         Wlc_NtkCleanCopy( Wlc_Ntk_t * p )                        { Vec_IntFill( &p->vCopies, p->nObjsAlloc, 0 );                            }
static inline int          Wlc_NtkHasCopy( Wlc_Ntk_t * p )                          { return Vec_IntSize( &p->vCopies ) > 0;                                   }
static inline void         Wlc_ObjSetCopy( Wlc_Ntk_t * p, int iObj, int i )         { Vec_IntWriteEntry( &p->vCopies, iObj, i );                               }
static inline int          Wlc_ObjCopy( Wlc_Ntk_t * p, int iObj )                   { return Vec_IntEntry( &p->vCopies, iObj );                                }
static inline Wlc_Obj_t *  Wlc_ObjCopyObj(Wlc_Ntk_t * pNew, Wlc_Ntk_t * p, Wlc_Obj_t * pObj) {return Wlc_NtkObj(pNew, Wlc_ObjCopy(p, Wlc_ObjId(p, pObj)));   }

static inline void         Wlc_NtkCleanNameId( Wlc_Ntk_t * p )                      { Vec_IntFill( &p->vNameIds, p->nObjsAlloc, 0 );                           }
static inline int          Wlc_NtkHasNameId( Wlc_Ntk_t * p )                        { return Vec_IntSize( &p->vNameIds ) > 0;                                  }
static inline void         Wlc_ObjSetNameId( Wlc_Ntk_t * p, int iObj, int i )       { Vec_IntWriteEntry( &p->vNameIds, iObj, i );                              }
static inline int          Wlc_ObjNameId( Wlc_Ntk_t * p, int iObj )                 { return Vec_IntEntry( &p->vNameIds, iObj );                               }

static inline Wlc_Obj_t *  Wlc_ObjFoToFi( Wlc_Ntk_t * p, Wlc_Obj_t * pObj )        { assert( pObj->Type == WLC_OBJ_FO ); return Wlc_NtkCo(p, Wlc_NtkCoNum(p) - Wlc_NtkCiNum(p) + Wlc_ObjCiId(pObj)); } 

////////////////////////////////////////////////////////////////////////
///                      MACRO DEFINITIONS                           ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                             ITERATORS                            ///
////////////////////////////////////////////////////////////////////////

#define Wlc_NtkForEachObj( p, pObj, i )                                             \
    for ( i = 1; (i < Wlc_NtkObjNumMax(p)) && (((pObj) = Wlc_NtkObj(p, i)), 1); i++ )
#define Wlc_NtkForEachObjVec( vVec, p, pObj, i )                                    \
    for ( i = 0; (i < Vec_IntSize(vVec)) && (((pObj) = Wlc_NtkObj(p, Vec_IntEntry(vVec, i))), 1); i++ )
#define Wlc_NtkForEachPi( p, pPi, i )                                               \
    for ( i = 0; (i < Wlc_NtkPiNum(p)) && (((pPi) = Wlc_NtkPi(p, i)), 1); i++ )
#define Wlc_NtkForEachPo( p, pPo, i )                                               \
    for ( i = 0; (i < Wlc_NtkPoNum(p)) && (((pPo) = Wlc_NtkPo(p, i)), 1); i++ )
#define Wlc_NtkForEachCi( p, pCi, i )                                               \
    for ( i = 0; (i < Wlc_NtkCiNum(p)) && (((pCi) = Wlc_NtkCi(p, i)), 1); i++ )
#define Wlc_NtkForEachCo( p, pCo, i )                                               \
    for ( i = 0; (i < Wlc_NtkCoNum(p)) && (((pCo) = Wlc_NtkCo(p, i)), 1); i++ )
#define Wlc_NtkForEachFf( p, pFf, i )                                               \
    for ( i = 0; (i < Vec_IntSize(&p->vFfs)) && (((pFf) = Wlc_NtkFf(p, i)), 1); i++ )

#define Wlc_ObjForEachFanin( pObj, iFanin, i )                                      \
    for ( i = 0; (i < Wlc_ObjFaninNum(pObj)) && (((iFanin) = Wlc_ObjFaninId(pObj, i)), 1); i++ )
#define Wlc_ObjForEachFaninReverse( pObj, iFanin, i )                               \
    for ( i = Wlc_ObjFaninNum(pObj) - 1; (i >= 0) && (((iFanin) = Wlc_ObjFaninId(pObj, i)), 1); i-- )


////////////////////////////////////////////////////////////////////////
///                    FUNCTION DECLARATIONS                         ///
////////////////////////////////////////////////////////////////////////

/*=== wlcAbs.c ========================================================*/
extern int            Wlc_NtkPairIsUifable( Wlc_Ntk_t * p, Wlc_Obj_t * pObj, Wlc_Obj_t * pObj2 );
extern Vec_Int_t *    Wlc_NtkCollectMultipliers( Wlc_Ntk_t * p );
extern Vec_Int_t *    Wlc_NtkFindUifableMultiplierPairs( Wlc_Ntk_t * p );
extern Wlc_Ntk_t *    Wlc_NtkAbstractNodes( Wlc_Ntk_t * pNtk, Vec_Int_t * vNodes );
extern Wlc_Ntk_t *    Wlc_NtkUifNodePairs( Wlc_Ntk_t * pNtk, Vec_Int_t * vPairs );
/*=== wlcBlast.c ========================================================*/
extern Gia_Man_t *    Wlc_NtkBitBlast( Wlc_Ntk_t * p, Vec_Int_t * vBoxIds );
/*=== wlcCom.c ========================================================*/
extern void           Wlc_SetNtk( Abc_Frame_t * pAbc, Wlc_Ntk_t * pNtk );
/*=== wlcNtk.c ========================================================*/
extern Wlc_Ntk_t *    Wlc_NtkAlloc( char * pName, int nObjsAlloc );
extern int            Wlc_ObjAlloc( Wlc_Ntk_t * p, int Type, int Signed, int End, int Beg );
extern int            Wlc_ObjCreate( Wlc_Ntk_t * p, int Type, int Signed, int End, int Beg, Vec_Int_t * vFanins );
extern void           Wlc_ObjSetCi( Wlc_Ntk_t * p, Wlc_Obj_t * pObj );
extern void           Wlc_ObjSetCo( Wlc_Ntk_t * p, Wlc_Obj_t * pObj, int fFlopInput );
extern char *         Wlc_ObjName( Wlc_Ntk_t * p, int iObj );
extern void           Wlc_ObjUpdateType( Wlc_Ntk_t * p, Wlc_Obj_t * pObj, int Type );
extern void           Wlc_ObjAddFanins( Wlc_Ntk_t * p, Wlc_Obj_t * pObj, Vec_Int_t * vFanins );
extern void           Wlc_NtkFree( Wlc_Ntk_t * p );
extern void           Wlc_NtkPrintNodes( Wlc_Ntk_t * p, int Type );
extern void           Wlc_NtkPrintStats( Wlc_Ntk_t * p, int fDistrib, int fVerbose );
extern Wlc_Ntk_t *    Wlc_NtkDupDfs( Wlc_Ntk_t * p );
extern void           Wlc_NtkTransferNames( Wlc_Ntk_t * pNew, Wlc_Ntk_t * p );
/*=== wlcReadSmt.c ========================================================*/
extern Wlc_Ntk_t *    Wlc_ReadSmtBuffer( char * pFileName, char * pBuffer, char * pLimit );
extern Wlc_Ntk_t *    Wlc_ReadSmt( char * pFileName );
/*=== wlcStdin.c ========================================================*/
extern int            Wlc_StdinProcessSmt( Abc_Frame_t * pAbc, char * pCmd );
/*=== wlcReadVer.c ========================================================*/
extern Wlc_Ntk_t *    Wlc_ReadVer( char * pFileName );
/*=== wlcWriteVer.c ========================================================*/
extern void           Wlc_WriteVer( Wlc_Ntk_t * p, char * pFileName );


ABC_NAMESPACE_HEADER_END

#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

