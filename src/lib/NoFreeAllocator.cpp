/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
//std C++
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cerrno>
//unix
#include <sys/mman.h>
//current
#include "NoFreeAllocator.hpp"

/***************** USING NAMESPACE ******************/
using namespace std;

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{

/********************  MACRO  ***********************/
/** By default, align on pointer size **/
#define FSC_BASE_ALIGNMENT (sizeof(void*))

/*******************  FUNCTION  *********************/
/** Global instance to be used by all FSC functions. **/
NoFreeAllocator gblNoFreeAllocator;
/** Memory to allocate the initial no free allocator. We use this way to get spinlock init correctly **/
char gblNoFreeAllocatorMem[sizeof(NoFreeAllocator)];

/*******************  FUNCTION  *********************/
/**
 * Init function to setup the allocator. We do not use the class constructor
 * as the instance might be init later than the first object. Thanks to the init()
 * approach we explicitly call the init function when the first FSC hook is used.
 * @param threadsafe Enable internal locking to be thread safe.
**/
NoFreeAllocator::NoFreeAllocator( void )
{
	//defaut state
	this->cur = NULL;
	
	//setup default memory
	this->setupNewSegment(true);
}

/*******************  FUNCTION  *********************/
/**
 * Setup a new big segment.
**/
void NoFreeAllocator::setupNewSegment( bool useInitSegment )
{
	//request to system
	NoFreeAllocatorSegment * segment;
	
	if (useInitSegment)
		segment = (NoFreeAllocatorSegment *)initSegment;
	else
		segment = (NoFreeAllocatorSegment *)mmap(NULL,NO_FREE_ALLOC_SEG_SIZE,PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE,0,0);
	
	//errors
	if (segment==NULL)
	{
		fprintf(stderr,"Failed to request memory from OS : %s !",strerror(errno));
		abort();
	}
	
	//update pointers
	segment->prev = this->cur;
	segment->data = segment+1;
	segment->pos = 0;
	
	//mark as current
	this->cur = segment;
}

/*******************  FUNCTION  *********************/
/**
 * Allocate a new user segment which will never freed.
 * @param size Define the requested size (will be round to FSC_BASE_ALIGNMENT).
 * @return Address of the allocated segment.
**/
void* NoFreeAllocator::allocate(size_t size)
{
	//vars
	void * ret = NULL;

	//round to multiple of pointer size
	if (size % FSC_BASE_ALIGNMENT != 0)
		size += FSC_BASE_ALIGNMENT - size%FSC_BASE_ALIGNMENT;
	
	//check
	assert(size % FSC_BASE_ALIGNMENT == 0);
	if (size > NO_FREE_ALLOC_SEG_SIZE - sizeof(NoFreeAllocatorSegment))
	{
		fprintf(stderr,"No free allocator do not accept requests larger than %lu but get %lu !",
				NO_FREE_ALLOC_SEG_SIZE - sizeof(NoFreeAllocatorSegment),size);
		abort();
	}
	
	//locks
	pthread_spin_lock(&lock);
	
		//check need new segment
		if (cur->pos + size >= NO_FREE_ALLOC_SEG_SIZE)
			setupNewSegment();
		
		//compute addr
		ret = (void*)((size_t)cur->data+cur->pos);
		
		//update status
		cur->pos += size;
	
	pthread_spin_unlock(&lock);
	
	//return
	return ret;
}

/*******************  FUNCTION  *********************/
/**
 * @return Return the maxium authorized allocation size.
**/
size_t NoFreeAllocator::getMaxSize ( void ) const
{
	return NO_FREE_ALLOC_SEG_SIZE - sizeof(NoFreeAllocatorSegment);
}

}
