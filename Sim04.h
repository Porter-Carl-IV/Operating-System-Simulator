
#ifndef SIM03_H
#define SIM03_H


typedef struct ConfigData
    {
     int version;
     char filePath[ MAX_STR_LENGTH ];
     int cpuScheduleCode;
     int quantumTime;
     int availableMemory;
     int processorCycleRate;
     int ioCycleRate;
     char logTo[ MIN_STR_LENGTH ];
     char logFilePath[ MIN_STR_LENGTH ];
    } ConfigData;

typedef struct ServiceCall
    {
     char componentLetter;
     char operationString[ MIN_STR_LENGTH ];
     int cycleTime;
     struct ServiceCall *next;
   } ServiceCall;

typedef struct Process
   {
    ServiceCall *serviceCallHeader;
    ServiceCall *currentServiceCall;
    bool interruptFlag;
    int state;
    int processID;
    struct Process *next;
  } Process;

typedef struct RunLog
    {
     char logString[ MAX_STR_LENGTH ];
     struct RunLog *next;
   } RunLog;

typedef struct AllocatedMemory
   {
    int address;
    int offset;
    int blockID;
    int callingPID;
    struct AllocatedMemory *next;
  } AllocatedMemory;

typedef struct IOPThread
  {
    Process *blockedProcess;
    int time;
  } IOPThread;

typedef struct processQueue
  {
    int processID;
    struct processQueue *next;
  } processQueue;

int main( int argc, char *argv[] );

/*
int runProcess( ServiceCall *headNode, int processNumber, char *timer, ConfigData *config, char *logArray[], int arrayIndex );

int printRunToFile( ServiceCall *headNode, FILE *filePtr, char *logArray[], int arrayIndex, int processNumber );

int totalProcessTime( ServiceCall *headNode, ConfigData *config );

void createActionString( char *actionString, ServiceCall *spawnNode );

int cycleToMSTime( ServiceCall *metaData, ConfigData *config );

void resizeArray( char *stringArray[], int size );
*/

#endif
