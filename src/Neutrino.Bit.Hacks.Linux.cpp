#include "Neutrino.Bit.Hacks.h"

namespace Neutrino {

    DWORD PopCount(DWORD x) {
		return __builtin_popcount(x);
	}

};
