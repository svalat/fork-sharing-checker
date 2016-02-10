/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Note : this file originaly come from MALT (memory profiling tool, also developped by Sébastien Valat at UVSQ).

/*******************  FUNCTION  *********************/
//std
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
//portability issue, used on linux
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//locals
#include "ProcPagemapReader.hpp"

/********************  MACRO  ***********************/
#define BUFFER_SIZE 1024

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{

/*******************  FUNCTION  *********************/
ProcPageMapReader::ProcPageMapReader(void)
{
	//lock and check
	fd = open("/proc/self/pagemap",O_RDONLY);
	if (fd <= 0)
	{
		fprintf(stderr,"Failed to open /proc/self/pagemap : %s\n",strerror(errno));
		fd = -1;
	}
}

/*******************  FUNCTION  *********************/
ProcPageMapReader::~ProcPageMapReader(void)
{
	if (fd != 0)
		close(fd);
}

/*******************  FUNCTION  *********************/
bool ProcPageMapReader::hasProcPagemap ( void )
{
	return fd > 0;
}

/*******************  FUNCTION  *********************/
size_t ProcPageMapReader::internalGetPhysicalSize ( void* ptr, size_t size )
{
	//check
	assert(ptr != NULL);
	assert(size <= BUFFER_SIZE * PAGE_SIZE);
	assert(fd > 0);
	
	//count
	int pages = size / PAGE_SIZE;
	int lastPageSize = size - pages * PAGE_SIZE;
	if (lastPageSize > 0)
		pages++;
	
	//seek
	size_t pos = (size_t)ptr / PAGE_SIZE;
	pos = lseek(fd,sizeof(ProcPageMapEntry) * pos,SEEK_SET);
	if (pos == -1)
	{
		perror("/proc/pagemap read after lseek");
		abort();
	}
	
	//read
	ProcPageMapEntry entries[BUFFER_SIZE];
	size_t tmp = read(fd,entries,sizeof(ProcPageMapEntry) * pages);
	if (tmp == -1)
	{
		perror("/proc/pagemap read after lseek");
		abort();
	}
	assert(tmp == sizeof(ProcPageMapEntry) * pages);
	
	//count physical
	size_t ret = 0;
	for (int i = 0 ; i < pages ; i++)
	{
		if (entries[i].present)
			ret += PAGE_SIZE;
	}
	
	//last page not full
	if (lastPageSize > 0 && entries[pages-1].present)
		ret -= PAGE_SIZE - lastPageSize;
	
	return ret;
}

/*******************  FUNCTION  *********************/
size_t ProcPageMapReader::getPhysicalSize ( void* ptr, size_t size )
{
	//vars
	size_t ret = 0;
	
	//init
	if (fd == 0)
		return 0;
	
	//trivial
	if (ptr == NULL || size == 0)
		return 0;
	if (size < PAGE_SIZE || fd == -1)
		return size;
	
	//loop y splitting
	for (size_t s = 0 ; s < size ; s+=BUFFER_SIZE*PAGE_SIZE)
	{
		if (size - s < BUFFER_SIZE*PAGE_SIZE)
			ret += internalGetPhysicalSize((char*)ptr+s,size - s);
		else
			ret += internalGetPhysicalSize((char*)ptr+s,BUFFER_SIZE*PAGE_SIZE);
	}
	
	return ret;
}

/*******************  FUNCTION  *********************/
void ProcPageMapReader::loadPFNs(void* base, size_t* pfns, size_t pages)
{
	//get PFN of base
	size_t basePFN = (size_t)base / PAGE_SIZE;
	
	//seek to entry
	lseek(fd,sizeof(ProcPageMapEntry) * basePFN,SEEK_SET);
	
	//allocate tmp buffer
	ProcPageMapEntry * buffer = new ProcPageMapEntry[pages];
	
	//read all
	read(fd,buffer,sizeof(ProcPageMapEntry) * pages);
	
	//extract what we want
	for (size_t i = 0 ; i < pages ; i++)
		pfns[i] = (buffer[i].present) ? buffer[i].pfn : 0;
	
	//free
	delete [] buffer;
}

}
