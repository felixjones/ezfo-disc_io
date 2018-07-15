/*

  io_ezfo.h

  Hardware Routines for reading the EZ Flash Omega filesystem

*/

#ifndef __IO_EZFO_H__
#define __IO_EZFO_H__

// 'EZFO'
#define DEVICE_TYPE_EZFO 0x4F465A45

#include <disc_io.h>

// Export interface
extern const DISC_INTERFACE _io_ezfo;

#endif	// define __IO_EZFO_H__
