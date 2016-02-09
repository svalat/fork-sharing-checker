fork-sharing-checker
====================

This is a simple tool to quicly check which pages are still shared after forking and accessing your data.
It can help to keep some statistics about the sharing over time in your application.

How to compile
--------------

You can compile with the configure script wrapping cmake :

```sh
	mkdir build
	cd build
	../configure --prefix=YOUR_PREFIX
```

Or use directly cmake if needed :

```sh
	mkdir build
	cd build
	cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=YOUR_PREFIX
```

How to use
----------

Just include `ForkSharingChecker.h` in you application and use it like :

```c
#include <ForkSharingChecker.h>

int main(void)
{
	//allocate you memory, do you stuff before fork
	
	//forking
	pid_t pid = fork();
	
	//if you want some nice name
	const char * extra;
	if (pid == 0)
		extra = "child";
	else
		extra = "parent";
	
	//dump just after fork if we want some ref
	forkSharingCheckerDump("example-dump-before","",false);
	
	//do stuff
	
	//now dump after touching
	forkSharingCheckerDump("example-dump-after","",false);
}
```

How it work
-----------

This tool use `/proc/self/map` to get the current mapping into virtual addresses, then it translate
to physical addresses using `/proc/self/pagemap`. The last one only be available for Linux kernel greater
then 2.6.25, to be check it looks that some new distribution tend to make it readable only from root 
(arround centos7, debian7).

Licence
-------

The fork-sharing-checker tool is distributed undre CeCILL-C licence which is similar to LGPL.
