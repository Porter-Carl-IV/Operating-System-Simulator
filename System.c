#ifndef SYSTEM_C
#define SYSTEM_C

#include "System.h"

int systemStart( Process *processHeader, ConfigData *config )
{
  Process *currentProcess;
  char *outputString = ( char * ) malloc( sizeof( char ) * MAX_STR_LENGTH );
  int errorCode, runType;
  bool idling = false;

  char *simClock = ( char * ) malloc( sizeof( char ) * MIN_STR_LENGTH );

  accessTimer( ZERO_TIMER, simClock );

  sprintf( outputString, "Time:  %s, System start", simClock );
  outputManager( outputString, INITIALIZE, config );

  accessTimer( LAP_TIMER, simClock );

  sprintf( outputString, "Time:  %s, Begin PCB Creation", simClock );
  outputManager( outputString, OUTPUT, NULL );

  accessTimer( LAP_TIMER, simClock );

  sprintf( outputString, "Time:  %s, All processes initialized in New state" , simClock );
  outputManager( outputString, OUTPUT, NULL );

  stateChecker( processHeader, INITIALIZE );

  accessTimer( LAP_TIMER, simClock );

  sprintf( outputString, "Time:  %s, All processes now set in Ready state", simClock );
  outputManager( outputString, OUTPUT, NULL );

  if( config->cpuScheduleCode == FCFSN || config->cpuScheduleCode == SJFN )
  {
    runType = NONPREEMPTIVE;
  }
  else
  {
    interruptManager( INITIALIZE, processHeader, config );
    runType = PREEMPTIVE;
  }

  while( !stateChecker( processHeader, CHECK_ALL_TERMINATED ) )
  {
      currentProcess = scheduler( processHeader, config, simClock );

      if( ( currentProcess == NULL ) )
      {
        if( !idling )
        {
          accessTimer( LAP_TIMER, simClock );

          sprintf( outputString, "Time:  %s, OS: CPU Idle" , simClock );
          outputManager( outputString, OUTPUT, NULL );
        }

        runTimer( config->processorCycleRate );

        if( interruptManager( CHECK, NULL, NULL ) == NEW )
        {
          interruptManager( RESOLVE, NULL, NULL );
        }

        idling = true;
      }
      else
      {
        idling = false;

        accessTimer( LAP_TIMER, simClock );

        sprintf( outputString, "Time:  %s, OS: Process %i set in Running state" , simClock, currentProcess->processID );
        outputManager( outputString, OUTPUT, NULL );

        errorCode = runProcess( currentProcess, config, simClock, runType );

        if( errorCode == SEGMENTATION_FAULT )
        {
          accessTimer( LAP_TIMER, simClock );

          sprintf( outputString, "Time:  %s, OS: Process %i Segmentation Fault (core dump)\n" , simClock, currentProcess->processID );
          outputManager( outputString, OUTPUT, NULL );

          currentProcess->state = EXIT;
        }

        if( ( errorCode != SUCCESSFUL ) && ( errorCode != SEGMENTATION_FAULT ) )
        {
          systemStop( config, processHeader );
          free( simClock );
          return errorCode;
        }

      }
  }

  accessTimer( LAP_TIMER, simClock );

  sprintf( outputString, "Time:  %s, System stop" , simClock );
  outputManager( outputString, OUTPUT, NULL );

  if( strCompare( config->logTo , "File\0" ) || strCompare( config->logTo, "Both\0" ) )
  {
    outputManager( NULL, PRINT, NULL );
  }

  systemStop( config, processHeader );
  free( simClock );
  free( outputString );

  return SUCCESSFUL;
}

