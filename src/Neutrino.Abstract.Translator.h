#ifndef _NEUTRINO_ABSTRACT_TRANSLATOR_H_
#define _NEUTRINO_ABSTRACT_TRANSLATOR_H_

#include "Neutrino.Types.h"

namespace Neutrino {
	/* X86 32-bit flags */
	const DWORD FLAG_EXT = 0x00000001;
	const DWORD FLAG_O16 = 0x00000020;
	const DWORD FLAG_A16 = 0x00000040;
	const DWORD FLAG_LOCK = 0x00000080;
	const DWORD FLAG_REP = 0x00000100;
	const DWORD FLAG_REPZ = 0x00000200;
	const DWORD FLAG_REPNZ = 0x00000400;
	const DWORD FLAG_FS = 0x00000800;
	const DWORD FLAG_GS = 0x00001000;

	/* X86 64-bit flags */
	const DWORD FLAG_REX = 0x40000000;
	const DWORD FLAG_64B = 0x00010000;
	const DWORD FLAG_64X = 0x00020000;
	const DWORD FLAG_64R = 0x00040000;
	const DWORD FLAG_64W = 0x00080000;


	const DWORD FLAG_JUMP = 0x80000000;

	const DWORD PARSED_OPCODE = 0x00000000;
	const DWORD PARSED_PREFIX = 0x00000001;

	const DWORD PATCH_TYPE_UNUSED = 0x00000000;
	const DWORD PATCH_TYPE_DIRECT = 0x00000001;
	const DWORD PATCH_TYPE_INDIRECT = 0x00000002;
	const DWORD PATCH_TYPE_JMP_REG_BKP = 0x00000003;
	const DWORD PATCH_TYPE_TRANSLATOR_SLOT = 0x00000010;
	//const DWORD PATCH_TYPE_TRANSLATOR_SLOT = 0x00000010;

	const DWORD PATCH_TYPE_DIRECT_64 = 0x00000101;
	const DWORD PATCH_TYPE_INDIRECT_64 = 0x00000102;
		
	const DWORD PATCH_TYPE_LAST_HASH_PTR = 0x00020001;
	const DWORD PATCH_TYPE_TUPLE_BASE = 0x00020002;


	struct CodePatch {
		DWORD jumpType;
		UINTPTR destination;
		UINTPTR *patch;
	};

	struct TranslationState;
	typedef void (*CodePfxFunc)(BYTE *&pOut, int &szOut, TranslationState &state);
	typedef void (*CodeSfxFunc)(BYTE *&pOut, int &szOut, TranslationState &state);

	struct TranslationState {
		BYTE *outStart;
		DWORD flags;
		BYTE opCode;
		BYTE subOpcode;
		BYTE pfxCount;

		QWORD ripJumpDest;
		DWORD pfxFuncCount;
		DWORD sfxFuncCount;
		CodePfxFunc codePfx[4];
		CodeSfxFunc codeSfx[4];

		DWORD patchCount; 
		CodePatch patch[16];
		

		TranslationState();

		void Init();

		void Patch(DWORD jump, UINTPTR dest, UINTPTR *addr);
		void AddPrefix(CodePfxFunc pfx);
		void AddSuffix(CodeSfxFunc sfx);

		void WritePrefix(BYTE *&pOut, int &szOut);
		void WriteSuffix(BYTE *&pOut, int &szOut);
	};

	class AbstractTranslator {
	public:
		virtual void Translate(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) = 0;
	};

}; // namespace Neutrino

#endif