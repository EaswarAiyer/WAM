#include "WindowsLogger.h"
#include <Shlobj.h>

/////////////////////////////////
// Method Declarations
/////////////////////////////////


/////////////////////////////////
// Method Definitions
/////////////////////////////////
DWORD __CallocA(LPSTR *dest, size_t destLength)
{
	destLength = destLength + sizeof(CHAR); // including the null terminating character for safer side.
	*dest = (LPSTR)calloc(1, destLength * sizeof(CHAR));
	if (*dest == NULL)
	{
		return ERROR_INVALID_DATA;
	}
	return ERROR_SUCCESS;
}

DWORD __AllocateAndDoStringCopyA(LPSTR *dest, LPSTR src)
{
	size_t destLength = strlen(src) + sizeof(CHAR);

	__CallocA(dest, destLength);

	if (*dest == NULL)
	{
		return ERROR_INVALID_DATA;
	}

	strcpy_s(*dest, destLength, src);
	return ERROR_SUCCESS;
}

/*****************************************************************************
*
*	Function Name	: NumberOfCharsToFirstOccurance()
*
*	Args			: 1. String(IN)
*					  2. Substring(IN)
*
*	Returns			: Number of characters to first occurance of substring.
*
*	Task			: It calculates number of characters to first occurance of substring.
*
*****************************************************************************/
INT32 __NumberOfCharsToFirstOccurance(CHAR *str, CHAR  *substr)
{
	CHAR *a = str;
	CHAR *b = substr;

	if (str == NULL || substr == NULL)
	{
		return -1;
	}

	while (*a != '\0')
	{
		if (*b == '\0'){
			return a - str - (b - substr);
		}
		else if (*a == *b) b++;
		else  b = substr;
		a++;
	}
	if (*a == '\0' && *b == '\0') return a - str - (b - substr);
	return 0;
}

/*****************************************************************************
*
*	Function Name	: GetStringBeforeSubString()
*
*	Args			: 1.String(IN)
*					  2.Substring(IN)
*
*	Returns			: returns a string before the substring.
*
*	Task			: It finds first occurance of the substring and returns the string before the substring.
*
*****************************************************************************/
CHAR *__GetStringBeforeSubString(CHAR *s, CHAR *d)
{
	INT32
		len;
	CHAR
		*ptr;

	if (s == NULL || d == NULL)
	{
		return NULL;
	}

	len = __NumberOfCharsToFirstOccurance(s, d);
	ptr = (CHAR *)calloc(1, len + 1);
	if (ptr == NULL)
	{
		return NULL;
	}
	strncpy(ptr, s, len);
	ptr[len] = '\0';
	return ptr;
}

/*****************************************************************************************
* Function Name  : GetLastFileName
* Purpose        : To get Last File/Folder Name
* Arg            : Source File Path
* Return         : Last File/Folder Name
****************************************************************************************/
LPSTR __GetLastFileName(LPSTR sSrcPath)
{
	//DEBUGMSG("@@@ Inside GetLastFileName Function Called with %s @@@\n", sSrcPath);
	LPSTR
		head = NULL,
		tail = NULL,
		sFileName = NULL;

	bool
		flag = true;
	sFileName = (LPSTR)calloc(1, ((strlen(sSrcPath) + 2) * sizeof(LPSTR)));

	size_t
		length = strlen("\\");

	head = sSrcPath;

	while (flag)
	{
		if ((tail = strstr(head, "\\")) == 0)
		{
			strcpy(sFileName, head);
			flag = false;
		}
		else
		{
			strncat(sFileName, head, tail - head);
			head = tail + length;
		}
	}
	return sFileName;
}

