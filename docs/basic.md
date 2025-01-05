# Basic Usage

## Hide Counterpart Applications

This is useful if you intend use the core applications of the respective desktop environment of your current session, and would like to declutter menus of the equivalent applications of other desktop environments. For example, you intend to use Console when on GNOME, but Konsole when on KDE Plasma; Nautilus when on GNOME, Dolphin when on KDE Plasma, etc. It does not apply to more complex applications that are not really equivalent with each other, such as GIMP on GNOME and Krita on KDE Plasma.

The action only modifies what is visible in application menus. All applications can still be launched from the command-line from any desktop environment.

The action works by collating all the available `.desktop` files that create menu entries. For each of the suggested applications, it uses the `desktop-file-edit` utility to add an `OnlyShowIn` key to the file, with a value indicating its native desktop environment. System-wide `.desktop` files are first copied into your home directory and then modified.

This action is meant to be run once, and simply applies the suggested configuration. The Main Menu application is recommended for further fine tuning if you do not agree with all of the suggested configuration. For example, Okular and Kate on KDE Plasma are somewhat more powerful than their counterparts Document Viewer and Text Editor on GNOME. If you wish to use them on GNOME, you can use the Main Menu application to restore their visibility there.

## Preserve themes

When the *preserve themes* feature is activated, Mending Wall starts a background process immediately, and on subsequent logins, that:

1. Backs up specific configuration settings for the current desktop environment.
2. Monitors for changes in these and, when changed, updates the backup. This means that you can change the theme or other settings freely.
3. When you log out, the background process terminates.
4. When you next log in to the same desktop environment, the backup is restored.

Between the log out and log in, you can use another desktop environment. It may change the specific configuration settings as needed for its own purposes, but when you log into the original desktop environment again, the backup is restored, and these changes will not affect the original experience.

## Restore Default GNOME Theme

Resets the above gsettings to their default values (probably Adwaita, maybe Yaru on Ubuntu). Alternatively, you could use the Appearance page in GNOME Tweaks.

