// header files
#include "main.h"

// globals
FILE* logFile;

// functions
void InitLogger(void)
{
	// open file for logging
	if (fopen_s(&logFile, "Sandbox.log", "w") != 0)
	{
		MessageBox(NULL, TEXT("Cannot open Sandbox.log file.."), TEXT("Error"), MB_OK | MB_ICONERROR);
		exit(0);
	}
#ifdef _DEBUG
	fprintf(logFile, "==== Application Started [debug build] ====\n");
#else
	fprintf(logFile, "==== Application Started [release build] ====\n");
#endif
}

void UninitLogger(void)
{
	if (logFile)
	{
		fprintf(logFile, "\n==== Application Terminated ====\n");
		fclose(logFile);
		logFile = NULL;
	}
}