Process *scheduler( Process *processHeader, ConfigData *config, char *simClock )
{
  Process *selectedProcess, *tempProcess;
  char *outputString = ( char * ) malloc( ( sizeof(char) * MAX_STR_LENGTH ) );
  int shortestTime, currentTime, processID;
  static bool initialized = false;

  selectedProcess = NULL;

  switch( config->cpuScheduleCode )
  {
    //////////////////FCFS-N case/////////////////////////////////
    case FCFSN:
      while( ( processHeader->state != READY ) && ( processHeader != NULL ) )
      {
        processHeader = processHeader->next;
      }

      selectedProcess = processHeader;

      sprintf( outputString, "Time:  %s, OS: FCFS-N Strategy selects process %i with time: %i mSec",
                           simClock, selectedProcess->processID, totalProcessTime( selectedProcess->serviceCallHeader, config ) );
      outputManager(  outputString, OUTPUT, NULL );

      accessTimer( LAP_TIMER , simClock );

      break;
    //////////////////SJF-N case/////////////////////////////////
    case SJFN:

      shortestTime = 0;

      while( processHeader != NULL )
      {
        if( processHeader->state == READY )
        {
          currentTime = totalProcessTime( processHeader->currentServiceCall, config );

          if( ( currentTime < shortestTime ) || ( shortestTime == 0 ) )
          {
            selectedProcess = processHeader;
            shortestTime = currentTime;
          }
        }

        processHeader = processHeader->next;
      }

      accessTimer( LAP_TIMER , simClock );

      sprintf( outputString, "Time:  %s, OS: SJF-N Strategy selects process %i with time: %i mSec",
                    simClock, selectedProcess->processID, totalProcessTime( selectedProcess->currentServiceCall, config ) );
      outputManager( outputString, OUTPUT, NULL );

      break;

      /////////////////////////FCFS-P CASE/////////////////////////
    case FCFSP:

      while( ( processHeader != NULL ) && ( processHeader->state != READY ) )
      {
        processHeader = processHeader->next;
      }

      selectedProcess = processHeader;

      if( selectedProcess != NULL )
      {
        accessTimer( LAP_TIMER , simClock );

        sprintf( outputString, "Time:  %s, OS: FCFS-P Strategy selects process %i with time: %i mSec",
                      simClock, selectedProcess->processID, totalProcessTime( selectedProcess->currentServiceCall, config ) );
        outputManager( outputString, OUTPUT, NULL );
      }
      break;

      ////////////////////////SRTF-P CASE/////////////////////////
    case SRTFP:
      shortestTime = 0;

      while( processHeader != NULL )
      {
        if( processHeader->state == READY )
        {
          currentTime = totalProcessTime( processHeader->currentServiceCall, config );

          if( ( currentTime < shortestTime ) || ( shortestTime == 0 ) )
          {
            selectedProcess = processHeader;
            shortestTime = currentTime;
          }
        }

        processHeader = processHeader->next;
      }

      if( selectedProcess != NULL )
      {
        accessTimer( LAP_TIMER , simClock );

        sprintf( outputString, "Time:  %s, OS: SRTF-P Strategy selects process %i with time: %i mSec",
                    simClock, selectedProcess->processID, totalProcessTime( selectedProcess->currentServiceCall, config ) );
                    outputManager( outputString, OUTPUT, NULL );
      }
      break;
///////////////////////////////////////////Round Robin Preemptive Case////////////////////////////////////////////////////
    case RRP:
      if( !initialized )
      {
        tempProcess = processHeader;

        while( tempProcess != NULL )
        {
          roundRobinQueue( ENQUEUE, tempProcess->processID );
          tempProcess = tempProcess->next;
        }

        initialized = true;
      }

      processID = roundRobinQueue( DEQUEUE, EMPTY );

      if( processID == EMPTY )
      {
        free( outputString );
        return NULL;
      }

      while( ( processHeader != NULL ) && ( processHeader->processID != processID ) )
      {
        processHeader = processHeader->next;
      }

      selectedProcess = processHeader;

      accessTimer( LAP_TIMER , simClock );

      sprintf( outputString, "Time:  %s, OS: RR-P Strategy selects process %i with time: %i mSec",
                  simClock, selectedProcess->processID, totalProcessTime( selectedProcess->currentServiceCall, config ) );
                  outputManager( outputString, OUTPUT, NULL );
      break;
      ///////////////////////END CASES///////////////////////////
  }

  free( outputString );
  return selectedProcess;
}

bool stateChecker( Process *processHead, int requestCode )
{
  switch( requestCode )
  {
    case INITIALIZE:

      processHead->state = READY;

      while( processHead->next != NULL )
      {
        processHead = processHead->next;
        processHead->state = READY;
      }

    break;

    case CHECK_ALL_TERMINATED:

      if( processHead->state != EXIT )
      {
        return false;
      }

      while( processHead != NULL )
      {

        if( processHead->state != EXIT )
        {
          return false;
        }

        processHead = processHead->next;
      }

      return true;
    break;

  }

  return true;
}

