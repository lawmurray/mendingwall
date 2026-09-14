# Q & A

## Why the name?

*Mending Wall* is a [poem written by Robert Frost](https://wikipedia.org/wiki/Mending_Wall), published in 1914. One of its themes is how walls can both unite and divide us. The narrator and their neighbor walk the line of a stone wall between their properties, making repairs. The famous line, "Good fences make good neighbors," is spoken by the neighbor, reflecting a different life view to the narrator.

When it comes to desktop environments, it is unclear where walls should and should not exist, and user preference surely has a role. Mending Wall, the app, expresses the view that the current state needs some repair.

## Which desktop environments does Mending Wall work with?

GNOME, KDE Plasma and COSMIC are used by developers and so the most tested. Xfce and Cinnamon have been tested but are not regularly used by developers.

## Which Linux distributions does Mending Wall work with?

Arch Linux and openSUSE Tumbleweed are used by developers and so the most tested. Debian, Fedora and Ubuntu have been tested but are not regularly used by developers.

Generally any distribution should work, however, but those that do extensive theming out-of-the-box are more likely to have [issues](https://github.com/lawmurray/mendingwall/issues).

## Help! My theme is already a mess!

If you can fix it manually then Mending Wall can maintain those fixes, but it is not yet able to restore from a broken state automatically.

On GNOME, most issues are fixed by using GNOME Tweaks and selecting *Reset to Defaults* from the menu. Be aware that this will usually restore a default GNOME look without any customizations from your Linux distribution. You may also need to use the Settings app to fix scaling and dark/light mode issues: head to the *Displays* and *Appearance* sections.

On KDE Plasma, applying a new global theme and cursor theme will get you most of the way out.

If that doesn't work, you can try the following approach (mileage may vary):

1. Make sure Mending Wall is running.
2. From a desktop environment different to the broken one, go to `~/.local/share/mendingwall/save`, and move the save for the broken desktop environment somewhere else (move, rather than delete, in case you want to restore it later).
3. Log into the broken desktop environment. Mending Wall will act as though you are logging in for the first time, and try to restore a default theme.
