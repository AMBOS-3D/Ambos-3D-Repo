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

static const char             *device = "/dev/spidev0.0" ;
static uint32_t                speed  = 500000 ;
static uint8_t                 mode   = SPI_MODE_0 ;
static uint8_t                 bits   = 8 ;

static unsigned char           tx[75] ;
static unsigned char           rx[75] ;

static struct spi_ioc_transfer parameters[1];

static int                     offset = 0 ;
static int                     spi ;

static int					   my_red;
static int					   my_green;
static int					   my_blue;

int OpenSPI(int red_offset, int green_offset, int blue_offset);
void CloseSPI();
void ResetState();
void Token( const char *token );
void Line( const char *ps );
void ReadFile( FILE *file );
void File( const char *filename );


