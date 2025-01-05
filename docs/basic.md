# Basic Usage


## Preserve themes

When the *preserve themes* feature is activated, Mending Wall starts a background process immediately, and on subsequent logins, that:

1. Backs up specific configuration settings for the current desktop environment.
2. Monitors for changes in these and, when changed, updates the backup. This means that you can change the theme or other settings freely.
3. When you log out, the background process terminates.
4. When you next log in to the same desktop environment, the backup is restored.

Between the log out and log in, you can use another desktop environment. It may change the specific configuration settings as needed for its own purposes, but when you log into the original desktop environment again, the backup is restored, and these changes will not affect the original experience.

