#include "Neutrino.Translator.X86.64.h"

namespace Neutrino {

	void ModRMPfxRax(BYTE *&pOut, int &szOut, TranslationState &state) {
		/* Switch out the registers */
		static const BYTE regPfx[] = {
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x00 - mov [], rax
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// 0x0A - mob rbx, imm64
		};

		const BYTE *pCode = regPfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(regPfx)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_TRANSLATOR_SLOT, 1, (UINTPTR *)&pStart[0x02]);
		*(QWORD *)(&pStart[0x0C]) = state.ripJumpDest;
	}

	template <BYTE reg>
	void ModRMPfxReg(BYTE *&pOut, int &szOut, TranslationState &state) {
		/* Switch out the registers */
		static const BYTE regPfx[] = {
			0x48, 0x90 + reg,											// 0x00 - xchg rax, rbx
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x02 - mov [], rax
			0x48, 0x90 + reg,											// 0x0C - xchg rax, rbx
			0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// 0x0E - mov rbx, imm64
		};

		const BYTE *pCode = regPfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(regPfx)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_TRANSLATOR_SLOT, 1, (UINTPTR *)&pStart[0x04]);
		*(QWORD *)(&pStart[0x10]) = state.ripJumpDest;
	}

	CodePfxFunc ModRMPfx[] = {
		ModRMPfxRax,
		ModRMPfxReg<1>,
		ModRMPfxReg<2>,
		ModRMPfxReg<3>,
		ModRMPfxReg<4>,
		ModRMPfxReg<5>,
		ModRMPfxReg<6>,
		ModRMPfxReg<7>
	};


	void ModRMSfxRax(BYTE *&pOut, int &szOut, TranslationState &state) {
		/* Restore the registers */
		static const BYTE regSfx[] = {
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x00 - mov rax, []
		};

		const BYTE *pCode = regSfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(regSfx)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_TRANSLATOR_SLOT, 1, (UINTPTR *)&pStart[0x02]);
	}

	template <BYTE reg>
	void ModRMSfxReg(BYTE *&pOut, int &szOut, TranslationState &state) {
		/* Restore the registers */
		static const BYTE regSfx[] = {
			0x48, 0x90 + reg,											// 0x00 - xchg rax, rbx
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x02 - mov rax, []
			0x48, 0x90 + reg											// 0x0C - xchg rax, rbx
		};

		const BYTE *pCode = regSfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(regSfx)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_TRANSLATOR_SLOT, 1, (UINTPTR *)&pStart[0x04]);
	}

	CodeSfxFunc ModRMSfx[] = {
		ModRMSfxRax,
		ModRMSfxReg<1>,
		ModRMSfxReg<2>,
		ModRMSfxReg<3>,
		ModRMSfxReg<4>,
		ModRMSfxReg<5>,
		ModRMSfxReg<6>,
		ModRMSfxReg<7>
	};

	void JumpModRMPrefix(BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codePfx[] = {
			0x48, 0x93,													// 0x00 - xchg rax, rbx
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x02 - mov [rbxSave], rbx
			0x48, 0x93													// 0x0C - xchg rax, rbx
		};

		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);


		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x04]);
	}

	void JumpModRMSuffix(BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codeSfx[] = {
			0x48, 0x93,													// 0x00 - xchg rax, rbx
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x02 - mov rax, []
			0x48, 0x93,													// 0x0C - xchg rax, rbx
			0xE9, 0x00, 0x00, 0x00, 0x00								// 0x0E - jmp SolveIndirect
		};

		const BYTE *pCode = codeSfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x04]);
		state.Patch(PATCH_TYPE_INDIRECT_64, 0, (UINTPTR *)&pStart[0x0F]);
	}

#define INPUT_RAX 0x00000001
#define INPUT_RCX 0x00000002
#define INPUT_RDX 0x00000004
#define INPUT_RBX 0x00000008
#define INPUT_RSP 0x00000010
#define INPUT_RBP 0x00000020
#define INPUT_RSI 0x00000040
#define INPUT_RDI 0x00000080
#define INPUT_R8  0x00000100
#define INPUT_R9  0x00000200
#define INPUT_R10 0x00000400
#define INPUT_R11 0x00000800
#define INPUT_R12 0x00001000
#define INPUT_R13 0x00002000
#define INPUT_R14 0x00004000
#define INPUT_R15 0x00008000

