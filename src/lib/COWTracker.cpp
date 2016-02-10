/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <fstream>
#include <signal.h>
#include <cassert>
#include <execinfo.h>
//unix
#include <sys/mman.h>
#include <unistd.h>
//internal
#include "LinuxProcMapReader.hpp"
#include "ProcPagemapReader.hpp"
#include "COWTracker.hpp"

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{
	
/********************  GLOBALS  *********************/
static COWTracker gblCOWTracker;
static std::string gblDumpFilename;

/*******************  FUNCTION  *********************/
COWCallSite::COWCallSite(void)
{
	this->line = -1;
}

/*******************  FUNCTION  *********************/
COWTracker::COWTracker(void)
{
	this->segmentCount = 0;
	this->in = false;
}

/*******************  FUNCTION  *********************/
COWTracker::~COWTracker(void)
{
	//avoid to enter again after destructor (data is not there anymore)
	this->in = true;
}

/*******************  FUNCTION  *********************/
void COWTracker::setupProtection(void)
{
	//load proc map
	LinuxProcMapReader reader;
	reader.load();
	
	//ok we first copy to the heap to not end-up with sefault due to protection of the map memory
	segmentCount = 0;
	for(LinuxProcMapReader::const_iterator it = reader.begin() ; it != reader.end() ; ++it)
	{
		size_t size = (size_t)it->upper-(size_t)it->lower;
		
		//only the anon one
		if (it->file.empty() && size >= 64 * 1024)// ok need to find a way to remove this trick of 64K limit
		{
			segments[segmentCount].base = it->lower;
			segments[segmentCount].size = size;
			segmentCount++;
			if (segmentCount >= COW_MAX_SEGMENTS)
			{
				fprintf(stderr,"More than %d entries in proc map, exhaust local ressources !",COW_MAX_SEGMENTS);
				abort();
			}
		}
	}
	
	//disable
	this->in = true;
	
	//loop on them and make anon segments mprotected
	for (int i = 0 ; i < segmentCount ; i++)
		mprotect(segments[i].base,segments[i].size,PROT_READ);
	
	//reenable
	this->in = false;
}

/*******************  FUNCTION  *********************/
bool COWTracker::onTouch(void* addr,void * caller)
{
	//check validity
	if (!isValid(addr))
		return false;
	
	//round to page
	(size_t&)addr &= (~(PAGE_SIZE-1));
	assert((size_t)addr % PAGE_SIZE == 0);
	
	//mprotect the page
	mprotect(addr,PAGE_SIZE,PROT_READ|PROT_WRITE);
	
	//avoir re-entrance issue
	if (this->in)
		return true;
	else
		this->in = true;
	
	//update stats
	stats[caller].count++;
	
	this->in = false;
	return true;
}

/*******************  FUNCTION  *********************/
bool COWTracker::isValid(void* addr)
{
	for (int i = 0 ; i < segmentCount ; i++)
		if (addr >= segments[i].base && addr < (char*)segments[i].base + segments[i].size)
			return true;

	return false;
}

/*******************  FUNCTION  *********************/
void COWTracker::solveSymbols(void)
{
	//load proc map
	LinuxProcMapReader reader;
	reader.load();
	
	//solve for each entry
	for (LinuxProcMapReader::const_iterator it = reader.begin() ; it != reader.end() ; ++it)
		if (it->file.empty() == false && it->file[0] != '[')
			solveSymbols(*it);
}

/*******************  FUNCTION  *********************/
void COWTracker::solveSymbols(const LinuxProcMapEntry& procMapEntry)
{
	//prepare command
	bool hasEntries = false;
	std::stringstream addr2lineCmd;
	addr2lineCmd << "addr2line -C -f -e " << procMapEntry.file;
	std::vector<COWCallSite*> lst;
	bool isSharedLib = false;
	
	//check if shared lib or exe
	if (procMapEntry.file.substr(procMapEntry.file.size()-3) == ".so")
		isSharedLib = true;
	
	//create addr2line args
	for (COWTrackerStats::iterator it = stats.begin() ; it != stats.end() ; ++it)
	{
		if (it->second.callSite.func.empty() && it->first >= procMapEntry.lower && it->first < procMapEntry.upper)
		{
			hasEntries = true;
			if (isSharedLib)
				addr2lineCmd << ' '  << (void*)((size_t)it->first - (size_t)procMapEntry.lower);
			else
				addr2lineCmd << ' '  << (void*)((size_t)it->first);
			lst.push_back(&it->second.callSite);
		}
	}
	
	//hide error if silent
	addr2lineCmd << ' ' << "2>/dev/null";
	
	//if no extry, exit
	if (!hasEntries)
		return;
	
	//run command
	//std::cerr << addr2lineCmd.str() << std::endl;
	FILE * fp = popen(addr2lineCmd.str().c_str(),"r");
	
	//check error, skip resolution
	if (fp == NULL)
	{
		fprintf(stderr,"Fail to use addr2line on %s to load symbols : %s.",procMapEntry.file.c_str(),strerror(errno));
		return;
	}
	
	//read all entries (need big for some big template based C++ application, 
	//seen at cern)
	static char bufferFunc[200*4096];
	static char bufferFile[20*4096];
	size_t i = 0;
	while (!feof(fp))
	{
		//read the two lines
		char * funcRes = fgets(bufferFunc,sizeof(bufferFunc),fp);
		char * fileRes = fgets(bufferFile,sizeof(bufferFile),fp);
		
		if (funcRes != bufferFunc || fileRes != bufferFile)
			break;

		//std::cerr << bufferFunc;
		//std::cerr << bufferFile;

		//check end of line and remove it
		int endLine = strlen(bufferFunc);
		if (bufferFunc[endLine-1] != '\n')
		{
			fprintf(stderr,"Missing \\n at end of line for the function or symbol name read from addr2line : %s.\n",bufferFunc);
			abort();
		}
		bufferFunc[endLine-1] = '\0';

		//check errors
		if (i >= lst.size())
		{
			fprintf(stderr,"Overpass lst size.");
			abort();
		}

		//search ':' separator at end of "file:line" string
		char * sep = strrchr(bufferFile,':');
		if (sep == NULL)
		{
			fprintf(stderr,"Warning, fail to split source location on ':' : %s\n",bufferFile);
		} else {
			*sep='\0';

			//extract line
			lst[i]->line = atoi(sep+1);
			
			//get filename and function name address
			lst[i]->file = bufferFile;//getstring()

			//if (strcmp(bufferFunc,"??") == 0)
			//	lst[i]->function = -1;
			//else
			lst[i]->func = bufferFunc;//getstring()
		}

		//move next
		i++;
		//std::cerr<< std::endl;
	}
	
	//close
	int res = pclose(fp);
	if (res != 0)
	{
		fprintf(stderr,"Get error while using addr2line on %s to load symbols : %s.\n",procMapEntry.file.c_str(),strerror(errno));
		return;
	}

	//error
	if (i =! lst.size())
	{
		fprintf(stderr,"Some entries are missing from addr2line, get %s, but expect %s. (%s)",i,lst.size(),procMapEntry.file.c_str());
		abort();
	}
}

/*******************  FUNCTION  *********************/
void COWTracker::resetCounter(void)
{
	for (COWTrackerStats::iterator it = stats.begin() ; it != stats.end() ; ++it)
		it->second.count = 0;
}

/*******************  FUNCTION  *********************/
struct COWEntrySortFunc
{
	bool operator()(const COWEntry * a,const COWEntry * b) {return a->count > b->count;};
};

/*******************  FUNCTION  *********************/
void COWTracker::dump(const std::string& filename)
{
	this->in = true;
	
	//solve symbols
	solveSymbols();
	
	//prepare output
	std::vector<COWEntry*> entries;
	entries.reserve(stats.size());
	for (COWTrackerStats::iterator it = stats.begin() ; it != stats.end() ; ++it)
		entries.push_back(&it->second);
	
	//sort
	COWEntrySortFunc func;
	std::sort(entries.begin(),entries.end(),func);
	
	//open
	FILE * fp = fopen(filename.c_str(),"w+");
	if (fp == NULL)
	{
		perror(filename.c_str());
		abort();
	}
	
	//header
	fprintf(fp,"#%12s  %40s   %s\n","Touched(KB)","func","source:line");
	
	//print
	for (std::vector<COWEntry*>::iterator it = entries.begin() ; it != entries.end() ; ++it)
	{
		if ((*it)->callSite.line != -1)
			fprintf(fp,"%12lu   %40s    %s:%d\n",(*it)->count*PAGE_SIZE/1024,(*it)->callSite.func.c_str(),(*it)->callSite.file.c_str(),(*it)->callSite.line);
		else
			fprintf(fp,"%12lu   %s\n",(*it)->count*PAGE_SIZE/1024,(*it)->callSite.func.c_str());
	}
		
	//close
	fclose(fp);
	
	//reset to continue
	this->resetCounter();
	
	this->in = false;
}

/*******************  FUNCTION  *********************/
static void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
	//use backtrace
	void * bt[4];
	backtrace(bt,3);

	bool status = ForkSharingChecker::gblCOWTracker.onTouch(si->si_addr,bt[2]);
	
	//this is for the first call
	if (si->si_addr == NULL)
		return;
	
	//ok real error
	if (status == false)
	{
		fprintf(stderr,"Get real segfault on address %p\n",si->si_addr);
		abort();
	}
}

