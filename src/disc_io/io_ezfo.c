/*

  io_ezfo.c

  Hardware Routines for reading the EZ Flash Omega filesystem

*/

#include "io_ezfo.h"

#include <gba_dma.h>

#define OMEGA_ROM_UNKNOWN ( 0x0000 )
#define OMEGA_ROM_PSRAM   ( 0x9780 )
#define OMEGA_ROM_NOR     ( 0xFFFF )

#define OMEGA_ROM_PAGE_UNKNOWN  ( 0xFFFF )
#define OMEGA_ROM_PAGE_PSRAM    ( 0x0200 )
#define OMEGA_ROM_PAGE_KERNEL   ( 0x8000 )

#define OMEGA_SD_BIT_ENABLE     ( 0x1 << 0 )
#define OMEGA_SD_BIT_READ_STATE ( 0x1 << 1 )

#define OMEGA_SD_CTL_ENABLE     ( OMEGA_SD_BIT_ENABLE )
#define OMEGA_SD_CTL_READ_STATE ( OMEGA_SD_BIT_ENABLE | OMEGA_SD_BIT_READ_STATE )
#define OMEGA_SD_CTL_DISABLE    ( 0x0 )

/**
 *
 * Miscellaneous utility functions
 *
 */

static inline void _Spin( u32 _cycles ) {
  while ( _cycles-- ) {
    asm (
      "nop"
    );
  }
}

/**
 *
 * Omega device functions
 *
 */

static inline void _Omega_SetROMPage( const u16 _page ) {
  *( vu16 * )0x9fe0000 = 0xd200;
  *( vu16 * )0x8000000 = 0x1500;
  *( vu16 * )0x8020000 = 0xd200;
  *( vu16 * )0x8040000 = 0x1500;
  *( vu16 * )0x9880000 = _page;
  *( vu16 * )0x9fc0000 = 0x1500;
}

static inline void _Omega_SetSDControl( const u16 _control ) {
  *( vu16 * )0x9fe0000 = 0xd200;
  *( vu16 * )0x8000000 = 0x1500;
  *( vu16 * )0x8020000 = 0xd200;
  *( vu16 * )0x8040000 = 0x1500;
  *( vu16 * )0x9400000 = _control;
  *( vu16 * )0x9fc0000 = 0x1500;
}

static inline u32 _Omega_WaitSDResponse() {
  vu16 respose;
  u32 waitSpin = 0;
  while ( waitSpin < 0x100000 ) {
    respose = *( vu16 * )0x9E00000;
    if ( respose != 0xEEE1 ) {
      return 0;
    }
    waitSpin += 1;
  }
  return 1;
}

/**
 *
 * Disc interface functions
 *
 */

bool _EZFO_startUp() {
  return *( vu16 * )0x9fe0000 == OMEGA_ROM_PSRAM;
}

bool _EZFO_isInserted() {
  return true;
}

bool EWRAM_CODE _EZFO_readSectors( u32 _address, u32 _count, void * _buffer ) {
  _Omega_SetROMPage( 0x8000 ); // Change to OS mode

  _Omega_SetSDControl( OMEGA_SD_CTL_ENABLE );

  u32 readsRemain = 2;
  for ( u16 ii = 0; ii < _count; ii += 4 ) {
    const u16 blocks = ( _count - ii > 4 ) ? 4 : ( _count - ii );

    while ( readsRemain ) {
      *( vu16 * )0x9fe0000 = 0xd200;
      *( vu16 * )0x8000000 = 0x1500;
      *( vu16 * )0x8020000 = 0xd200;
      *( vu16 * )0x8040000 = 0x1500;
      *( vu16 * )0x9600000 = ( ( _address + ii ) & 0x0000FFFF);
      *( vu16 * )0x9620000 = ( ( _address + ii ) & 0xFFFF0000) >> 16;
      *( vu16 * )0x9640000 = blocks;
      *( vu16 * )0x9fc0000 = 0x1500;

      _Omega_SetSDControl( OMEGA_SD_CTL_READ_STATE );
      const u32 response = _Omega_WaitSDResponse();
      _Omega_SetSDControl( OMEGA_SD_CTL_ENABLE );
      if ( response && --readsRemain ) {
        _Spin( 5000 );
      } else {
        dmaCopy( ( void * )0x9E00000, ( void * )( _buffer + ii * 512 ), blocks * 512 );
        break;
      }
    }
  }

  _Omega_SetSDControl( OMEGA_SD_CTL_DISABLE );

  _Omega_SetROMPage( 0x200 ); // Return to original mode
  return true;
}

bool EWRAM_CODE _EZFO_writeSectors( u32 _address, u32 _count, const void * _buffer ) {
  _Omega_SetROMPage( 0x8000 ); // Change to OS mode

  _Omega_SetSDControl( OMEGA_SD_CTL_READ_STATE );
  for ( u16 ii = 0; ii < _count; ii++ ) {
    const u16 blocks = ( _count - ii > 4 ) ? 4 : ( _count - ii );

    dmaCopy( _buffer + ii * 512, ( void * )0x9E00000, blocks * 512 );
    *( vu16 * )0x9fe0000 = 0xd200;
    *( vu16 * )0x8000000 = 0x1500;
    *( vu16 * )0x8020000 = 0xd200;
    *( vu16 * )0x8040000 = 0x1500;
    *( vu16 * )0x9600000 = ( ( _address + ii ) & 0x0000FFFF );
    *( vu16 * )0x9620000 = ( ( _address +ii ) & 0xFFFF0000 ) >> 16;
    *( vu16 * )0x9640000 = 0x8000 + blocks;
    *( vu16 * )0x9fc0000 = 0x1500;

    _Omega_WaitSDResponse();
  }

  _Spin( 3000 );

  _Omega_SetSDControl( OMEGA_SD_CTL_DISABLE );

  _Omega_SetROMPage( 0x200 ); // Return to original mode
  return true;
}

bool _EZFO_clearStatus() {
  return true;
}

bool _EZFO_shutdown() {
  return true;
}

const DISC_INTERFACE _io_ezfo = {
  DEVICE_TYPE_EZFO,
  FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
  ( FN_MEDIUM_STARTUP )&_EZFO_startUp,
  ( FN_MEDIUM_ISINSERTED )&_EZFO_isInserted,
  ( FN_MEDIUM_READSECTORS )&_EZFO_readSectors,
  ( FN_MEDIUM_WRITESECTORS )&_EZFO_writeSectors,
  ( FN_MEDIUM_CLEARSTATUS )&_EZFO_clearStatus,
  ( FN_MEDIUM_SHUTDOWN )&_EZFO_shutdown
};
