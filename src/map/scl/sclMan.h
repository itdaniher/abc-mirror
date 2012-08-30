/**CFile****************************************************************

  FileName    [sclMan.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Standard-cell library representation.]

  Synopsis    [Timing/gate-sizing manager.]

  Author      [Alan Mishchenko, Niklas Een]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - August 24, 2012.]

  Revision    [$Id: sclMan.h,v 1.0 2012/08/24 00:00:00 alanmi Exp $]

***********************************************************************/

#ifndef ABC__map__scl__sclMan_h
#define ABC__map__scl__sclMan_h


////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

ABC_NAMESPACE_HEADER_START

////////////////////////////////////////////////////////////////////////
///                         PARAMETERS                               ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                    STRUCTURE DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

typedef struct SC_Pair_         SC_Pair;
typedef struct SC_Man_          SC_Man;

struct SC_Pair_ 
{
    float          rise;
    float          fall;
};

struct SC_Man_ 
{
    SC_Lib *       pLib;       // library
    Abc_Ntk_t *    pNtk;       // network
    Vec_Int_t *    vGates;     // mapping of objId into gateId
    int            nObjs;      // allocated size
    SC_Pair *      pLoads;     // loads for each gate
    SC_Pair *      pTimes;     // arrivals for each gate
    SC_Pair *      pSlews;     // slews for each gate
    SC_Pair *      pTimes2;    // arrivals for each gate
    SC_Pair *      pSlews2;    // slews for each gate
    char *         pWLoadUsed; // name of the used WireLoad model
    clock_t        clkStart;   // starting time
    float          SumArea;    // total area
    float          MaxDelay;   // max delay
    float          SumArea0;   // total area at the begining 
    float          MaxDelay0;  // max delay at the begining
};

////////////////////////////////////////////////////////////////////////
///                       GLOBAL VARIABLES                           ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                       MACRO DEFINITIONS                          ///
////////////////////////////////////////////////////////////////////////

static inline SC_Cell * Abc_SclObjCell( SC_Man * p, Abc_Obj_t * pObj )      { return SC_LibCell( p->pLib, Vec_IntEntry(p->vGates, Abc_ObjId(pObj)) ); }

static inline SC_Pair * Abc_SclObjLoad( SC_Man * p, Abc_Obj_t * pObj )      { return p->pLoads + Abc_ObjId(pObj);  }
static inline SC_Pair * Abc_SclObjTime( SC_Man * p, Abc_Obj_t * pObj )      { return p->pTimes + Abc_ObjId(pObj);  }
static inline SC_Pair * Abc_SclObjSlew( SC_Man * p, Abc_Obj_t * pObj )      { return p->pSlews + Abc_ObjId(pObj);  }
static inline SC_Pair * Abc_SclObjTime2( SC_Man * p, Abc_Obj_t * pObj )     { return p->pTimes2 + Abc_ObjId(pObj); }
static inline SC_Pair * Abc_SclObjSlew2( SC_Man * p, Abc_Obj_t * pObj )     { return p->pSlews2 + Abc_ObjId(pObj); }

static inline float     Abc_SclObjTimeMax( SC_Man * p, Abc_Obj_t * pObj )   { return Abc_MaxFloat(Abc_SclObjTime(p, pObj)->rise, Abc_SclObjTime(p, pObj)->fall);  }

static inline void      Abc_SclObjDupFanin( SC_Man * p, Abc_Obj_t * pObj )  { assert( Abc_ObjIsCo(pObj) ); *Abc_SclObjTime(p, pObj) = *Abc_SclObjTime(p, Abc_ObjFanin0(pObj));  }
static inline float     Abc_SclObjGain( SC_Man * p, Abc_Obj_t * pObj )      { return (Abc_SclObjTime2(p, pObj)->rise - Abc_SclObjTime(p, pObj)->rise) + (Abc_SclObjTime2(p, pObj)->fall - Abc_SclObjTime(p, pObj)->fall); }

