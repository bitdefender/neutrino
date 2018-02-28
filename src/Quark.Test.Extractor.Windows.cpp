#include "Quark.Test.Extractor.h"

#include <Windows.h>
#include <Psapi.h> 

namespace Quark {

	TestExtractor::TestExtractor() {
		HMODULE hMod = LoadLibrary("neutrino.exe");
		DWORD ct = (DWORD)GetProcAddress(hMod, "currentTest");
		testVirtualAddress = ct - (DWORD)hMod;
		FreeLibrary(hMod);
	}

	bool TestExtractor::Perform(process_t process, Neutrino::Test & test) {
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

};