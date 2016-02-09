/*****************************************************
             PROJECT  : fork-sharing-checking
             VERSION  : 0.1.0
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef FSC_DUMP_H
#define FSC_DUMP_H

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
	void forkSharingCheckerDump(const char * dumpName,const char * extraName,bool pid);
#ifdef __cplusplus
}
#endif

#endif //FSC_DUMP_H
