
#ifndef FILEUPLOAD_C
#define FILEUPLOAD_C

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Utilities.h"
#include "FileUpload.h"
#include "Sim04.h"

//Calling function is responsible for de-allocating memory of struct when done

ServiceCall *newNode( char compLetter, int cycTime, char *opString )
    {
     ServiceCall *newNode = ( ServiceCall * ) malloc( sizeof( ServiceCall ) );
     newNode->componentLetter = compLetter;
     stringCopy( newNode->operationString, opString );
     newNode->cycleTime = cycTime;
     newNode->next = NULL;

     return newNode;
   }

Process *newProcess( Process *currentProcess )
    {
         static int processID = 1;
         static bool initialized = false;
         Process *newProcess = ( Process * ) malloc ( sizeof( Process ) );

         if( !initialized )
         {
           currentProcess->serviceCallHeader = NULL;
           currentProcess->currentServiceCall = NULL;
           currentProcess->interruptFlag = false;
           currentProcess->processID = processID;
           currentProcess->state = NEW;
           currentProcess->next = NULL;

           processID++;

           free( newProcess );

           initialized = true;

           return currentProcess;
         }

         newProcess->serviceCallHeader = NULL;
         newProcess->currentServiceCall = NULL;
         newProcess->interruptFlag = false;
         newProcess->processID = processID;
         newProcess->state = NEW;
         newProcess->next = NULL;
         currentProcess->next = newProcess;

         processID++;

         return newProcess;
    }

void clearServiceCalls( ServiceCall *headNode )
    {
     ServiceCall *clearNode;
     int count = 0;

     while( headNode != NULL )
         {
          clearNode = headNode;
          headNode = headNode->next;

          free( clearNode );
          count++;
        }
    }

int fileInput( char *fileName, Process *headProcess )
    {
      FILE *inputFile = fopen( fileName, "r" );
      char *needToParseStr = ( char * ) malloc( sizeof (char) * MAX_STR_LENGTH );
      ServiceCall *tempNode, *newNode, *workingNode;
      int character;

      if( inputFile == NULL )
          {
           free( needToParseStr );
           return FILE_ERROR;
          }

      while( ( character = fgetc( inputFile ) ) != ':' ) {} //Skips file header
      getNextExpression( inputFile, needToParseStr );

      if( ( newNode = parseStr( needToParseStr ) ) == NULL )
      {
        free( needToParseStr );
        fclose( inputFile );
        return FORMAT_ERROR;
      }

      headProcess = newProcess( headProcess );
      headProcess->serviceCallHeader = newNode;
      headProcess->currentServiceCall = newNode;

      workingNode = headProcess->serviceCallHeader;

      character = fgetc( inputFile ); //Skip the space
      getNextExpression( inputFile, needToParseStr );

      if( ( newNode = parseStr( needToParseStr ) ) == NULL )
      {
        free( needToParseStr );
        fclose( inputFile );
        return FORMAT_ERROR;
      }

      workingNode->next = newNode;

      do
      {
       character = fgetc( inputFile ); //Skip the space
       getNextExpression( inputFile, needToParseStr );
       tempNode = newNode;

       if( ( newNode = parseStr( needToParseStr ) ) == NULL )
       {
         free( needToParseStr );
         fclose( inputFile );
         return FORMAT_ERROR;
       }

       if( newNode->componentLetter == 'A' && strCompare( newNode->operationString, "start\0" ) )
       {

         headProcess = newProcess( headProcess );
         headProcess->serviceCallHeader = newNode;
         headProcess->currentServiceCall = newNode;

       }
       else
       {
         tempNode->next = newNode;
       }

     } while( lastChar( needToParseStr ) != '.' );

     free( needToParseStr );
     fclose( inputFile );
     return SUCCESSFUL;
    }

