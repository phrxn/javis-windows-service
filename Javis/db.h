#ifndef _DB_H_
#define _DB_H_

#include <winsock2.h>
#define _WINSOCKAPI_ 
#include <windows.h>
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <Odbcss.h>
#include "log.h"
#include "base.h"
#include "javis.h"

#define STR_CONNECTION_BUFFER_SIZE 1024

extern computer stComputer;

/*
	Create a struct computer register in database
	----
	Return: 0 if successful, otherwise, 1 if a error ocurred
 */
extern int SQLCreateComputer();

#endif // !_DB_H_


