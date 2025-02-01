# Q & A

## Why the name?

*Mending Wall* is a [poem written by Robert Frost](https://wikipedia.org/wiki/Mending_Wall), published in 1914. One of its themes is how walls can both unite and divide us. The narrator and their neighbor walk the line of a stone wall between their properties, making repairs. The famous line, "Good fences make good neighbors," is spoken by the neighbor, reflecting a different life view to the narrator.

When it comes to desktop environments, it is unclear where walls should and should not exist, and user preference surely has a role. Mending Wall, the app, expresses the view that the current state needs some repair.

## Which desktop environments are supported?

At this stage GNOME and KDE Plasma are supported, and Xfce and Cinnamon to a lesser degree (they have not been tested as thoroughly). Supported desktop environments are likely protected from interference from unsupported desktop environments, but not vice versa.

[Help is sought](contributing.md) for additional desktop environments, especially from daily drivers whose regular use will help weed out issues.

There is a table on the [Getting Started](getting-started.md) page to track desktop environment support.

## Which Linux distributions are supported?

Generally any, but those that do extensive theming out-of-the-box are more likely to have unresolved issues that are yet to be [reported](https://github.com/lawmurray/mendingwall/issues).

[Help is sought](contributing.md) for additional Linux distributions, especially from daily drivers whose regular use will help weed out issues.

There is a table on the [Getting Started](getting-started.md) page to track Linux distribution support.

## What if my theme is already broken?

If you can fix it manually then Mending Wall is able to keep your fixes and prevent it breaking again, but Mending Wall is not yet able to restore from a broken state.

!!! help "Work in progress"
    Restoring from a broken state is work in progress. Consider [contributing your time](contributing.md) or [becoming a sponsor](https://github.com/sponsors/lawmurray) to help.

On GNOME, most issues are fixed by using GNOME Tweaks and selecting *Reset to Defaults* from the menu. Be aware that this will usually restore a default GNOME look without any customizations from your Linux distribution, however. You may also need to use the Settings app to fix scaling and dark/light mode issues: head to the *Displays* and *Appearance* sections.

On KDE Plasma, applying a new global theme and cursor theme gets you most of the way.
