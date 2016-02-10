/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
//gnu
#include <getopt.h>
//unix
#include <libgen.h>
//locals
#include "../lib/Reader.hpp"
#include "../lib/ProcPagemapReader.hpp"
#include "../../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
using namespace ForkSharingChecker;

/*********************  STRUCT  *********************/
struct OutputEntry
{
	//functions
	OutputEntry(const std::string & name);
	void reset(const std::string & name,size_t size,size_t base);
	void print(bool percent) const;

	//values
	std::string name;
	size_t size;
	size_t mapped;
	size_t shared;
	size_t base;
};

/*********************  TYPES  **********************/
typedef std::vector<OutputEntry> OutputEntryVector;
typedef std::vector<OutputEntryVector> OutputTimelineVector;

/********************  GLOBALS  *********************/
static const char * cstHelp = "fork-sharing-checker -r {REF} -t {TARGET} [-h] [-p] [-a] \n\
                     [-s|-S|-m] [-H] [-j] [-T {FRAMES}]\n\
\n\
With:\n\
	-r {REF}    The reference dump.\n\
	-t {TARGET} The target dump.\n\
	-h          To print this help message\n\
	-p          Print percentage of mapped and shared instead of absolute size\n\
	-a          Print only the anonymous mappings\n\
	-s          Sort based on segment size\n\
	-S          Sort based on shared size or ratio\n\
	-m          Sort based on mapped size or ratio\n\
	-o          Only if has shared (remove 0)\n\
	-j          Output in json format\n\
	-T {FRAMES} Timeline mode, see next part for more details\n\
	-H          With timeline to write all the HTML files and JS files\n\
\n\
About Timeline mode: \n\
The timeline mode permit to scan evolution of the sharing over time\n\
between two processes. This mode require you provide ref and target names with {{frame}} \n\
to be replaced by frame ID. The output will be a json to be used into the html webpage \n\
dumped with -H option.\n";

/********************  ENUM  ************************/
enum SortMode
{
	SORT_ADDR,
	SORT_SIZE,
	SORT_SHARED,
	SORT_MAPPED
};

/*********************  STRUCT  *********************/
struct CmdOptions
{
	std::string ref;
	std::string target;
	bool percent;
	bool anon;
	SortMode sort;
	bool onlyShared;
	int frames;
	bool timeline;
	bool html;
	bool json;
};

/*********************  STRUCT  *********************/
class SortOutputEntry {
	public:
		SortOutputEntry(const CmdOptions* options);
		bool operator() (const OutputEntry & a,const OutputEntry & b);
	private:
		const CmdOptions * options;
};

/*******************  FUNCTION  *********************/
size_t getSizeOrPercent(size_t value,size_t ref,bool percent);
static void showHelp(FILE * fp = stdout);
void fatal(const std::string & message);
void parseArgs(CmdOptions & options,int argc, char ** argv);
std::string getEntryName(const std::string & file);

/*******************  FUNCTION  *********************/
namespace htopml 
{
	void convertToJson(JsonState & json,const OutputEntry & entry)
	{
		json.openStruct();
			json.printField("name",entry.name);
			json.printField("size",entry.size);
			json.printField("mapped",entry.mapped);
			json.printField("shared",entry.shared);
			json.printField("base",entry.base);
		json.closeStruct();
	}
}

/*******************  FUNCTION  *********************/
SortOutputEntry::SortOutputEntry(const CmdOptions* options)
{
	this->options = options;
}

/*******************  FUNCTION  *********************/
bool SortOutputEntry::operator()(const OutputEntry& a, const OutputEntry& b)
{
	switch(options->sort)
	{
		case SORT_ADDR:
			return a.base < b.base;
		case SORT_SIZE:
			return a.size > b.size;
		case SORT_SHARED:
			return getSizeOrPercent(a.shared,a.size,options->percent) > getSizeOrPercent(b.shared,b.size,options->percent);
		case SORT_MAPPED:
			return getSizeOrPercent(a.mapped,a.size,options->percent) > getSizeOrPercent(b.mapped,b.size,options->percent);
		default:
			fatal("Must not append");
	}
}


/*******************  FUNCTION  *********************/
size_t getSizeOrPercent(size_t value,size_t ref,bool percent)
{
	if (percent)
		return (100*value)/ref;
	else
		return value * PAGE_SIZE / 1024;
}

