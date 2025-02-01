# How You Can Help

Mending Wall is designed to make contributions as easy as possible, either with or without programming experience. Contributions from users are essential to expand coverage of desktop environments and Linux distributions.

<div class="grid cards" markdown>

-   :material-hand-heart: __Provide feedback__

     ---

     Tell us whether or not Mending Wall worked for you. It is helpful to know which Linux distribution you are using, and which desktop environments you are trying to use.

     [:octicons-arrow-right-24: Start a discussion](https://github.com/lawmurray/mendingwall/discussions)

-   :material-bug: __Report an issue__

    ---

    If you find a bug, or can be more specific about what is not working for you, open an issue. It is helpful to provide specific steps to reproduce it.

    [:octicons-arrow-right-24: Open an issue](https://github.com/lawmurray/mendingwall/issues)

-   :fontawesome-solid-gear: __Improve configuration__

    ---

    Mending Wall uses simple config files, and no programming experience is necessary to tweak them. This can involve adding new applications to *Tidy Menus*, additional theme config to *Mend Menus*, or support for a whole new desktop environment.

    [:octicons-arrow-right-24: Learn about configuring Mend Themes](mend-themes.md#configuration)

    [:octicons-arrow-right-24: Learn about configuring Tidy Menus](tidy-menus.md#configuration)

    [:octicons-arrow-right-24: Start a discussion to propose changes](https://github.com/lawmurray/mendingwall/discussions)

-   :fontawesome-solid-hammer: __Contribute code__

    ---

    Mending Wall is free software licensed under the [GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html). It is written in C as a GNOME application using [Adwaita](https://gnome.pages.gitlab.gnome.org/libadwaita/), [GTK4](https://gtk.org/), and [Blueprint](https://jwestman.pages.gitlab.gnome.org/blueprint-compiler/). The background program is written in C using just [GLib](https://docs.gtk.org/glib/).

    [:octicons-arrow-right-24: Submit a pull request](https://github.com/lawmurray/mendingwall/pull_requests)

</div>


## Suggested workflow

!!! info
    This section is written for potential contributors who would like to explore the issues that Mending Wall seeks to address, in order to improve the app and Linux desktop experience.

You have several options as a work environment:

1. Use a virtual machine, perhaps run via [Boxes](https://apps.gnome.org/Boxes/), [virt-manager](https://virt-manager.org/) or [VirtualBox](https://www.virtualbox.org/), and install a Linux distribution to work with. There is value in testing different Linux distributions, so consider it a chance to try something new!
2. Use your current system but first create a snapshot that you can restore in case of problems, perhaps using [Timeshift](https://github.com/linuxmint/timeshift) or [Snapper](http://snapper.io/).
3. Use your current system if you are determined to fix everything that breaks anyway. You do, after all, only live once.

### Investigating breakages

Once you have chosen a work environment, you need to install at least two desktop environments for testing (e.g. GNOME and KDE). In most cases, one will already have been installed when you installed Linux. After installation, a typical workflow is as follows:

1. Log into the first desktop environment and ensure that it is running correctly. Make a copy of the configuration (see below).
2. Log into the second desktop environment and ensure that it is running correctly. Usually there are no problems at this stage. Make a copy of the configuration.
3. Log into the first desktop environment again. Often something will now be broken, e.g. the theme, cursors, icons, fonts. Make a copy of the (broken) configuration.
4. Fix what is broken in the first desktop environment, which can usually be achieved through its settings app. Make a copy of the (now fixed) configuration.

You should now have four complete copies of the configuration in various states, from each of the four steps. You can now compare those (see below) and see what changed to cause the breakage, and propose updates to the Mending Wall config files to fix these breakages.

!!! tip
    As soon as you have some preliminary information, [start a discussion](https://github.com/lawmurray/mendingwall/discussions/) or [open an issue](https://github.com/lawmurray/mendingwall/issues/) to report. Other users may be able to help in understanding the problem and contributing a fix.

### Copying configuration

Desktop environments keep their configuration in one of two places:

1. GSettings, stored in the dconf database, and
2. config files, usually stored in `$HOME/.config/`.

To create a copy of these, the suggested approach is:

1. Choose a working directory, e.g. `$HOME/backup`. Make that directory, change into it, and create a subdirectory for the copy, e.g. `copy1`:
    ```
    mkdir -p $HOME/backup
    cd $HOME/backup
    mkdir copy1
    ```
2. Dump your entire dconf database into the subdirectory:
    ```
    dconf dump / > copy1/dconf.dump
    ```
3. Back up your entire `$HOME/.config` directory into the subdirectory:
    ```
    cp -r $HOME/.config copy1/.
    ```
    
You now have the first copy in `copy1`, and can repeat with subdirectories `copy2`, `copy3`, `copy4`, etc, to take a snapshot of the configuration state at various times.


### Comparing configuration

Once you have multiple copies of the configuration, you can make comparisons with a recursive `diff` from within the working directory. To show just the files that have changed, use:
```
diff -qr config1 config2
```
To show all the lines that have changed, use:
```
diff -r config1 config2
```
Changes between `dconf.dump` files indicate changes in GSettings paths.


### Understanding the changes

The above commands should give you a good idea of which GSettings paths and config files need to be backed up and restored to preserve the theme of the first desktop environment. But the next step is to understand what these changes actually do, and whether they can be fixed without Mending Wall at all, such as by using the settings provided by the desktop environment, or an option that restores the default theme.

This information is useful for users whose theme is already broken, and could also be automated with a new feature in Mending Wall, similar to the *Restore Default GNOME Theme* feature.


### Creating a fix

Once you understand what is going on, you can make changes to the `themes.conf` configuration file as described in [Mend Themes](mend-themes.md) to fix the issue. This will involve adding the GSettings and config files to be saved and restored. You may not want to add everything that you found, and indeed should not add things blindly, in case that actually creates more problems. It is worth understanding what each of the changed GSettings and config files does, at least to the extent that saving and restoring them can be justified.


### Testing the fix

You will want to do some testing now to ensure that the fix actually works. Using a fresh virtual machine is ideal to test from scratch. From a first desktop environment that is working as desired, a typical test sequence is:

1. Start Mending Wall and ensure that *Mend Themes* is running.
2. Log out of the first desktop environment.
3. Log into the second desktop environment.
4. Log out of the second desktop environment.
5. Log into the first desktop environment again.
6. See whether your fix worked.
7. Iterate if necessary.


## Useful tools

- For an app to browse the `dconf` database and make edits, consider [Dconf Editor](https://apps.gnome.org/DconfEditor/).

- For an app to modify `.desktop` files, consider [Libre Menu Editor](https://flathub.org/apps/page.codeberg.libre_menu_editor.LibreMenuEditor).

- To watch for changes to the dconf database, use the `dconf` command-line tool:
   ```
   dconf watch /
   ```
   The path `/` can be made more specific, e.g.
   ```
   dconf watch /org/gnome/desktop/
   ```
   This will output changes to the path live as you mess about.

