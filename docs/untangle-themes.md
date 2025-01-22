# Untangle Themes

Applications native to one desktop environment can look out of place when run on another. To improve the user experience, most desktop environments tweak the theme of other desktop environments in order that their applications look closer to native. Unfortunately, they do not revert these changes on logout, and when logging into that that other desktop environment again you may be met with a mess. For example, KDE Plasma changes the cursor and icon theme of GNOME so that its applications look more like native KDE applications and the overall user experience is improved. But on subsequent logins to GNOME those cursors and icons are still all set for KDE. Visual artifacts can also include cursors that change when hovering over non-native apps, cursors doubling in size with poor resolution when on HiDPI screens, and even crashes.

To have Mending Wall fix all this, enable its *Untangle Themes* feature. It ensures that the theme chosen for each desktop environment is maintained across sessions, even when running other desktop environments in between.


## What it does

*Untangle Themes* backs up the theme during each desktop session, and restores it the next time you log in to the same desktop environment. For example, while you are running GNOME, it creates a backup of the theme. If you log out of GNOME and into KDE Plasma, KDE will make some changes to the GNOME theme. If you then log out of KDE and back into GNOME, Mending Wall will restore the GNOME theme to its state when you were last running GNOME.


## How it works

!!! info
    This section is for users who want to know exactly what Mending Wall is doing to their system.

When *Untangle Themes* is enabled, Mending Wall starts a background process named `mendingwall-themes` that will also restart every time you log in. The process starts in one of two modes:

1. *Save* to perform a backup, monitor for changes, and update that backup. This mode is used when *Untangle Themes* is enabled for the first time (or re-enabled after being disabled).
2. *Restore* to restore a backup, monitor for changes, and update that backup. This is used on subsequent logins.

A backup includes:

1. Any number of [GSettings](https://docs.gtk.org/gio/class.Settings.html) paths. These are typically used for configuration by desktop environments based on [GTK](https://gtk.org), such as GNOME and Cinnamon. The backup is kept as a keyfile at `$XDG_CONFIG_HOME/mendingwall/save/$XDG_CURRENT_DESKTOP.gsettings`, where groups identify the GSettings paths and keys the GSettings keys.
2. Any number of config files under `$XDG_CONFIG_HOME/`. Such config files may be used by any desktop environments, including those based on GTK or [Qt](https://contribute.qt-project.org/), such as KDE Plasma. The backup is kept under `$XDG_CONFIG_HOME/mendingwall/save/$XDG_CURRENT_DESKTOP/`.

`XDG_CURRENT_DESKTOP` is an environment variable set by the desktop environment. `XDG_CONFIG_HOME` is an environment variable that may or may not be set by the desktop environment; if not set, the default value of `$HOME/.config` is used.


## Configuration

!!! info
    This section is for contributors to help improve Mending Wall. The installed configuration is meant to be suitable for everyone with no adjustments required.

The specific GSettings paths and config files to back up and restore are set in the config file `themes.conf`. If the environment variable `XDG_CONFIG_HOME` is set then `$XDG_CONFIG_HOME/mendingwall/` is checked for the file, otherwise the default directory `$HOME/.config/mendingwall/` is checked. If the file is not found, the directories listed in `XDG_CONFIG_DIRS` are checked in order, adding a subdirectory `mendingwall/` to each, until the file is first found.

If `themes.conf` is in a system directory and you wish to make changes to it, first copy it into `$XDG_CONFIG_HOME/mendingwall/` or `$HOME/.config/mendingwall/`.

The config file is in the [KeyFile](https://docs.gtk.org/glib/struct.KeyFile.html) format. It contains any number of group headers to identify desktop environments, each followed by key-value pairs that specify the GSettings paths and config files to backup and restore. For example:
```
[GNOME]
GSettings=org.gnome.desktop.interface

[KDE]
ConfigFiles=kwinrc;kcminputrc
```

The group headers are surrounded by square brackets (`[...]`) and identify the desktop environments. The environment variable `XDG_CURRENT_DESKTOP` is used to determine which group to use. If `XDG_CURRENT_DESKTOP` lists multiple desktop environments (e.g. `Budgie:GNOME`, note colon-separated in this case, as per the [specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/)) then it is treated as if one value; in this context such combinations really indicate a distinct desktop environment (e.g. `Budgie:GNOME` really indicates [Budgie](https://buddiesofbudgie.org/)).

Each key is followed by an equals sign (`=`), and where multiple values are required they are separated by semicolons (`;`). Recognized keys are:

| Key | Value |
| --- | ----- |
| `GSettings` | GSettings schemas to save and restore. The whole schema is saved, including children. |
| `ConfigFiles` | Config files to save and restore. Each should be relative to the `$XDG_CONFIG_HOME/` (or default `$HOME/.config/`) directory. There is no support for directories or recursion. |


## Related specifications

* The [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/latest/) for the `XDG_CONFIG_HOME` environment variable.
* The [Recognized desktop entry keys](https://specifications.freedesktop.org/desktop-entry-spec/latest/recognized-keys.html) section of the [XDG Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/) for the `XDG_CURRENT_DESKTOP` environment variable.