ServiceCall *parseStr( char *parsingString )
{
  char componentLetter;
  char operationString[MAX_STR_LENGTH];
  char cycleTime[MIN_STR_LENGTH];
  int parseIndex;
  int index;

       //Parse component letter from expression
       componentLetter = parsingString[ COMPONENT_INDEX ];

       if( !validArgument( parsingString, COMPONENT_LETTER ) )
           {
             printf("%c\n" , componentLetter );
            return NULL;
           }

       //parse operation string from expression
       if( parsingString[ OPEN_PAREN_INDEX ] != '(' )
           {
            return NULL;
           }
       parseIndex = OPEN_PAREN_INDEX + 1;
       index = 0;
       while( parsingString[ parseIndex ] != ')' )
           {
            if( parsingString[ parseIndex ] == ';' )
                {
                 return NULL;
                }
            operationString[ index ] = parsingString[ parseIndex ];

            parseIndex++;
            index++;
           }
       operationString[ index ] = '\0';

       if( !validArgument( operationString, OPERATION_STRING ) )
           {
            return NULL;
           }

       //Parse cycle time from expression
       index = 0;
       parseIndex++;

       while( parsingString[ parseIndex ] != ';' && parsingString[ parseIndex ] != '.')
           {
            cycleTime[ index ] = parsingString[ parseIndex ];
            parseIndex++;
            index++;
           }
           cycleTime[ index ] = '\0';

       if( !validArgument( cycleTime, CYCLE_TIME ) )
           {
            return NULL;
           }

      return newNode( componentLetter, strToInt( cycleTime ), operationString );
}

void getNextExpression( FILE *inputFile, char *returnString )
    {
     int currentChar;
     int index = 0;

     while( ( currentChar = fgetc( inputFile ) ) != ';' && currentChar != '.' )
         {
          if( currentChar != '\n')
              {
               returnString[ index ] = (char)currentChar;

               index++;
              }
         }
     returnString[ index ] = currentChar;
     returnString[ index + 1 ] = '\0';
    }

bool validArgument( char *argument, int type )
    {
     int index = 0;

     switch( type )
         {
          case CYCLE_TIME:
              while( argument[ index ] != '\0' )
                  {
                   if( !charIn( argument[ index ], "0123456789\0" ) )
                       {
                        return false;
                       }
                   index++;
                  }
              return true;
          break;
          case OPERATION_STRING:
              if( !strCompare( argument, "start\0") && !strCompare( argument, "allocate\0" ) &&
                  !strCompare( argument, "hard drive\0" ) && !strCompare( argument, "keyboard\0" )
                  && !strCompare( argument, "run\0" ) && !strCompare( argument, "printer\0" ) &&
                  !strCompare( argument, "access\0" ) && !strCompare( argument, "end\0" )
                  && !strCompare( argument, "monitor\0" ) )
                      {
                       return false;
                      }
              return true;
          break;
          case COMPONENT_LETTER:
              if( !charIn( argument[ index ], "SOMIPA\0" ) )
                  {
                   return false;
                  }
              return true;
          break;
        }
        return false;
    }
//////////////////////////////Config/////////////////////////////////////////