/*******************  FUNCTION  *********************/
static void showHelp(FILE * fp)
{
	fprintf(fp,cstHelp);
}

/*******************  FUNCTION  *********************/
void fatal(const std::string & message)
{
	fprintf(stderr,"%s\n",message.c_str());
	abort();
}

/*******************  FUNCTION  *********************/
void parseArgs(CmdOptions & options,int argc, char ** argv)
{
	int c;
	
	//set defaults
	options.percent = false;
	options.anon = false;
	options.sort = SORT_ADDR;
	options.onlyShared = false;
	options.frames = 0;
	options.timeline = false;
	options.html = false;
	options.json = true;

	//loop over options
	while ((c = getopt (argc, argv, "hr:t:pasSmoHT:j")) != -1)
	{
		switch(c)
		{
			case 'h':
			case '?':
				showHelp();
				exit(0);
				break;
			case 'r':
				options.ref = optarg;
				break;
			case 't':
				options.target = optarg;
				break;
			case 'p':
				options.percent = true;
				break;
			case 'a':
				options.anon = true;
				break;
			case 's':
				options.sort = SORT_SIZE;
				break;
			case 'S':
				options.sort = SORT_SHARED;
				break;
			case 'm':
				options.sort = SORT_MAPPED;
				break;
			case 'o':
				options.onlyShared = true;
				break;
			case 'j':
				options.json = true;
				break;
			case 'H':
				options.html = true;
				break;
			case 'T':
				options.frames = atoi(optarg);
				options.timeline = true;
				options.onlyShared = true;
				options.json = true;
				break;
			default:
				fprintf(stderr,"Unsupported option %c\n",c);
				abort();
				break;
		}
	}
	
	//checking
	if (options.ref.empty())
		fatal("You need to provide a reference directory with '-r {directory}', see help with '-h'");
	if (options.target.empty())
		fatal("You need to provide a target directory with '-t {directory}', see help with '-h'");
}

/*******************  FUNCTION  *********************/
std::string getEntryName(const std::string & file)
{
	char fname[4096] = "Anonymous";
	if (file.empty() == false)
	{
		strcpy(fname,file.c_str());
		return basename(fname);
	} else {
		return fname;
	}
}

/*******************  FUNCTION  *********************/
OutputEntry::OutputEntry(const std::string& name)
{
	this->reset(name,0,0);
}

/*******************  FUNCTION  *********************/
void OutputEntry::reset(const std::string& name, size_t size,size_t base)
{
	this->name = name;
	this->mapped = 0;
	this->shared = 0;
	this->size = size;
	this->base = base;
}

/*******************  FUNCTION  *********************/
void OutputEntry::print(bool percent) const
{
	printf("%-40s %11lu       %9lu    %8lu\n",
				this->name.c_str(),
				this->size * PAGE_SIZE / 1024,
				getSizeOrPercent(mapped,size,percent),
				getSizeOrPercent(shared,size,percent));
}

/*******************  FUNCTION  *********************/
void printTxt(const CmdOptions & options,OutputEntryVector & outVec)
{
	//header
	if (options.percent)
		printf("%-40s %11s       %s   %s\n","#File","Size(KB)","Mapped(%)","Shared(%)");
	else
		printf("%-40s %11s       %s   %s\n","#File","Size(KB)","Mapped(KB)","Shared(KB)");
	printf("#-------------------------------------------------------------------------------\n");
	
	//sort
	switch(options.sort)
	{
		case SORT_ADDR:
			//nothing to do, already sorted
			break;
		case SORT_SIZE:
			
			break;
		case SORT_SHARED:
			break;
		case SORT_MAPPED:
			break;
	}
	
	//sort
	SortOutputEntry sortFunc(&options);
	std::sort(outVec.begin(),outVec.end(),sortFunc);
	
	//comput total
	OutputEntry total("#TOTAL");
	
	//print values
	for (OutputEntryVector::const_iterator it = outVec.begin() ; it != outVec.end() ; ++it)
	{
		//print
		it->print(options.percent);
		
		//update total
		total.size += it->size;
		total.mapped += it->mapped;
		total.shared += it->shared;
	}
			
	//final
	printf("#-------------------------------------------------------------------------------\n");
	total.print(options.percent);
}

