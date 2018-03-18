/****************************************************************/
/* Parallel Combinatorial BLAS Library (for Graph Computations) */
/* version 1.6 -------------------------------------------------*/
/* date: 11/15/2016 --------------------------------------------*/
/* authors: Ariful Azad, Aydin Buluc, Adam Lugowski ------------*/
/****************************************************************/
/*
 Copyright (c) 2010-2016, The Regents of the University of California
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */


#ifndef COMBBLAS_H
#define COMBBLAS_H

// These macros should be defined before stdint.h is included
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>

#if defined(COMBBLAS_BOOST)
	#ifdef CRAYCOMP
		#include <boost/config/compiler/cray.hpp>
	#endif
	#include <boost/tr1/memory.hpp>
	#include <boost/tr1/unordered_map.hpp>
	#include <boost/tr1/tuple.hpp>
	#define joker boost	// namespace
#elif defined(COMBBLAS_TR1)
	#include <tr1/memory>
	#include <tr1/unordered_map>
	#include <tr1/tuple>
 	#include <tr1/type_traits>
	#define joker std::tr1
#elif defined(_MSC_VER) && (_MSC_VER < 1600)
	#include <memory>
	#include <unordered_map>
	#include <tuple>
	#include <type_traits>
	#define joker std::tr1
#else // C++11
	#include <memory>
	#include <unordered_map>
	#include <tuple>
	#include <type_traits>
	#define joker std
#endif
// for VC2008


// Just in case the -fopenmp didn't define _OPENMP by itself
#ifdef THREADED
	#ifndef _OPENMP
	#define _OPENMP
	#endif
#endif

#ifdef _OPENMP
	#include <omp.h>
#endif


//#ifdef _MSC_VER
//#pragma warning( disable : 4244 ) // conversion from 'int64_t' to 'double', possible loss of data
//#endif

extern int cblas_splits;
extern double cblas_alltoalltime;
extern double cblas_allgathertime;
extern double cblas_localspmvtime;
extern double cblas_mergeconttime;
extern double cblas_transvectime;


extern double mcl_Abcasttime;
extern double mcl_Bbcasttime;
extern double mcl_localspgemmtime;
extern double mcl_multiwaymergetime;
extern double mcl_kselecttime;
extern double mcl_prunecolumntime;



// An adapter function that allows using extended-callback EWiseApply with plain-old binary functions that don't want the extra parameters.
template <typename RETT, typename NU1, typename NU2, typename BINOP>
class EWiseExtToPlainAdapter
{
	public:
	BINOP plain_binary_op;
	
	EWiseExtToPlainAdapter(BINOP op): plain_binary_op(op) {}
	
	RETT operator()(const NU1& a, const NU2& b, bool aIsNull, bool bIsNull)
	{
		return plain_binary_op(a, b);
	}
};

#include "SpDefs.h"
#include "BitMap.h"
#include "SpTuples.h"
#include "SpDCCols.h"
#include "SpCCols.h"
#include "SpParMat.h"
#include "FullyDistVec.h"
#include "FullyDistSpVec.h"
#include "VecIterator.h"
#include "PreAllocatedSPA.h"
#include "ParFriends.h"
#include "BFSFriends.h"
#include "DistEdgeList.h"
#include "Semirings.h"
#include "Operations.h"
#include "MPIOp.h"
#include "MPIType.h"

#endif
