/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.2.0-dev
             DATE     : 03/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
//internal
#include "Checker.h"
#include "LinuxProcMapReader.hpp"
#include "ProcPagemapReader.hpp"
//htopml
#include "json/ConvertToJson.h"
//std
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <cerrno>
//unix dependent
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*******************  NAMESPACE  ********************/
using namespace ForkSharingChecker;
using namespace std;

/*******************  NAMESPACE  ********************/
namespace htopml
{

/*******************  FUNCTION  *********************/
static void convertToJson(JsonState& json, const LinuxProcMapEntry& value)
{
	json.openStruct();
		json.printField("lower",value.lower);
		json.printField("upper",value.upper);
		json.printField("offset",value.offset);
		json.printField("file",value.file);
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
static void convertToJson(JsonState& json, const LinuxProcMapReader & value)
{
	json.openArray();
		for (LinuxProcMapReader::const_iterator it = value.begin() ; it != value.end() ; ++it)
			json.printValue(*it);
	json.closeArray();
}

}

/*******************  FUNCTION  *********************/
static void dumpMapToFile(const LinuxProcMapReader & procMap,const string & directory,const string & filename)
{
	//path
	string path = directory + string("/")+filename+".json";
	
	//open file
	FILE * fp = fopen(path.c_str(),"w+");
	if (fp == NULL)
	{
		perror(path.c_str());
		abort();
	}
	
	//convert in memory (json format)
	stringstream buffer;
	htopml::convertToJson(buffer,procMap);
	
	//write to file
	string tmp = buffer.str();
	size_t res = fwrite(tmp.c_str(),1,tmp.size(),fp);
	assert(res == tmp.size());
	
	//close
	fclose(fp);
	
	//ok for convenient we also dump in txt format for C tools
	procMap.dumpAsTxt(directory+string("/")+filename+".txt");
}

/*******************  FUNCTION  *********************/
static void dumpPFNOfMapEntry(ProcPageMapReader & pageMapReader,const LinuxProcMapEntry & mapEntry,const string & dirname)
{
	//gen filename
	char fname[256];
	sprintf(fname,"/0x%lx-0x%lx.raw",mapEntry.lower,mapEntry.upper);
	string path = dirname + fname;
	
	//open file
	FILE * fp = fopen(path.c_str(),"w+");
	if (fp == NULL)
	{
		perror(path.c_str());
		abort();
	}
	
	//allocate buffer
	size_t pages = ((size_t)mapEntry.upper - (size_t)mapEntry.lower) / PAGE_SIZE;
	size_t * pfns = new size_t[pages];
	
	//load from pagemap
	pageMapReader.loadPFNs(mapEntry.lower,pfns,pages);
	
	//write to file
	size_t res = fwrite(pfns,sizeof(size_t),pages,fp);
	assert(res == pages);
	
	//free
	delete [] pfns;
	
	//close
	fclose(fp);
}

/*******************  FUNCTION  *********************/
void forkSharingChecker(const char* dumpName, const char * extraName,bool pid)
{
	//final dirname
	std::string dirName = dumpName;
	dirName += extraName;
	if (pid)
	{
		char buffer[16];
		sprintf(buffer,"%d",getpid());
		dirName += "-";
		dirName += buffer;
	}
	
	//create directory to store data
	int status = mkdir(dirName.c_str(),S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	if (status != 0 && errno != EEXIST)
	{
		perror(dirName.c_str());
		abort();
	}
	
	//load proc map to get segments and dump to file
	LinuxProcMapReader procMap;
	procMap.load();
	dumpMapToFile(procMap,dirName,"map");
	
	//open pagemap
	ProcPageMapReader pageMapReader;
	
	//dump physical PFN for each entries of page map
	for (LinuxProcMapReader::const_iterator it = procMap.begin() ; it != procMap.end() ; ++it)
		dumpPFNOfMapEntry(pageMapReader,*it,dirName);
}
