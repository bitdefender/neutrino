#ifndef _QUARK_TEST_EXTRACTOR_H_
#define _QUARK_TEST_EXTRACTOR_H_

#include "Neutrino.Test.h"

#ifdef _BUILD_WINDOWS
#include <Windows.h>
#endif // _BUILD_WINDOWS

#ifdef _BUILD_LINUX
#include <sys/ptrace.h>
#endif // _BUILD_LINUX

namespace Quark {

#ifdef _BUILD_WINDOWS
	typedef HANDLE process_t;
#endif // _BUILD_WINDOWS

#ifdef _BUILD_LINUX
	typedef int process_t;
#endif
	
	class TestExtractor {
	private :
		unsigned long long testVirtualAddress;

	public :
		TestExtractor();
		bool Perform(process_t process, Neutrino::Test &test);
	};

}; // namespace Quark

#endif