int runProcess( Process *runningProcess, ConfigData *config, char *simClock, int runType )
{
  switch( runType )
  {
    case PREEMPTIVE:
        return runProcessP( runningProcess, config, simClock, 0 );
      break;

    case NONPREEMPTIVE:
        return runProcessN( runningProcess, config, simClock );
      break;
  }

  return SUCCESSFUL;
}

int runProcessN( Process *runningProcess, ConfigData *config, char *simClock )
{
  ServiceCall *currentService;
  int errorCode = SUCCESSFUL;
  char *outputString = ( char * ) malloc( (sizeof( char ) * MAX_STR_LENGTH ) );

  currentService = runningProcess->serviceCallHeader;

  accessTimer( LAP_TIMER , simClock );

  while( currentService != NULL )
  {
    if( currentService->componentLetter == 'I' || currentService->componentLetter == 'O' )
    {
      errorCode = ioService( currentService, config, runningProcess->processID, simClock );
    }
    if( currentService->componentLetter == 'P' )
    {
      errorCode = runService( ( currentService->cycleTime * config->processorCycleRate ), simClock, runningProcess->processID, config );
    }
    if( currentService->componentLetter == 'M' )
    {
      errorCode = memoryService( currentService, runningProcess->processID, config );
    }

    if( errorCode != SUCCESSFUL )
    {
      return errorCode;
    }

    currentService = currentService->next;
  }

  accessTimer( LAP_TIMER, simClock );

  sprintf( outputString, "Time:  %s, OS: Process %i set in Exit state\n" , simClock, runningProcess->processID );
  outputManager( outputString, OUTPUT, NULL );

  runningProcess->state = EXIT;

  free( outputString );
  return SUCCESSFUL;
}

int ioService( ServiceCall *serviceRequest, ConfigData *config, int processID, char *simClock )
{
  char *ioSwitcher = ( char * ) malloc ( sizeof( char ) * MIN_STR_LENGTH );
  char *outputString = ( char * ) malloc ( sizeof( char ) * MAX_STR_LENGTH );
  pthread_t ioThread;
  int * time = ( int * ) malloc( sizeof(int ) );

  *time = serviceRequest->cycleTime * config->ioCycleRate;

  if( serviceRequest->componentLetter == 'I' )
  {
    stringCopy( ioSwitcher, "input\0" );
  }
  if( serviceRequest->componentLetter == 'O' )
  {
    stringCopy( ioSwitcher, "output\0" );
  }

  accessTimer( LAP_TIMER , simClock );

  sprintf( outputString, "Time:  %s, Process %i, %s %s start" , simClock, processID, serviceRequest->operationString, ioSwitcher );
  outputManager( outputString, OUTPUT, NULL );

  pthread_create( &ioThread, NULL, ioThreadRun, (void *) time );

  pthread_join( ioThread, NULL );

  accessTimer( LAP_TIMER , simClock );

  sprintf( outputString, "Time:  %s, Process %i, %s %s end" , simClock, processID, serviceRequest->operationString, ioSwitcher );
  outputManager( outputString, OUTPUT, NULL );

  free( ioSwitcher );
  free( outputString );

  return SUCCESSFUL;

}

void *ioThreadRun( void *input )
{
  int time = *((int *) input);

  runTimer( time );

  free(input);

  return NULL;
}

int memoryService( ServiceCall *memoryRequest, int processID, ConfigData *config )
{
  static bool initialized = false;
  static AllocatedMemory *mmu;
  int errorCode;

  if( !initialized )
  {
    mmu = ( AllocatedMemory * ) malloc( sizeof( AllocatedMemory ) );
    mmu->callingPID = INITIALIZE;
    mmu->next = NULL;
    initialized = true;
  }

  if( processID == TERMINATE )
  {
    return memoryDump( mmu );
  }

  if( strCompare( memoryRequest->operationString, "allocate\0" ) )
  {
    errorCode = memoryAllocate( mmu, memoryRequest->cycleTime, processID, config->availableMemory );

    if( errorCode == SEGMENTATION_FAULT )
    {
      mmu = segmentationFault( mmu, processID );

      if( mmu == NULL )
      {
        initialized = false;
      }
    }

    return errorCode;
  }

  if( strCompare( memoryRequest->operationString, "access\0" ) )
  {
    errorCode = memoryAccess( mmu, memoryRequest->cycleTime, processID , config->availableMemory );

    if( errorCode == SEGMENTATION_FAULT )
    {
      mmu = segmentationFault( mmu, processID );

      if( mmu == NULL )
      {
        initialized = false;
      }
    }

    return errorCode;
  }

  return FORMAT_ERROR;

}

