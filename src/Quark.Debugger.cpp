#include <Windows.h>
#include <Dbghelp.h>
#include <Psapi.h> 
#include <cstdio>

#include "Quark.Debugger.h"
#include "Neutrino.Test.h"

#include <experimental/filesystem> // C++-standard filesystem header file in VS15, VS17.
namespace fs = std::experimental::filesystem; // experimental for VS15, VS17.

PROCESS_INFORMATION pi;
//bool firstDbgBreak = true;

class TestExtractor {
private:
	DWORD testVirtualAddress;

public:
	TestExtractor() {
		HMODULE hMod = LoadLibrary("neutrino.exe");
		DWORD ct = (DWORD)GetProcAddress(hMod, "currentTest");
		testVirtualAddress = ct - (DWORD)hMod;
		FreeLibrary(hMod);
	}

	bool Perform(HANDLE process, Neutrino::Test &test) {
		HMODULE hMod;
		DWORD cbNeeded;

		if (FALSE == EnumProcessModulesEx(process, &hMod, sizeof(hMod), &cbNeeded, LIST_MODULES_ALL)) {
			return false;
		}

		DWORD testAddr, dwRead;

		if (FALSE == ReadProcessMemory(process, (LPCVOID)((DWORD)hMod + testVirtualAddress), &testAddr, sizeof(testAddr), &dwRead)) {
			return false;
		}

		if (FALSE == ReadProcessMemory(process, (LPCVOID)testAddr, &test, sizeof(test), &dwRead)) {
			return false;
		}

		unsigned char *buff = new unsigned char[test.size + 1];

		if (FALSE == ReadProcessMemory(process, (LPCVOID)test.buffer, buff, test.size + 1, &dwRead)) {
			return false;
		}

		test.buffer = buff;
		return true;
	}

} extractor;

char crashTestName[64];

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
			/*if ((0x80000003 == dbgEvt.u.Exception.ExceptionRecord.ExceptionCode) && (firstDbgBreak)) {
				firstDbgBreak = false;
				break;
			}*/

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

			strcpy(fileName, crashTestName);
			strcpy(&fileName[40], ".dmp");

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

			strcpy(&fileName[40], ".tst");
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

int main() {
	fs::path initialCorpusDir("./start_corpus");
	fs::path corpusDir("./corpus");
	fs::path outputDir("./output");
	fs::path exceptionsDir("./exceptions");
	fs::path dumpsDir("./dumps");

	fs::remove_all(corpusDir);
	fs::create_directory(corpusDir);

	fs::copy(initialCorpusDir, corpusDir, fs::copy_options::recursive);

	while (true) {

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

		do {
			if (!WaitForDebugEvent(&dbgEvent, INFINITE)) {
				return 0;
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
				return 0;
			}

			ret = ProcessDebugEvent(dbgEvent);

			if (DBG_CONTINUE == ret) {
				ContinueDebugEvent(
					dbgEvent.dwProcessId,
					dbgEvent.dwThreadId,
					ret
				);
			} else {
				TerminateProcess(pi.hProcess, ret);
			}
		}

		/*do {
			if (!WaitForDebugEvent(&dbgEvent, INFINITE)) {
				return 0;
			}

			ContinueDebugEvent(
				dbgEvent.dwProcessId,
				dbgEvent.dwThreadId,
				DBG_CONTINUE
			);
		} while (dbgEvent.dwDebugEventCode != EXIT_PROCESS_DEBUG_EVENT);*/

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		fs::path testPath = fs::path(crashTestName);
		testPath += ".tst";
		fs::copy(testPath, exceptionsDir, fs::copy_options::overwrite_existing);
		fs::remove(testPath);

		fs::path dumpPath = fs::path(crashTestName);
		dumpPath += ".dmp";
		fs::copy(dumpPath, dumpsDir, fs::copy_options::overwrite_existing);
		fs::remove(dumpPath);


		fs::remove_all(corpusDir);
		fs::create_directory(corpusDir);
		fs::copy(outputDir, corpusDir, fs::copy_options::recursive);
		fs::remove_all(outputDir);
		fs::create_directory(outputDir);
	}

	return 0;
}