static inline double    Abc_SclObjLoadFf( SC_Man * p, Abc_Obj_t * pObj, int fRise ) { return SC_LibCapFf( p->pLib, fRise ? Abc_SclObjLoad(p, pObj)->rise : Abc_SclObjLoad(p, pObj)->fall); }
static inline double    Abc_SclObjTimePs( SC_Man * p, Abc_Obj_t * pObj, int fRise ) { return SC_LibTimePs(p->pLib, fRise ? Abc_SclObjTime(p, pObj)->rise : Abc_SclObjTime(p, pObj)->fall); }
static inline double    Abc_SclObjSlewPs( SC_Man * p, Abc_Obj_t * pObj, int fRise ) { return SC_LibTimePs(p->pLib, fRise ? Abc_SclObjSlew(p, pObj)->rise : Abc_SclObjSlew(p, pObj)->fall); }

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Constructor/destructor of STA manager.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline SC_Man * Abc_SclManAlloc( SC_Lib * pLib, Abc_Ntk_t * pNtk )
{
    SC_Man * p;
    assert( Abc_NtkHasMapping(pNtk) );
    p = ABC_CALLOC( SC_Man, 1 );
    p->pLib     = pLib;
    p->pNtk     = pNtk;
    p->nObjs    = Abc_NtkObjNumMax(pNtk);
    p->pLoads   = ABC_CALLOC( SC_Pair, p->nObjs );
    p->pTimes   = ABC_CALLOC( SC_Pair, p->nObjs );
    p->pSlews   = ABC_CALLOC( SC_Pair, p->nObjs );
    p->pTimes2  = ABC_CALLOC( SC_Pair, p->nObjs );
    p->pSlews2  = ABC_CALLOC( SC_Pair, p->nObjs );
    p->clkStart = clock();
    return p;
}
static inline void Abc_SclManFree( SC_Man * p )
{
    Vec_IntFreeP( &p->vGates );
    ABC_FREE( p->pLoads );
    ABC_FREE( p->pTimes );
    ABC_FREE( p->pSlews );
    ABC_FREE( p->pTimes2 );
    ABC_FREE( p->pSlews2 );
    ABC_FREE( p );
}


/**Function*************************************************************

  Synopsis    [Stores/retrivies timing information for the logic cone.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Abc_SclConeStore( SC_Man * p, Vec_Int_t * vCone )
{
    SC_Pair Zero = { 0.0, 0.0 };
    Abc_Obj_t * pObj;
    int i;
    Abc_NtkForEachObjVec( vCone, p->pNtk, pObj, i )
    {
        *Abc_SclObjTime2(p, pObj) = *Abc_SclObjTime(p, pObj); *Abc_SclObjTime(p, pObj) = Zero;
        *Abc_SclObjSlew2(p, pObj) = *Abc_SclObjSlew(p, pObj); *Abc_SclObjSlew(p, pObj) = Zero;
    }
}
static inline void Abc_SclConeRestore( SC_Man * p, Vec_Int_t * vCone )
{
    Abc_Obj_t * pObj;
    int i;
    Abc_NtkForEachObjVec( vCone, p->pNtk, pObj, i )
    {
        *Abc_SclObjTime(p, pObj) = *Abc_SclObjTime2(p, pObj);
        *Abc_SclObjSlew(p, pObj) = *Abc_SclObjSlew2(p, pObj);
    }
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline float Abc_SclGetTotalArea( SC_Man * p )
{
    double Area = 0;
    Abc_Obj_t * pObj;
    int i;
    Abc_NtkForEachNode1( p->pNtk, pObj, i )
        Area += Abc_SclObjCell( p, pObj )->area;
    return Area;
}
static inline float Abc_SclGetMaxDelay( SC_Man * p )
{
    float fMaxArr = 0;
    Abc_Obj_t * pObj;
    SC_Pair * pArr;
    int i;
    Abc_NtkForEachCo( p->pNtk, pObj, i )
    {
        pArr = Abc_SclObjTime( p, pObj );
        if ( fMaxArr < pArr->rise )  fMaxArr = pArr->rise;
        if ( fMaxArr < pArr->fall )  fMaxArr = pArr->fall;
    }
    return fMaxArr;
}


/*=== sclTime.c =============================================================*/
extern Abc_Obj_t * Abc_SclFindCriticalCo( SC_Man * p, int * pfRise );
extern Abc_Obj_t * Abc_SclFindMostCriticalFanin( SC_Man * p, int * pfRise, Abc_Obj_t * pNode );
extern void        Abc_SclTimeNtkPrint( SC_Man * p, int fShowAll );
extern SC_Man *    Abc_SclManStart( SC_Lib * pLib, Abc_Ntk_t * pNtk );
extern void        Abc_SclTimeCone( SC_Man * p, Vec_Int_t * vCone );
/*=== sclTime.c =============================================================*/
extern void        Abc_SclComputeLoad( SC_Man * p );
extern void        Abc_SclUpdateLoad( SC_Man * p, Abc_Obj_t * pObj, SC_Cell * pOld, SC_Cell * pNew );


ABC_NAMESPACE_HEADER_END

#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
