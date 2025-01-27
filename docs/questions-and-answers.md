# Q & A

## Why the name?

*Mending Wall* is a [poem written by Robert Frost](https://wikipedia.org/wiki/Mending_Wall), published in 1914. One of its themes is how walls can both unite and divide us. Two neighbors walk the line of a stone wall between their respective properties, making repairs. The famous line, "Good fences make good neighbors," is spoken by one of them, reflecting a life view that is different from that of the other.

When it comes to desktop environments, it is unclear where walls should and should not exist, and user preference surely has a role. Mending Wall, the app, expresses the view that the current state needs some repair.

## Which desktop environments are supported?

At this stage only GNOME and KDE Plasma are supported. If other desktop environments are installed, these two are likely protected from them, but they are likely not protected from these two.

[Contributions are sought](contributing.md) to add support for additional desktop environments, especially from daily drivers of those desktop environments whose regular usage can help discover issues and their fixes.

This can be done just by changing config files. No coding is required. As long as the config files used by Mending Wall contain rules for a particular desktop environment, as identified by its setting of the `XDK_CURRENT_DESKTOP` environment variable, Mending Wall will apply the rules when running in that desktop environment.

