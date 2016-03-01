/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0
             DATE     : 03/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
//locals
#include "Reader.hpp"
#include "ProcPagemapReader.hpp"

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{

/*******************  FUNCTION  *********************/
ReaderEntry::ReaderEntry(void)
{
	this->pfns = NULL;
}

/*******************  FUNCTION  *********************/
ReaderEntry::ReaderEntry(const ForkSharingChecker::ReaderEntry& ref)
{
	this->base = ref.base;
	this->end = ref.end;
	this->file = ref.file;
	this->offset = ref.offset;
	this->pages = ref.pages;
	if (ref.pfns == NULL)
	{
		this->pfns = NULL;
	} else {
		this->pfns = new size_t[ref.pages];
		memcpy(this->pfns,ref.pfns,sizeof(size_t)*ref.pages);
	}
}

/*******************  FUNCTION  *********************/
bool ReaderEntry::contain(void* addr) const
{
	return (addr >= base && addr < end);
}

/*******************  FUNCTION  *********************/
size_t ReaderEntry::getPFN(void* addr) const
{
	//error
	if (!contain(addr))
	{
		fprintf(stderr,"Invalid address, out of bound !");
		abort();
	}
	
	//compute
	size_t id = ((size_t)addr - (size_t)base) / PAGE_SIZE;
	return pfns[id];
}

/*******************  FUNCTION  *********************/
ReaderEntry::~ReaderEntry(void)
{
	if (this->pfns != NULL)
		delete [] this->pfns;
}

/*******************  FUNCTION  *********************/
Reader::Reader(const std::string& dirname)
{
	this->load(dirname);
}

/*******************  FUNCTION  *********************/
void Reader::load(const std::string& dirname)
{
	//open map
	std::string fname = dirname + std::string("/map.txt");
	FILE * fp = fopen(fname.c_str(),"r");
	if (fp == NULL)
	{
		perror(fname.c_str());
		abort();
	}
	
	//loop over map entries en fill by loading raw PFNs
	ReaderEntry entry;
	while (!feof(fp))
	{
		char buffer[4096];
		char fname[4096] = "";
		
		//read
		char * ret = fgets(buffer,sizeof(buffer),fp);
		if (ret == NULL)
			break;
		
		//scan
		int status = sscanf(buffer,"%p-%p 0x%lx %s\n",&(entry.base),&(entry.end),&(entry.offset),fname);
		if (status == 0)
			status = sscanf(buffer,"%p-%p 0x%lx \n",&(entry.base),&(entry.end),&(entry.offset));
		
		//apply
		if (status == 4 || status == 3)
		{
			//fill & push
			entry.file = fname;
			entry.pages = ((size_t)entry.end - (size_t)entry.base) / PAGE_SIZE;
			entries.push_back(entry);
			
			//load PFNs
			ReaderEntry & finalEntry = entries.back();
			loadPFNs(dirname,finalEntry);
		} else {
			fprintf(stderr,"Invalid line in map.txt : '%s'\n",buffer);
		}
	}
	
	//close map
	fclose(fp);
}

/*******************  FUNCTION  *********************/
void Reader::loadPFNs(const std::string& dirname, ReaderEntry& entry)
{
	//compute filename
	char fname[1024];
	sprintf(fname,"%s/%p-%p.raw",dirname.c_str(),entry.base,entry.end);
	
	//open file
	FILE * fp = fopen(fname,"r");
	if (fp == NULL)
	{
		perror(fname);
		abort();
	}
	
	//allocate memory and read all
	entry.pfns = new size_t[entry.pages];
	size_t status = fread(entry.pfns,sizeof(size_t),entry.pages,fp);
	assert(status == entry.pages);
	
	//close file
	fclose(fp);
}

/*******************  FUNCTION  *********************/
const ReaderEntry* Reader::getEntry(void* addr) const
{
	//search
	for (ReaderEntryVector::const_iterator it = entries.begin() ; it != entries.end() ; ++it)
		if (it->base <= addr && it->end > addr)
			return &(*it);
		
	//nothing found
	return NULL;
}

/*******************  FUNCTION  *********************/
Reader::const_iterator Reader::begin() const
{
	return entries.begin();
}

/*******************  FUNCTION  *********************/
Reader::const_iterator Reader::end() const
{
	return entries.end();
}

}
