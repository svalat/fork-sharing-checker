/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.2.0-dev
             DATE     : 03/2016
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
#ifndef COW_MINI_STACK_SIZE
	#define COW_MINI_STACK_SIZE 2
#endif

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
	COWCallSite callSite[COW_MINI_STACK_SIZE];
};

/********************  STRUCT  **********************/
struct COWMiniStack
{
	void * stack[COW_MINI_STACK_SIZE];
};

/********************  STRUCT  **********************/
struct COWSegments
{
	void * base;
	size_t size;
};

/*********************  TYPES  **********************/
typedef std::map<COWMiniStack,COWEntry > COWTrackerStats;
struct LinuxProcMapEntry;

/*******************  FUNCTION  *********************/
bool operator < (const COWMiniStack & a,const COWMiniStack & b);

/*********************  CLASS  **********************/
class COWTracker
{
	public:
		COWTracker(void);
		~COWTracker(void);
		void setupProtection(void);
		bool onTouch(void* addr, ForkSharingChecker::COWMiniStack& caller);
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
