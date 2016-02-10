/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef FSC_READER_HPP
#define FSC_READER_HPP

/********************  HEADERS  *********************/
//standard
#include <list>
#include <string>

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{

/********************  STRUCT  **********************/
struct ReaderEntry
{
	//functions
	ReaderEntry(void);
	ReaderEntry(const ReaderEntry & ref);
	~ReaderEntry(void);
	size_t getPFN(void * addr) const;
	bool contain(void * addr) const;
	
	//members
	void * base;
	void * end;
	size_t pages;
	size_t offset;
	std::string file;
	size_t * pfns;
};

/*********************  TYPES  **********************/
typedef std::list<ReaderEntry> ReaderEntryVector;

/*********************  CLASS  **********************/
class Reader
{
	public:
		typedef ReaderEntryVector::const_iterator const_iterator;
	public:
		Reader(const std::string & dirname);
		const ReaderEntry * getEntry(void * addr) const;
		const_iterator begin() const;
		const_iterator end() const;
	private:
		void load(const std::string & dirname);
		void loadPFNs(const std::string & dirname,ReaderEntry & entry);
	private:
		ReaderEntryVector entries;
};

}

#endif //FSC_READER_HPP
