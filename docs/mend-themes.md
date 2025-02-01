# Mend Themes

Applications native to one desktop environment can look out of place when run on another. For example, a GNOME app running on KDE Plasma may have a different title bar, button style, and icon style; similarly for a KDE Plasma app run on GNOME.

To unify their look, desktop environments tweak themes to have non-native applications blend in better. They may not revert those tweaks on log out, however, leaving you with a mess when you then log in to a different desktop environment.

![Screenshot of Nautilus (a file manager) in light mode on GNOME before using KDE Plasma: the icons and cursor follow the default Adwaita theme.](assets/gnome_fixed_light.png#only-light){width=400}![Screenshot of Nautilus (a file manager) in dark mode on GNOME before using KDE Plasma: the icons and cursor follow the default Adwaita theme.](assets/gnome_fixed_dark.png#only-dark){width=400}
/// caption
GNOME before using KDE Plasma: the icons and cursor follow the default Adwaita theme.
///

![Screenshot of Nautilus (a file manager) in light mode on GNOME after using KDE: the cursor is large and poorly rendered, the icons and cursors now follow the Breeze theme (default for KDE Plasma).](assets/gnome_broken_light.png#only-light){width=400}![Screenshot of Nautilus (a file manager) in dark mode on GNOME after using KDE: the cursor is large and poorly rendered, the icons and cursors now follow the Breeze theme (default for KDE Plasma).](assets/gnome_broken_dark.png#only-dark){width=400}
/// caption
GNOME after using KDE Plasma: the cursor is large and poorly rendered, the icons and cursors now follow the Breeze theme (default for KDE Plasma).
///

To have Mending Wall fix all this, enable the *Mend Themes* feature. It ensures that the theme chosen for each desktop environment is maintained across sessions, even when running other desktop environments in between.


## What it does

*Mend Themes* saves the theme during each desktop session, then restores it when you next log in to the same desktop environment. For example:

1. While you are running GNOME, Mending Wall saves the theme.
2. If you log out of GNOME and into KDE Plasma, KDE will make some changes to the GNOME theme to improve the user experience on KDE. Mending Wall will save the theme for KDE too.
3. If you then log out of KDE and back into GNOME, Mending Wall will restore the GNOME theme that it previously saved, so that KDE's changes do not interfere with GNOME. GNOME may also make some tweaks to the KDE theme at this stage.
4. Similarly, when you log back into KDE in future, Mending Wall will restore the KDE theme that it previously saved, so that GNOME's tweaks do not interfere with KDE either.


## How it works

!!! info
    This section is for users who want to know exactly what Mending Wall is doing to their system.

When *Mend Themes* is enabled, Mending Wall starts a background process named `mendingwalld` that will also auto-start every time you log in. The process starts in one of two modes:

1. *Save* to make a copy of the theme, monitor for changes, and update the save. This mode is used when *Mend Themes* is enabled for the first time (or re-enabled after being disabled).
2. *Restore* to restore the previous save, monitor for changes, and update it. This is used for the auto-start every time you log in.

A save includes:

1. Any number of [GSettings](https://docs.gtk.org/gio/class.Settings.html) paths. These are typically used for configuration by desktop environments based on [GTK](https://gtk.org), such as GNOME and Cinnamon. The save is kept as a keyfile at `$XDG_CONFIG_HOME/mendingwall/save/$XDG_CURRENT_DESKTOP.gsettings`, where groups identify the GSettings paths and keys the GSettings keys.
2. Any number of config files under `$XDG_CONFIG_HOME/`. Such config files may be used by any desktop environments, including those based on GTK, but also [Qt](https://contribute.qt-project.org/), such as KDE Plasma, or other toolkits. Saved files are kept under `$XDG_CONFIG_HOME/mendingwall/save/$XDG_CURRENT_DESKTOP/`.

`XDG_CURRENT_DESKTOP` is an environment variable set by the desktop environment. `XDG_CONFIG_HOME` is an environment variable that may or may not be set by the desktop environment; if not set, the default value of `$HOME/.config` is used.


## Configuration

!!! info
    This section is for contributors to help improve Mending Wall. The installed configuration is meant to be suitable for everyone with no adjustments required, rather than something that is routinely modified.

The specific GSettings paths and config files to back up and restore are set in the config file `themes.conf`. If the environment variable `XDG_CONFIG_HOME` is set and `$XDG_CONFIG_HOME/mendingwall/themes.conf` exists then that file is used, otherwise `$HOME/.config/mendingwall/themes.conf`, otherwise the directories listed in `XDG_CONFIG_DIRS` are checked in order for `mendingwall/themes.conf` until the file is found.

If `themes.conf` is in a system directory and you wish to make changes to it, first copy it to `$XDG_CONFIG_HOME/mendingwall/themes.conf` or `$HOME/.config/mendingwall/themes.conf`.

The config file is a [KeyFile](https://docs.gtk.org/glib/struct.KeyFile.html). It contains any number of group headers to identify desktop environments, each followed by key-value pairs that specify the GSettings paths and config files to backup and restore. For example:
```
[GNOME]
GSettings=org.gnome.desktop.interface

[KDE]
ConfigFiles=kwinrc;kcminputrc
```

The group headers are surrounded by square brackets (`[...]`) and identify the desktop environments. The environment variable `XDG_CURRENT_DESKTOP` is used to determine which group to use. If `XDG_CURRENT_DESKTOP` lists multiple desktop environments (e.g. `Budgie:GNOME`, note colon-separated in this case, as per the [specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/)) then it is treated as if one value; in this context such combinations really indicate a distinct desktop environment (e.g. `Budgie:GNOME` really just indicates [Budgie](https://buddiesofbudgie.org/)).

Each key is followed by an equals sign (`=`), and where multiple values are required they are separated by semicolons (`;`). Recognized keys are:

| Key | Value |
| --- | ----- |
| `GSettings` | GSettings schemas to save and restore. |
| `ConfigFiles` | Config files to save and restore. Each should be relative to the `$XDG_CONFIG_HOME/` (or default `$HOME/.config/`) directory. There is no support for directories or recursion. |

!!! tip
    You may notice that the current `themes.conf` has the same `GSettings` and `ConfigFiles` for all desktop environments. If the pattern continues, the config file may be simplified in future, but for now it allows different configuration for different desktop environments, in case that becomes useful.

## Related specifications

* The [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/latest/) for the `XDG_CONFIG_HOME` environment variable.
* The [Recognized desktop entry keys](https://specifications.freedesktop.org/desktop-entry-spec/latest/recognized-keys.html) section of the [XDG Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/) for the `XDG_CURRENT_DESKTOP` environment variable.

