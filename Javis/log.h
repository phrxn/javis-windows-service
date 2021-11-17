#ifndef _LOG_H_
#define _LOG_H_

#define _WINSOCKAPI_ 
#include <windows.h>

#include <wchar.h>
#include <stdio.h>

#include "javis.h"

// size the buffers that will store text that will be saved in the log file.
#define BUFFER_LOG_SIZE 4096
#define BUFFER_LOG_HEADER_SIZE 200

enum LOG_LEVEL {
    LOG_ALL = 1,    
    LOG_DEBUG = 100,
    LOG_INFO = 200, 
    LOG_WARN = 400, 
    LOG_ERROR = 600, //default
    LOG_FATAL = 800, 
    LOG_OFF = 1000
};


//store the log level value that must be used.
static int iLogLevel = 600;

extern WCHAR szwLogFilePath[MAX_PATH];

//methods used to create a log
VOID Log      (int iLogLevel, const WCHAR* module, const WCHAR* format, ...);
VOID LogDebug (const WCHAR* module, const WCHAR* format, ...);
VOID LogInfo  (const WCHAR* module, const WCHAR* format, ...);
VOID LogWarn  (const WCHAR* module, const WCHAR* format, ...);
VOID LogErro  (const WCHAR* module, const WCHAR* format, ...);
VOID LogFatal (const WCHAR* module, const WCHAR* format, ...);
VOID LogWinApi(const WCHAR* module, DWORD errorMessageID, const WCHAR* format, ...);


/*
    Formats the text que will be saved on the log file. This log is formed by: date, hour, log type, modulo name and any possible additional details.
    ----
    bufferMsg - pointer to a buffer that will store the formatted text.
    type      - log type, see LOG_LEVEL for values. This value cannot be: LOG_ALL e LOG_OFF
    module    - module name (where the log was called)
    format    - additional information (this is a format string like a format string in the printf method)
    args      - args for the format string
 */
static VOID TextLogFormat(WCHAR* bufferMsg, WCHAR* type, const WCHAR* module, const WCHAR* format, va_list args);


/*
    Saves the log in the log file
    ----
    LOG_LEVEL - the log type
    module    - module name (where the log was called)
    format    - Additional information (this is a format string like a format string in the printf method)
    args      - args for the format string
 */
static VOID LogPrint(int LOG_LEVEL, const WCHAR* module, const WCHAR* format, va_list args);

#endif // ! _LOG_H_