AllocatedMemory *segmentationFault( AllocatedMemory *mmu, int dumpProcess )
{
  AllocatedMemory *temp;
  AllocatedMemory *returnMMU = mmu;

  while( ( mmu != NULL ) && ( mmu->callingPID == dumpProcess ) )
  {
    temp = mmu->next;
    returnMMU = temp;

    free( mmu );

    mmu = temp;
  }

  while( ( mmu != NULL ) && ( mmu->next != NULL ) )
  {
    temp = mmu->next;

    if( temp->callingPID == dumpProcess )
    {
      mmu->next = temp->next;

      free( temp );
    }
    mmu = mmu->next;
  }

  return returnMMU;
}

int memoryAllocate( AllocatedMemory *mmu, int requestCode, int processID, int totalMemory )
{
  int address, offset, blockID;
  AllocatedMemory *newNode;
  char *outputString = ( char * ) malloc( sizeof( char ) * MAX_STR_LENGTH );
  char *simClock = ( char * ) malloc( sizeof( char ) * MIN_STR_LENGTH );

  address = ( requestCode % 1000000 ) / 1000;
  offset = requestCode % 1000;
  blockID = requestCode / 1000000;

  accessTimer( LAP_TIMER, simClock );

  sprintf( outputString, "Time:  %s, Process %i, Memory management allocation action start ( %i/%i/%i )" , simClock, processID, blockID, address, offset );
  outputManager( outputString, OUTPUT, NULL );

  if( mmu->callingPID == INITIALIZE )
  {
    mmu->address = address;
    mmu->offset = offset;
    mmu->blockID = blockID;
    mmu->callingPID = processID;
    mmu->next = NULL;

    if( ( mmu->address + mmu->offset ) > totalMemory )
    {
      free( outputString );
      free( simClock );
      return SEGMENTATION_FAULT;
    }

    accessTimer( LAP_TIMER, simClock );

    sprintf( outputString, "Time:  %s, Process %i, Memory management allocation action end" , simClock, processID);
    outputManager( outputString, OUTPUT, NULL );

    free( outputString );
    free( simClock );
    return SUCCESSFUL;
  }

  do
  {

    if( ( address >= mmu->address ) && ( address <= ( mmu->address + mmu->offset ) ) )
    {
      free( outputString );
      free( simClock );
      return SEGMENTATION_FAULT;
    }
    if( ( ( address + offset ) >= mmu->address ) && ( ( address + offset ) <= ( mmu->address + mmu->offset ) ) )
    {
      free( outputString );
      free( simClock );
      return SEGMENTATION_FAULT;
    }
    if( mmu->address >= address && mmu->address <= ( address + offset ) )
    {
      free( outputString );
      free( simClock );
      return SEGMENTATION_FAULT;
    }

    if( mmu->next != NULL )
    {
      mmu = mmu->next;
    }

  } while( mmu->next != NULL );

  newNode = ( AllocatedMemory * ) malloc( sizeof( AllocatedMemory ) );

  newNode->address = address;
  newNode->offset = offset;
  newNode->blockID = blockID;
  newNode->callingPID = processID;
  newNode->next = NULL;
  mmu->next = newNode;

  accessTimer( LAP_TIMER, simClock );

  sprintf( outputString, "Time:  %s, Process %i, Memory management allocation action end" , simClock, processID);
  outputManager( outputString, OUTPUT, NULL );

  free( outputString );
  free( simClock );
  return SUCCESSFUL;
}

