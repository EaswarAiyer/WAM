#pragma once

#ifndef _WINDOWS_LOGGER_H_
#define _WINDOWS_LOGGER_H_

////////////////////////////
//Include Headers
////////////////////////////
#include <atlstr.h>
#include <string>

////////////////////////////
//Macros
////////////////////////////
#define KBSIZE(X)      X*1024      /*Converts X KB into X*1024 byte*/
#define MBSIZE(X)      X*1024*1024 /*Converts X MB into X*1024*1024 byte*/

#define TO_SECONDS(X)  X*60 

////////////////////////////
//Structures and enum
////////////////////////////
enum LOGGER_LEVEL
{
	TRACE_LEVEL = 0,
	DEBUG_LEVEL = 1,
	INFO_LEVEL = 2,
	WARNING_LEVEL = 3,
	ERROR_LEVEL = 4,
	FATAL_LEVEL = 5
};


typedef struct _LOG_FORMATTER_
{
	bool enableTime;/*If true, then system time will be display in log.*/
	bool enableProcessID;/*If true, then process ID will be display in log.*/
	bool enableThreadID;/*If true, then thread ID will be display in log.*/
	bool enableLoggerLevel;/*If true, then logger level will be display in log.*/

	_LOG_FORMATTER_()
	{
		enableTime = true;
		enableProcessID = false;
		enableThreadID = true;
		enableLoggerLevel = true;
	}

}LogFormatter, *pLogFormatter;


typedef struct _LOGFILE_DETAILS_
{
	CStringA fileName;/*Name of the file.*/

	CStringA filePath;/*Directory where the log file writes.*/

	UINT32 openMode;/*File Opening method*/

	INT32 rotationFileCount;/*Number of files to be maintained in directory.*/
	
	UINT64 rotationFileSize;/*Size of the file to be maintained. If exceeds, then the file gets rotated.*/
	
	bool enableRotationWithSize;/*If true, then the file will gets rotated if it reaches the rotationFileSize.*/

	_LOGFILE_DETAILS_()
	{
		fileName = "log.log";

		filePath = "logs\\";

		openMode = OPEN_ALWAYS;

		rotationFileCount = 5;

		rotationFileSize = MBSIZE(5);

		enableRotationWithSize = false;
	}

}LogFileDetails, *pLogFileDetails;


class CWindowsLoggerUtil
{
private:
	CRITICAL_SECTION CS_logFile;

	HANDLE fileHandle;

	CStringA fileName;

public:
	LogFileDetails logFileDetails;

	LogFormatter logFormatter;

	LOGGER_LEVEL loggerLevel;

public:
	CWindowsLoggerUtil(void);

	~CWindowsLoggerUtil(void);

	BOOL StartLogger(void);

	BOOL StopLogger(void);

	VOID WriteBufferWithLoggerLevel(LOGGER_LEVEL currentLoggerLevel, const CHAR* fmt, ...);

	VOID RotateLogger(BOOL bForceRotate = TRUE);

private:
	VOID WritebufferInFile(CStringA message);

};


#endif
