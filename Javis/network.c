#include "network.h"

const WCHAR* szNetworkModuleName = L"network";

IP_ADAPTER_ADDRESSES ipAdpterAddresses[IP_ADAPTER_ADDREES_ELEMENTS_SIZE];

void SetComputerIP(){

    /* Code based on the example of the windows api documentation on the MS site:
        https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses
    */

    // declare and initialize variables
    DWORD dwRetVal = 0;

    // set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;

    ULONG family = AF_INET;

    // allocate a 15 KB buffer to start with.
    ULONG outBufLen = WORKING_BUFFER_SIZE;

    PIP_ADAPTER_ADDRESSES pipAdpterAddresses = &ipAdpterAddresses[0],
                          pipAdpterAddressCurr = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;

    SecureZeroMemory(pipAdpterAddresses, WORKING_BUFFER_SIZE);

    // get adapters
    dwRetVal = GetAdaptersAddresses(family, flags, NULL, pipAdpterAddresses, &outBufLen);

    
    if (dwRetVal == NO_ERROR) {

        // if successful, output some information from the data we received
        pipAdpterAddressCurr = pipAdpterAddresses;

        BOOL bIsLoopingContinue = TRUE;

        while (pipAdpterAddressCurr && bIsLoopingContinue) {

            //search for DNS suffix
            if (wcscmp(L"SUBDOMAIN.DOMAIN", pipAdpterAddressCurr->DnsSuffix)) {
                pipAdpterAddressCurr = pipAdpterAddressCurr->Next;
                continue;
            }

            pUnicast = pipAdpterAddressCurr->FirstUnicastAddress;

            if (pUnicast != NULL){

                for (int i = 0; pUnicast != NULL; i++){

                    SOCKET_ADDRESS sk = pUnicast->Address;

                    SOCKADDR_STORAGE* ska1 = sk.lpSockaddr;

                    if (ska1->ss_family == AF_INET) {
                        SOCKADDR* ska = sk.lpSockaddr;
                        swprintf(stComputer.szIP4, IP4_STRING_SIZE, L"%d.%d.%d.%d", (unsigned char)ska->sa_data[2], (unsigned char)ska->sa_data[3], (unsigned char)ska->sa_data[4], (unsigned char)ska->sa_data[5]);
                        return;
                    }
                    pUnicast = pUnicast->Next;
                }
            } else {
                LogWarn(szNetworkModuleName, L"\tNo Unicast Addresses\n");
            }
            pipAdpterAddressCurr = pipAdpterAddressCurr->Next;
        }

        LogFatal(szNetworkModuleName, L"It was not possible to get the ip of the machine. IPV4 or by the SUBDOMAIN.DOMAIN domain");

    }else{

        switch (dwRetVal) {

        case ERROR_ADDRESS_NOT_ASSOCIATED:
            LogWinApi(szNetworkModuleName, dwRetVal, L"Error when getting the IP! Error: ERROR_ADDRESS_NOT_ASSOCIATED");
            break;
        case ERROR_BUFFER_OVERFLOW:
            LogWinApi(szNetworkModuleName, dwRetVal, L"Error when getting the IP! Error: ERROR_BUFFER_OVERFLOW");
            break;
        case ERROR_INVALID_PARAMETER:
            LogWinApi(szNetworkModuleName, dwRetVal, L"Error when getting the IP! Error: ERROR_INVALID_PARAMETER");
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            LogWinApi(szNetworkModuleName, dwRetVal, L"Error when getting the IP! Error: ERROR_NOT_ENOUGH_MEMORY");
            break;
        case ERROR_NO_DATA:
            LogWinApi(szNetworkModuleName, dwRetVal, L"Error when getting the IP! Error: ERROR_NO_DATA");
            break;
        default:
            LogWinApi(szNetworkModuleName, dwRetVal, L"Error when getting the IP!");
            break;
        }
    }

    swprintf(stComputer.szIP4, IP4_STRING_SIZE, L"-");
}


BOOL IsConnected() {

    DWORD dwSens;
    if (IsNetworkAlive(&dwSens) == FALSE) {
        return FALSE;
    }else {

        switch (dwSens) {

            case NETWORK_ALIVE_LAN:
                return TRUE;
            case NETWORK_ALIVE_WAN:
                return TRUE;
                break;
            default:
                return TRUE;
                break;
        }
    }

}

VOID SetComputerName1() {

    DWORD dComputerNameSize = MAX_COMPUTERNAME_LENGTH + 1;

    WCHAR szwComputerName[MAX_COMPUTERNAME_LENGTH + 1];

    BOOL getComputerNameReturn = GetComputerNameW(szwComputerName, &dComputerNameSize);

    swprintf(stComputer.szComputerName, MAX_COMPUTERNAME_LENGTH + 1, L"-");

    if (getComputerNameReturn) {
        swprintf(stComputer.szComputerName, MAX_COMPUTERNAME_LENGTH + 1, szwComputerName);
    }else {
        swprintf(stComputer.szComputerName, MAX_COMPUTERNAME_LENGTH + 1, L"-");
        LogWinApi(szNetworkModuleName, GetLastError(), L"Could not get computer name");
    }

    return;
}