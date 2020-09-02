
#ifndef UTILITIES_H
#define UTILITIES_H

enum { MIN_STR_LENGTH = 60 };
enum { MAX_STR_LENGTH = 100 };
enum { COMPONENT_INDEX = 0 };
enum { OPEN_PAREN_INDEX = 1 };
enum { CYCLE_TIME = 2 };
enum { OPERATION_STRING = 3 };
enum { COMPONENT_LETTER = 4 };
enum { SUCCESSFUL = 5 };
enum { FORMAT_ERROR = 6 };
enum { FILE_ERROR = 7 };
enum { SEGMENTATION_FAULT = 8 };
enum { FCFSN = 9 };
enum { SJFN = 10 };
enum { FCFSP = 22 };
enum { SRTFP = 23 };
enum { RRP = 28 };
enum { INITIALIZE = -1 };
enum { CHECK_ALL_TERMINATED = 12 };
enum { TERMINATE = -2 };
enum { READY = 13 };
enum { NEW = 14 };
enum { RUNNING = 21 };
enum { EXIT = 15 };
enum { BLOCKED = 26 };
enum { PRINT = 16 };
enum { OUTPUT = 17 };
enum { EMPTY = 18 };
enum { CHECK = 24 };
enum { RESOLVE = 25 };
enum { NONPREEMPTIVE = 19 };
enum { PREEMPTIVE = 20 };
enum { DEQUEUE = 26 };
enum { ENQUEUE = 27 };
enum { REMAINING = 28 };

bool charIn( char input, char *charPool );

bool strCompare( char *firstString, char *secondString);

char toLowercase( char inputChar );

int strToInt( char *str );

int charToInt( char inputChar );

int stringLength( char *inputStr );

int powerOfTen( int power );

char lastChar( char *str );

void stringCopy( char *copyString, char *sourceString );

void strConcat( char *firstString, char *secondString );

void strInsert( char *returnString, char *mainString, char *insertString );

int intLength( int input );

void intToString( char *returnString, int toConvert );

#endif