/*******************  FUNCTION  *********************/
void print(const CmdOptions & options,OutputEntryVector & outVec)
{
	if (options.json)
		htopml::convertToJson(std::cout,outVec);
	else
		printTxt(options,outVec);
}

/*******************  FUNCTION  *********************/
void computeShared(const CmdOptions & options,OutputEntryVector & outVec, const std::string & refFile,const std::string & targetFile,const std::string & maskFile = "")
{
	//load
	Reader refReader(refFile);
	Reader targetReader(targetFile);
	Reader * maskReader = &targetReader;
	
	//if mask
	if (maskFile.empty() == false)
		maskReader = new Reader(maskFile);
	
	//total
	OutputEntry out("");
	
	//loop on target
	for (Reader::const_iterator it = maskReader->begin() ; it != maskReader->end() ; ++it)
	{
		//some vars
		const ReaderEntry & range = *it;
		const ReaderEntry * ref = NULL;
		const ReaderEntry * target = NULL;
		
		//setup out
		out.reset(getEntryName(range.file),range.pages,(size_t)range.base);
		
		//skip if only anon
		if (options.anon && range.file.empty() == false)
			continue;
		
		//loop on pages from target
		for (void * i = range.base ; i < range.end ; (size_t&)i += PAGE_SIZE)
		{
			//load target
			target = refReader.getEntry(i);
			
			//load ref
			if (ref == NULL)
				ref = refReader.getEntry(i);
			else if (ref->contain(i) == false)
				ref = refReader.getEntry(i);
			
			//compare PFNs
			size_t targetPFN = 0;
			size_t refPFN = 0;
			
			//load
			if (target != NULL)
				targetPFN = target->getPFN(i);
			if (ref != NULL)
				refPFN = ref->getPFN(i);
			
			//update counter
			if (targetPFN != 0)
				out.mapped++;
			if (targetPFN == refPFN && refPFN != 0)
				out.shared++;
		}
		
		//check only shared
		if (out.size > 0 && (out.shared > 0 || options.onlyShared == false))
			outVec.push_back(out);
	}
}

/*******************  FUNCTION  *********************/
std::string replaceInString(std::string value,const std::string & pattern,const std::string & by)
{
	size_t pos = value.find(pattern);
	if (pos != std::string::npos)
		value.replace(pos,pattern.size(),by);
	
	return value;
}

/*******************  FUNCTION  *********************/
void loadFrame(const CmdOptions & options,OutputTimelineVector & timelineOut,int id)
{
	//add new entry
	OutputEntryVector tmp;
	timelineOut.push_back(tmp);
	OutputEntryVector & frame = timelineOut.back();
	
	//compute file names
	char idStr[64];
	sprintf(idStr,"%d",id);
	std::string ref = replaceInString(options.ref,"{{frame}}",idStr);
	std::string target = replaceInString(options.target,"{{frame}}",idStr);
	std::string mask = replaceInString(options.target,"{{frame}}","0");
	
	//load
	computeShared(options,frame,ref,target,mask);
}

/*******************  FUNCTION  *********************/
void copyFile(const std::string & input, const std::string & output)
{
	std::ifstream infile(input.c_str(), std::ios_base::binary);
	std::ofstream outfile(output.c_str(), std::ios_base::binary);
	outfile << infile.rdbuf();
}

/*******************  FUNCTION  *********************/
void printTimeline(const CmdOptions & options, OutputTimelineVector & timelineOut)
{
	if (options.html)
	{
		printf("Created timeline.js\n");
		std::ofstream out("timeline.js");
		out << "timeline = ";
		htopml::convertToJson(out,timelineOut);
		out << ";";
		out.close();
		printf("Created index.html\n");
		copyFile(std::string(VIEW_PATH)+"/index.html","index.html");
	} else {
		htopml::convertToJson(std::cout,timelineOut);
	}
}

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//args
	CmdOptions options;
	parseArgs(options,argc,argv);
	
	//rendering mode
	if (options.timeline)
	{
		OutputTimelineVector timelineOut;
		timelineOut.reserve(options.frames);
		for (int i = 0 ; i < options.frames ; i++)
			loadFrame(options,timelineOut,i);
		printTimeline(options,timelineOut);
	} else {
		OutputEntryVector outVec;
		computeShared(options,outVec,options.ref,options.target);
		//print
		print(options,outVec);
	}
	
	//ok finish
	return EXIT_SUCCESS;
}

