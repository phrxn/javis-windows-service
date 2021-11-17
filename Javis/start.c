#include "javis.h"


/* root directory where the service executable is, e.g.: C/pasta/ <- both slashs are backslashs */
WCHAR szwRootFolder[MAX_PATH];

// Path to the log file. This path is at the same level as the service executable. ex: C:/pasta/log.txt. <- both slashs are backslashs */
WCHAR szwLogFilePath[MAX_PATH];

const WCHAR* szwServiceName = L"Javis";
const WCHAR* szwLogFileName = L"log.txt";

computer stComputer = { 0 };

/*
	Set the szwRootFolder variable
*/
static VOID GetCaminhoServico() {

	// fill in the full path
	GetModuleFileNameW(0, szwRootFolder, MAX_PATH);

	// get the last backslash
	WCHAR* ultimaBarra = wcsrchr(szwRootFolder, L'\\');

	// closes the program if the last backslash is not found. (how could this happen)?
	if (ultimaBarra) {
		*++ultimaBarra = L'\0';
	}else {
		wprintf(L"Could not find the last bar!");
		exit(1);
	}

}

/*
	concat the szwDiretorioRaiz and szwNomeArquivoLog string into szwCaminhoArquivoLog
 */
static VOID GetCaminhoCompletoArquivoLog() {

	wcscat_s(szwLogFilePath, MAX_PATH, szwRootFolder);
	wcscat_s(szwLogFilePath, MAX_PATH, szwLogFileName);

}


VOID Start() {
	GetCaminhoServico();
	GetCaminhoCompletoArquivoLog();
}