/**CFile****************************************************************

  FileName    [mpmCore.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Configurable technology mapper.]

  Synopsis    [Core procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 1, 2013.]

  Revision    [$Id: mpmCore.c,v 1.00 2013/06/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "aig/gia/gia.h"
#include "mpmInt.h"

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
void Mpm_ManSetParsDefault( Mpm_Par_t * p )
{
    memset( p, 0, sizeof(Mpm_Par_t) );
    p->pLib           =   NULL;  // LUT library
    p->nNumCuts       =      8;  // cut number
    p->fUseTruth      =      0;  // uses truth tables
    p->fUseDsd        =      0;  // uses DSDs
    p->fCutMin        =      0;  // enables cut minimization
    p->DelayTarget    =     -1;  // delay target
    p->fDeriveLuts    =      0;  // use truth tables to derive AIG structure
    p->fVerbose       =      0;  // verbose output
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_Man_t * Mpm_ManPerformTest( Mig_Man_t * pMig, Mpm_Par_t * pPars )
{
    Gia_Man_t * pNew;
    Mpm_Man_t * p;
    p = Mpm_ManStart( pMig, pPars );
    Mpm_ManPrintStatsInit( p );
    Mpm_ManPrepare( p );
    Mpm_ManPerform( p );
    Mpm_ManPrintStats( p );
    pNew = (Gia_Man_t *)Mpm_ManFromIfLogic( p );
    Mpm_ManStop( p );
    return pNew;
}
Gia_Man_t * Mpm_ManMappingTest( Gia_Man_t * pGia, Mpm_Par_t * pPars )
{
    Mig_Man_t * p;
    Gia_Man_t * pNew;
    p = Mig_ManCreate( pGia );
    pNew = Mpm_ManPerformTest( p, pPars );
    Mig_ManStop( p );
    return pNew;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

