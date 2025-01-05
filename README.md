# Mending Wall

## Preserve themes

This section is meant for advanced users who want to know exactly what Mending Wall is doing to their system.

KDE Plasma modifies the following gsettings on login so that GNOME applications better fit with the KDE Plasma theme: 

```
/org/gnome/desktop/interface/color-scheme
/org/gnome/desktop/interface/icon-theme
/org/gnome/desktop/interface/cursor-theme
/org/gnome/desktop/interface/accent-color
```

It does not change them back on logout, nor does GNOME attempt to restore the previous values on login, so that the next GNOME session has KDE Plasma themed icons and cursors, and possibly some strange visual artifacts as a result, especially on HiDPI screens.

The gsettings are easily restored with GNOME Tweaks. Mending Wall just automates this process. While KDE Plasma does not seem to change the following gsettings, they are also handled by Mending Wall as they are modified by GNOME Tweaks, and perhaps by KDE Plasma in future:

```
/org/gnome/desktop/interface/gtk-theme
```

