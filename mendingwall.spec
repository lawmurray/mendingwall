Name: mendingwall
Version: 0.0.0
Release: %autorelease
Summary: Hop between multiple desktop environments
Vendor: Lawrence Murray <lawrence@indii.org>
License: GPL-3.0-or-later
URL: https://mendingwall.indii.org
Source0: %{name}-%{version}.tar.gz
BuildRequires: gcc meson blueprint-compiler glib2-devel gtk4-devel libadwaita-devel

%description

Linux distributions offer a choice of desktop environment, but installing
more than one can break themes and clutter menus. Mending Wall fixes this.

%prep
%setup -q -n %{name}-%{version}

%build
%meson
%meson_build

%install
%meson_install

%files
%license COPYING
%{_bindir}/%{name}
%{_bindir}/%{name}d
%{_datadir}/%{name}/*.conf
%{_datadir}/%{name}/*.desktop
%{_datadir}/%{name}/*.sh
%{_datadir}/applications/org.indii.%{name}*.desktop
%{_datadir}/dbus-1/services/org.indii.%{name}*.service
%{_datadir}/glib-2.0/schemas/org.indii.mendingwall.gschema.xml
%{_datadir}/metainfo/org.indii.mendingwall.metainfo.xml
%{_iconsdir}/hicolor/symbolic/apps/org.indii.%{name}*.svg
%{_iconsdir}/hicolor/scalable/apps/org.indii.%{name}*.svg

%changelog
%autochangelog
