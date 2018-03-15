#include "Neutrino.Trampoline.X86.32.h"

#include <cstring>

namespace Neutrino {
	DWORD TrampolineX8632::GetCodeMemSize() {
		return 0;
	}

	DWORD TrampolineX8632::MakeTrampoline(BYTE *outBuffer, UINTPTR virtualStack, UINTPTR jumpBuff, UINTPTR environment, UINTPTR destination, UINTPTR mem) {

		static const BYTE code[] = {
			0x87, 0x25, 0x00, 0x00, 0x00, 0x00,			// 0x00 - xchg esp, large ds:<dwVirtualStack>
			0x9C, 										// 0x06 - pushf
			0x60,										// 0x07 - pusha
			0x68, 0x46, 0x02, 0x00, 0x00,				// 0x08 - push 0x00000246 - NEW FLAGS
			0x9D,										// 0x0D - popf
			0x68, 0x00, 0x00, 0x00, 0x00,				// 0x0E - push <Environment>
			0xFF, 0x15, 0x00, 0x00, 0x00, 0x00,			// 0x13 - call <dwBranchHandler>
			0x83, 0xC4, 0x04,							// 0x19 - sub esp, 4
			0x61,										// 0x1C - popa
			0x9D,										// 0x1D - popf
			0x87, 0x25, 0x00, 0x00, 0x00, 0x00,			// 0x1E - xchg esp, large ds:<dwVirtualStack>
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00			// 0x24 - jmp large dword ptr ds:<jumpbuff>	
		};

		memcpy(outBuffer, code, sizeof(code));

		*(UINTPTR *)(&(outBuffer[0x02])) = virtualStack;
		*(UINTPTR *)(&(outBuffer[0x0F])) = environment;
		*(UINTPTR *)(&(outBuffer[0x15])) = destination;
		*(UINTPTR *)(&(outBuffer[0x20])) = virtualStack;
		*(UINTPTR *)(&(outBuffer[0x26])) = jumpBuff;

		return sizeof(code);
	}
};

