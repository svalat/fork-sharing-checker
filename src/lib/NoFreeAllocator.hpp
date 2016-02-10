/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Note : this file originaly come from MALT V2 (memory profiling tool, also developped by Sébastien Valat).

//Note : we need this because we will need to do allocation inside segfault handler which cannot be reentrant
//as we mprotect the segmentes also used by malloc.

/**
 * @file NoFreeAllocator.hpp
 * Here we implemented a custom allocator which do not return the memory.
 * As we didn't need to free the memory we can allocate the segments without
 * adding header. It reduce the memory fragmentation and open the door for
 * a faster impelmentation of allocation algorithm. It also permit to get
 * a better view on the no free memory at the end of execution.
**/

#ifndef FSC_NO_FREE_ALLOCATOR_HPP
#define FSC_NO_FREE_ALLOCATOR_HPP

/********************  HEADERS  *********************/
//std C++
#include <cstdlib>
#include <iostream>
//internals
#include <pthread.h>

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{
	
/********************  MACROS  **********************/
/**
 * This is the maximum allocation size. The allocator will request memory to
 * the OS with mmap with this request size.
**/
#define NO_FREE_ALLOC_SEG_SIZE (2*1024ul*1024ul)

/********************  STRUCT  **********************/
/**
 * Descriptor of internal allocator memory segments requested to the system.
 * Each segment act as a stack by incrementing the last used position untill
 * full.
**/
struct NoFreeAllocatorSegment
{
	/** Pointer to the previous allocated segment or NULL is none. **/
	NoFreeAllocatorSegment * prev;
	/** Pointer to the usable part of the segment. **/
	void * data;
	/** First free position into the segment (starting from this->data). **/
	size_t pos;
	/** To keep alignement on 2*sizeof(void*) **/
	void * unused;
};
	
/*********************  CLASS  **********************/
/**
 * @brief Implement a custom no free allocator.
 * 
 * Implement a fast allocator which do never free the allocated segment so its
 * implemented like a stack system.
**/
class NoFreeAllocator
{
	public:
		NoFreeAllocator(void);
		void * allocate(size_t size);
		size_t getMaxSize(void) const;
	private:
		void setupNewSegment(bool useInitSegment = false);
	private:
		/** Keep track of the current non full segment used by the allocator. **/
		NoFreeAllocatorSegment * cur;
		/** Locks to be thread safe **/
		pthread_spinlock_t lock;
		/** first segment to be used to avoid first mmap so infinite loop at init **/
		char initSegment[NO_FREE_ALLOC_SEG_SIZE];
};

/********************  GLOBALS  *********************/
/** Static instance of the allocator for use in all MALT routines. **/
extern NoFreeAllocator gblNoFreeAllocator;

/*******************  FUNCTION  *********************/
/** Short wrapper to ease desactivation of this allocator and usage of the standard one **/
#define FSC_NO_FREE_MALLOC(x) ForkSharingChecker::gblNoFreeAllocator->allocate(x)
#define FSC_NO_FREE_NEW(x) new (FSC_NO_FREE_MALLOC(sizeof(x))) x

/*******************  FUNCTION  *********************/
/** Function to use the global allocator **/
template <class T> T * noFreeMalloc(size_t cnt) {return (T*)gblNoFreeAllocator.allocate(sizeof(T)*cnt);};

}

#endif //FSC_NO_FREE_ALLOCATOR_HPP
