/*****************************************************
             PROJECT  : fork-sharing-checking
             VERSION  : 0.1.0
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include "../lib/ForkSharingChecker.h"

/*******************  FUNCTION  *********************/
int main(int argc,char ** argv)
{
	//allocate some memory
	size_t size = 32 * 1024 * 1024;
	char * sharedMem = new char[size];
	
	//touch it to be sure about mapping
	memset(sharedMem,0,size);
	
	//forking
	pid_t pid = fork();
	
	//out name
	const char * extra = (pid == 0) ? "-1" : "-2";
	
	//now dump before touching
	forkSharingCheckerDump("example-dump-before",extra,false);
	
	//each touch half
	memset(sharedMem,0,size/2);
	
	//now dump after touching
	forkSharingCheckerDump("example-dump-after",extra,false);
	
	//exit
	EXIT_SUCCESS;
}