int memoryAccess( AllocatedMemory *mmu, int requestCode, int processID, int totalMemory )
{
  int address, offset, blockID;
  char *outputString = ( char * ) malloc( sizeof( char ) * MAX_STR_LENGTH );
  char *simClock = ( char * ) malloc( sizeof( char ) * MIN_STR_LENGTH );

  address = ( requestCode % 1000000 ) / 1000;
  offset = requestCode % 1000;
  blockID = requestCode / 1000000;

  accessTimer( LAP_TIMER, simClock );

  sprintf( outputString, "Time:  %s, Process %i, Memory management access action start ( %i/%i/%i )" , simClock, processID, blockID, address, offset );
  outputManager( outputString, OUTPUT, NULL );

  do
  {

    if( mmu->callingPID == processID )
    {

      if( ( address >= mmu->address ) && ( address <= ( mmu->address + mmu->offset) ) )
      {

        if( ( address + offset ) <= ( mmu->address + mmu->offset ) )
        {
          accessTimer( LAP_TIMER, simClock );

          sprintf( outputString, "Time:  %s, Process %i, Memory management access action end" , simClock, processID );
          outputManager( outputString, OUTPUT, NULL );

          free( outputString );
          free( simClock );
          return SUCCESSFUL;

        }

        free( outputString );
        free( simClock );
        return SEGMENTATION_FAULT;

      }

    }

    mmu = mmu->next;

  } while( mmu != NULL );

  free( outputString );
  free( simClock );
  return SEGMENTATION_FAULT;

}

int memoryDump( AllocatedMemory *mmu )
{
  AllocatedMemory *temp;

  while( mmu != NULL )
  {
    temp = mmu->next;
    free( mmu );
    mmu = temp;
  }

  return SUCCESSFUL;
}

int runService( int runTime, char *simClock, int processID, ConfigData *config )
{
  char *outputString = ( char * ) malloc( sizeof( char ) * MAX_STR_LENGTH );

  accessTimer( LAP_TIMER , simClock );

  sprintf( outputString, "Time:  %s: Process %i, Run operation start" , simClock, processID );
  outputManager( outputString, OUTPUT, NULL );

  runTimer( runTime );

  accessTimer( LAP_TIMER , simClock );

  sprintf( outputString, "Time:  %s: Process %i, Run operation end" , simClock, processID );
  outputManager( outputString, OUTPUT, NULL );

  free( outputString );
  return SUCCESSFUL;
}
//////////////////////////////////////////////////PRE-EMPTIVE/////////////////////////////////////////////////////
int runProcessP( Process *runningProcess, ConfigData *config, char *simClock, int quantumTime )
{
  int errorCode = REMAINING;
  char *outputString = ( char * ) malloc( (sizeof( char ) * MAX_STR_LENGTH ) );

  if( strCompare( runningProcess->currentServiceCall->operationString , "start\0" ) )
  {
    runningProcess->currentServiceCall = runningProcess->currentServiceCall->next;

    if( strCompare( runningProcess->currentServiceCall->operationString , "start\0" ) )
    {
      runningProcess->currentServiceCall = runningProcess->currentServiceCall->next;
    }
  }

  if( strCompare( runningProcess->currentServiceCall->operationString , "end\0" ) )
  {
    accessTimer( LAP_TIMER, simClock );

    sprintf( outputString, "Time:  %s, OS: Process %i set in Exit state\n" , simClock, runningProcess->processID );
    outputManager( outputString, OUTPUT, NULL );

    runningProcess->state = EXIT;

    free( outputString );
    return SUCCESSFUL;
  }

  if( runningProcess->currentServiceCall->componentLetter == 'I' || runningProcess->currentServiceCall->componentLetter == 'O' )
  {
    errorCode = ioServiceP( runningProcess, config );

    if( interruptManager( CHECK, NULL, NULL ) == NEW )
    {
      interruptManager( RESOLVE, NULL, NULL );
    }

    free( outputString );
    return errorCode;
  }
  if( runningProcess->currentServiceCall->componentLetter == 'P' )
  {

    accessTimer( LAP_TIMER, simClock );

    sprintf( outputString, "Time:  %s, Process %i, Run operation start", simClock, runningProcess->processID );
    outputManager( outputString, OUTPUT, NULL );

    while( ( quantumTime < config->quantumTime ) && ( errorCode == REMAINING ) )
    {
      errorCode = runServiceP( runningProcess, config );

      quantumTime++;

      if( interruptManager( CHECK, NULL, NULL ) == NEW )
      {
        if( errorCode == SUCCESSFUL )
        {
          accessTimer( LAP_TIMER, simClock );

          sprintf( outputString, "Time:  %s, Process %i, Run operation end", simClock, runningProcess->processID );
          outputManager( outputString, OUTPUT, NULL );

          runningProcess->currentServiceCall = runningProcess->currentServiceCall->next;
        }
        
        accessTimer( LAP_TIMER, simClock );

        sprintf( outputString, "Time:  %s, OS: Process %i set in Ready state", simClock, runningProcess->processID );
        outputManager( outputString, OUTPUT, NULL );

        if( config->cpuScheduleCode == RRP )
        {
          roundRobinQueue( ENQUEUE, runningProcess->processID );
        }
        interruptManager( RESOLVE, NULL, NULL );

        return SUCCESSFUL;
      }
    }

    if( errorCode == SUCCESSFUL )
    {
      accessTimer( LAP_TIMER, simClock );

      sprintf( outputString, "Time:  %s, Process %i, Run operation end", simClock, runningProcess->processID );
      outputManager( outputString, OUTPUT, NULL );

      runningProcess->currentServiceCall = runningProcess->currentServiceCall->next;

      free( outputString );
      return runProcessP( runningProcess, config, simClock, quantumTime );
    }
    else
    {
      accessTimer( LAP_TIMER, simClock );

      sprintf( outputString, "Time:  %s, OS: Process %i quantum time out", simClock, runningProcess->processID );
      outputManager( outputString, OUTPUT, NULL );

      accessTimer( LAP_TIMER, simClock );

      sprintf( outputString, "Time:  %s, OS: Process %i set in Ready state\n", simClock, runningProcess->processID );
      outputManager( outputString, OUTPUT, NULL );

      roundRobinQueue( ENQUEUE, runningProcess->processID );

      free( outputString );
      return SUCCESSFUL;
    }
  }
  if( runningProcess->currentServiceCall->componentLetter == 'M' )
  {
    errorCode = memoryService( runningProcess->currentServiceCall, runningProcess->processID, config );

    if( errorCode == SUCCESSFUL )
    {
      runningProcess->currentServiceCall = runningProcess->currentServiceCall->next;

      if( interruptManager( CHECK, NULL, NULL ) == NEW )
      {
        interruptManager( RESOLVE, NULL, NULL );
      }

      free( outputString );
      return runProcessP( runningProcess, config, simClock, quantumTime );
    }

    free( outputString );
    return errorCode;
  }

  free( outputString );
  return errorCode;
}

