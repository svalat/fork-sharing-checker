/*****************************************************
             PROJECT  : fork-sharing-checking
             VERSION  : 0.1.0
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Note : this file originaly come from MALT (memory profiling tool, also developped by Sébastien Valat at UVSQ).

#ifndef FSC_PROC_PAGEMAP_READER_HPP
#define FSC_PROC_PAGEMAP_READER_HPP

/*******************  FUNCTION  *********************/
//standard
#include <cstdio>

/********************  MACROS  **********************/
//TODO use sysconf ideally
#define PAGE_SIZE 4096

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{

/********************  STRUCT  **********************/
//Infos from https://www.kernel.org/doc/Documentation/vm/pagemap.txt
struct ProcPageMapEntry
{
	unsigned long pfn:55;   // Bits 0-54  page frame number (PFN) if present
	                        // Bits 0-4   swap type if swapped
	                        // Bits 5-54  swap offset if swapped
    unsigned char dirty:1;  // Bit  55    pte is soft-dirty (see Documentation/vm/soft-dirty.txt)
    unsigned char zero:5;   // Bits 56-60 zero
    unsigned char file:1;   // Bit  61    page is file-page or shared-anon
    unsigned char swaped:1; // Bit  62    page swapped
    unsigned char present:1;// Bit  63    page present
};

/*********************  CLASS  **********************/
class ProcPageMapReader
{
	public:
		ProcPageMapReader(void);
		~ProcPageMapReader(void);
		size_t getPhysicalSize(void * ptr,size_t size);
		bool hasProcPagemap(void);
		void loadPFNs(void* base, size_t* pfns, size_t pages);
	private:
		size_t internalGetPhysicalSize(void * ptr,size_t size);
	private:
		int fd;
};

}

#endif //FSC_PROC_PAGEMAP_READER_HPP
