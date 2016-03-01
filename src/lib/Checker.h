/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0
             DATE     : 03/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef FSC_CHECKER_H
#define FSC_CHECKER_H

/*******************  FUNCTION  *********************/
#ifdef __cplusplus
extern "C" {
#endif
	/**
	 * Main function of fork-sharing-checker to be called into your code (usable from C/C++).
	 * It just require a dump name to create the directory to store the mapping files.
	 * @param dumpName Name of the directory which will be created to store the mapping.
	 * @param extraName An extra string to add after dumpName
	 * @param pid Add PID on directory name
	**/
	void forkSharingChecker(const char * dumpName,const char * extraName,bool pid);
	
	/**
	 * Enable tracking of cow event to build statistcs about which call site generate
	 * end of COW on shared pages. It uses mprotect and segmentation fault signal to track accesses
	 * to annonymous segment only. It will automatically dump at exit if not disabled.
	 * @param dumpName Name of the directory which will be created to store the mapping. If NULL do not dump automatically at exit.
	 * @param extraName An extra string to add after dumpName
	 * @param pid Add PID on directory name
	**/
	void forkSharingCOWTracker(const char * filename,const char * extraName,bool pid);
	
	/**
	 * Force dump of current statistics about COW.
	 * @param file File into which to write the data (JSON format).
	**/
	void forkSharingCOWDump(const char * filename,const char * extraName,bool pid);
#ifdef __cplusplus
}
#endif

#endif //FSC_CHECKER_H
