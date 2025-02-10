#!/usr/bin/env bash

# Need access to the host XDG_DATA_DIRS to see which applications are
# installed to tidy menus accordingly. These paths are hard-coded, which is
# unfortunate, but the only alternative seems to be calling `flatpak-spawn
# --host` to get them (which is done by e.g. dconf-editor and Refine), but
# that requires very broad sandbox permissions that are not otherwise required
# (--talk-name=org.freedesktop.Flatpak) and results in a warning on Flathub
# about arbitrary permissions. For the purposes of Mending Wall this
# hard-coded set ought to be sufficient anyway.
export XDG_DATA_DIRS="/var/lib/flatpak/exports/share:/var/lib/snapd/desktop:/run/host/usr/local/share:/run/host/usr/share:$XDG_DATA_DIRS"
unset XDG_CONFIG_HOME  # i.e. use default
export GSETTINGS_BACKEND=dconf
exec mendingwalld "$@"
