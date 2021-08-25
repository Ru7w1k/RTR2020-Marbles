#pragma once

#include "helper.h"

#define _LOG_FILE_
extern FILE* logFile;

#ifdef _LOG_FILE_

#ifdef _DEBUG
#define LogD(...)   fprintf(logFile, "[%10.6f] ", GetCurrentTimeMS());fprintf(logFile, __VA_ARGS__);fprintf(logFile, "\n");fflush(logFile);
#else
#define LogD(...)   
#endif

#define LogI(...)   fprintf(logFile, "[%10.6f] ", GetCurrentTimeMS());fprintf(logFile, __VA_ARGS__);fprintf(logFile, "\n");fflush(logFile);
#define LogE(...)   fprintf(logFile, "[%10.6f] ", GetCurrentTimeMS());fprintf(logFile, "ERROR: ");fprintf(logFile, __VA_ARGS__);fprintf(logFile, "\n");fflush(logFile);

#else

#define LogD(...)   printf(__VA_ARGS__);printf("\n");
#define LogI(...)   printf(__VA_ARGS__);printf("\n");
#define LogE(...)   printf("ERROR: ");printf(__VA_ARGS__);printf("\n");

#endif

void InitLogger(void);
void UninitLogger(void);
