/*****************************************************
             PROJECT  : fork-sharing-checking
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Note : this file originaly come from MALT (memory profiling tool, also developped by Sébastien Valat at UVSQ).

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include <sys/mman.h>
#include "../ProcPagemapReader.hpp"

/***************** USING NAMESPACE ******************/
using namespace ForkSharingChecker;

/*******************  FUNCTION  *********************/
TEST(ProcPagemapReader,structSize)
{
	EXPECT_EQ(8,sizeof(ProcPageMapEntry));
}

/*******************  FUNCTION  *********************/
TEST(ProcPagemapReader,small)
{
	char buffer[1024];
	ProcPageMapReader reader;
	size_t phys = reader.getPhysicalSize(buffer,sizeof(buffer));
	EXPECT_EQ(1024,phys);
}

/*******************  FUNCTION  *********************/
TEST(ProcPagemapReader,medium)
{
	char buffer[4096+1024];
	memset(buffer,0,sizeof(buffer));
	ProcPageMapReader reader;
	size_t phys = reader.getPhysicalSize(buffer,sizeof(buffer));
	EXPECT_EQ(4096+1024,phys);
}

/*******************  FUNCTION  *********************/
TEST(ProcPagemapReader,largeFull)
{
	const size_t size = 32*1024*1024;
	char * buffer = new char[size];
	memset(buffer,0,size);
	ProcPageMapReader reader;
	size_t phys = reader.getPhysicalSize(buffer,size);
	EXPECT_EQ(size,phys);
}

/*******************  FUNCTION  *********************/
TEST(ProcPagemapReader,largeHalfFull)
{
	const size_t size = 32*1024*1024;
	char * buffer = new char[size];
	memset(buffer,0,size/2);
	ProcPageMapReader reader;
	size_t phys = reader.getPhysicalSize(buffer,size);
	EXPECT_LT(size/2,phys);
	if (reader.hasProcPagemap())
		EXPECT_GT(size,phys);
}

/*******************  FUNCTION  *********************/
TEST(ProcPagemapReader,readPFNEmpty)
{
	//allocate
	const size_t size = 32*1024;
	void * buffer = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE,0,0);
	EXPECT_NE((void*)NULL,buffer);
	EXPECT_NE(MAP_FAILED,buffer);
	
	//read
	size_t pages = size/PAGE_SIZE;
	size_t map[pages];
	ProcPageMapReader reader;
	reader.loadPFNs(buffer,map,pages);
	
	//check
	for(int i = 0 ; i < pages ; i++)
		EXPECT_EQ(0,map[i]);
	
	//clean mem
	munmap(buffer,size);
}

/*******************  FUNCTION  *********************/
TEST(ProcPagemapReader,readPFNFull)
{
	//allocate
	const size_t size = 32*1024;
	void * buffer = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE,0,0);
	EXPECT_NE((void*)NULL,buffer);
	EXPECT_NE(MAP_FAILED,buffer);
	
	//first touch
	memset(buffer,0,size);
	
	//read
	size_t pages = size/PAGE_SIZE;
	size_t map[pages];
	ProcPageMapReader reader;
	reader.loadPFNs(buffer,map,pages);
	
	//check
	for(int i = 0 ; i < pages ; i++)
		EXPECT_NE(0,map[i]);
	
	//clean mem
	munmap(buffer,size);
}
