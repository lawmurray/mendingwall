# Protect Themes

In an effort to make foreign applications look more like native applications, most desktop environments make 

KDE Plasma modifies the following gsettings on login so that GNOME applications better fit with the KDE Plasma theme: 

```
/org/gnome/desktop/interface/color-scheme
/org/gnome/desktop/interface/icon-theme
/org/gnome/desktop/interface/cursor-theme
/org/gnome/desktop/interface/accent-color
```

It does not change them back on logout, nor does GNOME attempt to restore the previous values on login, so that the next GNOME session has KDE Plasma themed icons and cursors, and possibly some strange visual artifacts as a result, especially on HiDPI screens.

The gsettings are easily restored with GNOME Tweaks. Mending Wall just automates this process. While KDE Plasma does not seem to change the following gsettings, they are also handled by Mending Wall as they are modified by GNOME Tweaks, and perhaps by KDE Plasma in future:

```
/org/gnome/desktop/interface/gtk-theme
```


## How It Works

When the *preserve themes* feature is activated, Mending Wall starts a background process immediately, and on subsequent logins, that:

1. Backs up specific configuration settings for the current desktop environment.
2. Monitors for changes in these and, when changed, updates the backup. This means that you can change the theme or other settings freely.
3. When you log out, the background process terminates.
4. When you next log in to the same desktop environment, the backup is restored.

Between the log out and log in, you can use another desktop environment. It may change the specific configuration settings as needed for its own purposes, but when you log into the original desktop environment again, the backup is restored, and these changes will not affect the original experience.

The specific configuration settings that Mending Wall handles are of two types:

1. GSettings paths stored in the dconf database, used by desktop environments based on later versions of GTK, such as GNOME.

2. Configuration files stored in the `$HOME/.config` directory, used by desktop environments based on Qt, such as KDE Plasma and CuteFish, as well as those based on earlier versions of GTK, such as Cinnamon and Pantheon.

Apps use the same toolkits such as GTK and Qt, and are styled to look native to one desktop environment or another. Most desktop environments use both of the above configuration types, as they attempt to theme foreign apps to look more like native apps for a consistent user experience.

## Configuration

The specific GSettings paths and config files to backup and restore are configured with a config file `mendingwall/preserve-themes.conf`. If the environment variable `$XDG_CONFIG_HOME` is set, then the directory `$XDG_CONFIG_HOME` is first checked for the config file. If not found, the directories in `XDG_CONFIG_DIRS` are checked in order until the config file is found.

The config file contains group headers for desktop environments followed by key-value pairs that specify GSettings paths and config files save and restore when running that desktop environment. For example:
```
[GNOME]
gsettings=/org/gnome/desktop/interface/

[KDE]
files=~/.config/kwinrc;.config/kcminputrc
```
The group headers are surrounded by square brackets (`[...]`) and specify a desktop environment. The environment variable `$XDG_CURRENT_DESKTOP` is used to determine which desktop environment is running and so which group to use. If `$XDG_CURRENT_DESKTOP` lists multiple desktop environments (e.g. `GNOME;Unity`), then the union of multiple groups is used (values for the same key are concatenated with duplicates removed).

Each key is followed by an equal sign (`=`), and where multiple values are required they are separated by semicolons (`;`). Recognized keys are:

| Key | Value |
| --- | ----- |
| `GSettings` | GSettings paths to save and restore. Each should begin and end with a forward slash (`/`). All entries at the path are saved and restored, but there is no recursion into children, so these should be specified separately if desired. |
| `Files` | Configuration files to save and restore. Each should start with a tilde (`~`) so as to be relative to the user's home directory. |

## How You Can Help

### Choosing a test environment

You have several options as a work environment:

1. Use your current system if you are determined to fix anything that breaks anyway.
2. Use your current system but first create a snapshot that you can restore in case of problems.
3. Use a virtual machine, perhaps run with [Boxes](https://apps.gnome.org/Boxes/) or [VirtualBox](https://www.virtualbox.org/) to make it easy. Install a Linux distribution to work with. There is value in testing different Linux distributions and so consider going with the unfamiliar.

### Investigating breakages

Once you have chosen a work environment, you need to install at least two desktop environments for testing (e.g. GNOME and KDE). In most cases, one will already have been installed when you installed Linux. After installation, a typical workflow is as follows:

1. Log into the first desktop environment and ensure that it is running correctly. Make a copy of the configuration (see below).
2. Log into the second desktop environment and ensure that it is running correctly. Usually there are no problems at this stage. Make a copy of the configuration.
3. Log into the first desktop environment again. Often something will now be broken, e.g. the theme, cursors, icons, fonts. Make a copy of the (broken) configuration.
4. Fix what is broken in the first desktop environment, which can usually be achieved through its settings app. Make a copy of the (now fixed) configuration.

You should now have four complete copies of the configuration in various steps, from each of the four steps. You can now compare those to see what changed to cause the breakage.

!!! tip
    Now is a great time to [open an issue](https://github.com/lawmurray/mendingwall/issues/) on the Mending Wall GitHub repository to report the preliminary results of your investigation. Other users may be able to help in understanding the problem and contributing a fix.

### How to copy the configuration

GTK-based desktop environments (e.g. GNOME, Cinnamon, Pantheon) keep their configuration in one of two places:

1. The dconf database,
2. Configuration files in your `$HOME/.config` directory.

### How to compare configurations

If you have multiple copies of the configuration using the process described above, you can make comparisons just by using a recursive `diff`:
```
diff -qr config1 config2
```

### How to fix the configurations

Now that you understand the changes being made, you can automate the repair via Mending Wall's `preserve-themes.conf` configuration file. 

### Useful tools

To view the `dconf` database, and potentially make edits, consider [Dconf Editor](https://apps.gnome.org/DconfEditor/).

To watch live for changes to the dconf database, use the `dconf` command-line tool:
```
dconf watch /
```
The path `/` can be made more specific, e.g.
```
dconf watch /org/gnome/desktop
```

## Related links

- [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/latest/) for the `$XDG_CONFIG_HOME` and `$XDG_CONFIG_DIRS` environment variables.
- [XDG Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/) for the `$XDG_CURRENT_DESKTOP` environment variable.

