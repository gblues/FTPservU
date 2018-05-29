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

## Building

You will need the following:

- devkitPPC r28 (r29 and r29-1 are untested and may break things)
- libfat and libiosuhax installed under portlibs

The directory structure ends up looking like this:

    $DEVKITPRO (e.g. /opt/devkitpro)
    |
    +-$DEVKITPPC (typically devkitPPC)
    |
    +-portlibs
      |
      +-ppc
        |
        +-include
        |   fat.h
        |   iosuhax_devoptab.h
        |   iosuhax_disc_interface.h
        |   iosuhax.h
        |   libfatversion.h
        |
        +-lib
            libfat.a
            libiosuhax.a

Once you have the structure set up, building is simple:

1. Ensure DEVKITPRO and DEVKITPPC variables are set appropriately
2. run `make all` which will build both an ELF and an RPX.
3. Create the directory `/wiiu/apps/FTPservU/` on your WiiU's SD card
   and drop the ELF or RPX into it (one or the other, not both!)

TODO: Script together a dist directory so that it can be unzipped onto the
SD card and used by HBL easily.
