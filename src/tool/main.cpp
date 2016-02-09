/*****************************************************
             PROJECT  : fork-sharing-checking
             VERSION  : 0.1.0
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
//gnu
#include <getopt.h>
//unix
#include <libgen.h>
//locals
#include "../lib/Reader.hpp"
#include "../lib/ProcPagemapReader.hpp"

/*******************  NAMESPACE  ********************/
using namespace ForkSharingChecker;

/*********************  STRUCT  *********************/
struct OutputEntry
{
	//functions
	OutputEntry(const std::string & name);
	void reset(const std::string & name,size_t size);
	void print(bool percent) const;

	//values
	std::string name;
	size_t size;
	size_t mapped;
	size_t shared;
};

/*********************  TYPES  **********************/
typedef std::vector<OutputEntry> OutputEntryVector;

/********************  GLOBALS  *********************/
static const char * cstHelp = "fork-sharing-checker -r {REF} -t {TARGET} [-h] [-p] [-a] [-s|-S|-m]\n\
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
";

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
			return true;
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

	//loop over options
	while ((c = getopt (argc, argv, "hr:t:pasSmo")) != -1)
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
	this->reset(name,0);
}

/*******************  FUNCTION  *********************/
void OutputEntry::reset(const std::string& name, size_t size)
{
	this->name = name;
	this->mapped = 0;
	this->shared = 0;
	this->size = size;
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
void print(const CmdOptions & options,OutputEntryVector & outVec,OutputEntry & total)
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
	
	//print values
	for (OutputEntryVector::const_iterator it = outVec.begin() ; it != outVec.end() ; ++it)
		it->print(options.percent);
			
	//final
	printf("#-------------------------------------------------------------------------------\n");
	total.print(options.percent);
}

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//args
	CmdOptions options;
	parseArgs(options,argc,argv);
	
	//load
	Reader ref(options.ref);
	Reader target(options.target);
	
	//total
	OutputEntry total("#TOTAL");
	OutputEntry out("");
	OutputEntryVector outVec;
	
	//loop on target
	for (Reader::const_iterator it = target.begin() ; it != target.end() ; ++it)
	{
		//some vars
		const ReaderEntry & t = *it;
		const ReaderEntry * lastRef = NULL;
		
		//setup out
		out.reset(getEntryName(t.file),t.pages);
		
		//skip if only anon
		if (options.anon && t.file.empty() == false)
			continue;
		
		//loop on pages from target
		for (void * i = t.base ; i < t.end ; (size_t&)i += PAGE_SIZE)
		{
			//load ref
			if (lastRef == NULL)
				lastRef = ref.getEntry(i);
			else if (lastRef->contain(i) == false)
				lastRef = ref.getEntry(i);
			
			//compare PFNs
			size_t targetPFN = t.getPFN(i);
			size_t refPFN = 0;
			
			if (lastRef != NULL)
				refPFN = lastRef->getPFN(i);
			
			//update counter
			if (targetPFN != 0)
				out.mapped++;
			if (targetPFN == refPFN && refPFN != 0)
				out.shared++;
		}
		
		//check only shared
		if (out.shared > 0)
		{
			//add for output
			outVec.push_back(out);
			
			//update total
			total.size += t.pages;
			total.mapped += out.mapped;
			total.shared += out.shared;
		}
	}
	
	//print
	print(options,outVec,total);
	
	//ok finish
	return EXIT_SUCCESS;
}

