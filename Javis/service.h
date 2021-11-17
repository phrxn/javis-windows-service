#ifndef _SERVICE_H_
#define _SERVICE_H_

#include "javis.h"
#include "base.h"
#include "db.h"
#include "log.h"
#include "session.h"
#include "network.h"

#include <winbase.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <wtsapi32.h>
#include <string.h>

#pragma comment(lib, "Wtsapi32.lib")

extern VOID  WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
extern DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
extern DWORD WINAPI CtrlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

extern VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

/*
	Start the service
	----
	Return: 0 if successful, otherwise, a non-zero value from GetLastError() with the error code will be retorned.
 */
extern DWORD RunService();

/*
	Install the service
	----
	Return: TRUE if successful or FALSE otherwise.
 */
extern BOOL InstallService();

/*
	Uninstall the service
	----
	Return: TRUE if successful or FALSE otherwise.
 */
extern BOOL UninstallService();

extern computer stComputer;

#endif // !_SERVICE_H_

