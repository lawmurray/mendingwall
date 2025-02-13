#!/usr/bin/env sh

# See commentary in start-mendingwalld.sh for these environment variables.
# While mendingwallcli does not actually need them, it may spawn mendingwalld,
# which does.

export XDG_DATA_DIRS="/var/lib/flatpak/exports/share:/var/lib/snapd/desktop:/run/host/usr/local/share:/run/host/usr/share:$XDG_DATA_DIRS"
unset XDG_CONFIG_HOME

exec mendingwallcli "$@"
