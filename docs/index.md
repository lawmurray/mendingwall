---
description: A Linux app to help GNOME and KDE Plasma desktop environments be more neighborly.
---

<div style="text-align:center;">
  <p><img src="assets/logo.svg" width="200" height="200" alt="Mending Wall logo"></p>
  <h1 style="margin-top:1em;"><strong>Mending Wall</strong></h1>
  <h3 style="margin-top:-1.5em;"><strong>Hop between desktop environments without nuisance</strong></h3>
  <p style="margin-top:2em;margin-bottom:4em;">
    <a href="/getting-started" class="md-button md-button--primary">Install</a>
  </p>
</div>

Mending Wall is an application for the Linux desktop to help maintain multiple desktop environments. It allows you to install GNOME, KDE Plasma, Xfce and Cinnamon alongside each other on the same system, use them with the same user account, without the nuisance of breaking themes and cluttered menus.

## Untangle Themes

Mending Wall saves the configuration for each desktop environment separately and restores it each time you log in, keeping your themes consistent.

For example, **(first image)** when returning to GNOME after using KDE Plasma, the icon and cursor theme is stuck on Plasma preferences, and there may be visual artifacts such as an oversize cursor. With Mending Wall **(second image)**, you get your most recent GNOME configuration back when you log into GNOME.

![GNOME after using KDE: the cursor is large and poorly rendered, icon and cursor themes are Breeze, the default on KDE, rather than Adwaita, as on GNOME](assets/gnome_broken_light.png#gh-light-mode-only){width=320}![GNOME after using KDE: the cursor is large and poorly rendered, icon and cursor themes are Breeze, the default on KDE, rather than Adwaita, as on GNOME](assets/gnome_broken_dark.png#gh-dark-mode-only){width=320}![Fixed! Mending Wall saves and restores the theme configuration as you had under GNOME. KDE Plasma can still modify the theme as needed while running too.](assets/gnome_fixed_light.png#gh-light-mode-only){width=320}![Fixed! Mending Wall saves and restores the theme configuration as you had under GNOME. KDE Plasma can still modify the theme as needed while running too.](assets/gnome_fixed_dark.png#gh-dark-mode-only){width=320}

## Tidy Menus

Core applications tend to clutter up menus and are often redudant with counterpart applications of other desktop environments. Mending Wall tidies them up, organizing your menus to only display the core applications of your current desktop environment.

For example, with GNOME, KDE and Xfce installed, there are six file manager **(first image, KDE shown)** and six terminal **(second image, GNOME shown)** applications available. 
    
![The KDE menu (light mode) showing six file manager applications](assets/kde_many_file_manager_apps_light.png#gh-light-mode-only){width=320}![The KDE menu (dark mode) showing six file manager applications](assets/kde_many_file_manager_apps_dark.png#gh-dark-mode-only){width=320}![The GNOME menu showing six terminal applications](assets/gnome_many_terminal_apps.png){width=320}

## What works?

Mending Wall is currently being tested, and could do with [your help](how-you-can-help.md). Desktop environments are all configured differently, but Linux distributions may also configure the same desktop environment differently, and so it is important to test widely across the two dimensions. As a guide, the following represents the current status.

|              | GNOME | KDE | Xfce | Cinnamon | [Others](how-you-can-help.md) |
| ------------ | :---: | :-: | :--: | :------: | :----: |
| :simple-ubuntu:&nbsp;**Ubuntu**<br/>:simple-debian:&nbsp;*Debian*<br/>:simple-linuxmint:&nbsp;*Linux Mint* | ✓✓ | ✓✓ | ✓ | ✓ | ? |
| :simple-opensuse:&nbsp;**openSUSE** | ✓ | ✓ | ✓ | ✓ | ? |
| :simple-endeavouros:&nbsp;**EndeavourOS**<br/>:simple-archlinux:&nbsp;*Arch&nbsp;Linux*<br/>:simple-manjaro:&nbsp;*Manjaro*| ✓ | ✓ | ? | ? | ? |
| :simple-fedora:&nbsp;Fedora | ? | ? | ? | ? | ? |
| [:simple-linux: Others](how-you-can-help.md) | ? | ? | ? | ? | ? |

| Legend                    |                                                                       |
| :-----------------------: | --------------------------------------------------------------------- |
| **:simple-linux: Tested** | Distribution has been tested                                          |
| *:simple-linux: Similar*  | Distribution has not been tested, but is very similar to one that has |
| ✓✓                        | Reported tested and working with regular use                          |
| ✓                         | Reported tested and working                                           |
| ?                         | Unknown, [give it a try](getting-started.md)!                         |

