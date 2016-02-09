######################################################
#            PROJECT  : fork-sharing-checking        #
#            VERSION  : 0.1.0-dev                    #
#            DATE     : 02/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

Name: fork-sharing-checker
Version: 0.1.0
Release: 1%{?dist}
Summary: A short tool to check page sharing after forking to track COW (Copy On Write) usage.

Group: Development/Libraries
License: CeCILL-C
URL: https://github.com/downloads/svalat/svUnitTest/%{name}-%{version}.tar.bz2
Source0: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: cmake, gcc-c++
Requires: binutils

%description
A short tool to check page sharing after forking to track COW (Copy On Write) usage.

%prep
%setup -q

%build
%cmake
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_libdir}/*
%{_datadir}/*
%{_includedir}/*
%doc

%changelog
