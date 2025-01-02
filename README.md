# Mending Wall

## What does it do exactly?

This section is meant for advanced users who want to know exactly what Mending Wall is doing to their system.

KDE Plasma modifies the following gsettings on login so that GNOME applications better fit with the KDE Plasma theme: 

```
/org/gnome/desktop/interface/color-scheme
/org/gnome/desktop/interface/icon-theme
/org/gnome/desktop/interface/cursor-theme
```

It does not change them back on logout, nor does GNOME attempt to restore the previous values on login, so that the next GNOME session has KDE Plasma themed icons and cursors, and possibly some strange visual artifacts as a result, especially on HiDPI screens.

The gsettings are easily restored with GNOME Tweaks. Mending Wall just automates this process. While KDE Plasma does not seem to change the following gsettings, they are also handled by Mending Wall as they are modified by GNOME Tweaks, and perhaps by KDE Plasma in future:

```
/org/gnome/shell/extensions/user-theme/name
/org/gnome/desktop/interface/gtk-theme
```

> Incidentally, to work out what is changing, run `dconf watch /` in a terminal and keep an eye on it while making changes in Appearance or Tweaks on GNOME, or System Settings on KDE Plasma.

Once activated, Mending Wall starts a featherweight background process that:

1. Backs up the above gsettings.
2. Monitors for changes in those gsettings and, when changed, updates the backup. This means that you can change the theme with e.g. GNOME Tweaks and not have to run Mending Wall again.
3. On startup, copies the gsettings back from its own configuration.

The one-off actions provided by Mending Wall do the following.

### Restore Default GNOME Theme

Resets the above gsettings to their default values (probably Adwaita, maybe Yaru on Ubuntu). Alternatively, you could use the Appearance page in GNOME Tweaks.

## Hide Counterpart Applications

This is useful if you intend use the core applications of the respective desktop environment of your current session, and would like to declutter menus of the equivalent applications of other desktop environments. For example, you intend to use Console when on GNOME, but Konsole when on KDE Plasma; Nautilus when on GNOME, Dolphin when on KDE Plasma, etc. It does not apply to more complex applications that are not really equivalent with each other, such as GIMP on GNOME and Krita on KDE Plasma.

The action only modifies what is visible in application menus. All applications can still be launched from the command-line from any desktop environment.

The action works by collating all the available `.desktop` files that create menu entries. For each of the suggested applications, it uses the `desktop-file-edit` utility to add an `OnlyShowIn` key to the file, with a value indicating its native desktop environment. System-wide `.desktop` files are first copied into your home directory and then modified.

This action is meant to be run once, and simply applies the suggested configuration. The Main Menu application is recommended for further fine tuning if you do not agree with all of the suggested configuration. For example, Okular and Kate on KDE Plasma are somewhat more powerful than their counterparts Document Viewer and Text Editor on GNOME. If you wish to use them on GNOME, you can use the Main Menu application to restore their visibility there.
