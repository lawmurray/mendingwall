# How You Can Help

Mending Wall is designed to make contributions as easy as possible, either with or without programming experience. User contributions are essential to expand coverage of desktop environments and Linux distributions. You can help with:

- [Improve *Manage Menus*](manage-menus.md), e.g. adding a missing application or a whole new desktop environment. This only requires editing config files.
- [Improve *Protect Themes*](protect-themes.md), e.g. finding and fixing breakages or adding support for a whole new desktop environment. This only requires editing config files.
- [Report issues](https://github.com/lawmurray/mendingwall/issues), e.g. bugs in the app, problems with the above features on particular Linux distributions or in particular desktop environments, etc.
- Add whole new features. This requires programming. Mending Wall is written in C as a GNOME application using Adwaita, GTK4, and [Blueprint](https://jwestman.pages.gitlab.gnome.org/blueprint-compiler/).

All contributions can be made via the [GitHub repository](https://github.com/lawmurray/mendingwall), either via an [issue](https://github.com/lawmurray/mendingwall/issues) or a [pull request](https://github.com/lawmurray/mendingwall/pull_requests). If GitHub is unfamiliar to you, just start by opening an issue to describe the problem that you have found, the results of your investigation so far, and a proposed fix if you have one. Others can help from there.


## Suggested workflow

You have several options as a work environment:

1. Use a virtual machine, perhaps run with [Boxes](https://apps.gnome.org/Boxes/) or [VirtualBox](https://www.virtualbox.org/) to make it easy. Install a Linux distribution to work with. There is value in testing different Linux distributions as they often apply their own themes to desktop environments that may need special consideration. Consider trying a new distribution!
2. Use your current system but first create a snapshot that you can restore in case of problems.
3. Use your current system if you are determined to fix anything that breaks anyway.

### Investigating breakages

Once you have chosen a work environment, you need to install at least two desktop environments for testing (e.g. GNOME and KDE). In most cases, one will already have been installed when you installed Linux. After installation, a typical workflow is as follows:

1. Log into the first desktop environment and ensure that it is running correctly. Make a copy of the configuration (see below).
2. Log into the second desktop environment and ensure that it is running correctly. Usually there are no problems at this stage. Make a copy of the configuration.
3. Log into the first desktop environment again. Often something will now be broken, e.g. the theme, cursors, icons, fonts. Make a copy of the (broken) configuration.
4. Fix what is broken in the first desktop environment, which can usually be achieved through its settings app. Make a copy of the (now fixed) configuration.

You should now have four complete copies of the configuration in various steps, from each of the four steps. You can now compare those to see what changed to cause the breakage.

!!! tip
    Now is a great time to [open an issue](https://github.com/lawmurray/mendingwall/issues/) on the Mending Wall GitHub repository to report the preliminary results of your investigation. Other users may be able to help in understanding the problem and contributing a fix.

### Copying config

GTK-based desktop environments (e.g. GNOME, Cinnamon, Pantheon) keep their configuration in one of two places:

1. The dconf database,
2. Configuration files in your `$HOME/.config` directory.

### Comparing config

If you have multiple copies of the configuration using the process described above, you can make comparisons just by using a recursive `diff`:
```
diff -qr config1 config2
```

### Fixing config

Now that you understand the changes being made, you can automate the repair via Mending Wall's `themes.conf` configuration file. 

## Useful tools

- For an app to modify `.desktop` files, consider [Main Menu](https://flathub.org/apps/page.codeberg.libre_menu_editor.LibreMenuEditor).

- For an app to browse the `dconf` database and make edits, consider [Dconf Editor](https://apps.gnome.org/DconfEditor/).

- To watch for changes to the dconf database, use the `dconf` command-line tool:
   ```
   dconf watch /
   ```
   The path `/` can be made more specific, e.g.
   ```
   dconf watch /org/gnome/desktop/
   ```
   This will output changes to the path live as you mess about.

