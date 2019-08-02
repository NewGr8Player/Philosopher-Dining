/*
 * MtVerify.h
 *
 * Error handling for applications in
 * "Multitheading Applications in Win32"
 *
 * The function PrintError() is marked as __inline so that it can be
 * included from one or more C or C++ files without multiple definition
 * errors. For the examples in this book, this works fine.
 * To use the PrintError() in an application, it should be taken out,
 * placed in its own source file, and the "__inline" declaration removed
 * so the function will be globally available.
 */

#pragma comment( lib, "USER32" )

#include <crtdbg.h>
#include <locale.h>
#include <Strsafe.h>
#include <Windows.h>
#include <Windowsx.h>
#define MTASSERT(a) _ASSERTE(a)


#define MTVERIFY(a) if (!(a)) PrintError(L#a, __FILE__, __LINE__, GetLastError())

__inline void PrintError(LPTSTR linedesc, LPSTR pFileName, int lineno, DWORD errnum)
{
	_tsetlocale(LC_ALL, _T("")) ;

#ifdef UNICODE
	PWSTR pWideCharStr;
	int   nLenOfWideCharStr;

	nLenOfWideCharStr = MultiByteToWideChar(CP_ACP, 0, pFileName, -1, NULL, 0);

	pWideCharStr = (PWSTR)HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t));

	if (pWideCharStr == NULL)
		_tprintf_s(_T("MultiByteToWideChar Failure!\n")) ;

	MultiByteToWideChar(CP_ACP, 0, pFileName, -1, pWideCharStr, nLenOfWideCharStr);
	LPTSTR pCharStr = pWideCharStr ;
#else
	LPSTR pCharStr = pFileName ;
#endif

	LPSTR lpBuffer;
	TCHAR errbuf[256];

#ifdef _WINDOWS
	TCHAR modulename[MAX_PATH];
#else // _WINDOWS
	DWORD numread;
#endif // _WINDOWS

	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errnum,
		LANG_NEUTRAL,
		(LPTSTR)&lpBuffer,
		0,
		NULL );

	StringCchPrintf(errbuf, 256, 
		TEXT("\nThe following call failed at line %d in file: \n\n    %s:\n\n    %s\n\nReason: %s\n"),
		lineno, pCharStr, linedesc, lpBuffer) ;
		 
#ifndef _WINDOWS
// 	WriteFile(GetStdHandle(STD_ERROR_HANDLE), errbuf, _tcslen(errbuf), &numread, FALSE );
	_tprintf_s(_T("%s\n"), errbuf) ;
	Sleep(3000);
#else
	GetModuleFileName(NULL, modulename, MAX_PATH);
	MessageBox(NULL, errbuf, modulename, MB_ICONWARNING|MB_OK|MB_TASKMODAL|MB_SETFOREGROUND);
#endif
	exit(EXIT_FAILURE);

#ifdef UNICODE
	HeapFree(GetProcessHeap(), 0, pWideCharStr) ;
#endif
}


#define chHANDLE_DLGMSG(hWnd, uMsg, fn) \
	case (uMsg): return (SetDlgMsgResult(hWnd, uMsg, \
	HANDLE_##uMsg((hWnd), (wParam), (lParam), (fn))))

#define CREATE_DEBUG_CONSOLE \
	FILE * fpDebugOut = NULL ; \
	FILE * fpDebugIn  = NULL ; \
	FILE * fpDebugErr = NULL ; \
	if (!AllocConsole()) \
	MessageBox(NULL, _T("控制台生成失败！"), NULL, 0) ; \
	SetConsoleTitle(_T("Debug Window")) ; \
	_tfreopen_s(&fpDebugOut, _T("CONOUT$"), _T("w"), stdout) ; \
	_tfreopen_s(&fpDebugIn,  _T("CONIN$"),  _T("r"),  stdin) ; \
	_tfreopen_s(&fpDebugErr, _T("CONOUT$"), _T("w"), stderr) ; \
	_tsetlocale(LC_ALL, _T("")) ; \
	_tsystem(_T("color 0B")) ; \
	SMALL_RECT srctWindow = { 0, 0, 30, 30 } ; \
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &srctWindow) ;

#define RELEASE_DEBUG_CONSOLE \
	fclose(fpDebugOut) ; \
	fclose(fpDebugIn)  ; \
	fclose(fpDebugErr) ; \
	FreeConsole() ;
