Name:		opensaml
Version:	2.2.1
Release:	1
Summary:    OpenSAML SAML library
Group:		System Environment/Libraries
Vendor:		Internet2
License:	Apache 2.0
URL:		http://www.opensaml.org/
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
%if 0%{?suse_version} > 1030
BuildRequires:  libXerces-c-devel >= 2.8.0
BuildRequires:  libxml-security-c-devel >= 1.4.0
BuildRequires:  libxmltooling-devel >= 1.2
%{?_with_log4cpp:BuildRequires: liblog4cpp-devel >= 1.0}
%{!?_with_log4cpp:BuildRequires: liblog4shib-devel}
%else
BuildRequires:  xerces%{?xercesver}-c-devel >= 2.8.0
BuildRequires:  xml-security-c-devel >= 1.4.0
BuildRequires:  xmltooling-devel >= 1.2
%{?_with_log4cpp:BuildRequires: log4cpp-devel >= 1.0}
%{!?_with_log4cpp:BuildRequires: log4shib-devel}
%endif
BuildRequires:  gcc-c++
%{!?_without_doxygen:BuildRequires: doxygen}

%if "%{_vendor}" == "suse"
%define pkgdocdir %{_docdir}/%{name}
%else
%define pkgdocdir %{_docdir}/%{name}-%{version}
%endif

%description
OpenSAML is an open source implementation of the OASIS Security Assertion
Markup Language Specification. It contains a set of open source C++ classes
that support the SAML 1.0, 1.1, and 2.0 specifications.

%if 0%{?suse_version} > 1030
%package -n libsaml4
Summary:    OpenSAML SAML library
Group:      Development/Libraries
Provides:   opensaml = %{version}

%description -n libsaml4
OpenSAML is an open source implementation of the OASIS Security Assertion
Markup Language Specification. It contains a set of open source C++ classes
that support the SAML 1.0, 1.1, and 2.0 specifications.

This package contains just the shared library.
%endif

%if 0%{?suse_version} > 1030
%package -n libsaml-devel
Requires: libsaml4 = %version
%else
%package devel
Requires: %name = %version
%endif
Summary: OpenSAML development Headers
Group: Development/Libraries
%if 0%{?suse_version} > 1030
Requires: libXerces-c-devel >= 2.8.0
Requires: libxml-security-c-devel >= 1.4.0
Requires: libxmltooling-devel >= 1.2
%{?_with_log4cpp:Requires: liblog4cpp-devel >= 1.0}
%{!?_with_log4cpp:Requires: liblog4shib-devel}
%else
Requires: xerces%{?xercesver}-c-devel >= 2.8.0
Requires: xml-security-c-devel >= 1.4.0
Requires: xmltooling-devel >= 1.2
%{?_with_log4cpp:Requires: log4cpp-devel >= 1.0}
%{!?_with_log4cpp:Requires: log4shib-devel}
%endif

%if 0%{?suse_version} > 1030
%description -n libsaml-devel
%else
%description devel
%endif
OpenSAML is an open source implementation of the OASIS Security Assertion
Markup Language Specification. It contains a set of open source C++ classes
that support the SAML 1.0, 1.1, and 2.0 specifications.

This package includes files needed for development with OpenSAML.


%prep
%setup -q

%build
%configure %{?saml_options}
%{__make}

%install
%{__make} install DESTDIR=$RPM_BUILD_ROOT pkgdocdir=%{pkgdocdir}
# Don't package unit tester if present.
%{__rm} -f $RPM_BUILD_ROOT/%{_bindir}/samltest

%check
%{__make} check

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && %{__rm} -rf $RPM_BUILD_ROOT

%ifnos solaris2.8 solaris2.9 solaris2.10
%if 0%{?suse_version} > 1030
%post -n libsaml4 -p /sbin/ldconfig
%else
%post -p /sbin/ldconfig
%endif
%endif

%ifnos solaris2.8 solaris2.9 solaris2.10
%if 0%{?suse_version} > 1030
%postun -n libsaml4 -p /sbin/ldconfig
%else
%postun -p /sbin/ldconfig
%endif
%endif

%files
%defattr(-,root,root,-)
%{_bindir}/samlsign
%if 0%{?suse_version} > 1030
%files -n libsaml4
%defattr(-,root,root,-)
%endif
%{_libdir}/libsaml.so.*
%dir %{_datadir}/xml/opensaml
%{_datadir}/xml/opensaml/*

%if 0%{?suse_version} > 1030
%files -n libsaml-devel
%else
%files devel
%endif
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/*.so
%doc %{pkgdocdir}

%changelog
* Sat Aug 8 2009  Scott Cantor  <cantor.2@osu.edu>  - 2.2.1-1
- SuSE conventions
- Stop packaging unit tester

* Wed Dec 3 2008  Scott Cantor  <cantor.2@osu.edu>  - 2.2-1
- Bumping for minor update.
- Fixing SUSE Xerces dependency name.

* Tue Jul 1 2008  Scott Cantor  <cantor.2@osu.edu>  - 2.1-1
- Bumping for minor update.

* Mon Mar 17 2008  Scott Cantor  <cantor.2@osu.edu>  - 2.0-6
- Official release.

* Fri Jan 18 2008  Scott Cantor  <cantor.2@osu.edu>  - 2.0-5
- Release candidate 1.

* Thu Nov 08 2007 Scott Cantor  <cantor.2@osu.edu>  - 2.0-4
- Second public beta.

* Thu Aug 16 2007 Scott Cantor  <cantor.2@osu.edu>  - 2.0-3
- First public beta.

* Fri Jul 13 2007  Scott Cantor  <cantor.2@osu.edu>  - 2.0-2
- Second alpha.

* Sun Apr 16 2007  Scott Cantor  <cantor.2@osu.edu>  - 2.0-1
- First SPEC file for 2.0.
