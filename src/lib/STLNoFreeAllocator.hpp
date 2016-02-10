/*****************************************************
             PROJECT  : fork-sharing-checker
             VERSION  : 0.1.0-dev
             DATE     : 02/2016
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Note : this file originaly come from MALT (memory profiling tool, also developped by Sébastien Valat at UVSQ).

#ifndef FSC_STL_NO_FREE_ALLOCATOR_HPP
#define FSC_STL_NO_FREE_ALLOCATOR_HPP

/********************  HEADERS  *********************/
#include "NoFreeAllocator.hpp"

/*******************  NAMESPACE  ********************/
namespace ForkSharingChecker
{

/*********************  CLASS  **********************/
/**
 * Inspired from http://www.codeproject.com/Articles/4795/C-Standard-Allocator-An-Introduction-and-Implement
 * Thanks to Lai Shiaw San Kent
 * 
 * @brief STL allocator to redirect STL map/list onto our own internal allocator.
**/
template<typename T>
class STLNoFreeAllocator {
	public : 
		//    typedefs
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

	public : 
		//    convert an allocator<T> to allocator<U>
		template<typename U>
		struct rebind {
			typedef STLNoFreeAllocator<U> other;
		};

	public : 
		inline STLNoFreeAllocator() {}
		inline ~STLNoFreeAllocator() {}
		inline STLNoFreeAllocator(STLNoFreeAllocator<T> const&) {}
		template<typename U>
		inline STLNoFreeAllocator(STLNoFreeAllocator<U> const&) {}

		//    address
		inline pointer address(reference r) { return &r; }
		inline const_pointer address(const_reference r) { return &r; }

		//    memory allocation
		inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0) { 
		return reinterpret_cast<pointer>(noFreeMalloc<T>(cnt)); 
		}
		inline void deallocate(pointer p, size_type) { 
		}

		//    size
		inline size_type max_size() const { 
			return gblNoFreeAllocator.getMaxSize();
		}

		//    construction/destruction
		inline void construct(pointer p, const T& t) { new(p) T(t); }
		inline void destroy(pointer p) { p->~T(); }

		inline bool operator==(STLNoFreeAllocator<T> const&) { return true; }
		inline bool operator!=(STLNoFreeAllocator<T> const& a) { return !operator==(a); }
};    //    end of class Allocator

}

#endif //FSC_STL_NO_FREE_ALLOCATOR_HPP
