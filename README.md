fork-sharing-checker
====================

[![Build Status](https://travis-ci.org/svalat/fork-sharing-checker.png?branch=master)](https://travis-ci.org/svalat/fork-sharing-checker)

This is a simple tool to quickly check which pages are still shared after forking and accessing your data.
It can help to keep some statistics about the sharing over time in your application.

How to compile
--------------

You can compile with the configure script wrapping cmake :

```sh
mkdir build
cd build
../configure --prefix=YOUR_PREFIX
make
make test
make install
```

Or use directly cmake if needed :

```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=YOUR_PREFIX
make test
make install
```

How to use
----------

Just include `fork-sharing-checker/ForkSharingChecker.h` in you application and use it like :

```c
#include <fork-sharing-checker/Checker.h>

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
	forkSharingChecker("example-dump-before",extra,false);
	
	//do stuff
	
	//now dump after touching
	forkSharingChecker("example-dump-after",extra,false);
}
```

What it produce
---------------

It will create a firectory with a `map.json` which is a dump of the `/proc/self/map` file in JSON format.
And a raw file for each entry which contain a list of `size_t` entry containing the physical page frame number (PFN)
of each virtual page (considering 4K) in the segment.

Reading the profiles
--------------------

Now just link to `fork-sharing-checker` library :

```sh
gcc main.cpp -lfork-sharing-checker
```

Now you can run you application and use the `fork-sharing-checker` command to get some statistics :

```sh
fork-sharing-checker -r example-dump-after-parent/ -t example-dump-after-child/
```

You will get the sharing between ref (here parent) and target (here child) considering the mapping of target :

```
#File                                       Size(KB)       Mapped(%)   Shared(%)
#-------------------------------------------------------------------------------
example                                            4             100         100
example                                            4             100         100
example                                            4             100           0
[heap]                                           132             100           0
Anonymous                                      32772             100          49
libc-2.17.so                                    1752              25          20
libc-2.17.so                                    2048               0           0
libc-2.17.so                                      16             100         100
libc-2.17.so                                       8             100           0
Anonymous                                         20              60          20
libgcc_s-4.8.5-20150702.so.1                      84              14           4
libgcc_s-4.8.5-20150702.so.1                    2044               0           0
libgcc_s-4.8.5-20150702.so.1                       4             100         100
libgcc_s-4.8.5-20150702.so.1                       4             100         100
libm-2.17.so                                    1028               6           0
libm-2.17.so                                    2044               0           0
libm-2.17.so                                       4             100         100
libm-2.17.so                                       4             100         100
libstdc++.so.6.0.19                              932              51          36
libstdc++.so.6.0.19                             2048               0           0
libstdc++.so.6.0.19                               32             100         100
libstdc++.so.6.0.19                                8             100           0
Anonymous                                         84              19           4
libfork-sharing-checker.so                       100              84          84
libfork-sharing-checker.so                      2044               0           0
libfork-sharing-checker.so                         4             100         100
libfork-sharing-checker.so                         4             100           0
ld-2.17.so                                       132              84          18
Anonymous                                         20             100          80
Anonymous                                         12              66          66
ld-2.17.so                                         4             100         100
ld-2.17.so                                         4             100           0
Anonymous                                          4             100         100
[stack]                                          136              17           5
[vdso]                                             8              50           0
[vsyscall]                                         4               0           0
#-------------------------------------------------------------------------------
#TOTAL                                         47556              72          36
```

Tool options
------------

The `fork-sharing-checker` command support the options : 

```
fork-sharing-checker -r {REF} -t {TARGET} [-h] [-p] [-a] 
                     [-s|-S|-m] [-H] [-j] [-T {FRAMES}]

With:
        -r {REF}    The reference dump.
        -t {TARGET} The target dump.
        -h          To print this help message
        -p          Print percentage of mapped and shared instead of absolute size
        -a          Print only the anonymous mappings
        -s          Sort based on segment size
        -S          Sort based on shared size or ratio
        -m          Sort based on mapped size or ratio
        -o          Only if has shared (remove 0)
        -j          Output in json format
        -T {FRAMES} Timeline mode, see next part for more details
        -H          With timeline to write all the HTML files and JS files

About Timeline mode: 
The timeline mode permit to scan evolution of the sharing over time
between two processes. This mode require you provide ref and target names with {{frame}} 
to be replaced by frame ID. The output will be a json to be used into the html webpage 
dumped with -H option.

```

About history line
------------------

You can get a kind of view how your sharing reduces by taking some snapshots over time and naming the
frame directories with an ID in the name. Then you can run the `fork-sharing-checker` as :

```sh
fork-sharing-checker -r example-dump-after-parent-{{frame}} -t example-dump-after-child-{{frame}} -H -T 10
```

[![Timeline view](https://raw.githubusercontent.com/svalat/fork-sharing-checker/master/src/view/screenshot.png)](https://raw.githubusercontent.com/svalat/fork-sharing-checker/master/src/view/screenshot.png)

Find source of COW
------------------

If you observe that you loose some of your sharing due to copy on write (COW) you can use the `forkSharingCOWTracker`
function which will mprotect all the anonymous memory and account accesses which make COW of the pages.
It then produce a summary at the end of execution.

```
int main(void)
{
	//setup memory
	fork();
	forkSharingCOWTracker("cow-checker","",true);
	//use mem
}
```

You can also make manual snapshots instead of waiting the exit time with `forkSharingCOWDump`.

The produced file looks like :

```
# Touched(KB)                                      func   source:line
       32768                              __memset_sse2    :0
           4                              __memset_sse2    :0
           4                                         ??    ??:0
```

*Limitation 1* : it currently do not support multi-threads (or at least is not protected by mutexes which make it likely to fail).

*Limitation 2* : due to a bug it filters out all the anonymous segment smaller than 64 KB so you will potentially not see all your memory
if it is really fragmented in small peaces.

*Limitation 3* : unmapping a segment don't trigger the tool, so if something do unmap then remap it it don't see it.

How it work
-----------

This tool use `/proc/self/map` to get the current mapping into virtual addresses, then it translate
to physical addresses using `/proc/self/pagemap`. The last one only be available for Linux kernel greater
then 2.6.25, to be check it looks that some new distribution tend to make it readable only from root 
(arround centos7, debian7).

Limitation
----------

The command line tool currently don't take cake of possible usage of `mremap` so it consider shared pages
only while looking at the same virtual address.

Licence
-------

The fork-sharing-checker tool is distributed undre CeCILL-C licence which is similar to LGPL.
