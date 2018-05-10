#include "Quark.Test.Extractor.h"

#include <Windows.h>
#include <Psapi.h> 

namespace Quark {

	TestExtractor::TestExtractor() {
		HMODULE hMod = LoadLibrary("neutrino.exe");
		UINT_PTR ct = (UINT_PTR)GetProcAddress(hMod, "currentTest");
		testVirtualAddress = ct - (UINT_PTR)hMod;
		FreeLibrary(hMod);
	}

	bool TestExtractor::Perform(process_t process, Neutrino::TestData & test) {
		HMODULE hMod;
		DWORD cbNeeded;

		if (FALSE == EnumProcessModulesEx(process, &hMod, sizeof(hMod), &cbNeeded, LIST_MODULES_ALL)) {
			return false;
		}

		SIZE_T testAddr, szRead;

		if (FALSE == ReadProcessMemory(process, (LPCVOID)((UINT_PTR)hMod + testVirtualAddress), &testAddr, sizeof(testAddr), &szRead)) {
			return false;
		}

		if (FALSE == ReadProcessMemory(process, (LPCVOID)testAddr, &test, sizeof(test), &szRead)) {
			return false;
		}

		unsigned char *buff = new unsigned char[test.size + 1];

		if (FALSE == ReadProcessMemory(process, (LPCVOID)test.buffer, buff, test.size + 1, &szRead)) {
			return false;
		}

		test.buffer = buff;
		return true;
	}

};