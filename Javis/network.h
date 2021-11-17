#ifndef _NETWORK_H_
#define _NETWORK_H_


#include <winsock2.h>
#define _WINSOCKAPI_ 
#include <windows.h>
#include <ws2tcpip.h>
#include <sensapi.h>
#include <iphlpapi.h>

#include "base.h"
#include "log.h"

#pragma comment(lib, "Sensapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#define IP_ADAPTER_ADDREES_ELEMENTS_SIZE 40
#define WORKING_BUFFER_SIZE (IP_ADAPTER_ADDREES_ELEMENTS_SIZE * sizeof(IP_ADAPTER_ADDRESSES))
#define IP4_STRING_SIZE     17

extern computer stComputer;

/*
	Set the current computer's IP in the stComputer.szIP4. It uses the DNS suffix to find the IP.
	If any error ocurred, a hyphen(-) will be used instead of the IP and the error can be seen in the log
 */
extern VOID SetComputerIP();

/*
	Check if the computer is connected in a network... web or local...
*/
extern BOOL IsConnected();

/*
	Set computer's name in the stComputer.szComputerName
	If any error ocurred, a hyphen(-) will be used instead of the IP and the error can be seen in the log
*/
extern VOID SetComputerName1();

#endif
