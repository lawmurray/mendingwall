#!/usr/bin/env sh

# See commentary in start-mendingwalld.sh for this environment variable. While
# mendingwall does not require it directly, but may spawn mendingwalld, which
# will
export XDG_DATA_DIRS="/var/lib/flatpak/exports/share:/var/lib/snapd/desktop:/run/host/usr/local/share:/run/host/usr/share:$XDG_DATA_DIRS"
export GSETTINGS_BACKEND=dconf
exec mendingwall "$@"
