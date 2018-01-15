#ifndef _NEUTRINO_ABSTRACT_TRANSLATOR_H_
#define _NEUTRINO_ABSTRACT_TRANSLATOR_H_

#include "Neutrino.Types.h"

namespace Neutrino {
	const DWORD FLAG_EXT = 0x00000001;
	const DWORD FLAG_O16 = 0x00000020;
	const DWORD FLAG_A16 = 0x00000040;
	const DWORD FLAG_LOCK = 0x00000080;
	const DWORD FLAG_REP = 0x00000100;
	const DWORD FLAG_REPZ = 0x00000200;
	const DWORD FLAG_REPNZ = 0x00000400;
	const DWORD FLAG_FS = 0x00000800;
	const DWORD FLAG_GS = 0x00001000;
	const DWORD FLAG_JUMP = 0x80000000;

	const DWORD PARSED_OPCODE = 0x00000000;
	const DWORD PARSED_PREFIX = 0x00000001;

	const DWORD PATCH_TYPE_UNUSED = 0x00000000;
	const DWORD PATCH_TYPE_DIRECT = 0x00000001;
	const DWORD PATCH_TYPE_INDIRECT = 0x00000002;
	const DWORD PATCH_TYPE_JMP_REG_BKP = 0x00000003;
	
	const DWORD PATCH_TYPE_LAST_HASH_PTR = 0x00020001;
	const DWORD PATCH_TYPE_TUPLE_BASE = 0x00020002;


	struct CodePatch {
		DWORD jumpType;
		UINTPTR destination;
		UINTPTR *patch;
	};

	struct TranslationState {
		DWORD flags;
		BYTE opCode;
		BYTE pfxCount;

		CodePatch patch[16];
		DWORD patchCount;

		TranslationState();
		void Patch(DWORD jump, UINTPTR dest, UINTPTR *addr);
	};

	class AbstractTranslator {
	public:
		virtual void Translate(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) = 0;
	};

}; // namespace Neutrino

#endif