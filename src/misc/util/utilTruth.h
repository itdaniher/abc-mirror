/**CFile****************************************************************

  FileName    [utilTruth.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Truth table manipulation.]

  Synopsis    [Truth table manipulation.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - October 28, 2012.]

  Revision    [$Id: utilTruth.h,v 1.00 2012/10/28 00:00:00 alanmi Exp $]

***********************************************************************/
 
#ifndef ABC__misc__util__utilTruth_h
#define ABC__misc__util__utilTruth_h

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

static word s_Truths6[6] = {
    0xAAAAAAAAAAAAAAAA,
    0xCCCCCCCCCCCCCCCC,
    0xF0F0F0F0F0F0F0F0,
    0xFF00FF00FF00FF00,
    0xFFFF0000FFFF0000,
    0xFFFFFFFF00000000
};

static word s_Truths6Neg[6] = {
    0x5555555555555555,
    0x3333333333333333,
    0x0F0F0F0F0F0F0F0F,
    0x00FF00FF00FF00FF,
    0x0000FFFF0000FFFF,
    0x00000000FFFFFFFF
};

static word s_CMasks6[5] = {
    0x1111111111111111,
    0x0303030303030303,
    0x000F000F000F000F,
    0x000000FF000000FF,
    0x000000000000FFFF
};

