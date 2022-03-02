/******************************************************************************/
/*                                                                            */
/*                                                         FILE: ledstrip.cpp */
/*                                                                            */
/*    Displays the contents of a file on a LED strip                          */
/*    ==============================================                          */
/*                                                                            */
/*    V0.01  18-DEC-2015   Te                                                 */
/*                                                                            */
/******************************************************************************/
#pragma once
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

int OpenSPI(int red_offset, int green_offset, int blue_offset);
void CloseSPI();
void ResetState();
void Token( const char *token );
void Line( const char *ps );
void ReadFile( FILE *file );
void File( const char *filename );


