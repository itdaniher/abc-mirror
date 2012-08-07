/**CFile****************************************************************

  FileName    [giaAbsRef2.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Scalable AIG package.]

  Synopsis    [Refinement manager.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: giaAbsRef2.h,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/
 
#ifndef ABC__aig__gia__giaAbsRef2_h
#define ABC__aig__gia__giaAbsRef2_h


////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                         PARAMETERS                               ///
////////////////////////////////////////////////////////////////////////

ABC_NAMESPACE_HEADER_START


////////////////////////////////////////////////////////////////////////
///                         BASIC TYPES                              ///
////////////////////////////////////////////////////////////////////////

typedef struct Rf2_Man_t_ Rf2_Man_t; // refinement manager

////////////////////////////////////////////////////////////////////////
///                      MACRO DEFINITIONS                           ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                    FUNCTION DECLARATIONS                         ///
////////////////////////////////////////////////////////////////////////

/*=== giaAbsRef.c ===========================================================*/
extern Rf2_Man_t *  Rf2_ManStart( Gia_Man_t * pGia );
extern void         Rf2_ManStop( Rf2_Man_t * p, int fProfile );
extern double       Rf2_ManMemoryUsage( Rf2_Man_t * p );
extern Vec_Int_t *  Rf2_ManRefine( Rf2_Man_t * p, Abc_Cex_t * pCex, Vec_Int_t * vMap, int fPropFanout, int fVerbose );



ABC_NAMESPACE_HEADER_END



#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

