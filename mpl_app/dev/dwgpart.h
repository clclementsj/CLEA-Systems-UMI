#define __EXTENSIONS__
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

#define ON 1
#define OFF 0
#define YES 1
#define NO 0
#define ERROR -1
#define NO_ERROR 0
#define LOW 1
#define MED 2
#define HIGH 3

#define LOG_DEBUG 4
#define LOG_INFO 3
#define LOG_WARN 2
#define LOG_ERROR 1
#define LOG_OFF 0

#define IPB_NUM_LEN 41
#define MAX_FILE_NAME_LEN 100
#define MAX_BUFFER_SIZE 5000
#define LOG_BUFFER_LEN 256
#define TIME_BUFFER_LEN 256


FILE *inputFP;
int LogLevel = LOG_ERROR;

int checkArguments(int NumArgs, char *Arguments[], int *FNArg);

int processFile(char *FileName);

int readLine(FILE *fp, char **Buffer);

int getTag(char *OpenTagPtr, char *CloseTagPtr, char **Buffer, char *TagListPtr);

int writeSQLStatement(FILE *FP, char *DwgNumPtr, char *DwgCagePtr, char *PartNumPtr, char *PartCagePtr, char *PartTypePtr, int LL);

int Log(const char *Tag, const char *Message);

int convertToUpperCase(char *Ptr);
