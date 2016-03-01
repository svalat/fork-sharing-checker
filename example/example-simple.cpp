/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.2.0-dev
             DATE     : 03/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include "../src/lib/Checker.h"

/*******************  FUNCTION  *********************/
int main(int argc,char ** argv)
{
	//allocate some memory
	size_t size = 64 * 1024 * 1024;
	char * sharedMem = new char[size];
	
	//touch it to be sure about mapping
	memset(sharedMem,0,size);
	
	//forking
	pid_t pid = fork();
	
	//out name
	const char * extra = (pid == 0) ? "-1" : "-2";
	
	//now dump before touching
	forkSharingChecker("example-dump-before",extra,false);
	forkSharingCOWTracker("example-cause",extra,false);
	
	//each touch half
	memset(sharedMem,0,size/2);
	
	//new allocation not shared
	char * notShared = new char[size];
	
	//now dump after touching
	forkSharingChecker("example-dump-after",extra,false);
	
	//exit
	EXIT_SUCCESS;
}