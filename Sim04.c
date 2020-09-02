
#ifndef SIM03_C
#define SIM03_C

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Utilities.h"
#include "FileUpload.h"
#include "System.h"
#include "Sim04.h"
#include "simtimer.h" //Sim Timer code written by Michael Leverington

/*
-Concurrency accomplished with threads. Main loop spawns an IO thread then
continues on, when the IO thread finishes it sets an interrupt flag to "true".
Interrupt manager function loops through processes looking for "true" interrupt
flags then resolves.

-Memory management implemented and functioning. I can't find any errors.
*/

int main( int argc, char *argv[] )
    {
     Process *tempProcess;
     ConfigData *myConfigData =
                      ( ConfigData * ) malloc( sizeof ( ConfigData ) );

     Process *myProcess =
                      ( Process * ) malloc( sizeof ( Process ) );

     if( argc > 2 )
         {
           printf( "ERROR: Too many arguments. Config file name only.\n");
           free( myConfigData );
           free( myProcess );
           return 0;
         }

     switch( loadConfigFile( argv[ 1 ], myConfigData ) )
         {
          case FILE_ERROR:
              printf( "ERROR: Config file doesn't exist. Exiting program.\n" );
              free( myConfigData );
              free( myProcess );

              return 0;
              break;
          case FORMAT_ERROR:
              printf( "ERROR: Incorrect config file format. Exiting program.\n" );
              free( myConfigData );
              free( myProcess );

              return 0;
              break;
          case SUCCESSFUL:
              printf( "Config file Succesfully uploaded.\n" );
              break;
         }

         switch( fileInput( myConfigData->filePath, myProcess ) )
         {
          case FILE_ERROR:
              printf( "ERROR: MetaData file doesn't exist. Exiting program.\n" );
              free( myConfigData );
              free( myProcess );

              return 0;
              break;
          case FORMAT_ERROR:
              printf( "ERROR: Incorrect MetaData file format. Exiting program.\n" );
              free( myConfigData );
              while( myProcess != NULL )
              {
                clearServiceCalls( myProcess->serviceCallHeader );
                tempProcess = myProcess;
                myProcess = myProcess->next;

                free( tempProcess );
              }

              return 0;
              break;
          case SUCCESSFUL:
              printf( "MetaData file successfully uploaded.\n" );
              break;
         }

         switch( systemStart( myProcess, myConfigData ) )
         {
           case SUCCESSFUL:
               return 0;
               break;
           case SEGMENTATION_FAULT:
               printf( "OS Segmentation Fault (core dump)\n");
               return 0;
               break;
           case FORMAT_ERROR:
               printf( "Memory request code format incorrect. Shutting down.\n");
               return 0;
               break;
         }

        return 0;
    }

#endif
