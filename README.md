# ezfo-disc_io
libfat Gameboy Advance Disc Interface for EZ Flash Omega flash cartridge

# Usage #

Compile io_ezfo.c and include io_ezfo.h in your FAT initialisation.

The EZ Flash Omega SD can be mounted with libfat using:
```c
const bool ismount = fatMountSimple( "fat", &_io_ezfo ); // _io_ezfo from io_ezfo.h
if ( ismount ) {
  const int cderr = chdir( "fat:/" ); // Change working directory to fat:/ device
  if ( cderr == 0 ) {
    // Mount success
  } else {
    // Change directory fail
  }
} else {
  // Mount fail
}
```

fatInitDefault can still be used to keep compatibility with other flash-carts:
```c
if ( fatInitDefault() || ( fatMountSimple( "fat", &_io_ezfo ) && chdir( "fat:/" ) == 0 ) ) {
  // Mount success
} else {
  // Mount fail
}
```

# Known issues #

## PSRAM only ##

For now, only ROMs copies to EZ Flash Omega's PSRAM are supported. NOR ROMs definitely can work, however there is a lot of complexity involved with switching back to NOR game mode from  OS kernel mode related to detecting ROM binaries.

When a better method for retrieving a game's NOR page is discovered this issue can be easily solved.

## Code is copied to EWRAM ##

Switching to OS kernel mode changes the contents of the ROM, so it cannot be used with the memory map. This means execution of EZ Flash Omega FAT read/write must happen in memory, not in ROM.

This is not ideal as it means some of your EWRAM will be taken up by EZFO disc_io, even for other flash carts.

The solution to this is to pre-compile the read/write routines as a binary and copy that binary from ROM into memory when it is needed (and then free it after).

## Working directory is not binary's location on disk ##

This is a general problem with flash carts and homebrew. Not much can be done about this, unless flash carts start exposing the executable path.

Means everything generally must be done at the root of the filesystem. 