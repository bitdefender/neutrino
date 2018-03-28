#ifndef _NEUTRINO_UTIL_H_
#define _NEUTRINO_UTIL_H_

#include <cstring>

#ifdef _BUILD_WINDOWS
#define DEBUG_BREAK __debugbreak()
#endif

#ifdef _BUILD_LINUX
#define DEBUG_BREAK asm volatile("int $0x3")
#endif

namespace Neutrino {
	template<int count> inline bool CopyBytes(const BYTE *&bIn, BYTE *&bOut, int &szOut) {
		if (szOut < 0) {
			DEBUG_BREAK;
		}

		if (count >= szOut) {
			DEBUG_BREAK;
			return false;
		}

		memcpy(bOut, bIn, count);

		bOut += count;
		bIn += count;
		szOut -= count;
		return true;
	}
};

#endif
