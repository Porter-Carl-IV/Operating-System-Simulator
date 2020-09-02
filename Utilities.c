#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Utilities.h"
#include "FileUpload.h"


bool charIn( char input, char *charPool )
    {
     int loopCounter = 0;

     while( charPool[ loopCounter ] != '\0')
         {
          if( input == charPool[ loopCounter ] )
              {
               return true;
              }

          loopCounter++;
         }
     return false;
    }

//Returns true if equivalent, false if different
bool strCompare( char *firstString, char *secondString )
    {
     int loopCounter = 0;

     while( firstString[ loopCounter ] != '\0' || secondString[ loopCounter ] != '\0' )
         {
          if( firstString[ loopCounter ] != secondString[ loopCounter ] )
              {
               return false;
              }
          loopCounter++;
         }
     return true;
    }

char toLowercase( char inputChar )
    {
     if( inputChar >= 'a' && inputChar <= 'Z' )
         {
          return inputChar - 'A' + 'a';
         }
     return inputChar;
    }

int strToInt( char *str )
    {
     int loopCounter = 0;
     int returnInt = 0;
     int strLength = stringLength( str ) - 1;
     int workingInt;

     while( str[ loopCounter ] != '\0' )
         {
          workingInt = (int)str[ loopCounter ] - '0';
          returnInt += workingInt * powerOfTen( strLength - loopCounter - 1 );

          loopCounter++;
         }

     return returnInt;
    }

int charToInt( char inputChar )
    {
     return (int)inputChar - '0';
    }

int stringLength( char *inputStr )
    {
     int lengthCounter = 0;

     while( inputStr[ lengthCounter ] != '\0')
         {
          lengthCounter++;
         }

     return lengthCounter + 1;
    }

int powerOfTen( int power )
    {
     int value = 10;
     int loopCounter;

     if( power == 0 )
         {
          return 1;
         }

     for( loopCounter = 0; loopCounter < power - 1; loopCounter++ )
         {
          value *= 10;
         }

     return value;
    }

char lastChar( char *str )
    {
     int index = 0;

     while( str[ index ] != '\0' )
         {
          index++;
         }
     if( index != 0 )
         {
          return str[ index - 1 ];
         }
     return '\0';
    }

void stringCopy( char *copyString, char *sourceString )
    {
     int index = 0;

     while( sourceString[index] != '\0' )
     {
       copyString[ index ] = sourceString[ index ];
       index++;
     }

     copyString[ index ] = '\0';

    }

void strConcat( char *firstString, char *secondString )
{
  int index = 0;
  int secondIndex = 0;

  while( firstString[ index ] != '\0' )
  {
    index++;
  }

  do
  {
    firstString[ index ] = secondString[ secondIndex ];

    index++;
    secondIndex++;

  } while(secondString[ secondIndex ] != '\0');

  firstString[ index ] = '\0';
}

//Takes 2 strings, inserts one string into the other at the ~ character then returns
void strInsert( char *returnString, char *mainString, char *insertString )
    {
      int returnIndex = 0;
      int insertIndex = 0;
      int mainIndex = 0;
      bool inserting = false;

      while( mainString[ mainIndex ] != '\0' )
      {
        if( mainString[ mainIndex ] == '~' || inserting )
        {
          inserting = false;

          if( insertString[ insertIndex ] != '\0' )
          {
            inserting = true;

            returnString[ returnIndex ] = insertString[ insertIndex ];

            insertIndex++;
          }
        }
        else
        {
          returnString[ returnIndex ] = mainString[ mainIndex ];

          mainIndex++;
        }

        returnIndex++;
      }

      returnString[ returnIndex ] = '\0';
    }
int intLength( int input )
{
  int count = 0;

  while( input % powerOfTen( count ) != input )
  {
    count++;
  }

  return count;
}

void intToString( char *returnString, int toConvert )
{
  int index;

  for( index = 0; index < intLength( toConvert ); index++)
  {
    returnString[ index ] = ( ( toConvert % powerOfTen( index + 1 ) )/ powerOfTen( index ) );
  }

  returnString[ index ] = '\0';
}
