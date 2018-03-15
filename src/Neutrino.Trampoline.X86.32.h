#ifndef _NEUTRINO_TRAMPOLINE_X86_32_H_
#define _NEUTRINO_TRAMPOLINE_X86_32_H_

#include "Neutrino.Types.h"

namespace Neutrino {

	class TrampolineX8632 {
	public :
		static DWORD GetCodeMemSize();
		static DWORD MakeTrampoline(BYTE *outBuffer, UINTPTR virtualStack, UINTPTR jumpBuff, UINTPTR environment, UINTPTR destination, UINTPTR mem);
	};

};

#endif
