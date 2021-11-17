#ifndef _JAVIS_H_
#define _JAVIS_H_

//#define TESTE

#define _WINSOCKAPI_ 
#include <windows.h>

#include "base.h"

#define JAVIS_VERSION L"2.0.0W"


//folder where the service executable is, e.g.: c:/foo/
extern WCHAR szwRootFolder[MAX_PATH];

//path log file, e.g.: c:/foo/log.log
extern WCHAR szwLogFilePath[MAX_PATH];

extern const WCHAR* szwServiceName;
extern const WCHAR* szwLogFileName;

extern computer stComputer;

/*
	Setting szwRootFolder and szwLogFilePath variables.
	This method must be called before registering the service.
*/
extern VOID Start();

#endif