#define OUTPUT_RAX 0x00010000
#define OUTPUT_RCX 0x00020000
#define OUTPUT_RDX 0x00040000
#define OUTPUT_RBX 0x00080000
#define OUTPUT_RSP 0x00100000
#define OUTPUT_RBP 0x00200000
#define OUTPUT_RSI 0x00400000
#define OUTPUT_RDI 0x00800000
#define OUTPUT_R8  0x01000000
#define OUTPUT_R9  0x02000000
#define OUTPUT_R10 0x04000000
#define OUTPUT_R11 0x08000000
#define OUTPUT_R12 0x10000000
#define OUTPUT_R13 0x20000000
#define OUTPUT_R14 0x40000000
#define OUTPUT_R15 0x80000000

	const DWORD usedRegs[2][0x100] = {
		{
			/* 0x00 */ 0, 0, 0, 0,
			/* 0x04 */ INPUT_RAX | OUTPUT_RAX, INPUT_RAX | OUTPUT_RAX, 0, 0,
			/* 0x08 */ 0, 0, 0, 0,
			/* 0x0C */ INPUT_RAX | OUTPUT_RAX, INPUT_RAX | OUTPUT_RAX, 0, 0,

			/* 0x10 */ 0, 0, 0, 0,
			/* 0x14 */ INPUT_RAX | OUTPUT_RAX, INPUT_RAX | OUTPUT_RAX, 0, 0,
			/* 0x18 */ 0, 0, 0, 0,
			/* 0x1C */ INPUT_RAX | OUTPUT_RAX, INPUT_RAX | OUTPUT_RAX, 0, 0,

			/* 0x20 */ 0, 0, 0, 0,
			/* 0x24 */ INPUT_RAX | OUTPUT_RAX, INPUT_RAX | OUTPUT_RAX, 0, 0,
			/* 0x28 */ 0, 0, 0, 0,
			/* 0x2C */ INPUT_RAX | OUTPUT_RAX, INPUT_RAX | OUTPUT_RAX, 0, 0,

			/* 0x30 */ 0, 0, 0, 0,
			/* 0x34 */ INPUT_RAX | OUTPUT_RAX, INPUT_RAX | OUTPUT_RAX, 0, 0,
			/* 0x38 */ 0, 0, 0, 0,
			/* 0x3C */ INPUT_RAX | OUTPUT_RAX, INPUT_RAX | OUTPUT_RAX, 0, 0,

			/* 0x40 */ 0, 0, 0, 0,
			/* 0x44 */ 0, 0, 0, 0,
			/* 0x48 */ 0, 0, 0, 0,
			/* 0x4C */ 0, 0, 0, 0,

			/* 0x50 */ 0, 0, 0, 0,
			/* 0x54 */ 0, 0, 0, 0,
			/* 0x58 */ 0, 0, 0, 0,
			/* 0x5C */ 0, 0, 0, 0,

			/* 0x60 */ 0, 0, 0, 0,
			/* 0x64 */ 0, 0, 0, 0,
			/* 0x68 */ 0, 0, 0, 0,
			/* 0x6C */ 0, 0, 0, 0,

			/* 0x70 */ 0, 0, 0, 0,
			/* 0x74 */ 0, 0, 0, 0,
			/* 0x78 */ 0, 0, 0, 0,
			/* 0x7C */ 0, 0, 0, 0,

			/* 0x80 */ 0, 0, 0, 0,
			/* 0x84 */ 0, 0, 0, 0,
			/* 0x88 */ 0, 0, 0, 0,
			/* 0x8C */ 0, 0, 0, 0,

			/* 0x90 */ 0, 0, 0, 0,
			/* 0x94 */ 0, 0, 0, 0,
			/* 0x98 */ 0, 0, 0, 0,
			/* 0x9C */ 0, 0, 0, 0,

			/* 0xA0 */ 0, 0, 0, 0,
			/* 0xA4 */ 0, 0, 0, 0,
			/* 0xA8 */ 0, 0, 0, 0,
			/* 0xAC */ 0, 0, 0, 0,

			/* 0xB0 */ 0, 0, 0, 0,
			/* 0xB4 */ 0, 0, 0, 0,
			/* 0xB8 */ 0, 0, 0, 0,
			/* 0xBC */ 0, 0, 0, 0,

			/* 0xC0 */ 0, 0, 0, 0,
			/* 0xC4 */ 0, 0, 0, 0,
			/* 0xC8 */ 0, 0, 0, 0,
			/* 0xCC */ 0, 0, 0, 0,

			/* 0xD0 */ 0, 0, 0, 0,
			/* 0xD4 */ 0, 0, 0, 0,
			/* 0xD8 */ 0, 0, 0, 0,
			/* 0xDC */ 0, 0, 0, 0,

			/* 0xE0 */ 0, 0, 0, 0,
			/* 0xE4 */ 0, 0, 0, 0,
			/* 0xE8 */ 0, 0, 0, 0,
			/* 0xEC */ 0, 0, 0, 0,

			/* 0xF0 */ 0, 0, 0, 0,
			/* 0xF4 */ 0, 0, 0, 0,
			/* 0xF8 */ 0, 0, 0, 0,
			/* 0xFC */ 0, 0, 0, 0
		},{
			/* 0x00 */ 0, 0, 0, 0,
			/* 0x04 */ 0, 0, 0, 0,
			/* 0x08 */ 0, 0, 0, 0,
			/* 0x0C */ 0, 0, 0, 0,

			/* 0x10 */ 0, 0, 0, 0,
			/* 0x14 */ 0, 0, 0, 0,
			/* 0x18 */ 0, 0, 0, 0,
			/* 0x1C */ 0, 0, 0, 0,

			/* 0x20 */ 0, 0, 0, 0,
			/* 0x24 */ 0, 0, 0, 0,
			/* 0x28 */ 0, 0, 0, 0,
			/* 0x2C */ 0, 0, 0, 0,

			/* 0x30 */ 0, 0, 0, 0,
			/* 0x34 */ 0, 0, 0, 0,
			/* 0x38 */ 0, 0, 0, 0,
			/* 0x3C */ 0, 0, 0, 0,

			/* 0x40 */ 0, 0, 0, 0,
			/* 0x44 */ 0, 0, 0, 0,
			/* 0x48 */ 0, 0, 0, 0,
			/* 0x4C */ 0, 0, 0, 0,

			/* 0x50 */ 0, 0, 0, 0,
			/* 0x54 */ 0, 0, 0, 0,
			/* 0x58 */ 0, 0, 0, 0,
			/* 0x5C */ 0, 0, 0, 0,

			/* 0x60 */ 0, 0, 0, 0,
			/* 0x64 */ 0, 0, 0, 0,
			/* 0x68 */ 0, 0, 0, 0,
			/* 0x6C */ 0, 0, 0, 0,

			/* 0x70 */ 0, 0, 0, 0,
			/* 0x74 */ 0, 0, 0, 0,
			/* 0x78 */ 0, 0, 0, 0,
			/* 0x7C */ 0, 0, 0, 0,

			/* 0x80 */ 0, 0, 0, 0,
			/* 0x84 */ 0, 0, 0, 0,
			/* 0x88 */ 0, 0, 0, 0,
			/* 0x8C */ 0, 0, 0, 0,

			/* 0x90 */ 0, 0, 0, 0,
			/* 0x94 */ 0, 0, 0, 0,
			/* 0x98 */ 0, 0, 0, 0,
			/* 0x9C */ 0, 0, 0, 0,

			/* 0xA0 */ 0, 0, 0, 0,
			/* 0xA4 */ 0, 0, 0, 0,
			/* 0xA8 */ 0, 0, 0, 0,
			/* 0xAC */ 0, 0, 0, 0,

			/* 0xB0 */ INPUT_RAX | OUTPUT_RAX, INPUT_RAX | OUTPUT_RAX, 0, 0,
			/* 0xB4 */ 0, 0, 0, 0,
			/* 0xB8 */ 0, 0, 0, 0,
			/* 0xBC */ 0, 0, 0, 0,

			/* 0xC0 */ 0, 0, 0, 0,
			/* 0xC4 */ 0, 0, 0, INPUT_RAX | INPUT_RDX | OUTPUT_RAX | OUTPUT_RDX,
			/* 0xC8 */ 0, 0, 0, 0,
			/* 0xCC */ 0, 0, 0, 0,

			/* 0xD0 */ 0, 0, 0, 0,
			/* 0xD4 */ 0, 0, 0, 0,
			/* 0xD8 */ 0, 0, 0, 0,
			/* 0xDC */ 0, 0, 0, 0,

			/* 0xE0 */ 0, 0, 0, 0,
			/* 0xE4 */ 0, 0, 0, 0,
			/* 0xE8 */ 0, 0, 0, 0,
			/* 0xEC */ 0, 0, 0, 0,

			/* 0xF0 */ 0, 0, 0, 0,
			/* 0xF4 */ 0, 0, 0, 0,
			/* 0xF8 */ 0, 0, 0, 0,
			/* 0xFC */ 0, 0, 0, 0
		}
	};

	DWORD GetRegUsage(const TranslationState &state) {
		int tbl = (state.flags & FLAG_EXT) ? 1 : 0;

		return usedRegs[tbl][state.opCode];
	}

	DWORD NextPow2(DWORD x) {
		x = x - 1;

		x |= x >> 16;
		x |= x >> 8;
		x |= x >> 4;
		x |= x >> 2;
		x |= x >> 1;

		return x + 1;
	}

	DWORD BinLog2(DWORD x) {
		DWORD ret = 0;

		if (x & 0xFFFF0000) { ret += 16; x >>= 16; }
		if (x & 0x0000FF00) { ret +=  8; x >>=  8; }
		if (x & 0x000000F0) { ret +=  4; x >>=  4; }
		if (x & 0x0000000C) { ret +=  2; x >>=  2; }
		if (x & 0x00000002) { ret +=  1; x >>=  1; }

		return ret;
	}
};