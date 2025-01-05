# Advanced Usage: Counterpart Applications

The specific applications to manage are configured with a config file `mendingwall/counterpart-applications.conf`. If the environment variable `$XDG_CONFIG_HOME` is set, then the directory `$XDG_CONFIG_HOME` is first checked for the config file. If not found, the directories in `XDG_CONFIG_DIRS` are checked in order until the config file is found.

The config file contains group headers for applications followed by key-value pairs that specify the desktop environments in which to show the app. For example:
```
[org.gnome.Calculator]
OnlyShowIn=GNOME

[org.gnome.Calendar]
OnlyShowIn=GNOME

[org.gnome.Console]
OnlyShowIn=GNOME

[org.kde.Konsole]
OnlyShowIn=KDE
```
The group headers are surrounded by square brackets (`[...]`) and specify the applications. These should be the name of the `.desktop` file of the application, but without the `.desktop` extension. They are usually reverse domain-name identifiers (e.g. `org.gnome.*`), but not always.

Each key is followed by an equal sign (`=`), and where multiple values are required they are separated by semicolons (`;`). Recognized keys are:

| Key | Value |
| --- | ----- |
| `OnlyShowIn` | The application will only be shown in these desktop environments. |
| `NotShowIn` | The application will be shown in all desktop environments except for these. |

It only makes sense to use one of these keys. They are used to set the same keys in the application's `.desktop` file for the application. The values are matched against the environment variable `$XDG_CURRENT_DESKTOP` to determine whether or not to show the application in the current desktop environment.

See the [Recognized desktop entry keys](https://specifications.freedesktop.org/desktop-entry-spec/latest/recognized-keys.html) section of the [XDG Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/) for a precise explanation of how these keys work in `.desktop` files.

## Related links

- [XDG Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/)

