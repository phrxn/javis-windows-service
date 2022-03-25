#include "service.h"
#include "log.h"

static WCHAR* szServiceModuleName = L"service";

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;

HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
HANDLE                g_LoopingStopEvent = INVALID_HANDLE_VALUE;

/*
    Report changes in service status to SCM
    See: https://docs.microsoft.com/en-us/windows/win32/services/writing-a-servicemain-function
    ----
    dwCurrentState -
    dwWin32ExitCode -
    dwWaitHint -
 */
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {

    static DWORD dwCheckPoint = 1;

    g_ServiceStatus.dwCurrentState = dwCurrentState;
    g_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
    g_ServiceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        g_ServiceStatus.dwControlsAccepted = 0;
    else
        g_ServiceStatus.dwControlsAccepted = (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PRESHUTDOWN | SERVICE_ACCEPT_POWEREVENT);


    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
        g_ServiceStatus.dwCheckPoint = 0;
    else
        g_ServiceStatus.dwCheckPoint = dwCheckPoint++;

    //report the status of the service to the SCM.
    if (!SetServiceStatus(g_StatusHandle, &g_ServiceStatus)) {
        LogErro(szServiceModuleName, L"Error reporting status to SCM method error (ReportSvcStatus)");
    }
}


/*
    Service main method
    See: https://docs.microsoft.com/en-us/windows/win32/services/service-servicemain-function
 */
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {

    Start();

    //clear g_ServiceStatus memory block to avoid errors...
    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwControlsAccepted = 0; //
    g_ServiceStatus.dwWin32ExitCode = NO_ERROR;
    g_ServiceStatus.dwCheckPoint = 0;
    g_ServiceStatus.dwWaitHint = 0;


    //registering the Control Handler in the SCM.
    g_StatusHandle = RegisterServiceCtrlHandlerExW(szwServiceName, CtrlHandlerEx, NULL);

    //if g_StatusHandler value is NULL, the RegisterServiceCtrlHandlerEx failed
    if (g_StatusHandle == NULL) {
        LogErro(szServiceModuleName, L"ServiceMain error registering service with RegisterServiceCtrlHandlerExW");
        return;
    }

    //notifying the SCM of the start of the service...
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 0);


    //====== Running the necessary tasks to start the service ======//


    //creates a stop event to notify the ServiceWorkerThread when the shutting down event come by the Control Handler.
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    //if the event is not created, something has failed... so it cannot proceed.
    if (g_ServiceStopEvent == NULL) {
        
        // reports to the SCM that the service has stopped...
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    //starting the ServiceWorkerThread.
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    //waiting until the Service Worker Thread terminates, indicating that the service needs to terminate.
    WaitForSingleObject(hThread, INFINITE);

    //clear the handler
    CloseHandle(g_ServiceStopEvent);

    //notifying the SCM that the service has stopped.
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);

}


/*
    Handler that will handle service controls
    See: https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nc-winsvc-lphandler_function_ex
 */
DWORD WINAPI CtrlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {

    switch (dwControl) {

    //the service does not support Interrogate, but the documentation asks us to return NO_ERROR.
    case SERVICE_CONTROL_INTERROGATE:
        return NO_ERROR;

    case SERVICE_CONTROL_STOP:
    {

        ReportSvcStatus(SERVICE_STOP_PENDING, 0, 1500);
        stComputer.iServiceStatus = 0;
        SQLCreateComputer();
        SetEvent(g_ServiceStopEvent);
        return NO_ERROR;
    }

    case SERVICE_CONTROL_PRESHUTDOWN:
    case SERVICE_CONTROL_SHUTDOWN:
    {

        ReportSvcStatus(SERVICE_STOP_PENDING, 0, 1500);
        stComputer.iServiceStatus = 0;
        stComputer.iComputerStatus = 0;
        stComputer.iSession = 0;
        SQLCreateComputer();
        SetEvent(g_ServiceStopEvent);

        return NO_ERROR;
    }

    case SERVICE_CONTROL_SESSIONCHANGE:
    {

        PWTSSESSION_NOTIFICATION pPWTSSESSION_NOTIFICATION = (PWTSSESSION_NOTIFICATION)lpEventData;

        switch (dwEventType) {

            case WTS_SESSION_LOGON:
            case WTS_SESSION_UNLOCK:
            case WTS_SESSION_LOGOFF:
            {
                SessionEvent(pPWTSSESSION_NOTIFICATION, dwEventType);
                SQLCreateComputer();
                break;
            }

        }

        return NO_ERROR;
    }
    case SERVICE_CONTROL_POWEREVENT:
    {
        switch (dwEventType) {

        case PBT_APMRESUMESUSPEND:
        {
            stComputer.iServiceStatus = 1;
            stComputer.iComputerStatus = 1;
            SQLCreateComputer();
            break;
        }
        case PBT_APMSUSPEND:
        {
            stComputer.iServiceStatus = 0;
            stComputer.iComputerStatus = 0;
            stComputer.iSession = 0;
            SQLCreateComputer();
            break;
        }

        }
        return NO_ERROR;
    }

    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}


/*
    Main thread of the service that is running until the g_ServiceStopEvent event is signaled.
    When signaled the Thread stops and ends...
*/
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam) {

    LogDebug(szServiceModuleName, L"Starting ServiceWorkerThread.");

    BOOL bConnected = 0;
    short sCount = 0;

    //tries for 10 seconds to check the network connection...
    while (!bConnected && sCount < 50 && !(WaitForSingleObject(g_ServiceStopEvent, 0) == WAIT_OBJECT_0)) {
        bConnected = IsConnected();
        sCount++;
        Sleep(200);
    }

    //if the computer is connected, the stComputer structure is filled in and sent to the database
    if (bConnected) {

        stComputer.iServiceStatus = 1;
        stComputer.iComputerStatus = 1;
        SetSessionType();
        SetUserSession();
        SetComputerName1();
        SetComputerIP();
        stComputer.isManual = 0;

        SQLCreateComputer();

    }

    //wait until the service is stopped by some event from the system.
    while (WaitForSingleObject(g_ServiceStopEvent, INFINITE));

    LogDebug(szServiceModuleName, L"Terminating Thread Service (main)");

    return 0;
}




