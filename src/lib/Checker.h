/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef FORK_SHARING_CHECKER_H
#define FORK_SHARING_CHECKER_H

/*******************  FUNCTION  *********************/
#ifdef __cplusplus
extern "C" {
#endif
	/**
	 * Main function of fork-sharing-checker to be called into your code (usable from C/C++).
	 * It just require a dump name to create the directory to store the mapping files.
	 * @param dumpName Name of the directory which will be created to store the mapping.
	 * @param pid Add PID on directory name
	**/
	void forkSharingChecker(const char * dumpName,const char * extraName,bool pid);
#ifdef __cplusplus
}
#endif

#endif //FORK_SHARING_CHECKER_H
