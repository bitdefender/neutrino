#include "Quark.Debugger.h"

#include <Windows.h>
#include <Dbghelp.h>

#include "Neutrino.Test.h"
#include "Quark.Test.Extractor.h"

namespace Quark {

	class Debugger::DbgImpl {
	private:
		std::string processName;
		PROCESS_INFORMATION pi;
		TestExtractor extractor;

		char crashTestName[64];

		DWORD ProcessDebugEvent(DEBUG_EVENT &dbgEvt) {
			DWORD ret = DBG_CONTINUE;

			switch (dbgEvt.dwDebugEventCode) {
				case OUTPUT_DEBUG_STRING_EVENT: {
					OUTPUT_DEBUG_STRING_INFO &DebugString = dbgEvt.u.DebugString;

					WCHAR *msg = new WCHAR[DebugString.nDebugStringLength];
					// Don't care if string is ANSI, and we allocate double...

					ReadProcessMemory(
						pi.hProcess,       // HANDLE to Debuggee
						DebugString.lpDebugStringData, // Target process' valid pointer
						msg,                           // Copy to this address space
						DebugString.nDebugStringLength, NULL);

					printf("%ws\n", msg);

					delete[] msg;
					// Utilize strEventMessage
					break;
				}

				case EXCEPTION_DEBUG_EVENT: {
					Neutrino::Test crashTest;
					if (extractor.Perform(pi.hProcess, crashTest)) {
						printf("Succesfully extracted test %s\n", crashTest.buffer);
					}

					char fileName[64];
					const char digits[] = "0123456789abcdef";

					for (int i = 0; i < sizeof(crashTest.name.digest8); ++i) {
						crashTestName[2 * i + 0] = digits[crashTest.name.digest8[i] >> 0x4];
						crashTestName[2 * i + 1] = digits[crashTest.name.digest8[i] & 0x0F];
					}
					crashTestName[sizeof(crashTest.name.digest8) << 1] = '\0';

					strcpy(fileName, crashTestName);
					strcpy(&fileName[sizeof(crashTest.name.digest8) << 1], ".dmp");

					HANDLE hDump = CreateFile(
						fileName,
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

					strcpy(&fileName[sizeof(crashTest.name.digest8) << 1], ".tst");
					HANDLE hTest = CreateFile(
						fileName,
						GENERIC_WRITE,
						FILE_SHARE_READ,
						nullptr,
						CREATE_ALWAYS,
						0,
						nullptr
					);

					DWORD dwWritten;
					WriteFile(hTest, crashTest.buffer, crashTest.size, &dwWritten, nullptr);
					CloseHandle(hTest);

					ret = DBG_TERMINATE_PROCESS;

					break;
				}
			}

			return ret;
		}
	public :
		DbgImpl(std::string procName) : processName(procName) {	
		}

		bool Perform(std::string &crashName) {
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
				return false;
			}

			DEBUG_EVENT dbgEvent = { 0 };

			do {
				if (!WaitForDebugEvent(&dbgEvent, INFINITE)) {
					return false;
				}

				ContinueDebugEvent(
					dbgEvent.dwProcessId,
					dbgEvent.dwThreadId,
					DBG_CONTINUE
				);
			} while (dbgEvent.dwDebugEventCode != EXCEPTION_DEBUG_EVENT);



			DWORD ret = DBG_CONTINUE;
			while (DBG_CONTINUE == ret) {
				if (!WaitForDebugEvent(&dbgEvent, INFINITE)) {
					return false;
				}

				ret = ProcessDebugEvent(dbgEvent);

				if (DBG_CONTINUE == ret) {
					ContinueDebugEvent(
						dbgEvent.dwProcessId,
						dbgEvent.dwThreadId,
						ret
					);
				}
				else {
					TerminateProcess(pi.hProcess, ret);
				}
			}

			crashName = crashTestName;
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			return true;
		}
	};


	Debugger::Debugger(std::string procName) : pImpl(new Debugger::DbgImpl(procName)) { }

	Debugger::~Debugger() { }

	bool Debugger::Perform(std::string &crashName) {
		return pImpl->Perform(crashName);
	}



};