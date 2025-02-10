#!/usr/bin/env bash

# Access is needed to host data directories to see which applications are
# installed for the purposes of tidying menus. Outside of Flatpak, these are
# given by XDG_DATA_DIRS; inside of Flatpak they are overridden. We need to
# get them back.
#
# There are a couple of ways to achieve this:
#
#   1. They can be hard-coded.
#   2. They can be acquired with `flatpak-spawn --host`, but this requires the
#      --talk-name=org.freedesktop.Flatpak sandbox permission, which is very
#      broad, and results in a Flathub warning about the app's ability to
#      acquire arbitrary permissions.
#
# The former is chosen here to keep sandbox permissions tight. The latter is
# chosen by other apps such as dconf-editor and Refine, which offer a good
# reference if this needs to change.

export XDG_DATA_DIRS="/var/lib/flatpak/exports/share:/var/lib/snapd/desktop:/run/host/usr/local/share:/run/host/usr/share:$XDG_DATA_DIRS"


# Access is needed to the configuration files of desktop environments for the
# purposes of saving and restoring themes. Outside of Flatpak, the
# configuration directory is given by XDG_CONFIG_HOME, or if not set the
# default ~/.config; inside of Flatpak this is overridden to a directory used
# by the current app only, typically somewhere under ~/.app. We need to get it
# back.
#
# We don't need any config files of our own in there, as all configuration is
# kept in GSettings with the dconf backend for storage. So we unset the
# XDG_CONFIG_HOME as set by Flatpak, and use the default ~/.config to find the
# configuration files of desktop environments. More precisely, code can then
# use the convenience function g_get_user_config_dir() from GLib to get the
# right directory.

unset XDG_CONFIG_HOME


# Phew, fixed, we can launch.

exec mendingwalld "$@"