/*******************  FUNCTION  *********************/
static void dumpOnExit(void)
{
	gblCOWTracker.dump(gblDumpFilename);
}

}

/*******************  FUNCTION  *********************/
void forkSharingCOWTracker(const char* filename, const char* extraName, bool pid)
{
	//final dirname
	std::string dirName = filename;
	dirName += extraName;
	if (pid)
	{
		char buffer[16];
		sprintf(buffer,"%d",getpid());
		dirName += "-";
		dirName += buffer;
	}
	dirName += ".txt";
	ForkSharingChecker::gblDumpFilename = dirName;
	
	//setup segfault handler
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = ForkSharingChecker::segfault_sigaction;
	sa.sa_flags   = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL);
	
	//reg atexit
	atexit(ForkSharingChecker::dumpOnExit);
	
	//just to be sure to get resolution bebore protect otherwise get dead segfault on first sefault handling
	siginfo_t info;
	info.si_addr = NULL;
	ForkSharingChecker::segfault_sigaction(0,&info,NULL);
	
	//protect
	ForkSharingChecker::gblCOWTracker.setupProtection();
}

/*******************  FUNCTION  *********************/
void forkSharingCOWDump(const char* filename, const char* extraName, bool pid)
{
	//final dirname
	std::string dirName = filename;
	dirName += extraName;
	if (pid)
	{
		char buffer[16];
		sprintf(buffer,"%d",getpid());
		dirName += "-";
		dirName += buffer;
	}
	dirName += ".txt";
	
	ForkSharingChecker::gblCOWTracker.dump(dirName);
}
