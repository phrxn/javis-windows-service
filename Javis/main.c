#include "service.h"
#define _WINSOCKAPI_ 
#include <windows.h>

int _tmain(int argc, WCHAR* argv[]) {

    if (argc > 1 && lstrcmpiW(argv[1], L"install") == 0) {
        fwprintf(stdout, L"installing...\n");

        if (!InstallService())
            fwprintf(stdout, L"Service has not been installed.\n");

    }else if (argc > 1 && lstrcmpiW(argv[1], L"uninstall") == 0) {
        fwprintf(stdout, L"uninstalled...\n");

        if (!UninstallService())
            fwprintf(stdout, L"Service has not been uninstalled.\n");
    }else {
        fwprintf(stdout, L"Starting...\n");

        if (RunService())
            fwprintf(stdout, L"Error starting the service.\n");
    }

    return 0;
}