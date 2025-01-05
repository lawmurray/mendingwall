# Advanced Usage: Preserve Themes

The specific configuration settings that Mending Wall handles are of two types:

1. GSettings paths stored in the dconf database, used by desktop environments based on later versions of GTK, such as GNOME.

2. Configuration files stored in the `~/.config` directory, used by desktop environments based on Qt, such as KDE Plasma and CuteFish, as well as those based on earlier versions of GTK, such as Cinnamon and Pantheon.

Apps use the same toolkits such as GTK and Qt, and are styled to look native to one desktop environment or another. Most desktop environments use both of the above configuration types, as they attempt to theme foreign apps to look more like native apps for a consistent user experience.

The specific GSettings paths and config files to backup and restore are configured with a config file `mendingwall/preserve-themes.conf`. If the environment variable `$XDG_CONFIG_HOME` is set, then the directory `$XDG_CONFIG_HOME` is first checked for the config file. If not found, the directories in `XDG_CONFIG_DIRS` are checked in order until the config file is found.

The config file contains group headers for desktop environments followed by key-value pairs that specify GSettings paths and config files save and restore when running that desktop environment. For example:
```
[GNOME]
gsettings=/org/gnome/desktop/interface/

[KDE]
files=~/.config/kwinrc;.config/kcminputrc
```
Each key is followed by an equal sign (`=`), and where multiple values are required they are separated by semicolons (`;`). Recognized keys are:

| Key | Value |
| --- | ----- |
| `gsettings` | GSettings paths to save and restore. Each should begin and end with a forward slash (`/`). All entries at the path are saved and restored, but there is no recursion into children, so these should be specified separately if desired. |
| `files` | Configuration files to save and restore. Each should start with a tilde (`~`) so as to be relative to the user's home directory. |

The group headers are surrounded by square brackets (`[...]`) and specify a desktop environment. The environment variable `$XDG_CURRENT_DESKTOP` is used to determine which desktop environment is running and so which group to use. If `$XDG_CURRENT_DESKTOP` lists multiple desktop environments (e.g. `GNOME;Unity`), then the union of multiple groups is used (values for the same key are concatenated with duplicates removed).

## Related links

- [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/latest/) for the `$XDG_CONFIG_HOME` and `$XDG_CONFIG_DIRS` environment variables.
- [XDG Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/) for the `$XDG_CURRENT_DESKTOP` environment variable.