int ioServiceP( Process *runningProcess, ConfigData *config )
{
  char *ioSwitcher = ( char * ) malloc ( sizeof( char ) * MIN_STR_LENGTH );
  char *outputString = ( char * ) malloc ( sizeof( char ) * MAX_STR_LENGTH );
  char *simClock = ( char * ) malloc ( sizeof( char ) * MIN_STR_LENGTH );
  pthread_t ioThread;
  IOPThread *argPtr = ( IOPThread * ) malloc (sizeof( IOPThread ) );

  argPtr->time = runningProcess->currentServiceCall->cycleTime * config->ioCycleRate;
  argPtr->blockedProcess = runningProcess;

  if( runningProcess->currentServiceCall->componentLetter == 'I' )
  {
    stringCopy( ioSwitcher, "input\0" );
  }
  if( runningProcess->currentServiceCall->componentLetter == 'O' )
  {
    stringCopy( ioSwitcher, "output\0" );
  }

  accessTimer( LAP_TIMER , simClock );

  sprintf( outputString, "Time:  %s, Process %i, %s %s start" , simClock, runningProcess->processID, runningProcess->currentServiceCall->operationString, ioSwitcher );
  outputManager( outputString, OUTPUT, NULL );

  pthread_create( &ioThread, NULL, ioThreadRunP, (void *) argPtr );

  accessTimer( LAP_TIMER, simClock );

  sprintf( outputString, "Time:  %s, OS: Process %i set in Blocked state\n", simClock, runningProcess->processID );
  outputManager( outputString, OUTPUT, NULL );

  runningProcess->state = BLOCKED;

  free( ioSwitcher );
  free( outputString );
  free( simClock );

  return SUCCESSFUL;
}

void *ioThreadRunP( void *input )
{
  IOPThread *argPtr =  (IOPThread*) input;

  runTimer( argPtr->time );

  argPtr->blockedProcess->interruptFlag = true;

  free(argPtr);

  pthread_exit( NULL );
}

