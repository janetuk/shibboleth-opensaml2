Name:		opensaml
Version:	2.4.1
Release:	1
Summary:    OpenSAML SAML library
Group:		Development/Libraries/C and C++
Vendor:		Internet2
License:	Apache 2.0
URL:		http://www.opensaml.org/
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
%if 0%{?suse_version} > 1030 && 0%{?suse_version} < 1130
BuildRequires:  libXerces-c-devel >= 2.8.0
%else
BuildRequires:  libxerces-c-devel >= 2.8.0
%endif
BuildRequires:  libxml-security-c-devel >= 1.4.0
BuildRequires:  libxmltooling-devel >= 1.4
%{?_with_log4cpp:BuildRequires: liblog4cpp-devel >= 1.0}
%{!?_with_log4cpp:BuildRequires: liblog4shib-devel}
BuildRequires:  gcc-c++
%if 0%{?suse_version} > 1000
BuildRequires: pkg-config
%endif
%{!?_without_doxygen:BuildRequires: doxygen}
%if "%{_vendor}" == "redhat"
BuildRequires: redhat-rpm-config
%endif

%if "%{_vendor}" == "suse"
%define pkgdocdir %{_docdir}/%{name}
%else
%define pkgdocdir %{_docdir}/%{name}-%{version}
%endif

%description
OpenSAML is an open source implementation of the OASIS Security Assertion
Markup Language Specification. It contains a set of open source C++ classes
that support the SAML 1.0, 1.1, and 2.0 specifications.

%package -n opensaml-bin
Summary:    Utilities for OpenSAML library
Group:      Development/Libraries/C and C++

%description -n opensaml-bin
OpenSAML is an open source implementation of the OASIS Security Assertion
Markup Language Specification. It contains a set of open source C++ classes
that support the SAML 1.0, 1.1, and 2.0 specifications.

This package contains the utility programs.

%package -n libsaml7
Summary:    OpenSAML SAML library
Group:      Development/Libraries/C and C++
Provides:   opensaml = %{version}-%{release}
Obsoletes:  opensaml < %{version}-%{release}

%description -n libsaml7
OpenSAML is an open source implementation of the OASIS Security Assertion
Markup Language Specification. It contains a set of open source C++ classes
that support the SAML 1.0, 1.1, and 2.0 specifications.

This package contains just the shared library.

%package -n libsaml-devel
Summary:	OpenSAML development Headers
Group:		Development/Libraries/C and C++
Requires:	libsaml7 = %{version}-%{release}
Provides:	opensaml-devel = %{version}-%{release}
Obsoletes:	opensaml-devel < %{version}-%{release}
%if 0%{?suse_version} > 1030 && 0%{?suse_version} < 1130
BuildRequires:  libXerces-c-devel >= 2.8.0
%else
BuildRequires:  libxerces-c-devel >= 2.8.0
%endif
Requires: libxml-security-c-devel >= 1.4.0
Requires: libxmltooling-devel >= 1.4
%{?_with_log4cpp:Requires: liblog4cpp-devel >= 1.0}
%{!?_with_log4cpp:Requires: liblog4shib-devel}

%description -n libsaml-devel
OpenSAML is an open source implementation of the OASIS Security Assertion
Markup Language Specification. It contains a set of open source C++ classes
that support the SAML 1.0, 1.1, and 2.0 specifications.

This package includes files needed for development with OpenSAML.

%package -n opensaml-schemas
Summary:	OpenSAML schemas and catalog
Group:		Development/Libraries/C and C++

%description -n opensaml-schemas
OpenSAML is an open source implementation of the OASIS Security Assertion
Markup Language Specification. It contains a set of open source C++ classes
that support the SAML 1.0, 1.1, and 2.0 specifications.

This package includes XML schemas and related files.

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
%post -n libsaml7 -p /sbin/ldconfig
%endif

%ifnos solaris2.8 solaris2.9 solaris2.10
%postun -n libsaml7 -p /sbin/ldconfig
%endif

%files -n opensaml-bin
%defattr(-,root,root,-)
%{_bindir}/samlsign

%files -n libsaml7
%defattr(-,root,root,-)
%{_libdir}/libsaml.so.*

%files -n opensaml-schemas
%defattr(-,root,root,-)
%dir %{_datadir}/xml/opensaml
%{_datadir}/xml/opensaml/*

%files -n libsaml-devel
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/pkgconfig/opensaml.pc
%doc %{pkgdocdir}

%changelog
* Tue Oct 26 2010  Scott Cantor  <cantor.2@osu.edu>  - 2.4-1
- Update version
- Add pkg-config support.
- Sync package names for side by side install.
- Adjust Xerces dependency name and Group setting
- Split out schemas into separate subpackage

* Mon Aug 31 2009   Scott Cantor  <cantor.2@osu.edu>  - 2.3-1
- Bump soname for SUSE packaging.

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
