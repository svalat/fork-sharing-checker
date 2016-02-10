/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef FSC_COW_TRACKER_HPP
#define FSC_COW_TRACKER_HPP

/********************  HEADERS  *********************/
//standard
#include <map>

/********************  MACROS  **********************/
#define COW_MAX_SEGMENTS (2*4096)

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{

/********************  STRUCT  **********************/
struct COWCallSite
{
	COWCallSite(void);
	int line;
	std::string file;
	std::string func;
};

/********************  STRUCT  **********************/
struct COWEntry
{
	COWEntry(void) {this->count = 0;};
	size_t count;
	COWCallSite callSite;
};

/********************  STRUCT  **********************/
struct COWSegments
{
	void * base;
	size_t size;
};

/*********************  TYPES  **********************/
typedef std::map<void*,COWEntry > COWTrackerStats;
struct LinuxProcMapEntry;

/*********************  CLASS  **********************/
class COWTracker
{
	public:
		COWTracker(void);
		~COWTracker(void);
		void setupProtection(void);
		bool onTouch(void* addr, void* caller);
		void dump(const std::string & filename);
	private:
		bool isValid(void * addr);
		void solveSymbols(void);
		void solveSymbols(const LinuxProcMapEntry & procMapEntry);
		void resetCounter(void);
	private:
		COWTrackerStats stats;
		COWSegments segments[COW_MAX_SEGMENTS];
		size_t segmentCount;
		volatile bool in;
};

}

/*******************  FUNCTION  *********************/
extern "C" {
	void forkSharingCOWTracker(const char * filename,const char * extraName,bool pid);
	void forkSharingCOWDump(const char * filename,const char * extraName,bool pid);
}

#endif //FSC_COW_TRACKER_HPP
