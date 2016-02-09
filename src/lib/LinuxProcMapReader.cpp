/*****************************************************
             PROJECT  : fork-sharing-checking
             VERSION  : 0.1.0
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Note : this file originaly come from MALT (memory profiling tool, also developped by Sébastien Valat at UVSQ).

/********************  HEADERS  *********************/
//standard
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdlib>
//locals
#include "LinuxProcMapReader.hpp"

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{

/*******************  FUNCTION  *********************/
void LinuxProcMapReader::load(void)
{
	//errors
	assert(procMap.empty());
	
	//open proc map
	FILE * fp = fopen("/proc/self/maps","r");
	if(fp == NULL)
	{
		fprintf(stderr,"Failed to read segment mapping in %s : %s.\n","/proc/self/map",strerror(errno));
		abort();
	}
	//loop on entries
	char buffer[4096];
	char ignored[4096];
	char fileName[4096];
	size_t ignored2;
	LinuxProcMapEntry entry;

	//loop on lines
	while (!feof(fp))
	{
		//load buffer
		char * res = fgets(buffer,sizeof(buffer),fp);
		
		//if ok, parse line
		if (res == buffer)
		{
			//parse
			int cnt = sscanf(buffer,"%p-%p %s %p %s %lu %s\n",&(entry.lower),&(entry.upper),ignored,&(entry.offset),ignored,&ignored2,fileName);
			
			//check args
			if (cnt == 7) {
				entry.file = fileName;
			} else if (cnt == 6) {
				entry.file.clear();
			} else {
				fprintf(stderr,"Invalid readline of proc/map entry : %s.\n",buffer);
				abort();
			}
			
			//ok push
			procMap.push_back(entry);
		}
	}
	
	//close
	fclose(fp);
}

/*******************  FUNCTION  *********************/
const LinuxProcMapEntry * LinuxProcMapReader::getEntry(void* addr) const
{
	//search
	for (LinuxProcMap::const_iterator it = procMap.begin() ; it != procMap.end() ; ++it)
	{
		if ((size_t)it->lower <= (size_t)addr && (size_t)it->upper >= (size_t)addr)
			return &(*it);
	}
	
	//ok not found
	return NULL;
}

}
