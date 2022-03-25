#include "session.h"

const WCHAR* szwSessionModuleName = L"session";


static VOID SetSession(BOOL isConnectionTypeBits, unsigned int value);

static unsigned int ConnectionType(const WCHAR* info);


VOID SessionEvent(PWTSSESSION_NOTIFICATION pPWTSSESSION_NOTIFICATION, unsigned int eventType) {

	DWORD idSession = pPWTSSESSION_NOTIFICATION->dwSessionId;

	if (idSession == 0)
		return;

	unsigned int iConnectionType = SESSION_CONNECTION_TYPE_ERROR;

	if (eventType == WTS_SESSION_LOGON || eventType == WTS_SESSION_UNLOCK) {

		LPWSTR szwBufferInfo;
		DWORD  dwBufferInfoSize;

		// get user session type (rdp, console...)
		BOOL bool_wsi1 = WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, idSession, WTSWinStationName, &szwBufferInfo, &dwBufferInfoSize);

		if (bool_wsi1) {

			iConnectionType = ConnectionType(szwBufferInfo);

			// free the memory allocated to the szwBufferInfo pointer, allowing szwBufferInfo to be reused and avoiding memory leaks.
			WTSFreeMemory(szwBufferInfo);
		}


		SetSession(TRUE, iConnectionType);

		// get the username of the logged in user
		BOOL bWTSQueSesInfW = WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, idSession, WTSUserName, &szwBufferInfo, &dwBufferInfoSize);

		if (!bWTSQueSesInfW) {
			LogWinApi(szwSessionModuleName, GetLastError(), L"Error get username.");
			return;
		}

		// set the name of the user who is logged in.
		wcscpy_s(stComputer.szUsername, UNLEN, szwBufferInfo);

		// free the memory allocated to the szwBufferInfo pointer
		WTSFreeMemory(szwBufferInfo);

	} else if (eventType == WTS_SESSION_LOGOFF) {
		SetSession(FALSE, SESSION_LOGOFF);
	}

}



VOID SetSessionType() {

	WTS_SESSION_INFOW* pSessionInfo = NULL;
	DWORD dwCount = 0;

	unsigned int iConnectionType = SESSION_CONNECTION_TYPE_ERROR;

	BOOL bWTSEnumeraSessW = WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount);

	if (bWTSEnumeraSessW) {

		for (DWORD count = 0; count < dwCount; count++) {

			WTS_SESSION_INFOW sessionInfo = pSessionInfo[count];

			if (sessionInfo.State == WTSActive) {

				iConnectionType = ConnectionType(sessionInfo.pWinStationName);

			}

		}

		WTSFreeMemory(pSessionInfo);

	}

	SetSession(TRUE, iConnectionType);

}



VOID SetUserSession() {

	WTS_SESSION_INFOW* pSessionInfo = NULL;
	DWORD dwCount = 0;
	DWORD dwSessionId = 0;

	wcscpy_s(stComputer.szUsername, UNLEN, L"");

	BOOL bWTSEnumeraSessW = WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount);

	if (!bWTSEnumeraSessW) {
		LogWinApi(szwSessionModuleName, GetLastError(), L"Error when enumerating sessions.");
		return;
	}

	for (DWORD count = 0; count < dwCount; count++) {

		WTS_SESSION_INFOW sessionInfo = pSessionInfo[count];

		if (sessionInfo.State == WTSActive) {

			dwSessionId = sessionInfo.SessionId;
		
		}

	}

	// free memory
	WTSFreeMemory(pSessionInfo);

	LPWSTR lpwUserName = NULL;
	DWORD dwUserNameSize;

	// search for username by idSession
	BOOL bWTSQueSesInfW = WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, dwSessionId, WTSUserName, &lpwUserName, &dwUserNameSize);

	if (!bWTSQueSesInfW) {
		LogWinApi(szwSessionModuleName, GetLastError(), L"Error get username.");
		return;
	}

	// copy the username to the computer structure
	wcscpy_s(stComputer.szUsername, UNLEN, lpwUserName);

	// free memory
	WTSFreeMemory(lpwUserName);

	return;

}


/*
	Set the stComputer.iSession value, this method can set the connection type bits and the LogOff bit
	----
	isConnectionTypeBits - TRUE if the bits that will be changed are of the connection type or FALSE if the bit that will be changed is the LogOff bit
	value - bits values
 */
static VOID SetSession(BOOL isConnectionTypeBits, unsigned int value) {

	if (isConnectionTypeBits) {

		//set the connection type in the structure
		stComputer.iSession = (value << 1);
	}
	else {

		//set 0 to LogOff bit
		stComputer.iSession &= ~1;

		stComputer.iSession |= value;
	}
}


/*
	Returns an unsigned int with the Connection type
	----
	info - a string containing the winStationName
	----
	Return: the connection type
 */
static unsigned int ConnectionType(const WCHAR* info) {
	if (wcsstr(info, L"Console")) {
		return SESSION_CONNECTION_TYPE_CONSOLE;
	}
	else if (wcsstr(info, L"RDP-Tcp")) {
		return SESSION_CONNECTION_TYPE_RDP;
	}
	return SESSION_CONNECTION_TYPE_ERROR;
}