int runServiceP( Process *runningProcess, ConfigData *config )
{
  runTimer( config->processorCycleRate );

  runningProcess->currentServiceCall->cycleTime -= 1;

  if( runningProcess->currentServiceCall->cycleTime == 0 )
  {
    return SUCCESSFUL;
  }

  return REMAINING;
}

int roundRobinQueue( int controlCode, int processID )
{
  static processQueue *queue;
  static bool initialized = false;
  processQueue *temp;
  int returnPID;

  if( !initialized )
  {
    if( controlCode == DEQUEUE )
    {
      return EMPTY;
    }

    queue = ( processQueue * ) malloc( sizeof( processQueue ) );

    queue->processID = processID;
    queue->next = NULL;

    initialized = true;

    return SUCCESSFUL;
  }

  switch( controlCode )
  {
    case DEQUEUE:
      temp = queue;
      returnPID = queue->processID;

      queue = queue->next;

      free( temp );

      if( queue == NULL )
      {
        initialized = false;
      }

      break;

    case ENQUEUE:
      temp = queue;

      while( temp->next != NULL )
      {
        temp = temp->next;
      }

      temp->next = ( processQueue * ) malloc( sizeof( processQueue ) );
      temp->next->processID = processID;
      temp->next->next = NULL;

      returnPID = SUCCESSFUL;
      break;
  }

  return returnPID;

}

int interruptManager( int controlCode, Process *processInitializer, ConfigData *configInitializer )
{
  static bool initialized = false;
  static Process *processHeader;
  static ConfigData *config;
  Process *currentProcess;
  bool started = false;
  char *outputString = ( char * ) malloc( sizeof( char ) * MAX_STR_LENGTH );
  char *simClock = ( char * ) malloc( sizeof( char ) * MIN_STR_LENGTH );
  char *ioSwitcher = ( char * ) malloc( sizeof( char ) * MIN_STR_LENGTH );

  if( !initialized )
  {
    processHeader = processInitializer;
    config = configInitializer;
    initialized = true;

    free( outputString );
    free( simClock );
    free( ioSwitcher );
    return SUCCESSFUL;
  }

  switch( controlCode )
  {
    case CHECK:

      currentProcess = processHeader;

      while( currentProcess != NULL )
      {
        if( currentProcess->interruptFlag )
        {
          accessTimer( LAP_TIMER, simClock );

          sprintf( outputString, "Time:  %s, OS: Interrupt, Process %i" , simClock, currentProcess->processID );
          outputManager( outputString, OUTPUT, NULL );

          free( simClock );
          free( ioSwitcher );
          free( outputString );
          return NEW;
        }

        currentProcess = currentProcess->next;
      }

      free( simClock );
      free( ioSwitcher );
      free( outputString );
      return EMPTY;
      break;

    case RESOLVE:

      currentProcess = processHeader;

      while( currentProcess != NULL && !started )
      {
        if( currentProcess->interruptFlag )
        {
          if( currentProcess->currentServiceCall->componentLetter == 'I' )
          {
            stringCopy( ioSwitcher, "input\0" );
          }
          if( currentProcess->currentServiceCall->componentLetter == 'O' )
          {
            stringCopy( ioSwitcher, "output\0" );
          }

          accessTimer( LAP_TIMER, simClock );

          sprintf( outputString, "Time:  %s, Process %i, %s %s end" , simClock, currentProcess->processID,
                                        currentProcess->currentServiceCall->operationString, ioSwitcher );
          outputManager( outputString, OUTPUT, NULL );

          currentProcess->currentServiceCall = currentProcess->currentServiceCall->next;

          accessTimer( LAP_TIMER, simClock );

          sprintf( outputString, "Time:  %s, OS: Process %i set in Ready state\n", simClock, currentProcess->processID );
          outputManager( outputString, OUTPUT, NULL );

          if( config->cpuScheduleCode == RRP )
          {
            roundRobinQueue( ENQUEUE, currentProcess->processID );
          }

          currentProcess->state = READY;
          currentProcess->interruptFlag = false;
          started = true;

        }

        currentProcess = currentProcess->next;
      }

      while( currentProcess != NULL )
      {
        if( currentProcess->interruptFlag )
        {
          if( currentProcess->currentServiceCall->componentLetter == 'I' )
          {
            stringCopy( ioSwitcher, "input\0" );
          }
          if( currentProcess->currentServiceCall->componentLetter == 'O' )
          {
            stringCopy( ioSwitcher, "output\0" );
          }

          accessTimer( LAP_TIMER, simClock );

          sprintf( outputString, "Time:  %s, OS: Interrupt, Process %i" , simClock, currentProcess->processID );
          outputManager( outputString, OUTPUT, NULL );

          accessTimer( LAP_TIMER, simClock );

          sprintf( outputString, "Time:  %s, Process %i, %s %s end" , simClock, currentProcess->processID,
                                        currentProcess->currentServiceCall->operationString, ioSwitcher );
          outputManager( outputString, OUTPUT, NULL );

          currentProcess->currentServiceCall = currentProcess->currentServiceCall->next;

          sprintf( outputString, "Time:  %s, OS: Process %i set in Ready state\n", simClock, currentProcess->processID );
          outputManager( outputString, OUTPUT, NULL );

          if( config->cpuScheduleCode == RRP )
          {
            roundRobinQueue( ENQUEUE, currentProcess->processID );
          }

          currentProcess->state = READY;
          currentProcess->interruptFlag = false;
        }

        currentProcess = currentProcess->next;
      }

      free( simClock );
      free( ioSwitcher );
      free( outputString );
      return SUCCESSFUL;
      break;
  }

  free( outputString );
  free( simClock );
  free( ioSwitcher );
  return SUCCESSFUL;
}

