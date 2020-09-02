
#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include "Utilities.h"
#include "Sim04.h"
#include "simtimer.h"
#include "FileUpload.h"

int systemStart( Process *processHeader, ConfigData *config );

Process *scheduler( Process *processHeader, ConfigData *config, char *simClock );

bool stateChecker( Process *processHead, int requestCode );

int runProcess( Process *runningProcess, ConfigData *config, char *simClock, int runType );

int runProcessN( Process *runningProcess, ConfigData *config, char *simClock );

int ioService( ServiceCall *serviceRequest, ConfigData *config, int processID, char *simClock );

void *ioThreadRun( void *time );

int memoryService( ServiceCall *memoryRequest, int processID, ConfigData *config );

AllocatedMemory *segmentationFault( AllocatedMemory *mmu, int dumpProcess );

int memoryAllocate( AllocatedMemory *mmu, int requestCode, int processID, int totalMemory );

int memoryAccess( AllocatedMemory *mmu, int requestCode, int processID, int totalMemory );

int memoryDump( AllocatedMemory *mmu );

int runService( int runTime, char *simClock, int processID, ConfigData *config );

int runProcessP( Process *runningProcess, ConfigData *config, char *simClock, int quantumTime );

int roundRobinQueue( int controlCode, int processID );

int interruptManager( int controlCode, Process *processInitializer, ConfigData *configInitializer );

int ioServiceP( Process *runningProcess, ConfigData *config );

void *ioThreadRunP( void *input );

int runServiceP( Process *runningProcess, ConfigData *config );

void printToFile( char *fileName , RunLog *runRecord );

RunLog *addNewLog( RunLog *currentLog );

void outputManager( char *outputString, int controlCode, ConfigData *configInitialize );

int totalProcessTime( ServiceCall *headNode, ConfigData *config );

void systemStop( ConfigData *config, Process *processHeader );

#endif
