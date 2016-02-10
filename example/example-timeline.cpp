/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <cstdio>
#include "../src/lib/Checker.h"

/*******************  FUNCTION  *********************/
int main(int argc,char ** argv)
{
	//allocate some memory
	size_t size = 16 * 1024 * 1024;
	char * sharedMem1 = new char[size];
	char * sharedMem2 = new char[size];
	char * sharedMem3 = new char[size];
	
	//touch it to be sure about mapping
	memset(sharedMem1,0,size);
	memset(sharedMem2,0,size);
	memset(sharedMem3,0,size);
	
	//forking
	pid_t pid = fork();
	
	//out name
	const char * extra = (pid == 0) ? "child" : "parent";
	
	//allocate new
	char * notSharedMem = new char[2*size];
	
	//free middle shared
	delete [] sharedMem2;
	
	//steps
	for (int i = 0 ; i < 10 ; i++)
	{
		memset(sharedMem1,pid,i*size/20);
		memset(sharedMem3,pid,i*size/20);
		memset(notSharedMem,pid,i*size/10);
		
		char buffer[64];
		sprintf(buffer,"-%s-%d",extra,i);
		forkSharingChecker("example-history-dump",buffer,false);
	}
	
	//exit
	EXIT_SUCCESS;
}