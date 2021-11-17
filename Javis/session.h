#ifndef _SESSION_H_
#define _SESSION_H_

#define _WINSOCKAPI_ 
#include <windows.h>

#include<wtsapi32.h>

#include "javis.h"
#include "base.h"
#include "log.h"

#pragma comment(lib, "Wtsapi32.lib")

#define SESSION_LOGON 0x0
#define SESSION_LOGOFF 0X1

#define SESSION_CONNECTION_TYPE_ERROR 0x0
#define SESSION_CONNECTION_TYPE_CONSOLE 0x1
#define SESSION_CONNECTION_TYPE_RDP 0x2

extern computer stComputer;

/*
	Handles session events for a logged in user, these events are: WTS_SESSION_LOGON, WTS_SESSION_UNLOCK e WTS_SESSION_LOGOFF
	Also update the stComputer struct with new data, see:
	If the event type is: WTS_SESSION_LOGON or WTS_SESSION_UNLOCK
		1º Set stComputer.iSession bits: set the session type bits and also set the logOff bit to 0.
		2º Set the username (stComputador.szNomeUsuario)
	If the event type is: WTS_SESSION_LOGOFF:
		-Set the logoff bit to 1
	----
	pPWTSSESSION_NOTIFICATION - session notification struct from Windows to get the user ID.
	eventType - session event type from Windows, e.g.: WTS_SESSION_LOGON, WTS_SESSION_UNLOCK and so on...

 */
extern VOID SessionEvent(PWTSSESSION_NOTIFICATION pPWTSSESSION_NOTIFICATION, unsigned int eventType);

/*
	Set the actual session type in stComputer.iSession. The session type can have 3 values:
		0 - No session type
		1 - Console (normal user connection)
		2 - RDP     (the user is using a remote RDP connection)
*/
extern VOID SetSessionType();

/*
	Set the stComputador.szNomeUsuario variable with the username of the current logged in user.
	The stComputador.szNomeUsuario can be an empty string ("") if:
		1)An error occurs
		2)It's the system account
*/
extern VOID SetUserSession();


#endif // !_SESSION_H_
