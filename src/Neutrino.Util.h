#ifndef _NEUTRINO_UTIL_H_
#define _NEUTRINO_UTIL_H_

#include <cstring>

namespace Neutrino {
	template<int count> inline bool CopyBytes(const BYTE *&bIn, BYTE *&bOut, int &szOut) {
		if (szOut < 0) {
			__debugbreak();
		}

		if (count >= szOut) {
			__debugbreak();
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
