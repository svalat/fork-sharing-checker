/*****************************************************
             PROJECT  : fork-sharing-checking
             VERSION  : 0.1.0
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstdio>
#include <cstdlib>
#include <cstring>
//unix
#include <libgen.h>
//locals
#include "../lib/Reader.hpp"
#include "../lib/ProcPagemapReader.hpp"

/*******************  NAMESPACE  ********************/
using namespace ForkSharingChecker;

/********************  GLOBALS  *********************/
static const char * cstHelp = "fork-sharing-checker {REF} {TARGET}\n";

/*******************  FUNCTION  *********************/
static void showHelp(FILE * fp = stdout)
{
	fprintf(fp,cstHelp);
}

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//error bad args
	if (argc != 3)
	{
		showHelp(stderr);
		abort();
	}
	
	//load
	Reader ref(argv[1]);
	Reader target(argv[2]);
	
	//header
	printf("%-40s %-10s %s   %s\n","#File","Size(KB)","Mapped","Shared");
	
	//total
	size_t totalPages = 0;
	size_t totalMapped = 0;
	size_t totalShared = 0;
	
	//loop on target
	for (Reader::const_iterator it = target.begin() ; it != target.end() ; ++it)
	{
		//some vars
		const ReaderEntry & t = *it;
		const ReaderEntry * lastRef = NULL;
		size_t shared = 0;
		size_t mapped = 0;
		
		//loop on pages from target
		for (void * i = t.base ; i < t.end ; (size_t&)i += PAGE_SIZE)
		{
			//load ref
			if (lastRef == NULL)
				lastRef = ref.getEntry(i);
			else if (lastRef->contain(i) == false)
				lastRef = ref.getEntry(i);
			
			//compare PFNs
			size_t targetPFN = t.getPFN(i);
			size_t refPFN = 0;
			
			if (lastRef != NULL)
				refPFN = lastRef->getPFN(i);
			
			//update counter
			if (targetPFN != 0)
				mapped++;
			if (targetPFN == refPFN && refPFN != 0)
				shared++;
		}
		
		//print
		char fname[4096] = "Anonymous";
		if (t.file.empty() == false)
			strcpy(fname,t.file.c_str());
		printf("%-40s %-10lu %3lu      %3lu\n",
				basename(fname),
				t.pages * PAGE_SIZE/1024,
				(100*mapped)/t.pages,
				(100*shared)/t.pages);
		
		//update total
		totalPages += t.pages;
		totalMapped += mapped;
		totalShared += shared;
	}
	
	//final
	printf("%-40s %-10lu %3lu      %3lu\n",
				"#TOTAL",
				totalPages * PAGE_SIZE/1024,
				(100*totalMapped)/totalPages,
				(100*totalShared)/totalPages);
	
	//ok finish
	return EXIT_SUCCESS;
}

