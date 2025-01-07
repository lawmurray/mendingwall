# How You Can Help

Mending Wall is designed to make contributions as easy as possible, either with or without programming experience. Contributions from users are essential to expand coverage of desktop environments and Linux distributions.

Areas where you can help include:

- [Improve the *Manage Menus* feature](manage-menus.md) by e.g. adding a missing application or a whole new desktop environment. This only requires editing config files.
- [Improve the *Protect Themes* feature](protect-themes.md) by e.g. investigating issues when multiple desktop environments are installed, determining which GSettings and config files should be backed up and restored to fix them, and perhaps adding support for a whole new desktop environment that you use (or would like to use, if only it would not break stuff!). This only requires editing config files.
- [Report issues](https://github.com/lawmurray/mendingwall/issues) such as bugs in the app or in current features, which may be specific to particular Linux distributions or particular desktop environments.
- Add whole new features. This requires programming. The Mending Wall app is written in C as a GNOME application using [Adwaita](https://gnome.pages.gitlab.gnome.org/libadwaita/), [GTK4](https://gtk.org/), and [Blueprint](https://jwestman.pages.gitlab.gnome.org/blueprint-compiler/). The monitoring programs are written in C using mostly just [GLib](https://docs.gtk.org/glib/).

All contributions can be made via the [GitHub repository](https://github.com/lawmurray/mendingwall), either via an [issue](https://github.com/lawmurray/mendingwall/issues) or a [pull request](https://github.com/lawmurray/mendingwall/pull_requests). If GitHub is unfamiliar to you, just start by opening an issue to describe the problem that you have found, the results of your investigation so far, and a proposed fix if you have one. Others can help from there.

Mending Wall is free software licensed under the [GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html).

## Suggested workflow

You have several options as a work environment:

1. Use a virtual machine, perhaps run via [Boxes](https://apps.gnome.org/Boxes/) or [VirtualBox](https://www.virtualbox.org/) to make it easy. Install a Linux distribution to work with. There is value in testing different Linux distributions as they often apply their own themes to desktop environments and this may reveal new issues that require attention. This might be a good chance to try a new distribution!
2. Use your current system but first create a snapshot that you can restore in case of problems, perhaps using [Timeshift](https://github.com/linuxmint/timeshift) or [Snapper](http://snapper.io/)
3. Use your current system if you are determined to fix everything that breaks anyway. You do, after all, only live once.

### Investigating breakages

Once you have chosen a work environment, you need to install at least two desktop environments for testing (e.g. GNOME and KDE). In most cases, one will already have been installed when you installed Linux. After installation, a typical workflow is as follows:

1. Log into the first desktop environment and ensure that it is running correctly. Make a copy of the configuration (see below).
2. Log into the second desktop environment and ensure that it is running correctly. Usually there are no problems at this stage. Make a copy of the configuration.
3. Log into the first desktop environment again. Often something will now be broken, e.g. the theme, cursors, icons, fonts. Make a copy of the (broken) configuration.
4. Fix what is broken in the first desktop environment, which can usually be achieved through its settings app. Make a copy of the (now fixed) configuration.

You should now have four complete copies of the configuration in various states, from each of the four steps. You can now compare those and see what changed to cause the breakage, and propose updates to the Mending Wall config files to address these.

!!! tip
    Now is a great time to [open an issue](https://github.com/lawmurray/mendingwall/issues/) to report the preliminary results of your investigation. Other users may be able to help in understanding the problem and contributing a fix.

### Copying configuration

Desktop environments keep their configuration in one of two places:

1. GSettings, stored in the dconf database, and
2. config files, usually stored in `$HOME/.config/`.

Choose a directory in which to create copies. By way of example, we can use `$HOME/backup`. Make that directory, change into it, and create a subdirectory for the new copy:
```
mkdir -p $HOME/backup
cd $HOME/backup
mkdir config1
```
Back up your entire `$HOME/.config` directory into there:
```
cp -r $HOME/.config config1/.
```
Dump your entire dconf database into there:
```
dconf dump / > config1/dconf.dump
```
You now have one copy, and can repeat with subdirectories `config2`, `config3`, `config4`, etc, at various steps.


### Comparing configuration

Once you have multiple copies of the configuration using the process described above, you can make comparisons just by using a recursive `diff` from within the working directory. To show just the files that have changed, use:
```
diff -qr config1 config2
```
To show all the lines that have changed, use:
```
diff -r config1 config2
```
Changes in the `dconf.dump` file that you created with `dconf dump` indicate changes in GSettings paths, not config files, of course.


### Understanding the changes

The above commands should give you a good idea of which GSettings paths and config files need to be backed up and restored to protect the theme of the first desktop environment. But the next step is to understand what these changes actually do, and whether they can be fixed without Mending Wall at all, such as by using the settings provided by the desktop environment, or an option that restores the default theme.

This information is useful for users whose theme is already broken, and could also be automated with a new feature in Mending Wall, similar to the *Restore Default GNOME Theme* feature.


### Creating a fix

Once you understand what is going on, you can make changes to the `themes.conf` configuration file as described in [Protect Themes](protect-themes.md) to fix the issue. This will involve adding the GSettings and config files to be saved and restored. You may not want to add everything that you found, and indeed should not add things blindly, in case that actually creates more problems. It is worth understanding what each of the changed GSettings and config files does, at least to the extent that saving and restoring them can be justified.


### Testing the fix

You will want to do some testing now to ensure that the fix actually works. If you found a way to fix the theme without using Mending Wall, then your work environment should be in a good state to continue. Otherwise, you may have to create a fresh virtual machine where you can at least test whether your fix can prevent the problems in the first place.

From a first desktop environment that is working as desired, a typical test sequence is:

1. Start Mending Wall and ensure that *Protect Themes* is running.
2. Log out of the first desktop environment.
3. Log into the second desktop environment, then log out again.
4. Log into the first desktop environment again.
5. See whether your fix worked.
6. Iterate if necessary.


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

