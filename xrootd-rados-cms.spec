%define _unpackaged_files_terminate_build 0
Name:		xrootd-rados-cms
Version:	1.0.0
Release:	1
Summary:	XRootD CMS plugin for RADOS pools
Prefix:         /usr
Group:		Applications/File
License:	GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Source:        %{name}-%{version}-%{release}.tar.gz
BuildRoot:     %{_tmppath}/%{name}-root

BuildRequires: git
BuildRequires: cmake >= 2.6
BuildRequires: ceph-libs ceph-devel
BuildRequires: xrootd-server-devel >= 3.3.0
BuildRequires: xrootd-private-devel >= 3.3.0

Requires:      ceph-libs ceph


%description
XRootD CMS plugin to locate objects in a RADOS pool

%prep
%setup -n %{name}-%{version}-%{release}

%build
test -e $RPM_BUILD_ROOT && rm -r $RPM_BUILD_ROOT
%if 0%{?rhel} < 6
export CC=/usr/bin/gcc44 CXX=/usr/bin/g++44 
%endif

mkdir -p build
cd build
cmake ../ -DRELEASE=%{release} -DCMAKE_BUILD_TYPE=RelWithDebInfo
%{__make} %{_smp_mflags} 

%install
cd build
%{__make} install DESTDIR=$RPM_BUILD_ROOT
echo "Installed!"

%post 
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
/usr/lib64/libRadosCms.so
/usr/lib64/libRadosCms.so


