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
#include "ledstripPbL.hpp"

/*************/
 int OpenSPI(int red_offset, int green_offset, int blue_offset)
/*************/

{
   spi = open( device, O_RDWR ) ;
   if( spi < 0 )
   {
      fprintf( stderr, "can't open device\n" ) ;
      return 1 ;
   }

   int ret = ioctl( spi, SPI_IOC_WR_MODE, &mode ) ;
   if( ret == -1 )
   {
      close( spi ) ;
      fprintf( stderr, "can't set mode\n" ) ;
      return 2 ;
   }

   ret = ioctl( spi, SPI_IOC_WR_BITS_PER_WORD, &bits ) ;
   if( ret == -1 )
   {
      close( spi ) ;
      fprintf( stderr, "can't set bits\n" ) ;
      return 3 ;
   }

   ret = ioctl( spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed ) ;
   if( ret == -1 )
   {
      close( spi ) ;
      fprintf( stderr, "can't set speed\n" ) ;
      return 4 ;
   }

   memset( &parameters, 0, sizeof(spi) ) ;
 
   parameters[0].tx_buf        = (unsigned long)tx ;
   parameters[0].rx_buf        = (unsigned long)rx ;
   parameters[0].len           = 75 ;
   parameters[0].delay_usecs   = 0 ; 
   parameters[0].speed_hz      = speed ;
   parameters[0].bits_per_word = bits ;
   parameters[0].cs_change     = 0 ;

   my_red = red_offset;
   my_green = green_offset;
   my_blue = blue_offset;

   return 0 ;
}

/***************/
 void CloseSPI()
/***************/

{
   close( spi ) ;
}

/*****************/
 void ResetState()
/*****************/

{
   memset( tx, 0, sizeof(tx) ) ;
}


/****************/
 void ShowState()
/****************/

{
   if( ioctl(spi,SPI_IOC_MESSAGE(1),&parameters) == -1 )
      fprintf( stderr, "can't transfer data\n" ) ;
}

/*******************************/
 void Token( const char *token )
/*******************************/

{
   if( isdigit(*token) )
   {
      int value = atoi( token )  - 1 ;

      if( (value >= 0) && (value <= 24) )
         tx[value*3+offset] = 8;
   }
   else
   {
      switch( tolower(*token) )
      {
         case 'r' : offset = my_red ;
                    break ; 
         case 'b' : offset = my_blue ;
                    break ; 
         case 'g' : offset = my_green ;
                    break ; 
      }
   }
}

/***************************/
 void Line( const char *ps )
/***************************/

{
   ResetState() ;

   char    token[25] ;
   size_t  i = 0 ;

   while( *ps != '\0' )
   {
      if( isspace(*ps) )
      {
         if( i > 0 )
         {
            token[i] = '\0' ;
            Token( token ) ;
         }

         i = 0 ;
      }
      else
      {
         if( i < sizeof(token) - 1 )
            token[i++] = *ps ;
      }
            
      ++ps ;
   }

   ShowState() ;
}


/***************************/
 void ReadFile( FILE *file )
/***************************/

{
   char            line[1024] ;
   struct timespec in ;
   struct timespec out ;
 

   while( fgets(line,sizeof(line),file) != NULL )
   {
      Line( line ) ;

      in.tv_sec  = 0 ;
      in.tv_nsec = 125000000 ;
      nanosleep( &in, &out ) ;
   }
}

/*********************************/
 void File( const char *filename )
/*********************************/

{
   FILE *file = fopen( filename, "r" ) ;

   if( file != NULL )
   {
      ReadFile( file ) ;
      fclose( file ) ;
   }
}

