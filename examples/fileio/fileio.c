/*

  fileio.c

  Example libfat & ezfo-disc_io Gameboy Advance file handling

*/

#include <gba.h>
#include <fat.h>

#include <stdio.h> // iprintf, fopen, fwrite, fclose
#include <unistd.h> // chdir

// EZ Flash Omega disc interface
#include "io_ezfo.h"

// EZ Flash Omega mounts an SD card
#define EZFO_DEVNAME "sd"

#define DEMO_FILENAME "fileio_demo.txt"

#define EVER

/**
 * Locks device into loop
 */
static void waitForever() {
	for ( EVER ) {
		VBlankIntrWait();
  }
}

/**
 * Mounts EZ Flash Omega SD
 * or uses whatever fatsystem libgba might be available
 */
static bool initFatSystem() {
  const char * devname;

  // Attempt to mount EZFO and change working directory into device
	if ( fatMountSimple( EZFO_DEVNAME, &_io_ezfo ) && chdir( EZFO_DEVNAME ":/" ) == 0 ) {
    devname = EZFO_DEVNAME;
  } else if ( fatInitDefault() ) {
    devname = "fat";
	} else {
    devname = NULL;
	}

  if ( !devname ) {
    iprintf( "[FAIL] Failed to mount or init a FAT device.\n" );
    return false;
  }

  iprintf( "[ OK ] Mounted dev %s.\n", devname );
  return true;
}

/**
 * Program entry
 */
int main() {
  FILE * file;
  u32 value; // Number in the example file

  irqInit();
	irqEnable( IRQ_VBLANK ); // Enable IRQ_VBLANK for VBlankIntrWait

  // libgba console
	consoleDemoInit();

  iprintf( "Welcome to GBA file IO demo!\n\n" );

  if ( !initFatSystem() ) {
    waitForever();
  }

  file = fopen( DEMO_FILENAME, "r" );
  if ( file ) {
    fscanf( file, "%ld", &value );
    fclose( file );

    iprintf( "[ OK ] Opened " DEMO_FILENAME " (%ld).\n", value );

    value++; // Increment reads
  } else {
    iprintf( "[FAIL] " DEMO_FILENAME " does not exist.\n" );

    value = 1; // First read
  }

  file = fopen( DEMO_FILENAME, "w" );
  if ( !file ) {
    iprintf( "[FAIL] " DEMO_FILENAME " could not be created.\n" );
    waitForever();
  }

  if ( fprintf( file, "%ld", value ) ) {
      iprintf( "[ OK ] Wrote %ld.\n", value );
  } else {
      iprintf( "[FAIL] Write failure.\n" );
  }
  fclose( file );

  iprintf( "[ OK ] File saved.\n" );
  waitForever();
}