////////////////////////////////////////////////////////////////////////
///                      MACRO DEFINITIONS                           ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                    FUNCTION DECLARATIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int  Abc_TtWordNum( int nVars ) { return nVars <= 6 ? 1 : 1 << (nVars-6); }

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Abc_TtNot( word * pOut, int nWords )
{
    int w;
    for ( w = 0; w < nWords; w++ )
        pOut[w] = ~pOut[w];
}
static inline void Abc_TtCopy( word * pOut, word * pIn, int nWords, int fCompl )
{
    int w;
    if ( fCompl )
        for ( w = 0; w < nWords; w++ )
            pOut[w] = ~pIn[w];
    else
        for ( w = 0; w < nWords; w++ )
            pOut[w] = pIn[w];
}
static inline void Abc_TtAnd( word * pOut, word * pIn1, word * pIn2, int nWords, int fCompl )
{
    int w;
    if ( fCompl )
        for ( w = 0; w < nWords; w++ )
            pOut[w] = ~(pIn1[w] & pIn2[w]);
    else
        for ( w = 0; w < nWords; w++ )
            pOut[w] = pIn1[w] & pIn2[w];
}
static inline int Abc_TtEqual( word * pIn1, word * pIn2, int nWords )
{
    int w;
    for ( w = 0; w < nWords; w++ )
        if ( pIn1[w] != pIn2[w] )
            return 0;
    return 1;
}
static inline int Abc_TtCompare( word * pIn1, word * pIn2, int nWords )
{
    int w;
    for ( w = 0; w < nWords; w++ )
        if ( pIn1[w] != pIn2[w] )
            return (pIn1[w] < pIn2[w]) ? -1 : 1;
    return 0;
}
static inline int Abc_TtCompareRev( word * pIn1, word * pIn2, int nWords )
{
    int w;
    for ( w = nWords - 1; w >= 0; w-- )
        if ( pIn1[w] != pIn2[w] )
            return (pIn1[w] < pIn2[w]) ? -1 : 1;
    return 0;
}

 
/**Function*************************************************************

  Synopsis    [Compares Cof0 and Cof1.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_TtCompare1VarCofs( word * pTruth, int nWords, int iVar )
{
    if ( nWords == 1 )
    {
        word Cof0 = pTruth[0] & s_Truths6Neg[iVar];
        word Cof1 = (pTruth[0] >> (1 << iVar)) & s_Truths6Neg[iVar];
        if ( Cof0 != Cof1 )
            return Cof0 < Cof1 ? -1 : 1;
        return 0;
    }
	if ( iVar <= 5 )
	{
        word Cof0, Cof1;
		int w, shift = (1 << iVar);
		for ( w = 0; w < nWords; w++ )
        {
            Cof0 = pTruth[w] & s_Truths6Neg[iVar];
            Cof1 = (pTruth[w] >> shift) & s_Truths6Neg[iVar];
            if ( Cof0 != Cof1 )
                return Cof0 < Cof1 ? -1 : 1;
        }
        return 0;
	}
	// if ( iVar > 5 )
	{
        word * pLimit = pTruth + nWords;
		int i, iStep = Abc_TtWordNum(iVar);
        assert( nWords >= 2 );
		for ( ; pTruth < pLimit; pTruth += 2*iStep )
			for ( i = 0; i < iStep; i++ )
                if ( pTruth[i] != pTruth[i + iStep] )
                    return pTruth[i] < pTruth[i + iStep] ? -1 : 1;
        return 0;
	}	
}
static inline int Abc_TtCompare1VarCofsRev( word * pTruth, int nWords, int iVar )
{
    if ( nWords == 1 )
    {
        word Cof0 = pTruth[0] & s_Truths6Neg[iVar];
        word Cof1 = (pTruth[0] >> (1 << iVar)) & s_Truths6Neg[iVar];
        if ( Cof0 != Cof1 )
            return Cof0 < Cof1 ? -1 : 1;
        return 0;
    }
	if ( iVar <= 5 )
	{
        word Cof0, Cof1;
		int w, shift = (1 << iVar);
		for ( w = nWords - 1; w >= 0; w-- )
        {
            Cof0 = pTruth[w] & s_Truths6Neg[iVar];
            Cof1 = (pTruth[w] >> shift) & s_Truths6Neg[iVar];
            if ( Cof0 != Cof1 )
                return Cof0 < Cof1 ? -1 : 1;
        }
        return 0;
	}
	// if ( iVar > 5 )
	{
        word * pLimit = pTruth + nWords;
		int i, iStep = Abc_TtWordNum(iVar);
        assert( nWords >= 2 );
		for ( pLimit -= 2*iStep; pLimit >= pTruth; pLimit -= 2*iStep )
			for ( i = iStep - 1; i >= 0; i-- )
                if ( pLimit[i] != pLimit[i + iStep] )
                    return pLimit[i] < pLimit[i + iStep] ? -1 : 1;
        return 0;
	}	
}

/**Function*************************************************************

  Synopsis    [Checks pairs of cofactors w.r.t. adjacent variables.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_TtCheckEqual2VarCofs( word * pTruth, int nWords, int iVar, int Num1, int Num2 )
{
    assert( Num1 < Num2 && Num2 < 4 );
    if ( nWords == 1 )
        return ((pTruth[0] >> (Num2 * (1 << iVar))) & s_CMasks6[iVar]) == ((pTruth[0] >> (Num1 * (1 << iVar))) & s_CMasks6[iVar]);
	if ( iVar <= 4 )
	{
		int w, shift = (1 << iVar);
		for ( w = 0; w < nWords; w++ )
            if ( ((pTruth[w] >> Num2 * shift) & s_CMasks6[iVar]) != ((pTruth[w] >> Num1 * shift) & s_CMasks6[iVar]) )
                return 0;
        return 1;
	}
	if ( iVar == 5 )
	{
        unsigned * pTruthU = (unsigned *)pTruth;
        unsigned * pLimitU = (unsigned *)(pTruth + nWords);
        assert( nWords >= 2 );
		for ( ; pTruthU < pLimitU; pTruthU += 4 )
            if ( pTruthU[Num2] != pTruthU[Num1] )
                return 0;
        return 1;
	}
	// if ( iVar > 5 )
	{
        word * pLimit = pTruth + nWords;
		int i, iStep = Abc_TtWordNum(iVar);
        assert( nWords >= 4 );
		for ( ; pTruth < pLimit; pTruth += 4*iStep )
			for ( i = 0; i < iStep; i++ )
                if ( pTruth[i+Num2*iStep] != pTruth[i+Num1*iStep] )
                    return 0;
        return 1;
	}	
}
static inline int Abc_TtCompare2VarCofs( word * pTruth, int nWords, int iVar, int Num1, int Num2 )
{
    assert( Num1 < Num2 && Num2 < 4 );
    if ( nWords == 1 )
    {
        word Cof1 = (pTruth[0] >> (Num1 * (1 << iVar))) & s_CMasks6[iVar];
        word Cof2 = (pTruth[0] >> (Num2 * (1 << iVar))) & s_CMasks6[iVar];
        if ( Cof1 != Cof2 )
            return Cof1 < Cof2 ? -1 : 1;
        return 0;
    }
	if ( iVar <= 4 )
	{
        word Cof1, Cof2;
		int w, shift = (1 << iVar);
		for ( w = 0; w < nWords; w++ )
        {
            Cof1 = (pTruth[w] >> Num1 * shift) & s_CMasks6[iVar];
            Cof2 = (pTruth[w] >> Num2 * shift) & s_CMasks6[iVar];
            if ( Cof1 != Cof2 )
                return Cof1 < Cof2 ? -1 : 1;
        }
        return 0;
	}
	if ( iVar == 5 )
	{
        unsigned * pTruthU = (unsigned *)pTruth;
        unsigned * pLimitU = (unsigned *)(pTruth + nWords);
        assert( nWords >= 2 );
		for ( ; pTruthU < pLimitU; pTruthU += 4 )
            if ( pTruthU[Num1] != pTruthU[Num2] )
                return pTruthU[Num1] < pTruthU[Num2] ? -1 : 1;
        return 0;
	}
	// if ( iVar > 5 )
	{
        word * pLimit = pTruth + nWords;
		int i, iStep = Abc_TtWordNum(iVar);
        int Offset1 = Num1*iStep;
        int Offset2 = Num2*iStep;
        assert( nWords >= 4 );
		for ( ; pTruth < pLimit; pTruth += 4*iStep )
			for ( i = 0; i < iStep; i++ )
                if ( pTruth[i + Offset1] != pTruth[i + Offset2] )
                    return pTruth[i + Offset1] < pTruth[i + Offset2] ? -1 : 1;
        return 0;
	}	
}
static inline int Abc_TtCompare2VarCofsRev( word * pTruth, int nWords, int iVar, int Num1, int Num2 )
{
    assert( Num1 < Num2 && Num2 < 4 );
    if ( nWords == 1 )
    {
        word Cof1 = (pTruth[0] >> (Num1 * (1 << iVar))) & s_CMasks6[iVar];
        word Cof2 = (pTruth[0] >> (Num2 * (1 << iVar))) & s_CMasks6[iVar];
        if ( Cof1 != Cof2 )
            return Cof1 < Cof2 ? -1 : 1;
        return 0;
    }
	if ( iVar <= 4 )
	{
        word Cof1, Cof2;
		int w, shift = (1 << iVar);
		for ( w = nWords - 1; w >= 0; w-- )
        {
            Cof1 = (pTruth[w] >> Num1 * shift) & s_CMasks6[iVar];
            Cof2 = (pTruth[w] >> Num2 * shift) & s_CMasks6[iVar];
            if ( Cof1 != Cof2 )
                return Cof1 < Cof2 ? -1 : 1;
        }
        return 0;
	}
	if ( iVar == 5 )
	{
        unsigned * pTruthU = (unsigned *)pTruth;
        unsigned * pLimitU = (unsigned *)(pTruth + nWords);
        assert( nWords >= 2 );
		for ( pLimitU -= 4; pLimitU >= pTruthU; pLimitU -= 4 )
            if ( pLimitU[Num1] != pLimitU[Num2] )
                return pLimitU[Num1] < pLimitU[Num2] ? -1 : 1;
        return 0;
	}
	// if ( iVar > 5 )
	{
        word * pLimit = pTruth + nWords;
		int i, iStep = Abc_TtWordNum(iVar);
        int Offset1 = Num1*iStep;
        int Offset2 = Num2*iStep;
        assert( nWords >= 4 );
		for ( pLimit -= 4*iStep; pLimit >= pTruth; pLimit -= 4*iStep )
			for ( i = iStep - 1; i >= 0; i-- )
                if ( pLimit[i + Offset1] != pLimit[i + Offset2] )
                    return pLimit[i + Offset1] < pLimit[i + Offset2] ? -1 : 1;
        return 0;
	}	
}

/**Function*************************************************************

  Synopsis    [Checks pairs of cofactors w.r.t. two variables.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_TtCheckEqualCofs( word * pTruth, int nWords, int iVar, int jVar, int Num1, int Num2 )
{
    assert( Num1 < Num2 && Num2 < 4 );
    assert( iVar < jVar );
    if ( nWords == 1 )
    {
        word Mask = s_Truths6Neg[jVar] & s_Truths6Neg[iVar];
        int shift1 = (Num1 >> 1) * (1 << jVar) + (Num1 & 1) * (1 << iVar);
        int shift2 = (Num2 >> 1) * (1 << jVar) + (Num2 & 1) * (1 << iVar);
        return ((pTruth[0] >> shift1) & Mask) == ((pTruth[0] >> shift2) & Mask);
    }
	if ( jVar <= 5 )
	{
        word Mask = s_Truths6Neg[jVar] & s_Truths6Neg[iVar];
        int shift1 = (Num1 >> 1) * (1 << jVar) + (Num1 & 1) * (1 << iVar);
        int shift2 = (Num2 >> 1) * (1 << jVar) + (Num2 & 1) * (1 << iVar);
		int w;
		for ( w = 0; w < nWords; w++ )
            if ( ((pTruth[w] >> shift1) & Mask) != ((pTruth[w] >> shift2) & Mask) )
                return 0;
        return 1;
	}
	if ( iVar <= 5 && jVar > 5 )
	{
        word * pLimit = pTruth + nWords;
        int j, jStep = Abc_TtWordNum(jVar);
		int shift1 = (Num1 & 1) * (1 << iVar);
		int shift2 = (Num2 & 1) * (1 << iVar);
        int Offset1 = (Num1 >> 1) * jStep;
        int Offset2 = (Num2 >> 1) * jStep;
		for ( ; pTruth < pLimit; pTruth += 2*jStep )
			for ( j = 0; j < jStep; j++ )
                if ( ((pTruth[j + Offset1] >> shift1) & s_Truths6Neg[iVar]) != ((pTruth[j + Offset2] >> shift2) & s_Truths6Neg[iVar]) )
                    return 0;
        return 1;
	}
	{
        word * pLimit = pTruth + nWords;
		int j, jStep = Abc_TtWordNum(jVar);
		int i, iStep = Abc_TtWordNum(iVar);
        int Offset1 = (Num1 >> 1) * jStep + (Num1 & 1) * iStep;
        int Offset2 = (Num2 >> 1) * jStep + (Num2 & 1) * iStep;
		for ( ; pTruth < pLimit; pTruth += 2*jStep )
			for ( i = 0; i < jStep; i += 2*iStep )
				for ( j = 0; j < iStep; j++ )
                    if ( pTruth[Offset1 + i + j] != pTruth[Offset2 + i + j] )
                        return 0;
        return 1;
	}	
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Abc_TtCofactor0( word * pTruth, int nWords, int iVar )
{
    if ( nWords == 1 )
        pTruth[0] = ((pTruth[0] & s_Truths6Neg[iVar]) << (1 << iVar)) | (pTruth[0] & s_Truths6Neg[iVar]);
	else if ( iVar <= 5 )
	{
		int w, shift = (1 << iVar);
		for ( w = 0; w < nWords; w++ )
            pTruth[w] = ((pTruth[w] & s_Truths6Neg[iVar]) << shift) | (pTruth[w] & s_Truths6Neg[iVar]);
	}
	else // if ( iVar > 5 )
	{
        word * pLimit = pTruth + nWords;
		int i, iStep = Abc_TtWordNum(iVar);
		for ( ; pTruth < pLimit; pTruth += 2*iStep )
			for ( i = 0; i < iStep; i++ )
                pTruth[i + iStep] = pTruth[i];
	}	
}
static inline void Abc_TtCofactor1( word * pTruth, int nWords, int iVar )
{
    if ( nWords == 1 )
        pTruth[0] = (pTruth[0] & s_Truths6[iVar]) | ((pTruth[0] & s_Truths6[iVar]) >> (1 << iVar));
	else if ( iVar <= 5 )
	{
		int w, shift = (1 << iVar);
		for ( w = 0; w < nWords; w++ )
            pTruth[w] = (pTruth[w] & s_Truths6[iVar]) | ((pTruth[w] & s_Truths6[iVar]) >> shift);
	}
	else // if ( iVar > 5 )
	{
        word * pLimit = pTruth + nWords;
		int i, iStep = Abc_TtWordNum(iVar);
		for ( ; pTruth < pLimit; pTruth += 2*iStep )
			for ( i = 0; i < iStep; i++ )
                pTruth[i] = pTruth[i + iStep];
	}	
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_Tt6Cof0IsConst0( word t, int iVar ) { return (t & s_Truths6Neg[iVar]) == 0;                                          }
static inline int Abc_Tt6Cof0IsConst1( word t, int iVar ) { return (t & s_Truths6Neg[iVar]) == s_Truths6Neg[iVar];                         }
static inline int Abc_Tt6Cof1IsConst0( word t, int iVar ) { return (t & s_Truths6[iVar]) == 0;                                             }
static inline int Abc_Tt6Cof1IsConst1( word t, int iVar ) { return (t & s_Truths6[iVar]) == s_Truths6[iVar];                               }
static inline int Abc_Tt6CofsOpposite( word t, int iVar ) { return ((t >> (1 << iVar)) & s_Truths6Neg[iVar]) == (~t & s_Truths6Neg[iVar]); } 

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_TtTruthIsConst0( word * p, int nWords ) { int w; for ( w = 0; w < nWords; w++ ) if ( p[w] != 0        ) return 0; return 1; }
static inline int Abc_TtTruthIsConst1( word * p, int nWords ) { int w; for ( w = 0; w < nWords; w++ ) if ( p[w] != ~(word)0 ) return 0; return 1; }

static inline int Abc_TtCof0IsConst0( word * t, int nWords, int iVar ) 
{ 
    if ( iVar < 6 )
    {
        int i;
        for ( i = 0; i < nWords; i++ )
            if ( t[i] & s_Truths6Neg[iVar] )
                return 0;
        return 1;
    }
    else
    {
        int i, Step = (1 << (iVar - 6));
        word * tLimit = t + nWords;
        for ( ; t < tLimit; t += 2*Step )
            for ( i = 0; i < Step; i++ )
                if ( t[i] )
                    return 0;
        return 1;
    }
}
static inline int Abc_TtCof0IsConst1( word * t, int nWords, int iVar ) 
{ 
    if ( iVar < 6 )
    {
        int i;
        for ( i = 0; i < nWords; i++ )
            if ( (t[i] & s_Truths6Neg[iVar]) != s_Truths6Neg[iVar] )
                return 0;
        return 1;
    }
    else
    {
        int i, Step = (1 << (iVar - 6));
        word * tLimit = t + nWords;
        for ( ; t < tLimit; t += 2*Step )
            for ( i = 0; i < Step; i++ )
                if ( ~t[i] )
                    return 0;
        return 1;
    }
}
static inline int Abc_TtCof1IsConst0( word * t, int nWords, int iVar ) 
{ 
    if ( iVar < 6 )
    {
        int i;
        for ( i = 0; i < nWords; i++ )
            if ( t[i] & s_Truths6[iVar] )
                return 0;
        return 1;
    }
    else
    {
        int i, Step = (1 << (iVar - 6));
        word * tLimit = t + nWords;
        for ( ; t < tLimit; t += 2*Step )
            for ( i = 0; i < Step; i++ )
                if ( t[i+Step] )
                    return 0;
        return 1;
    }
}
static inline int Abc_TtCof1IsConst1( word * t, int nWords, int iVar ) 
{ 
    if ( iVar < 6 )
    {
        int i;
        for ( i = 0; i < nWords; i++ )
            if ( (t[i] & s_Truths6[iVar]) != s_Truths6[iVar] )
                return 0;
        return 1;
    }
    else
    {
        int i, Step = (1 << (iVar - 6));
        word * tLimit = t + nWords;
        for ( ; t < tLimit; t += 2*Step )
            for ( i = 0; i < Step; i++ )
                if ( ~t[i+Step] )
                    return 0;
        return 1;
    }
}
static inline int Abc_TtCofsOpposite( word * t, int nWords, int iVar ) 
{ 
    if ( iVar < 6 )
    {
        int i, Shift = (1 << iVar);
        for ( i = 0; i < nWords; i++ )
            if ( ((t[i] << Shift) & s_Truths6[iVar]) != (~t[i] & s_Truths6[iVar]) )
                return 0;
        return 1;
    }
    else
    {
        int i, Step = (1 << (iVar - 6));
        word * tLimit = t + nWords;
        for ( ; t < tLimit; t += 2*Step )
            for ( i = 0; i < Step; i++ )
                if ( t[i] != ~t[i+Step] )
                    return 0;
        return 1;
    }
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Abc_TtPrintDigit( int Digit )
{
    assert( Digit >= 0 && Digit < 16 );
    if ( Digit < 10 )
        printf( "%d", Digit );
    else
        printf( "%c", 'A' + Digit-10 );
}
static inline void Abc_TtPrintHex( word * pTruth, int nVars )
{
    word * pThis, * pLimit = pTruth + Abc_TtWordNum(nVars);
    int k;
    assert( nVars >= 2 );
    for ( pThis = pTruth; pThis < pLimit; pThis++ )
        for ( k = 0; k < 16; k++ )
            Abc_TtPrintDigit( (int)(pThis[0] >> (k << 2)) & 15 );
    printf( "\n" );
}
static inline void Abc_TtPrintHexRev( word * pTruth, int nVars )
{
    word * pThis;
    int k;
    assert( nVars >= 2 );
    for ( pThis = pTruth + Abc_TtWordNum(nVars) - 1; pThis >= pTruth; pThis-- )
        for ( k = 15; k >= 0; k-- )
            Abc_TtPrintDigit( (int)(pThis[0] >> (k << 2)) & 15 );
    printf( "\n" );
}
static inline void Abc_TtPrintHexSpecial( word * pTruth, int nVars )
{
    word * pThis;
    int k;
    assert( nVars >= 2 );
    for ( pThis = pTruth + Abc_TtWordNum(nVars) - 1; pThis >= pTruth; pThis-- )
        for ( k = 0; k < 16; k++ )
            Abc_TtPrintDigit( (int)(pThis[0] >> (k << 2)) & 15 );
    printf( "\n" );
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Abc_TtPrintBinary( word * pTruth, int nVars )
{
    word * pThis, * pLimit = pTruth + Abc_TtWordNum(nVars);
    int k;
    assert( nVars >= 2 );
    for ( pThis = pTruth; pThis < pLimit; pThis++ )
        for ( k = 0; k < 64; k++ )
            printf( "%d", Abc_InfoHasBit( (unsigned *)pThis, k ) );
    printf( "\n" );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_TtSuppFindFirst( int Supp )
{
    int i;
    assert( Supp > 0 );
    for ( i = 0; i < 32; i++ )
        if ( Supp & (1 << i) )
            return i;
    return -1;
}
static inline int Abc_TtSuppOnlyOne( int Supp )
{
    assert( Supp > 0 );
    return (Supp & (Supp-1)) == 0;
}
static inline int Abc_TtSuppIsMinBase( int Supp )
{
    assert( Supp > 0 );
    return (Supp & (Supp+1)) == 0;
}
static inline int Abc_Tt6HasVar( word t, int iVar )
{
    return ((t >> (1<<iVar)) & s_Truths6Neg[iVar]) != (t & s_Truths6Neg[iVar]);
}
static inline int Abc_TtHasVar( word * t, int nVars, int iVar )
{
    int nWords = Abc_TtWordNum( nVars );
    assert( iVar < nVars );
    if ( iVar < 6 )
    {
        int i, Shift = (1 << iVar);
        for ( i = 0; i < nWords; i++ )
            if ( ((t[i] >> Shift) & s_Truths6Neg[iVar]) != (t[i] & s_Truths6Neg[iVar]) )
                return 1;
        return 0;
    }
    else
    {
        int i, Step = (1 << (iVar - 6));
        word * tLimit = t + nWords;
        for ( ; t < tLimit; t += 2*Step )
            for ( i = 0; i < Step; i++ )
                if ( t[i] != t[Step+i] )
                    return 1;
        return 0;
    }
}
static inline int Abc_TtSupport( word * t, int nVars )
{
    int v, Supp = 0;
    for ( v = 0; v < nVars; v++ )
        if ( Abc_TtHasVar( t, nVars, v ) )
            Supp |= (1 << v);
    return Supp;
}
static inline int Abc_TtSupportSize( word * t, int nVars )
{
    int v, SuppSize = 0;
    for ( v = 0; v < nVars; v++ )
        if ( Abc_TtHasVar( t, nVars, v ) )
            SuppSize++;
    return SuppSize;
}
static inline int Abc_TtSupportAndSize( word * t, int nVars, int * pSuppSize )
{
    int v, Supp = 0;
    *pSuppSize = 0;
    for ( v = 0; v < nVars; v++ )
        if ( Abc_TtHasVar( t, nVars, v ) )
            Supp |= (1 << v), (*pSuppSize)++;
    return Supp;
}
static inline int Abc_Tt6SupportAndSize( word t, int nVars, int * pSuppSize )
{
    int v, Supp = 0;
    *pSuppSize = 0;
    assert( nVars <= 6 );
    for ( v = 0; v < nVars; v++ )
        if ( Abc_Tt6HasVar( t, v ) )
            Supp |= (1 << v), (*pSuppSize)++;
    return Supp;
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Abc_TtFlip( word * pTruth, int nWords, int iVar )
{
    if ( nWords == 1 )
        pTruth[0] = ((pTruth[0] << (1 << iVar)) & s_Truths6[iVar]) | ((pTruth[0] & s_Truths6[iVar]) >> (1 << iVar));
	else if ( iVar <= 5 )
	{
		int w, shift = (1 << iVar);
		for ( w = 0; w < nWords; w++ )
            pTruth[w] = ((pTruth[w] << shift) & s_Truths6[iVar]) | ((pTruth[w] & s_Truths6[iVar]) >> shift);
	}
	else // if ( iVar > 5 )
	{
        word * pLimit = pTruth + nWords;
		int i, iStep = Abc_TtWordNum(iVar);
		for ( ; pTruth < pLimit; pTruth += 2*iStep )
			for ( i = 0; i < iStep; i++ )
                ABC_SWAP( word, pTruth[i], pTruth[i + iStep] );
	}	
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Abc_TtSwapAdjacent( word * pTruth, int nWords, int iVar )
{
    static word PMasks[5][3] = {
        { 0x9999999999999999, 0x2222222222222222, 0x4444444444444444 },
        { 0xC3C3C3C3C3C3C3C3, 0x0C0C0C0C0C0C0C0C, 0x3030303030303030 },
        { 0xF00FF00FF00FF00F, 0x00F000F000F000F0, 0x0F000F000F000F00 },
        { 0xFF0000FFFF0000FF, 0x0000FF000000FF00, 0x00FF000000FF0000 },
        { 0xFFFF00000000FFFF, 0x00000000FFFF0000, 0x0000FFFF00000000 }
    };
    if ( iVar < 5 )
    {
        int i, Shift = (1 << iVar);
        for ( i = 0; i < nWords; i++ )
            pTruth[i] = (pTruth[i] & PMasks[iVar][0]) | ((pTruth[i] & PMasks[iVar][1]) << Shift) | ((pTruth[i] & PMasks[iVar][2]) >> Shift);
    }
    else if ( iVar == 5 )
    {
        unsigned * pTruthU = (unsigned *)pTruth;
        unsigned * pLimitU = (unsigned *)(pTruth + nWords);
		for ( ; pTruthU < pLimitU; pTruthU += 4 )
            ABC_SWAP( unsigned, pTruthU[1], pTruthU[2] );
    }
    else // if ( iVar > 5 )
    {
        word * pLimit = pTruth + nWords;
		int i, iStep = Abc_TtWordNum(iVar);
		for ( ; pTruth < pLimit; pTruth += 4*iStep )
			for ( i = 0; i < iStep; i++ )
                ABC_SWAP( word, pTruth[i + iStep], pTruth[i + 2*iStep] );
    }
}

static inline void Abc_TtSwapVars( word * pTruth, int nVars, int iVar, int jVar )
{
	static word PPMasks[5][6][3] = {
		{ 
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 0 0  
            { 0x9999999999999999, 0x2222222222222222, 0x4444444444444444 }, // 0 1  
            { 0xA5A5A5A5A5A5A5A5, 0x0A0A0A0A0A0A0A0A, 0x5050505050505050 }, // 0 2 
            { 0xAA55AA55AA55AA55, 0x00AA00AA00AA00AA, 0x5500550055005500 }, // 0 3 
            { 0xAAAA5555AAAA5555, 0x0000AAAA0000AAAA, 0x5555000055550000 }, // 0 4 
            { 0xAAAAAAAA55555555, 0x00000000AAAAAAAA, 0x5555555500000000 }  // 0 5 
        },
		{ 
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 1 0  
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 1 1  
            { 0xC3C3C3C3C3C3C3C3, 0x0C0C0C0C0C0C0C0C, 0x3030303030303030 }, // 1 2 
            { 0xCC33CC33CC33CC33, 0x00CC00CC00CC00CC, 0x3300330033003300 }, // 1 3 
            { 0xCCCC3333CCCC3333, 0x0000CCCC0000CCCC, 0x3333000033330000 }, // 1 4 
            { 0xCCCCCCCC33333333, 0x00000000CCCCCCCC, 0x3333333300000000 }  // 1 5 
        },
		{ 
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 2 0  
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 2 1  
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 2 2 
            { 0xF00FF00FF00FF00F, 0x00F000F000F000F0, 0x0F000F000F000F00 }, // 2 3 
            { 0xF0F00F0FF0F00F0F, 0x0000F0F00000F0F0, 0x0F0F00000F0F0000 }, // 2 4 
            { 0xF0F0F0F00F0F0F0F, 0x00000000F0F0F0F0, 0x0F0F0F0F00000000 }  // 2 5 
        },
		{ 
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 3 0  
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 3 1  
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 3 2 
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 3 3 
            { 0xFF0000FFFF0000FF, 0x0000FF000000FF00, 0x00FF000000FF0000 }, // 3 4 
            { 0xFF00FF0000FF00FF, 0x00000000FF00FF00, 0x00FF00FF00000000 }  // 3 5 
        },
		{ 
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 4 0  
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 4 1  
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 4 2 
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 4 3 
            { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }, // 4 4 
            { 0xFFFF00000000FFFF, 0x00000000FFFF0000, 0x0000FFFF00000000 }  // 4 5 
        }
	};
	if ( iVar == jVar )
		return;
	if ( jVar < iVar )
        ABC_SWAP( int, iVar, jVar );
    assert( iVar < jVar && jVar < nVars );
    if ( nVars <= 6 )
    {
        word * pMasks = PPMasks[iVar][jVar];
        int shift = (1 << jVar) - (1 << iVar);
        pTruth[0] = (pTruth[0] & pMasks[0]) | ((pTruth[0] & pMasks[1]) << shift) | ((pTruth[0] & pMasks[2]) >> shift);
    }
    else
    {
	    if ( jVar <= 5 )
	    {
            word * pMasks = PPMasks[iVar][jVar];
	        int nWords = Abc_TtWordNum(nVars);
		    int w, shift = (1 << jVar) - (1 << iVar);
		    for ( w = 0; w < nWords; w++ )
                pTruth[w] = (pTruth[w] & pMasks[0]) | ((pTruth[w] & pMasks[1]) << shift) | ((pTruth[w] & pMasks[2]) >> shift);
	    }
	    else if ( iVar <= 5 && jVar > 5 )
	    {
	        word low2High, high2Low;
            word * pLimit = pTruth + Abc_TtWordNum(nVars);
            int j, jStep = Abc_TtWordNum(jVar);
		    int shift = 1 << iVar;
		    for ( ; pTruth < pLimit; pTruth += 2*jStep )
			    for ( j = 0; j < jStep; j++ )
			    {
                    low2High = (pTruth[j] & s_Truths6[iVar]) >> shift;
                    high2Low = (pTruth[j+jStep] << shift) & s_Truths6[iVar];
                    pTruth[j] = (pTruth[j] & ~s_Truths6[iVar]) | high2Low;
                    pTruth[j+jStep] = (pTruth[j+jStep] & s_Truths6[iVar]) | low2High;
			    }
	    }
	    else
	    {
            word * pLimit = pTruth + Abc_TtWordNum(nVars);
		    int i, iStep = Abc_TtWordNum(iVar);
		    int j, jStep = Abc_TtWordNum(jVar);
		    for ( ; pTruth < pLimit; pTruth += 2*jStep )
			    for ( i = 0; i < jStep; i += 2*iStep )
				    for ( j = 0; j < iStep; j++ )
                        ABC_SWAP( word, pTruth[iStep + i + j], pTruth[jStep + i + j] );
	    }	
    }
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_TtCountOnesSlow( word t )
{
    t =    (t & 0x5555555555555555) + ((t>> 1) & 0x5555555555555555);
    t =    (t & 0x3333333333333333) + ((t>> 2) & 0x3333333333333333);
    t =    (t & 0x0F0F0F0F0F0F0F0F) + ((t>> 4) & 0x0F0F0F0F0F0F0F0F);
    t =    (t & 0x00FF00FF00FF00FF) + ((t>> 8) & 0x00FF00FF00FF00FF);
    t =    (t & 0x0000FFFF0000FFFF) + ((t>>16) & 0x0000FFFF0000FFFF);
    return (t & 0x00000000FFFFFFFF) +  (t>>32);
}
static inline int Abc_TtCountOnes( word x )
{
    x = x - ((x >> 1) & 0x5555555555555555);   
    x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);    
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0F;    
    x = x + (x >> 8);
    x = x + (x >> 16);
    x = x + (x >> 32); 
    return (int)(x & 0xFF);
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Abc_TtCountOnesInTruth( word * pTruth, int nVars )
{   
    int nWords = Abc_TtWordNum( nVars );
    int k, Counter = 0;
    for ( k = 0; k < nWords; k++ )
        Counter += Abc_TtCountOnes( pTruth[k] );
    return Counter;
}
static inline void Abc_TtCountOnesInCofs( word * pTruth, int nVars, int * pStore )
{
    int i, k, Counter, nWords;
    memset( pStore, 0, sizeof(int) * nVars );
    if ( nVars <= 6 )
    {
        for ( i = 0; i < nVars; i++ )
            pStore[i] = Abc_TtCountOnes( pTruth[0] & s_Truths6Neg[i] );
        return;
    }
    assert( nVars > 6 );
    nWords = Abc_TtWordNum( nVars );
    for ( k = 0; k < nWords; k++ )
    {
        // count 1's for the first six variables
        for ( i = 0; i < 6; i++ )
            pStore[i] += Abc_TtCountOnes( (pTruth[k] & s_Truths6Neg[i]) | ((pTruth[k+1] & s_Truths6Neg[i]) << (1 << i)) );
        // count 1's for all other variables
        Counter = Abc_TtCountOnes( pTruth[k] );
        for ( i = 6; i < nVars; i++ )
            if ( (k & (1 << (i-6))) == 0 )
                pStore[i] += Counter;
        // count 1's for all other variables
        Counter = Abc_TtCountOnes( pTruth[++k] );
        for ( i = 6; i < nVars; i++ )
            if ( (k & (1 << (i-6))) == 0 )
                pStore[i] += Counter;
    } 
}
static inline void Abc_TtCountOnesInCofsSlow( word * pTruth, int nVars, int * pStore )
{
    static int bit_count[256] = {
      0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
      1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
      1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
      2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
      1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
      2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
      2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
      3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
    };
    int i, k, nBytes;
    unsigned char * pTruthC = (unsigned char *)pTruth;
    nBytes = 8 * Abc_TtWordNum( nVars );
    memset( pStore, 0, sizeof(int) * nVars );
    for ( k = 0; k < nBytes; k++ )
    {
        pStore[0] += bit_count[ pTruthC[k] & 0x55 ];
        pStore[1] += bit_count[ pTruthC[k] & 0x33 ];
        pStore[2] += bit_count[ pTruthC[k] & 0x0F ];
        for ( i = 3; i < nVars; i++ )
            if ( (k & (1 << (i-3))) == 0 )
                pStore[i] += bit_count[pTruthC[k]];
    }
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline unsigned Abc_TtSemiCanonicize( word * pTruth, int nVars, char * pCanonPerm )
{
    int pStore[16];
    int nWords = Abc_TtWordNum( nVars );
    int i, Temp, fChange, nOnes;
    unsigned  uCanonPhase = 0;
    assert( nVars <= 16 );
    // normalize polarity    
    nOnes = Abc_TtCountOnesInTruth( pTruth, nVars );
    if ( nOnes > nWords * 32 )
    {
        Abc_TtNot( pTruth, nWords );
        nOnes = nWords*64 - nOnes;
        uCanonPhase |= (1 << nVars);
    }
    // normalize phase
    Abc_TtCountOnesInCofs( pTruth, nVars, pStore );
    for ( i = 0; i < nVars; i++ )
    {
        if ( pStore[i] >= nOnes - pStore[i] )
            continue;
        Abc_TtFlip( pTruth, nWords, i );
        uCanonPhase |= (1 << i);
        pStore[i] = nOnes - pStore[i]; 
    }
    
    do {
        fChange = 0;
        for ( i = 0; i < nVars-1; i++ )
        {
            if ( pStore[i] <= pStore[i+1] )
                continue;            
           
            Temp = pCanonPerm[i];
            pCanonPerm[i] = pCanonPerm[i+1];
            pCanonPerm[i+1] = Temp;
            
            Temp = pStore[i];
            pStore[i] = pStore[i+1];
            pStore[i+1] = Temp;
            
            if ( ((uCanonPhase >> i) & 1) != ((uCanonPhase >> (i+1)) & 1) )
            {
                uCanonPhase ^= (1 << i);
                uCanonPhase ^= (1 << (i+1));
            }
            Abc_TtSwapAdjacent( pTruth, nWords, i );            
            fChange = 1;
        }
    } while ( fChange );
    return uCanonPhase;
}



/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Abc_TtReverseVars( word * pTruth, int nVars )
{
    int k;
    for ( k = 0; k < nVars/2 ; k++ )
        Abc_TtSwapVars( pTruth, nVars, k, nVars - 1 - k );
}
static inline void Abc_TtReverseBits( word * pTruth, int nVars )
{
    static unsigned char pMirror[256] = {
          0, 128,  64, 192,  32, 160,  96, 224,  16, 144,  80, 208,  48, 176, 112, 240,
          8, 136,  72, 200,  40, 168, 104, 232,  24, 152,  88, 216,  56, 184, 120, 248,
          4, 132,  68, 196,  36, 164, 100, 228,  20, 148,  84, 212,  52, 180, 116, 244,
         12, 140,  76, 204,  44, 172, 108, 236,  28, 156,  92, 220,  60, 188, 124, 252,
          2, 130,  66, 194,  34, 162,  98, 226,  18, 146,  82, 210,  50, 178, 114, 242,
         10, 138,  74, 202,  42, 170, 106, 234,  26, 154,  90, 218,  58, 186, 122, 250,
          6, 134,  70, 198,  38, 166, 102, 230,  22, 150,  86, 214,  54, 182, 118, 246,
         14, 142,  78, 206,  46, 174, 110, 238,  30, 158,  94, 222,  62, 190, 126, 254,
          1, 129,  65, 193,  33, 161,  97, 225,  17, 145,  81, 209,  49, 177, 113, 241,
          9, 137,  73, 201,  41, 169, 105, 233,  25, 153,  89, 217,  57, 185, 121, 249,
          5, 133,  69, 197,  37, 165, 101, 229,  21, 149,  85, 213,  53, 181, 117, 245,
         13, 141,  77, 205,  45, 173, 109, 237,  29, 157,  93, 221,  61, 189, 125, 253,
          3, 131,  67, 195,  35, 163,  99, 227,  19, 147,  83, 211,  51, 179, 115, 243,
         11, 139,  75, 203,  43, 171, 107, 235,  27, 155,  91, 219,  59, 187, 123, 251,
          7, 135,  71, 199,  39, 167, 103, 231,  23, 151,  87, 215,  55, 183, 119, 247,
         15, 143,  79, 207,  47, 175, 111, 239,  31, 159,  95, 223,  63, 191, 127, 255
    };
    unsigned char Temp, * pTruthC = (unsigned char *)pTruth;
    int i, nBytes = (nVars > 6) ? (1 << (nVars - 3)) : 8;
    for ( i = 0; i < nBytes/2; i++ )
    {
        Temp = pMirror[pTruthC[i]];
        pTruthC[i] = pMirror[pTruthC[nBytes-1-i]];
        pTruthC[nBytes-1-i] = Temp;
    }
}


/**Function*************************************************************

  Synopsis    [Stretch truthtable to have more input variables.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Abc_TtStretch5( unsigned * pInOut, int nVarS, int nVarB )
{
    int w, i, step, nWords;
    if ( nVarS == nVarB )
        return;
    assert( nVarS < nVarB );
    step = Abc_TruthWordNum(nVarS);
    nWords = Abc_TruthWordNum(nVarB);
    if ( step == nWords )
        return;
    assert( step < nWords );
    for ( w = 0; w < nWords; w += step )
        for ( i = 0; i < step; i++ )
            pInOut[w + i] = pInOut[i];              
}
static inline void Abc_TtStretch6( word * pInOut, int nVarS, int nVarB )
{
    int w, i, step, nWords;
    if ( nVarS == nVarB )
        return;
    assert( nVarS < nVarB );
    step = Abc_Truth6WordNum(nVarS);
    nWords = Abc_Truth6WordNum(nVarB);
    if ( step == nWords )
        return;
    assert( step < nWords );
    for ( w = 0; w < nWords; w += step )
        for ( i = 0; i < step; i++ )
            pInOut[w + i] = pInOut[i];              
}

/*=== utilTruth.c ===========================================================*/


ABC_NAMESPACE_HEADER_END

#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