int loadConfigFile( char *fileName, ConfigData *returnConfigData )
    {
      FILE *configFile = fopen( fileName, "r" );
      char *workingStr = ( char *) malloc( sizeof ( char )  * MAX_STR_LENGTH );

      if( configFile == NULL )
          {
           free( workingStr );
           return FILE_ERROR;
          }

      //Skip first line
      getNextDataEntry( configFile, workingStr );
      getNextDataLabel( configFile, workingStr );

      if( !strCompare( workingStr, "Version/Phase\0") )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

      getNextDataEntry( configFile, workingStr );

      if( !charIn( workingStr[ 0 ], "0123456789\0" ) )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

      returnConfigData->version = charToInt( workingStr[0] );

      getNextDataLabel( configFile, workingStr );

      if( !strCompare( workingStr, "File Path\0" ) )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

      getNextDataEntry( configFile, workingStr );
      stringCopy( returnConfigData->filePath, workingStr );

      getNextDataLabel( configFile, workingStr );

      if( !strCompare( workingStr, "CPU Scheduling Code\0" ) )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

      getNextDataEntry( configFile, workingStr );

      if( strCompare( workingStr, "NONE\0" ) || strCompare( workingStr, "FCFS-N\0" ))
          {
           returnConfigData->cpuScheduleCode = FCFSN;
          }
      if( strCompare( workingStr, "SJF-N\0" ) )
          {
           returnConfigData->cpuScheduleCode = SJFN;
          }
      if( strCompare( workingStr, "FCFS-P\0" ) )
          {
           returnConfigData->cpuScheduleCode = FCFSP;
          }
      if( strCompare( workingStr, "SRTF-P\0" ) )
          {
           returnConfigData->cpuScheduleCode = SRTFP;
          }
      if( strCompare( workingStr, "RR-P\0" ) )
          {
           returnConfigData->cpuScheduleCode = RRP;
          }

      getNextDataLabel( configFile, workingStr );

      if( !strCompare( workingStr, "Quantum Time (cycles)\0") )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

      getNextDataEntry( configFile, workingStr );

      if( strToInt( workingStr ) < 0 || strToInt( workingStr ) > 100 )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

      returnConfigData->quantumTime = strToInt ( workingStr );

      getNextDataLabel( configFile, workingStr );

      if( !strCompare( workingStr, "Memory Available (KB)\0" ) )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

      getNextDataEntry( configFile, workingStr );

      if( strToInt( workingStr ) < 0 || strToInt( workingStr ) > 1048576 )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

      returnConfigData->availableMemory = strToInt( workingStr );

      getNextDataLabel( configFile, workingStr );

      if( !strCompare( workingStr, "Processor Cycle Time (msec)\0" ) )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

      getNextDataEntry( configFile, workingStr );

      if( strToInt( workingStr ) < 0 || strToInt( workingStr ) > 1000 )
          {
           fclose( configFile );
           free( workingStr );
           return FORMAT_ERROR;
          }

     returnConfigData->processorCycleRate = strToInt( workingStr );

     getNextDataLabel( configFile, workingStr );

     if( !strCompare( workingStr, "I/O Cycle Time (msec)\0" ) )
         {
          fclose( configFile );
          free( workingStr );
          return FORMAT_ERROR;
         }

     getNextDataEntry( configFile, workingStr );

     if( strToInt( workingStr ) < 0 || strToInt( workingStr ) > 10000 )
         {
          fclose( configFile );
          free( workingStr );
          return FORMAT_ERROR;
         }

     returnConfigData->ioCycleRate = strToInt( workingStr );

     getNextDataLabel( configFile, workingStr );

     if( !strCompare( workingStr, "Log To\0" ) )
         {
          fclose( configFile );
          free( workingStr );
          return FORMAT_ERROR;
         }

    getNextDataEntry( configFile, workingStr );

    if( !strCompare( workingStr, "Monitor\0" ) && !strCompare( workingStr, "File\0" ) && !strCompare( workingStr, "Both\0" ) )
        {
         fclose( configFile );
         free( workingStr );
         return FORMAT_ERROR;
        }

    stringCopy( returnConfigData->logTo, workingStr );

    getNextDataLabel( configFile, workingStr );

    if( !strCompare( workingStr, "Log File Path\0" ) )
        {
         fclose( configFile );
         free( workingStr );
         return FORMAT_ERROR;
        }

   getNextDataEntry( configFile, workingStr );

   stringCopy( returnConfigData->logFilePath, workingStr);

   fclose( configFile );
   free( workingStr );

   return SUCCESSFUL;
  }
void getNextDataLabel( FILE *fileName, char *returnString )
    {
     int index = 0;
     int character;

     while( ( character = fgetc( fileName ) ) != ':' )
         {
          returnString[ index ] = (char)character;
          index++;
         }

     returnString[ index ] = '\0';
    }

void getNextDataEntry( FILE *fileName, char *returnString )
    {
     int index = 0;
     int character;


     while( ( character = fgetc( fileName ) ) != '\n' )
         {
          if( character != ' ' )
              {
               returnString[ index ] = (char)character;
               index++;
              }
         }

     returnString[ index ] = '\0';
    }
#endif