DWORD RunService()
{
    SERVICE_TABLE_ENTRYW serviceTable[] =
    {
        {szwServiceName, ServiceMain},
        { 0, 0 }
    };

    if (StartServiceCtrlDispatcherW(serviceTable)) {
        return FALSE;
    }
    else {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
            wprintf_s(L"RunService Erro StartServiceCtrlDispatcher : ERROR_FAILED_SERVICE_CONTROLLER_CONNECT.\n");
        }
        else if (dwError == ERROR_INVALID_DATA) {
            wprintf_s(L"RunService Erro StartServiceCtrlDispatcher: ERROR_INVALID_DATA.\n");
        }
        else if (dwError == ERROR_SERVICE_ALREADY_RUNNING) {
            wprintf_s(L"RunService Erro StartServiceCtrlDispatcher: ERROR_SERVICE_ALREADY_RUNNING.\n");
        }
        else {
            wprintf_s(L"RunService Erro StartServiceCtrlDispatcher: Unknown!\n");
        }
        return FALSE;
    }
}



BOOL InstallService()
{
    BOOL bReturn = FALSE;
    wprintf_s(L"InstallService: starting the installation of the service.\n");

    SC_HANDLE serviceControlManager = OpenSCManagerW(0, 0, SC_MANAGER_CREATE_SERVICE);

    if (!serviceControlManager) {
        LogWinApi(szServiceModuleName, GetLastError(), L"Error while opening SCM.");
        return bReturn;
    }

    WCHAR szwPath[_MAX_PATH + 1];
   
    if (GetModuleFileNameW(0, szwPath, sizeof(szwPath) / sizeof(WCHAR)) == 0) {
        LogWinApi(szServiceModuleName, GetLastError(), L"Error while retrieving the full path where the service is.");
        return bReturn;
    } 
    
    SC_HANDLE service = CreateServiceW(serviceControlManager,
                szwServiceName, szwServiceName,
                SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, szwPath,
                0, 0, 0, 0, 0);

    if (service) {

        wprintf_s(L"InstallService: service installed successfully.\n");

        bReturn = TRUE;

        CloseServiceHandle(service);

    }else {
        LogWinApi(szServiceModuleName, GetLastError(), L"Error during service installation.");
    }
        
    CloseServiceHandle(serviceControlManager);
    

    return bReturn;
}



BOOL UninstallService()
{
    BOOL bReturn = FALSE;

    SC_HANDLE serviceControlManager = OpenSCManagerW(0, 0, SC_MANAGER_CONNECT);

    if (serviceControlManager)
    {
        SC_HANDLE service = OpenServiceW(serviceControlManager,
            szwServiceName, SERVICE_QUERY_STATUS | DELETE);
        if (service)
        {
            SERVICE_STATUS serviceStatus;
            if (QueryServiceStatus(service, &serviceStatus))
            {
                if (serviceStatus.dwCurrentState == SERVICE_STOPPED) {
                 
                    BOOL bDelSerReturn = DeleteService(service);

                    if (bDelSerReturn) {
                        wprintf_s(L"UninstallService: service uninstalled successfully.\n");
                        bReturn = TRUE;
                    }
                }
            }

            CloseServiceHandle(service);
        }

        CloseServiceHandle(serviceControlManager);
    }
    return bReturn;
}