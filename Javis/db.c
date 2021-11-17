#include "db.h"


static WCHAR* szwDataBaseServer = L"000.000.000.000";
static WCHAR* szwDataBasePort = L"1433";
#ifdef TEST
static const WCHAR* szwDataBaseServer = L"db_teste";
static const WCHAR* szwDataBasePort = L"db_teste_admin";
#else
static WCHAR* szwDataBaseName = L"vnc_info";
static WCHAR* szwDataBaseUser = L"vnc_info_user";
#endif
static WCHAR* szwDataBasePassword = L"password";

//log module variable
static WCHAR* szwDBModuleName = L"db";

/*
	Create a LOG wih a SQL message from ODBC API
*/
static void HandleSQLMessages(int iLogLevel, unsigned int handletype, const SQLHANDLE handle);

/*
	The PrepareStatement. Create the SQL statement and fill it with the data from struct stComputer.
	----
	sqlstatementhandle - o sqlHandler...
	----
	Return: SQL_SUCCESS if successful. The return value is for the ODBC API. For more details see the documentation:
	https://docs.microsoft.com/en-us/sql/odbc/reference/syntax/odbc-api-reference?view=sql-server-ver15
 */
static SQLRETURN SQLPrepareComputer(SQLHANDLE* sqlstatementhandle);

void HandleSQLMessages(int iTipoLog, unsigned int handletype, const SQLHANDLE handle) {

	SQLWCHAR szwSqlstate[1024];
	SQLWCHAR szwMessage[1024];

	if (SQL_SUCCESS == SQLGetDiagRecW(handletype, handle, 1, szwSqlstate, NULL, szwMessage, 1024, NULL)) {
		Log(iTipoLog, szwDBModuleName, L"Message: %s SQLSTATE : %s", szwMessage, szwSqlstate);
	}
}

int SQLCreateComputer() {

	SQLHANDLE sqlenvhandle;
	SQLHANDLE sqlconnectionhandle = NULL;
	SQLHANDLE sqlstatementhandle = NULL;

	SQLRETURN retcode = 0;

	SQLSMALLINT NumParams = 0;

	WCHAR szConnectionData[STR_CONNECTION_BUFFER_SIZE];

	SecureZeroMemory(szConnectionData, sizeof(szConnectionData));
	swprintf(szConnectionData, STR_CONNECTION_BUFFER_SIZE, L"Driver={SQL Server};Server=%s, %s;Database=%s;UID=%s;PWD=%s;", szwDataBaseServer, szwDataBasePort, szwDataBaseName, szwDataBaseUser, szwDataBasePassword);

	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenvhandle)) {
		LogFatal(szwDBModuleName, L"Fatal error: SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenvhandle)");
		return 1;
	}
	else if (SQL_SUCCESS != SQLSetEnvAttr(sqlenvhandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0)) {
		LogFatal(szwDBModuleName, L"Fatal error: SQLSetEnvAttr(sqlenvhandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0)");
		return 1;
	}
	else if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlenvhandle, &sqlconnectionhandle)) {
		LogFatal(szwDBModuleName, L"Fatal error: SQLAllocHandle(SQL_HANDLE_DBC, sqlenvhandle, &sqlconnectionhandle)");
		return 1;
	}

	SQLWCHAR retconstring[1024];

	switch (SQLDriverConnectW(sqlconnectionhandle, NULL, szConnectionData, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT)){
	case SQL_SUCCESS_WITH_INFO:
		HandleSQLMessages(LOG_INFO, SQL_HANDLE_DBC, sqlconnectionhandle);
		break;
	case SQL_INVALID_HANDLE:
	case SQL_ERROR:
		HandleSQLMessages(LOG_ERROR, SQL_HANDLE_DBC, sqlconnectionhandle);
		retcode = -1;
		break;
	default:
		break;
	}


	if (retcode == -1)
		return 1;
	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle))
		return 1;
	
	retcode = SQLPrepareComputer(&sqlstatementhandle);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		HandleSQLMessages(LOG_ERROR, SQL_HANDLE_STMT, sqlstatementhandle);
		return 1;
	}

	retcode = SQLNumParams(sqlstatementhandle, &NumParams);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		HandleSQLMessages(LOG_ERROR, SQL_HANDLE_STMT, sqlstatementhandle);
		return 1;
	}

	retcode = SQLExecute(sqlstatementhandle);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		HandleSQLMessages(LOG_ERROR, SQL_HANDLE_STMT, sqlstatementhandle);
		return 1;
	}


	SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
	SQLDisconnect(sqlconnectionhandle);
	SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
	SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);

	return 0;

}


WCHAR version[10] = JAVIS_VERSION;
SQLLEN cbValue = SQL_NTS;


SQLRETURN SQLPrepareComputer(SQLHANDLE* sqlstatementhandle) {

	SQLRETURN retcode = 0;

	retcode = SQLBindParameter(*sqlstatementhandle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stComputer.iServiceStatus, 0, NULL);
	retcode = SQLBindParameter(*sqlstatementhandle, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stComputer.iComputerStatus, 0, NULL);
	retcode = SQLBindParameter(*sqlstatementhandle, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stComputer.iSession, 0, NULL);

	retcode = SQLBindParameter(*sqlstatementhandle, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, (wcslen(stComputer.szUsername)+1)   , 0, stComputer.szUsername,    0, &cbValue);
	retcode = SQLBindParameter(*sqlstatementhandle, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, (wcslen(stComputer.szComputerName)+1), 0, stComputer.szComputerName, 0, &cbValue);
	retcode = SQLBindParameter(*sqlstatementhandle, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, (wcslen(stComputer.szIP4)+1)            , 0, stComputer.szIP4,             0, &cbValue);
	
	retcode = SQLBindParameter(*sqlstatementhandle, 7, SQL_PARAM_INPUT, SQL_C_BIT, SQL_BIT, 0, 0, &stComputer.isManual, 0, NULL);
	retcode = SQLBindParameter(*sqlstatementhandle, 8, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, (wcslen(version) + 1), 0, version, 0, &cbValue);

	retcode = SQLPrepareW(*sqlstatementhandle, L"INSERT INTO dbo.vnc3 (statusServico, statusComputador, tipoSessao, usuario, computador, ip, manual, dataLogin, versao) VALUES (?, ?, ?, ?, ?, ?, ?, GETDATE(), ?)", SQL_NTS);

	return retcode;
}
