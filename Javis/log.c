#include "log.h"

static const WCHAR* szBufferMask = L"%s\n";

//==== log fuctions implementation
VOID Log(int iLogLevel, const WCHAR* module, const WCHAR* format, ...) {
    
    if (iLogLevel < 100 || iLogLevel > 800)
        return;

    va_list args;
    va_start(args, format);
    LogPrint(iLogLevel, module, format, args);
    va_end(args);
}

VOID LogDebug(const WCHAR* module, const WCHAR* format, ...) {
    va_list args;
    va_start(args, format);
    LogPrint(LOG_DEBUG, module, format, args);
    va_end(args);
}

VOID LogInfo(const  WCHAR* module, const WCHAR* format, ...) {
    va_list args;
    va_start(args, format);
    LogPrint(LOG_INFO, module, format, args);
    va_end(args);
}
VOID LogWarn(const WCHAR* module, const WCHAR* format, ...) {
    va_list args;
    va_start(args, format);
    LogPrint(LOG_WARN, module, format, args);
    va_end(args);
}
VOID LogErro(const WCHAR* module, const WCHAR* format, ...) {
    va_list args;
    va_start(args, format);
    LogPrint(LOG_ERROR, module, format, args);
    va_end(args);
}
VOID LogFatal(const WCHAR* module, const WCHAR* format, ...) {
    va_list args;
    va_start(args, format);
    LogPrint(LOG_FATAL, module, format, args);
    va_end(args);
}


/*
    Handles a error from Windows API. This method:
    1º Checks what is the error message;
    2º Formats this message to the Log standard;
    3º Stores this message into the LOG file
*/
VOID LogWinApi(const WCHAR* module, DWORD errorMessageID, const WCHAR* format, ...) {

    LPWSTR messageBuffer = NULL;
    DWORD size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, 
                                errorMessageID, 
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                (LPWSTR)&messageBuffer,
                                0, 
                                NULL);

    if (messageBuffer) {

           //removes the break line. The Log itself will make one.
           WCHAR* breakLine = wcsstr(messageBuffer, L"\r\n");

           if (breakLine) {
               *breakLine = L'\0';
           }

            WCHAR bufferMsg[BUFFER_LOG_SIZE];
            wmemset(bufferMsg, L'\0', BUFFER_LOG_SIZE);       

            va_list args;
            va_start(args, format);

            wsprintfW(bufferMsg, L"{Details: %s} {Windows: %s}", format, messageBuffer);

            LogPrint(LOG_FATAL, module, bufferMsg, args);

            va_end(args);

            LocalFree(messageBuffer);
    }
}


static VOID LogPrint(int LOG_LEVEL, const WCHAR* module, const WCHAR* format, va_list args) {

#ifndef TESTE 
        if (LOG_LEVEL < iLogLevel)
                return;
#endif // !TESTE

    WCHAR szLogMsg[BUFFER_LOG_SIZE];
    wmemset(szLogMsg, L'\0', BUFFER_LOG_SIZE);
     
    WCHAR szType[10];
    wmemset(szType, L'\0', 10);

    switch (LOG_LEVEL)
    {

        case LOG_DEBUG:
        {
            wcscpy_s(szType, 10, L"DEBUG");
            break;
        }
        case LOG_INFO: 
        {
            wcscpy_s(szType, 10, L"INFO");
            break;
        }
        case LOG_WARN:
        {
            wcscpy_s(szType, 10, L"WARN");
            break;
        }
        case LOG_ERROR:
        {
            wcscpy_s(szType, 10, L"ERROR");
            break;
        }
        case LOG_FATAL: 
        {
            wcscpy_s(szType, 10, L"FATAL");
            break;
        }
        default:
            break;

    }

    TextLogFormat(szLogMsg, szType, module, format, args);

#ifndef TESTE
    FILE* pFileLog = NULL;

    errno_t open = _wfopen_s(&pFileLog, szwLogFilePath, L"a+");

    if (!open) {
        fwprintf(pFileLog, szBufferMask, szLogMsg);
        fclose(pFileLog);
    }
#else
    wprintf(szBufferMask, szLogMsg);
#endif // !TESTE



}

static VOID TextLogFormat(WCHAR *bufferMsg, WCHAR* type, const WCHAR* module, const WCHAR* format, va_list args) {

    //clear the bufferMsg
    wmemset(bufferMsg, L'\0', BUFFER_LOG_SIZE);

    //log header->   [dd/mm/yyy-HH:mm:ss type module]: 
    WCHAR bufferHeader[BUFFER_LOG_HEADER_SIZE];
    wmemset(bufferHeader, L'\0', BUFFER_LOG_HEADER_SIZE);

    //log details that will be inserted after the header
    WCHAR bufferDetails[BUFFER_LOG_SIZE];
    wmemset(bufferDetails, L'\0', BUFFER_LOG_SIZE);

    //getting system time
    SYSTEMTIME st;
    GetLocalTime(&st);


    wsprintfW(bufferHeader, L"[%02hu/%02hu/%hu-%02hu:%02hu:%02hu %s %s]: ",
        st.wDay,
        st.wMonth,
        st.wYear,
        st.wHour,
        st.wMinute,
        st.wSecond,
        type,
        module);

    wvsprintfW(bufferDetails, format, args);
 
    wcscat_s(bufferMsg, BUFFER_LOG_SIZE, bufferHeader);
    wcscat_s(bufferMsg, BUFFER_LOG_SIZE, bufferDetails);

}
