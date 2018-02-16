#include <Windows.h>
#include <Dbghelp.h>
#include <cstdio>
#include "Quark.Debugger.h"

DWORD testVirtualAddress;

PROCESS_INFORMATION pi;
bool firstDbgBreak = true;

DWORD ProcessDebugEvent(DEBUG_EVENT &dbgEvt) {
	DWORD ret = DBG_CONTINUE;

	switch (dbgEvt.dwDebugEventCode) {
		case OUTPUT_DEBUG_STRING_EVENT:
		{
			OUTPUT_DEBUG_STRING_INFO &DebugString = dbgEvt.u.DebugString;

			WCHAR *msg = new WCHAR[DebugString.nDebugStringLength];
			// Don't care if string is ANSI, and we allocate double...

			ReadProcessMemory(
				pi.hProcess,       // HANDLE to Debuggee
				DebugString.lpDebugStringData, // Target process' valid pointer
				msg,                           // Copy to this address space
				DebugString.nDebugStringLength, NULL);

			printf("%ws\n", msg);

			delete [] msg;
			// Utilize strEventMessage
			break;
		}

		case EXCEPTION_DEBUG_EVENT: 
		{
			if ((0x80000003 == dbgEvt.u.Exception.ExceptionRecord.ExceptionCode) && (firstDbgBreak)) {
				firstDbgBreak = false;
				break;
			}

			HANDLE hDump = CreateFile(
				"quark.dmp",
				GENERIC_WRITE,
				FILE_SHARE_READ,
				nullptr,
				CREATE_ALWAYS,
				0,
				nullptr
			);

			if (INVALID_HANDLE_VALUE == hDump) {
				break;
			}

			if (FALSE == MiniDumpWriteDump(
				pi.hProcess,
				pi.dwProcessId,
				hDump,
				MiniDumpNormal,
				(PMINIDUMP_EXCEPTION_INFORMATION)&dbgEvt.u.Exception.ExceptionRecord,
				nullptr,
				nullptr
			)) {
				printf("MiniDumpWriteDump() error %d\n", GetLastError());
			}

			CloseHandle(hDump);

			ret = DBG_TERMINATE_PROCESS;

			break;
		}
	}

	return ret;
}

int main() {
	HMODULE hMod = LoadLibrary("neutrino.exe");
	DWORD ct = (DWORD)GetProcAddress(hMod, "currentTest");
	testVirtualAddress = ct - (DWORD)hMod;
	FreeLibrary(hMod);


	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (FALSE == CreateProcess(
		"neutrino.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		DEBUG_ONLY_THIS_PROCESS,
		NULL,
		NULL,
		&si,
		&pi
	)) {
		printf("CreateProcess() error %d\n", GetLastError());
		return 0;
	}

	DEBUG_EVENT dbgEvent = { 0 };

	DWORD ret = DBG_CONTINUE;

	while (DBG_CONTINUE == ret) {
		if (!WaitForDebugEvent(&dbgEvent, INFINITE)) {
			return 0;
		}

		ret = ProcessDebugEvent(dbgEvent);

		ContinueDebugEvent(
			dbgEvent.dwProcessId,
			dbgEvent.dwThreadId,
			ret
		);
	}

	return 0;
}