BOOL __DirectoryExists(LPCSTR szPath)
{
	DWORD dwAttrib = GetFileAttributesA(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void __createDirectoryRecursively(std::string path)
{
	unsigned int pos = 0;
	do
	{
		pos = path.find_first_of("\\/", pos + 1);
		CreateDirectoryA(path.substr(0, pos).c_str(), NULL);
	} while (pos != std::string::npos);
}

int __FindNumberOfRepeatedFileNames(CStringA logsDir, CStringA originalName)
{
	//DEBUGMSG("@@@ Inside FindNoOfRepeatedFileNames @@@\n");

	CStringA
		fileNameOnly = "",
		szFormattedPath = "";
	DWORD
		error = ERROR_SUCCESS;
	WIN32_FIND_DATAA
		FileData;
	BOOL
		fFinished = FALSE;
	HANDLE
		hSearch = NULL;
	int
		NoOfFiles = 0;
	try{
		int findDot = originalName.ReverseFind('.');
		//USERMSG("FindNoOfRepeatedFileNames : File Extension Dot found at : %d\n", findDot);
		if (findDot == -1)
		{
			fileNameOnly = originalName;
		}
		else
		{
			fileNameOnly = originalName.Mid(0, findDot);
		}
		//DEBUGMSG("FindNoOfRepeatedFileNames : File Name without Extension : %s\n", fileNameOnly.GetBuffer());

		if (!logsDir.IsEmpty())
		{
			szFormattedPath = logsDir;
			szFormattedPath += "\\";
		}
		szFormattedPath += fileNameOnly;
		szFormattedPath += "*.*";
		//USERMSG("FindNoOfRepeatedFileNames : Successfully allocated szFormattedPath and the value is %s .\n", szFormattedPath.GetBuffer());

		hSearch = FindFirstFileA(szFormattedPath.GetBuffer(), &FileData);

		//USERMSG("FindNoOfRepeatedFileNames : Successfully finded out first file  %d  .\n", hSearch);

		if (hSearch == INVALID_HANDLE_VALUE)
		{
			error = GetLastError();
			//ERRMSG("FindNumberOfRepeatedFileNames : Unable to find the files/folder with error code : %d .\n", error);
			NoOfFiles = -1;
			goto Cleanup;
		}
		while (!fFinished)
		{
			CStringA tempFiles = "";
			tempFiles = logsDir;
			tempFiles += "\\";
			tempFiles += FileData.cFileName;
			//USERMSG("FindNoOfRepeatedFileNames : File Name is %s .\n", tempFiles.GetBuffer());
			NoOfFiles++;
			if (!FindNextFileA(hSearch, &FileData))
			{
				error = GetLastError();
				if (error == ERROR_NO_MORE_FILES)
				{
					fFinished = TRUE;
				}
				else
				{
					//ERRMSG("FindNumberOfRepeatedFileNames : Couldn't find next file : %d\n", error);
					NoOfFiles = -1;
					goto Cleanup;
				}
			}
		}
	}
	catch (...)
	{
		printf("FindNoOfRepeatedFileNames : Exception caught.\n");
	}
Cleanup:
	//DEBUGMSG("@@@ End of FindNoOfRepeatedFileNames Method with return value of No of file count : %d\t  @@@\n", NoOfFiles);
	return NoOfFiles;
}

BOOL __GetFileSize64__(CStringA lpszPath, PLARGE_INTEGER lpFileSize)
{
	BOOL rc = FALSE;
	if (lpszPath && lpFileSize)
	{
		lpFileSize->QuadPart = 0;
		HANDLE hFile = NULL;

		hFile = CreateFileA(lpszPath.GetBuffer(),
			READ_CONTROL,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			rc = GetFileSizeEx(hFile, lpFileSize);
			CloseHandle(hFile);
		}
	}
	return rc;
}

CStringA __AllocateFileNumber(CStringA logsDir, CStringA fileName, int fileNumber)
{
	CStringA
		returnFileName = "",
		fileNameOnly = "",
		extensionOnly = "";
	try
	{
		int findDot = fileName.ReverseFind('.');
		//USERMSG("FindNoOfRepeatedFileNames : File Extension Dot found at : %d\n", findDot);
		if (findDot == -1)
		{
			fileNameOnly = fileName;
		}
		else
		{
			fileNameOnly = fileName.Mid(0, findDot);
			extensionOnly = fileName.Mid(findDot + 1);
		}

		if (!logsDir.IsEmpty())
		{
			returnFileName.Format("%s\\%s%d.%s", logsDir.GetBuffer(), fileNameOnly.GetBuffer(), fileNumber, extensionOnly.GetBuffer());
		}
		else
		{
			returnFileName.Format("%s%d.%s", fileNameOnly.GetBuffer(), fileNumber, extensionOnly.GetBuffer());
		}
		//DEBUGMSG("AllocateFileNumber : New File Name is : %s\n", returnFileName.GetBuffer());
	}
	catch (...)
	{
		printf("AllocateFileNumber : Exception caught.\n");
	}
	return returnFileName;
}

DWORD __RotateSpecificLogs(CStringA logsDir, CStringA logFileNames, int loggerMaxCount, __int64 loggerMaxSize, BOOL forceRotate)
{
	printf("@@@ Inside RotateSpecificLogs @@@\n");

	DWORD
		returnCode = ERROR_SUCCESS,
		errorCode = ERROR_SUCCESS;
	HANDLE
		hFile;
	int
		NoOfFileCnt = 0,
		nTokenPos = 0;
	CStringA
		loggerName = "",
		requiredFileName = "",
		tempFileName1 = "",
		tempFileName2 = "";

	try
	{
		if (logsDir.IsEmpty())
		{
			printf("RotateSpecificLogs : Given Logs Directory Value is empty.\n");
		}
		if (logFileNames.IsEmpty())
		{
			printf("RotateSpecificLogs : Given Log FileNames Value is empty.\n");
			goto Cleanup;
		}

		loggerName = logFileNames.Tokenize(";", nTokenPos);

		while (!loggerName.IsEmpty())
		{
			printf("RotateSpecificLogs : Log File Name : %s\n", loggerName.GetBuffer());

			LARGE_INTEGER li;
			li.QuadPart = 0;

			if (!logsDir.IsEmpty())
			{
				requiredFileName = "";
				requiredFileName = logsDir;
				requiredFileName += "\\";
			}
			requiredFileName += loggerName;
			printf("RotateSpecificLogs : Required FileName : %s\n", requiredFileName.GetBuffer());

			if (!forceRotate)
			{
				hFile = CreateFileA(requiredFileName.GetBuffer(),
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

				returnCode = GetLastError();

				if (hFile == INVALID_HANDLE_VALUE || returnCode == ERROR_FILE_NOT_FOUND)
				{
					printf("RotateSpecificLogs : hFile is NULL or not AVAILABLE (Errorcode : %d) for the file name [ %s ]. \n", returnCode, requiredFileName.GetBuffer());
					CloseHandle(hFile);
					goto NextToken;
				}

				BOOL rc = __GetFileSize64__(requiredFileName, &li);
				if (rc == 0)
				{
					printf("RotateSpecificLogs : The %s File size detection is failed and the error code is %d \n", requiredFileName, GetLastError());
					li.QuadPart = 0;
					if (DeleteFileA(requiredFileName))//safer side we are deleting the file
					{
						printf("RotateSpecificLogs : The file %s was successfully deleted.\n", requiredFileName);
					}
					else
					{
						printf("RotateSpecificLogs : Unable to delete the file %s and the error code is : %d\n", requiredFileName, GetLastError());
					}
				}
				else
				{
					printf("RotateSpecificLogs : Sucessfully got the %s File size = %lld \n", requiredFileName, li.QuadPart);
				}

				CloseHandle(hFile);
			}

			if ((li.QuadPart > loggerMaxSize) || (forceRotate == TRUE))
			{
				NoOfFileCnt = __FindNumberOfRepeatedFileNames(logsDir, loggerName);
				printf("RotateSpecificLogs : The return Value from FindNumberOfRepeatedFileNames is : %d \n", NoOfFileCnt);

				do
				{
					tempFileName1 = "";
					tempFileName2 = "";

					tempFileName1 = __AllocateFileNumber(logsDir, loggerName, NoOfFileCnt - 1);
					printf("RotateSpecificLogs : The tempFileName1 : %s .\n", tempFileName1.GetBuffer());

					if (NoOfFileCnt >= loggerMaxCount)
					{
						if (DeleteFileA(tempFileName1.GetBuffer()))
						{
							printf("RotateSpecificLogs : The file %s was successfully deleted...\n", tempFileName1);
						}
						else
						{
							printf("RotateSpecificLogs : Unable to delete the file %s and the error code is : %d\n", tempFileName1, GetLastError());
						}
						NoOfFileCnt = loggerMaxCount - 1;
						tempFileName1 = "";
						tempFileName1 = __AllocateFileNumber(logsDir, loggerName, NoOfFileCnt - 1);
					}

					tempFileName2 = __AllocateFileNumber(logsDir, loggerName, NoOfFileCnt);
					printf("RotateSpecificLogs : The tempFileName2 : %s .\n", tempFileName2.GetBuffer());

					if (NoOfFileCnt == 1 && forceRotate == FALSE)
					{
						if (CopyFileA(requiredFileName, tempFileName2, FALSE) == FALSE)
						{
							printf("RotateSpecificLogs : CopyFile failed to move from %s to %s and the error code is %d \n", requiredFileName, tempFileName2, GetLastError());
						}
						else
						{
							printf("RotateSpecificLogs : File successfully copied from %s to  %s . \n", requiredFileName, tempFileName2);
							if (DeleteFileA(requiredFileName.GetBuffer())) // Delete the old file
							{
								printf("RotateSpecificLogs : The file %s was successfully deleted...\n", requiredFileName);
							}
							else
							{
								printf("RotateSpecificLogs : Unable to delete the file %s and the error code is  : %d\n", requiredFileName, GetLastError());
							}
						}
					}
					else if (NoOfFileCnt == 1 && forceRotate == TRUE)
					{
						if (CopyFileA(requiredFileName.GetBuffer(), tempFileName2.GetBuffer(), FALSE) == FALSE)
						{
							printf("RotateSpecificLogs : CopyFile failed to move from %s to %s and the error code is %d \n", requiredFileName.GetBuffer(), tempFileName2.GetBuffer(), GetLastError());
						}
						else
						{
							printf("RotateSpecificLogs : File successfully copied from %s to %s so we are going to truncate the content of the first one \n", requiredFileName.GetBuffer(), tempFileName2.GetBuffer());

							HANDLE hFileTruncate = CreateFileA(requiredFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, TRUNCATE_EXISTING, 0, NULL);
							if (hFileTruncate == INVALID_HANDLE_VALUE)
							{
								/* Don't Change this printf to Logger as this will lead to Dead Lock */
								printf("RotateSpecificLogs : Truncate Existing backup File failed file Name [ %s ] errorcode - %d\n", requiredFileName.GetBuffer(), GetLastError());
							}
							else
							{
								CloseHandle(hFileTruncate);
							}

						}
					}
					else
					{
						if (CopyFileA(tempFileName1.GetBuffer(), tempFileName2.GetBuffer(), FALSE) == FALSE)
						{
							printf("RotateSpecificLogs : CopyFile failed to move from %s to %s and the error code is %d \n", tempFileName1.GetBuffer(), tempFileName2.GetBuffer(), GetLastError());
						}
						else
						{
							printf("RotateSpecificLogs : File copied from %s to %s . \n", tempFileName1.GetBuffer(), tempFileName2.GetBuffer());
							if (DeleteFileA(tempFileName1.GetBuffer()))
							{
								printf("RotateSpecificLogs : The file %s was successfully deleted...\n", tempFileName1.GetBuffer());
							}
							else
							{
								printf("RotateSpecificLogs : Unable to delete the file %s and the error code is : %d\n", tempFileName1.GetBuffer(), GetLastError());
							}
						}
					}

					NoOfFileCnt--;
				} while (NoOfFileCnt >= 1);

			}//End of if-statement

		NextToken:
			loggerName = logFileNames.Tokenize(";", nTokenPos);//Get Next Token
		}

	}
	catch (...)
	{
		printf("RotateSpecificLogs : Exception caught.\n");
	}

Cleanup:
	printf("@@@ End of RotateSpecificLogs @@@\n");
	return errorCode;
}


/////////////////////////////////
// Class Declarations
/////////////////////////////////
class LoggerUtilClass
{
public:
	CStringA TimeAsString();

	CStringA LoggerLevelAsString(LOGGER_LEVEL loggerLevel);

	CStringA CurrentThreadIdAsString();

	CStringA CurrentProcessIdAsString();

	CStringA GetMessageWithFormat(LogFormatter LogFormatterObj, LOGGER_LEVEL loggerLevel, CStringA message);

};


/////////////////////////////////
// Class Definitions
/////////////////////////////////
CStringA LoggerUtilClass::TimeAsString()
{
	SYSTEMTIME lt;
	CStringA   timeStr;

	try{
		GetLocalTime(&lt);

		timeStr.Format("[ %04d-%02d-%02d %02d:%02d:%02d:%03d ]", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);

		//printf("The local time is: %04d-%02d-%02d %02d:%02d:%02d:%03d\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
	}
	catch (...)
	{
		printf("TimeAsString : Exception caught.\n");
	}
	return timeStr;
}

CStringA LoggerUtilClass::LoggerLevelAsString(LOGGER_LEVEL loggerLevel)
{
	CStringA
		loggerLevelStr = "",
		tempStr = "";

	switch (loggerLevel)
	{
	case TRACE_LEVEL:
		tempStr = "TRACE";
		break;

	case DEBUG_LEVEL:
		tempStr = "DEBUG";
		break;

	case INFO_LEVEL:
		tempStr = "INFO";
		break;

	case WARNING_LEVEL:
		tempStr = "WARN";
		break;

	case ERROR_LEVEL:
		tempStr = "ERROR";
		break;

	case FATAL_LEVEL:
		tempStr = "FATAL";
		break;
	}

	loggerLevelStr.Format("[ %s ]", tempStr.GetBuffer());

	//printf("LoggerLevelAsString : LoggerLevel As String : %s\n", tempStr.GetBuffer());

	return loggerLevelStr;
}

CStringA LoggerUtilClass::CurrentThreadIdAsString()
{
	DWORD threadId = 0;
	CStringA threadIdStr = "";

	threadId = GetCurrentThreadId();

	//printf("CurrentThreadIdAsString : Current Thread ID : %lu\n", threadId);

	threadIdStr.Format("[ %lu ]", threadId);

	return threadIdStr;
}

CStringA LoggerUtilClass::CurrentProcessIdAsString()
{
	DWORD processId = 0;
	CStringA processIdStr = "";

	processId = GetCurrentProcessId();

	//printf("CurrentProcessIdAsString : Current Process ID : %lu\n", processId);

	processIdStr.Format("[ %lu ]", processId);

	return processIdStr;
}

CStringA LoggerUtilClass::GetMessageWithFormat(LogFormatter LogFormatterObj, LOGGER_LEVEL loggerLevel, CStringA message)
{
	CStringA messageStr = "";

	if (LogFormatterObj.enableTime)
	{
		messageStr += TimeAsString();
		messageStr += " ";
	}

	if (LogFormatterObj.enableProcessID)
	{
		messageStr += CurrentProcessIdAsString();
		messageStr += " ";
	}

	if (LogFormatterObj.enableThreadID)
	{
		messageStr += CurrentThreadIdAsString();
		messageStr += " ";
	}

	if (LogFormatterObj.enableLoggerLevel)
	{
		messageStr += LoggerLevelAsString(loggerLevel);
		messageStr += " ";
	}

	messageStr += message;

	return messageStr;
}



CWindowsLoggerUtil::CWindowsLoggerUtil(void)
{
	loggerLevel = LOGGER_LEVEL::INFO_LEVEL;
	InitializeCriticalSection(&CS_logFile);
	fileHandle = NULL;
}


CWindowsLoggerUtil::~CWindowsLoggerUtil(void)
{
	StopLogger();
	DeleteCriticalSection(&CS_logFile);
}


BOOL CWindowsLoggerUtil::StartLogger(void)
{
	printf("@@@ Inside StartLogger @@@\n");

	try
	{
		if (logFileDetails.filePath.IsEmpty())
		{
			fileName.Format("%s", logFileDetails.fileName);
		}
		else
		{
			fileName.Format("%s\\%s", logFileDetails.filePath, logFileDetails.fileName);

			__createDirectoryRecursively(logFileDetails.filePath.GetBuffer());

			if (__DirectoryExists(logFileDetails.filePath.GetBuffer()) == FALSE)
			{
				printf("StartLogger : Unable to create the directory.\n");
				return FALSE;
			}

		}

		printf("FileName : %s\n", fileName.GetBuffer());

		EnterCriticalSection(&CS_logFile);

		fileHandle = CreateFileA(
			fileName.GetBuffer(),               // file to create
			FILE_APPEND_DATA,                   // open for writing
			FILE_SHARE_READ | FILE_SHARE_WRITE, // do not share
			NULL,                               // default security
			logFileDetails.openMode,            // overwrite existing
			FILE_ATTRIBUTE_NORMAL,              // file attributes
			NULL);

		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			printf("StartLogger : File Opening failed.\n");
			return FALSE;
		}

		//Rotate the Logs
		__RotateSpecificLogs(logFileDetails.filePath, logFileDetails.fileName, logFileDetails.rotationFileCount, logFileDetails.rotationFileSize, FALSE);

		LeaveCriticalSection(&CS_logFile);

	}
	catch (...)
	{
		printf("StartLogger : Exception caught.\n");
		return FALSE;
	}
	printf("@@@ End of StartLogger @@@\n");
	return TRUE;
}


BOOL CWindowsLoggerUtil::StopLogger(void)
{
	printf("@@@ Inside StopLogger @@@\n");
	try
	{
		if (fileHandle != NULL)
		{
			CloseHandle(fileHandle);
			fileHandle = NULL;
		}
	}
	catch (...)
	{
		printf("StopLogger : Exception caught.\n");
		return FALSE;
	}

	printf("@@@ End of StopLogger @@@\n");
	return TRUE;
}


VOID CWindowsLoggerUtil::RotateLogger(BOOL bForceRotate)
{
	EnterCriticalSection(&CS_logFile);
	__RotateSpecificLogs(logFileDetails.filePath, logFileDetails.fileName, logFileDetails.rotationFileCount, logFileDetails.rotationFileSize, bForceRotate);
	LeaveCriticalSection(&CS_logFile);
}


VOID CWindowsLoggerUtil::WriteBufferWithLoggerLevel(LOGGER_LEVEL currentLoggerLevel, const CHAR* formatMsg, ...)
{
	//printf("@@@ Inside WriteBufferInFile @@@\n");

	try
	{

		EnterCriticalSection(&CS_logFile);
		int len = 0;
		CHAR * buffer = NULL;
		CStringA message = "";
		va_list args;
		LoggerUtilClass loggerUtil;

		va_start(args, formatMsg);

		len = _vscprintf(formatMsg, args) + 1;
		buffer = (CHAR*)malloc(len * sizeof(CHAR));
		if (buffer == NULL)
		{
			printf("WriteBufferWithLoggerLevel : Error while allocating buffer memory.\n");
			return;
		}
		vsprintf(buffer, formatMsg, args);

		message = loggerUtil.GetMessageWithFormat(logFormatter, currentLoggerLevel, buffer);

		if (currentLoggerLevel >= loggerLevel)
			WritebufferInFile(message);

		if (logFileDetails.enableRotationWithSize)
		{
			DWORD fileSize = GetFileSize(fileHandle, NULL);

			//printf("@@@@@ FileSize         : %lu\n", fileSize);
			//printf("@@@@@ rotationFileSize : %lu\n", logFileDetails.rotationFileSize);

			if (fileSize > logFileDetails.rotationFileSize)
			{
				printf("WriteBufferWithLoggerLevel : Going to rotate the file due to file reach.\n");
				WritebufferInFile(loggerUtil.GetMessageWithFormat(logFormatter, LOGGER_LEVEL::INFO_LEVEL, "Going to rotate the file, since size reached.\n"));

				//Rotate the Logs
				__RotateSpecificLogs(logFileDetails.filePath, logFileDetails.fileName, logFileDetails.rotationFileCount, logFileDetails.rotationFileSize, TRUE);

				WritebufferInFile(loggerUtil.GetMessageWithFormat(logFormatter, LOGGER_LEVEL::INFO_LEVEL, "File rotated successfully.\n"));
			}

		}

	}
	catch (...)
	{
		printf("WriteBufferWithLoggerLevel : Exception caught.\n");
	}

	LeaveCriticalSection(&CS_logFile);
	//printf("@@@ End of WriteBufferInFile @@@\n");
}


VOID CWindowsLoggerUtil::WritebufferInFile(CStringA message)
{
	//printf("@@@ Inside WritebufferInFile @@@\n");
	DWORD	NoofBytesWritten = 0;
	try
	{
		WriteFile(fileHandle, (LPCVOID)message.GetBuffer(), message.GetLength(), &NoofBytesWritten, NULL);
	}
	catch (...)
	{
		printf("WritebufferInFile : Exception caught.\n");
	}

	//printf("@@@ End of WritebufferInFile @@@\n");
}



