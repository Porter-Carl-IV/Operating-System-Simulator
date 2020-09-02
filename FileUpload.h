
#ifndef FILEUPLOAD_H
#define FILEUPLOAD_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Utilities.h"
#include "Sim04.h"

ServiceCall *newNode( char compLetter, int cycTime, char operationString[] );

void clearServiceCalls( ServiceCall *headNode );

int fileInput( char *fileName, Process *headNode );

void getNextExpression( FILE *inputFile, char *returnString );

ServiceCall *parseStr( char *parsingString );

bool validArgument( char *argument, int type );

int loadConfigFile( char *fileName, ConfigData *returnConfigData );

void getNextDataLabel( FILE *fileName, char *returnString );

void getNextDataEntry( FILE *fileName, char *returnString );

#endif
