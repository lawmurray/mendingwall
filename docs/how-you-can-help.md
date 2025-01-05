# How You Can Help

Mending Wall is designed so that you can contribute to it without programming experience. As described in [How It Works](how-it-works.md), it relies on config files that define patterns to apply while running each desktop environment. You can edit these config files to help yourself, and commit them back to the project to help others. This is particular important because the developers cannot exhaustively test all combinations of desktop environment and Linux distributions, it must be a collaborative effort.

## Choosing a test environment

You have several options as a work environment:

1. Use your current system if you are determined to fix anything that breaks anyway.
2. Use your current system but first create a snapshot that you can restore in case of problems.
3. Use a virtual machine, perhaps run with [Boxes](https://apps.gnome.org/Boxes/) or [VirtualBox](https://www.virtualbox.org/) to make it easy. Install a Linux distribution to work with. There is value in testing different Linux distributions and so consider going with the unfamiliar.

## Investigating breakages

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

### Automating your repairs

Now that you understand the changes being made, you can automate the repair via Mending Wall's `preserve-themes.conf` configuration file. 

## Cleaning up counterpart applications

This is a simpler task to investigating breakages. Most desktop environments have core applications such as a file manager, text editor, and calculator, perhaps dictionary or maps, that have counterparts in other desktop environments. Some desktop environments mark these only for display in that desktop environment, others desktop environments do not, which can clutter menus.

To declutter the menus, applications can be restricted to only appear in certain desktop environments via Mending Wall's `counterpart-applications.conf` configuration file.

## Contributing repairs back

To contribute your repairs for other users, submit a pull request on the [GitHub repository](https://github.com/lawmurray/mendingwall). You will need to sign up for a GitHub account if you do not have one already. 

## Useful tools

To view the `dconf` database, and potentially make edits, consider [Dconf Editor](https://apps.gnome.org/DconfEditor/).

To watch live for changes to the dconf database, use the `dconf` command-line tool:
```
dconf watch /
```
The path `/` can be made more specific, e.g.
```
dconf watch /org/gnome/desktop
```

