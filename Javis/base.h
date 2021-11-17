#ifndef _BASE_H_
#define _BASE_H_

#include <wchar.h>
#include <Lmcons.h>

#define IP4_STRING_SIZE     17

/*struct for store computer info*/
typedef struct computer {

	// the service is running? 1-Yes, 0-No
	int iServiceStatus;   

	// the computer is power on? 1-Yes, 0-No
	int	iComputerStatus; 

	/*
		use a unsigned integer for store two infos
		                                                [L] [S]
		32-bit: high bit- [0000000000000000000000000000  00  0] -low bit

		[L] -> is logoff (1-Yes, 0-No) <- Is the user logged off?
	    [S] -> session type  (0-nothing, 1-console, 2-RDP) <- What is session type?
	*/
	unsigned int iSession;

	//logged in username, e.g.: john
	WCHAR szUsername[UNLEN];

	//computer name, e.g.: PC001
	WCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];

	// current IP4 used by the computer
	WCHAR szIP4[IP4_STRING_SIZE];

	//Was the record created by a user? (isManual = 1), if the record was created by the service (isManual = 0)
	char isManual;

} computer;

#endif // !_BASE_H_