void printToFile( char *fileName , RunLog *runRecord )
{
  FILE *filePtr = fopen( fileName, "w" );

  do
  {

    fprintf( filePtr, "%s\n", runRecord->logString );

    runRecord = runRecord->next;

  } while( runRecord->next != NULL );

  fclose( filePtr );

}

RunLog *addNewLog( RunLog *currentLog )
{
  RunLog *newNode = ( RunLog * ) malloc( sizeof( RunLog ) );

  newNode->next = NULL;
  currentLog->next = newNode;

  return newNode;
}

void outputManager( char *outputString, int controlCode, ConfigData *configInitialize )
{
  static ConfigData *config;
  static RunLog *captainsLog;
  static RunLog *currentLog;
  static bool initialized = false;

  if( !initialized )
  {
    config = configInitialize;

    captainsLog = ( RunLog * ) malloc( sizeof( RunLog ) );

    stringCopy( captainsLog->logString, outputString );
    currentLog = addNewLog( captainsLog );

    if( strCompare( config->logTo, "Monitor\0" ) || strCompare( config->logTo, "Both\0" ) )
    {
      printf( "%s\n" , outputString );
    }

    initialized = true;
    return;
  }

  switch( controlCode )
  {
    case OUTPUT:

    stringCopy( currentLog->logString, outputString );
    currentLog = addNewLog( currentLog );

    if( strCompare( config->logTo, "Monitor\0" ) || strCompare( config->logTo, "Both\0" ) )
    {
      printf( "%s\n" , outputString );
    }
      break;

    case PRINT:

      printToFile( config->logFilePath, captainsLog );

      break;

    case TERMINATE:

      while( captainsLog->next != NULL )
      {
        currentLog = captainsLog;
        captainsLog = captainsLog->next;

        free( currentLog );
      }

      free( captainsLog );
  }

}

int totalProcessTime( ServiceCall *headNode, ConfigData *config )
{
  int totalTime = 0;

  while( headNode != NULL )
  {
    if( headNode->componentLetter == 'P' )
    {
         totalTime += ( headNode->cycleTime * config->processorCycleRate );
    }
    if( headNode->componentLetter == 'I' || headNode->componentLetter == 'O' )
    {
         totalTime += ( headNode->cycleTime * config->ioCycleRate );
    }

    headNode = headNode->next;
  }

  return totalTime;
}

void systemStop( ConfigData *config, Process *processHeader )
{
  Process *tempProcess;

  while( processHeader != NULL )
  {
    clearServiceCalls( processHeader->serviceCallHeader );

    tempProcess = processHeader->next;
    free( processHeader );
    processHeader = tempProcess;
  }

  outputManager( NULL, TERMINATE, NULL );

  free( config );

  memoryService( NULL, TERMINATE, NULL );
}


#endif
