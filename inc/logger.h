#pragma once

#include "helper.h"

// #define _LOG_FILE_
extern FILE* logFile;

#ifdef _LOG_FILE_

#ifdef _DEBUG
#define LogD(...)                                      \
  fopen_s(&logFile, "Sandbox.log", "a");               \
  if (logFile) {                                       \
    fprintf(logFile, "[%10.6f] ", GetCurrentTimeMS()); \
    fprintf(logFile, __VA_ARGS__);                     \
    fprintf(logFile, "\n");                            \
    fclose(logFile);                                   \
    logFile = NULL;                                    \
  }
#else

#define LogD(...)
#endif

#define LogI(...)                                      \
  fopen_s(&logFile, "Sandbox.log", "a");               \
  if (logFile) {                                       \
    fprintf(logFile, "[%10.6f] ", GetCurrentTimeMS()); \
    fprintf(logFile, __VA_ARGS__);                     \
    fprintf(logFile, "\n");                            \
    fclose(logFile);                                   \
    logFile = NULL;                                    \
  }

#define LogE(...)                                      \
  fopen_s(&logFile, "Sandbox.log", "a");               \
  if (logFile) {                                       \
    fprintf(logFile, "[%10.6f] ", GetCurrentTimeMS()); \
    fprintf(logFile, "ERROR: ");                       \
    fprintf(logFile, __VA_ARGS__);                     \
    fprintf(logFile, "\n");                            \
    fclose(logFile);                                   \
    logFile = NULL;                                    \
  }

#else

#define LogD(...)      \
  printf(__VA_ARGS__); \
  printf("\n");
#define LogI(...)      \
  printf(__VA_ARGS__); \
  printf("\n");
#define LogE(...)      \
  printf("ERROR: ");   \
  printf(__VA_ARGS__); \
  printf("\n");

#endif

void InitLogger(void);
void UninitLogger(void);
