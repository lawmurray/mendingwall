![Mending Wall icon](docs/assets/logo.svg)

# Mending Wall

Mending Wall is a Linux application to fix issues when hopping between multiple desktop environments, such as tiny/huge cursors, tiny/huge fonts, light/dark mode switches and scaling issues. It also tidies application menus, showing core applications in their native environment only.

Mending Wall is free software, copyright (C) 2025 Lawrence Murray, licensed under the terms of the [GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html).

> [!CAUTION]
> This is an early release. During development, it has been tested with Ubuntu 24.04, Fedora 41, openSUSE Tumbleweed and EndeavourOS with GNOME and KDE, and to a lesser extent Xfce and Cinnamon. I would recommend trying it in a virtual machine, or creating a snapshot of your system before proceeding (with e.g. [Timeshift](https://github.com/linuxmint/timeshift) or [Snapper](http://snapper.io/)), in case Mending Wall fails to fix breakages caused by installing a second desktop environment in your main system, which may leave you with a manual cleanup.


## Installation

Packages are available from [download.indii.org](https://download.indii.org). You can also install from source following the instructions provided here. A local installation to your home directory is sufficient; root permissions are not required to install or run.

Building requires Meson, Blueprint, GTK 4, GLib 2, libadwaita, and libportal.

On Debian-based systems, install these with:
```
sudo apt install \
    meson \
    blueprint-compiler \
    libgtk-4-dev \
    libglib2.0-dev \
    libadwaita-1-dev \
    libportal-dev \
    libportal-gtk3-dev \
    libportal-gtk4-dev \
    gettext \
    desktop-file-utils
```
On Fedora:
```
sudo dnf install \
    meson \
    blueprint-compiler \
    glib2-devel \
    gtk4-devel \
    libadwaita-devel \
    libportal-devel \
    libportal-gtk3-devel \
    libportal-gtk4-devel \
    gettext \
    desktop-file-utils
```
On openSUSE:
```
sudo zypper install \
    meson \
    blueprint-compiler \
    glib2-devel \
    gtk4-devel \
    libadwaita-devel \
    libportal-devel \
    libportal-gtk3-devel \
    libportal-gtk4-devel \
    gettext \
    desktop-file-utils
```
On Arch-based:
```
sudo pacman -S meson blueprint-compiler glib2 gtk4 libadwaita libportal libportal-gtk3 libportal-gtk4 gettext desktop-file-utils
```
Other Linux distributions will almost certainly provide these packages too, although names may differ.

To build and install Mending Wall, run from within the base directory:
```bash
meson setup build --prefix=$HOME/.local
cd build
meson compile
meson install
gdbus call --session --dest org.freedesktop.DBus --object-path /org/freedesktop/DBus --method org.freedesktop.DBus.ReloadConfig
```
The last line just ensures that newly-installed `.service` files in `$HOME/.local/share/dbus-1/services/` are found by D-Bus, in case that directory did not exist already. If you try to launch Mending Wall from an application menu and nothing happens, running that line should fix it, and it does not hurt to run it anyway.

## Quick start

After installing, launch Mending Wall and enable one or both of the features *Mend Themes* and *Tidy Menus*. Both are recommended. Once enabled, it will start doing its work, and you can log in to a second desktop environment.

If you have already logged in to a second desktop environment before running Mending Wall, so that your themes may be broken already, you will need to fix your themes manually through the settings of your desktop environment, but can enable Mending Wall to preserve those settings going forward.

In brief, Mending Wall starts a background process `mendingwalld` that:

* If *Mend Themes* is enabled, monitors for changes in certain dconf settings and config files for the current desktop session and saves them to `~/.local/share/mendingwall/save`. Then, when logging back into that desktop environment in future, the save is restored (via an autostart in `~/.local/autostart`, and for KDE, a pre-start script in `~/.local/plasma-workspace/env`). In this way each desktop environment effectively has its own separate set of dconf settings and config files, so that they do not interfere with each other.

* If *Tidy Menus* is enabled, monitors for the installation of certain core applications (e.g. terminals, file managers, text editors), as well as many games, and configures the `OnlyShowIn` and `NotShowIn` entries of their `.desktop` files (in e.g. `/usr/share/applications`) to show only in their native desktop environment, saving to the standard directory `~/.local/share/applications`. So, for example, GNOME Terminal shows in GNOME but not KDE, Konsole shows in KDE but not GNOME, etc. If *Tidy Menus* is later disabled, these changes are undone, as long as each `.desktop` file has not been modified with further custom changes (e.g. with a menu editor app).

