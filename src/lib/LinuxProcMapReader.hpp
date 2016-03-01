/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0
             DATE     : 03/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Note : this file originaly come from MALT (memory profiling tool, also developped by Sébastien Valat at UVSQ).

#ifndef FSC_LINUX_PROC_MAP_HPP
#define FSC_LINUX_PROC_MAP_HPP

/********************  HEADERS  *********************/
//standard
#include <vector>
#include <string>

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{

/********************  STRUCT  **********************/
struct LinuxProcMapEntry
{
	void * lower;
	void * upper;
	void * offset;
	std::string file;
};

/*********************  TYPES  **********************/
typedef std::vector<LinuxProcMapEntry> LinuxProcMap;

/*********************  CLASS  **********************/
class LinuxProcMapReader
{
	public:
		typedef LinuxProcMap::const_iterator const_iterator;
	public:
		void load(void);
		void dumpAsTxt(const std::string & filename) const;
		const LinuxProcMapEntry* getEntry(void* addr) const;
		const_iterator begin(void) const { return procMap.begin(); };
		const_iterator end(void) const { return procMap.end(); };
	private:
		LinuxProcMap procMap;
};

}

#endif //FSC_LINUX_PROC_MAP_HPP
