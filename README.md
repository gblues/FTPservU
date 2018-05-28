# FTPservU

A rewrite of FTPiiU, an FTP server for WiiU homebrew

## About this project

This started as an attempt to add a new feature to FTPiiU, but there's
a number of problems with that codebase:

- It requires libogc, which is a Wii library
- Tons of compiler warnings
- Doesn't work when built with current tools

So, it became a rewrite of sorts:

1. I started with the Wii U bootstrap code from RetroArch
2. Created `main()` and built it out following FTPiiU as a
   guide, but
3. Applying my own design standards to it to improve code
   quality.

## What's new?

Well, nothing, yet. Right now, it doesn't even work.

The feature that I'm trying to add is the ability to unmount and
remount filesystems, so that a user can swap SD cards at runtime.

This is borne out of necessity: my PC's SD card reader can't read 
my new SDHC card, and I don't want to buy a reader.
