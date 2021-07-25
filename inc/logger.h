#pragma once

#define _LOG_FILE_
extern FILE* logFile;

#ifdef _LOG_FILE_

#ifdef _DEBUG
#define LogD(...)   fprintf(logFile, __VA_ARGS__);fprintf(logFile, "\n");fflush(logFile);
#else
#define LogD(...)   
#endif

#define LogI(...)   fprintf(logFile, __VA_ARGS__);fprintf(logFile, "\n");fflush(logFile);
#define LogE(...)   fprintf(logFile, "ERROR: ");fprintf(logFile, __VA_ARGS__);fprintf(logFile, "\n");fflush(logFile);

#else

#define LogD(...)   printf(__VA_ARGS__);printf("\n");
#define LogI(...)   printf(__VA_ARGS__);printf("\n");
#define LogE(...)   printf("ERROR: ");printf(__VA_ARGS__);printf("\n");

#endif

void InitLogger(void);
void UninitLogger(void);
