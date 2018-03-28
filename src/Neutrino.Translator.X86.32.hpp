#include "Neutrino.Translator.X86.32.h"
#ifndef _NEUTRINO_TRANSLATOR_X86_32_HPP_
#define _NEUTRINO_TRANSLATOR_X86_32_HPP_

namespace Neutrino {

	template <typename STRATEGY>
	template<BYTE destSize>
	int TranslationTableX8632<STRATEGY>::OpcodeJmp(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE code[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
		};

		const BYTE *pCode = code;
		UINTPTR offset;

		switch (destSize) {
		case 1:
			offset = *(int8_t *)(&pIn[1]);
			break;
		case 4:
			offset = (UINTPTR)(*(DWORD *)(&pIn[1]));
			break;
		default:
			offset = 0;
		};

		const void *dest = &pIn[1] + offset + destSize;

		TouchStatic(pOut, szOut, state, (UINTPTR)dest);

		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)dest, (UINTPTR *)&pOut[2]);
		state.flags |= FLAG_JUMP;

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		pIn += 1 + destSize;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	template<BYTE destSize>
	int TranslationTableX8632<STRATEGY>::OpcodeJxx(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE code[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
		};

		const BYTE *pCode = code;
		UINTPTR offset;

		switch (destSize) {
		case 1:
			offset = *(int8_t *)(&pIn[1]);
			break;
		case 4:
			offset = (UINTPTR)(*(DWORD *)(&pIn[1]));
			break;
		default:
			offset = 0;
		};

		const void *fallthrough = &pIn[1] + destSize;
		const void *taken = (const BYTE *)fallthrough + offset;

		BYTE *pStart = pOut;

		CopyBytes<1 + destSize>(pIn, pOut, szOut);
		pIn += 1 + destSize;

		BYTE *pNow = pOut;

		TouchStatic(pOut, szOut, state, (UINTPTR)fallthrough);
		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)fallthrough, (UINTPTR *)&pOut[2]);

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		switch (destSize) {
		case 1:
			*(BYTE *)&pStart[1] = (BYTE)(pOut - pNow);
			break;
		case 4:
			*(DWORD *)&pStart[1] = (DWORD)(pOut - pNow);
			break;
		default:
			offset = 0;
		};

		TouchStatic(pOut, szOut, state, (UINTPTR)taken);
		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)taken, (UINTPTR *)&pOut[2]);

		state.flags |= FLAG_JUMP;

		pCode = code;
		CopyBytes<sizeof(code)>(pCode, pOut, szOut);
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	int TranslationTableX8632<STRATEGY>::OpcodeCall(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		UINTPTR offset;

		static const BYTE code[] = {
			0x68, 0x00, 0x00, 0x00, 0x00,						// 0x00 - push imm32
			0xff, 0x25, 0x00, 0x00, 0x00, 0x00					// 0x05 - jmp [dest]
		};

		const BYTE *pCode = code;

		offset = *(DWORD *)(&pIn[1]);
		const void *dest = &pIn[1] + offset + 4;

		TouchStatic(pOut, szOut, state, (UINTPTR)dest);

		UINTPTR *pPatch = (UINTPTR *)&pOut[1];

		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)dest, (UINTPTR *)&pOut[0x07]);
		state.flags |= FLAG_JUMP;

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);
		*pPatch = ((UINTPTR)pIn + 5);

		pIn += 5;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	int TranslationTableX8632<STRATEGY>::OpcodeRet(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codePfx[] = {
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov [ebxSave], ebx
			0x5B												// 0x06 - pop ebx
		};
		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);

		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov ebx, [ebxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00						// 0x06 - jmp SolveIndirect
		};
		pCode = codeSfx;
		pStart = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pStart[0x07]);

		pIn++;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	int TranslationTableX8632<STRATEGY>::OpcodeRetn(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codePfx[] = {
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov [ebxSave], ebx
			0x5B												// 0x06 - pop ebx
		};
		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);

		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov ebx, [ebxSave]
			0x8D, 0xA4, 0x24, 0x00, 0x00, 0x00, 0x00,			// 0x06 - lea esp, [esp + off]
			0xE9, 0x00, 0x00, 0x00, 0x00						// 0x0D - jmp SolveIndirect
		};
		pCode = codeSfx;
		pStart = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);
		*(int *)&pStart[0x09] = (int)(*(short *)&pIn[1]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pStart[0x0E]);

		pIn += 3;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template<typename STRATEGY>
	DWORD TranslationTableX8632<STRATEGY>::GetCodeMemSize() {
		return 0;
	}

	template <typename STRATEGY>
	int TranslationTableX8632<STRATEGY>::OpcodeCallModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codePfx[] = {
			0x68, 0x00, 0x00, 0x00, 0x00,					// 0x00 - push imm32
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x05 - mov [ebxSave], ebx
			0x8B,											// 0x0B - mov ebx, [<---->]
		};

		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;

		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		pIn += 1; szOut -= 1;
		OperandModRm(pIn, pOut, szOut, state);

		*(UINTPTR *)&pStart[1] = (UINTPTR)pIn;

		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x07]);

		pStart[0x0C] &= 0xC7; // clear ext
		pStart[0x0C] |= 0x18; // set ext to EBX

		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x00 - mov ebx, [ebxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00					// 0x06 - jmp SolveIndirect
		};

		pCode = codeSfx;
		BYTE *pCont = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pCont[0x02]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pCont[0x07]);

		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	int TranslationTableX8632<STRATEGY>::OpcodeJumpModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codePfx[] = {
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x00 - mov [ebxSave], ebx
			0x8B,											// 0x06 - mov ebx, [<---->]
		};

		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;

		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		pIn += 1; szOut -= 1;
		OperandModRm(pIn, pOut, szOut, state);

		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);

		pStart[0x07] &= 0xC7; // clear ext
		pStart[0x07] |= 0x18; // set ext to EBX

		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x00 - mov ebx, [ebxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00					// 0x06 - jmp SolveIndirect
		};

		pCode = codeSfx;
		BYTE *pCont = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pCont[0x02]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pCont[0x07]);

		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	int TranslationTableX8632<STRATEGY>::OpcodeFarJump(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		//const RiverInstruction &ri, RelocableCodeBuffer &px86, nodep::DWORD &pFlags, nodep::DWORD &instrCounter) {
		ClearPrefixes(pIn, state);

		static const unsigned char codePfx[] = {
			0x8F, 0x05, 0x00, 0x00, 0x00, 0x00,			// 0x00 - pop [ebxSave]
			0xEB, 0x07,									// 0x06 - jmp $+7
			0xEA, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00,   // 0x08 - jmpf 0x33:12345678 - the actual syscall
			0xE8, 0xF4, 0xFF, 0xFF, 0xFF,				// 0x0F - call $-7
			0x87, 0x1D, 0x00, 0x00, 0x00, 0x00//,		// 0x14 - xchg ebx, [ebxSave]
			//0x5B										// 0x1A - pop ebx
		};

		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);
		*(int *)&pStart[0x09] = *(int *)&pIn[1];
		*(short *)&pStart[0x0D] = *(short *)&pIn[5];
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x16]);

		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov ebx, [ebxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00						// 0x06 - jmp SolveIndirect
		};
		pCode = codeSfx;
		pStart = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pStart[0x07]);

		pIn += 7;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	void TranslationTableX8632<STRATEGY>::OperandModRm(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		BYTE modRM = pIn[0];
		BYTE mod = modRM & 0xC0;

		CopyBytes<1>(pIn, pOut, szOut);

		if ((0x4 == (modRM & 0x7)) && (mod != 0xC0)) {
			// also copy the SIB byte
			BYTE sib = pIn[0];

			CopyBytes<1>(pIn, pOut, szOut);

			if (0x5 == (sib & 0x7)) {
				if ((mod == 0x00) || (mod == 0x80)) {
					CopyBytes<4>(pIn, pOut, szOut);
				}
				else if (mod == 0x40) {
					CopyBytes<1>(pIn, pOut, szOut);
				}

				// this might be bad!
				return;
			}
		}

		if (mod == 0x40) {
			// also copy byte displacement
			CopyBytes<1>(pIn, pOut, szOut);
		}
		else if ((mod == 0x80) || ((mod == 0x00) && (0x5 == (modRM & 0x7)))) {
			// also copy dword displacement
			CopyBytes<4>(pIn, pOut, szOut);
		}
	}

#define OPCODE_NULL &TranslationTableX86Base::OpcodeNull
#define OPCODE_ERROR &TranslationTableX86Base::OpcodeErr
#define OPCODE_DEFAULT &TranslationTableX86Base::OpcodeDefault
#define OPCODE_EXT(s) &TranslationTableX86Base::OpcodeExt< s >
#define OPCODE_JMP(idx) (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8632<STRATEGY>::OpcodeJmp<(idx)>
#define OPCODE_JXX(idx) (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8632<STRATEGY>::OpcodeJxx<(idx)>
#define OPCODE_CALL (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8632<STRATEGY>::OpcodeCall
#define OPCODE_RET (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8632<STRATEGY>::OpcodeRet
#define OPCODE_RETN (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8632<STRATEGY>::OpcodeRetn
#define OPCODE_CALLMODRM (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8632<STRATEGY>::OpcodeCallModRM
#define OPCODE_JMPMODRM (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8632<STRATEGY>::OpcodeJumpModRM
#define OPCODE_FARJMP (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8632<STRATEGY>::OpcodeFarJump

#define OPERAND_NULL &TranslationTableX86Base::OperandNull

	template<typename STRATEGY>
	TranslationTableX86Base::OpcodeFunc TranslationTableX8632<STRATEGY>::opcode0xFF[8] = {
		/* 0x00 */ OPCODE_DEFAULT,
		/* 0x01 */ OPCODE_DEFAULT,
		/* 0x02 */ OPCODE_CALLMODRM, //TranslateCallModRM,
		/* 0x03 */ OPCODE_ERROR,
		/* 0x04 */ OPCODE_JMPMODRM, //TranslateJumpModRM,
		/* 0x05 */ OPCODE_ERROR,
		/* 0x06 */ OPCODE_DEFAULT,
		/* 0x07 */ OPCODE_ERROR
	};

	template<typename STRATEGY>
	inline TranslationTableX8632<STRATEGY>::TranslationTableX8632(TranslationTableX86Base base) :
		TranslationTableX86Base(
			tableX86Base,
			TranslationTableX86Base(
				(TranslationTableX86Base::OperandFunc)&TranslationTableX8632::OperandModRm,
				{
					{
						/* 0x00 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x04 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x08 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x0C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x10 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x14 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x18 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x1C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x20 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x24 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x28 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x2C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x30 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x34 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x38 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x3C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x40 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
						/* 0x44 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
						/* 0x48 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
						/* 0x4C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,

						/* 0x50 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x54 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x58 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x5C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x60 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x64 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x68 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x6C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x70 */ OPCODE_JXX(1), OPCODE_JXX(1), OPCODE_JXX(1), OPCODE_JXX(1),
						/* 0x74 */ OPCODE_JXX(1), OPCODE_JXX(1), OPCODE_JXX(1), OPCODE_JXX(1),
						/* 0x78 */ OPCODE_JXX(1), OPCODE_JXX(1), OPCODE_JXX(1), OPCODE_JXX(1),
						/* 0x7C */ OPCODE_JXX(1), OPCODE_JXX(1), OPCODE_JXX(1), OPCODE_JXX(1),

						/* 0x80 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x84 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x88 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x8C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x90 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x94 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x98 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x9C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xA0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xA4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xA8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xAC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xB0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xB4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xB8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xBC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xC0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_RETN, OPCODE_RET,
						/* 0xC4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xC8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xCC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xD0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xD4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xD8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xDC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xE0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xE4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xE8 */ OPCODE_CALL, OPCODE_JMP(4), OPCODE_FARJMP, OPCODE_JMP(1),
						/* 0xEC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xF0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xF4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xF8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xFC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_EXT(TranslationTableX8632<STRATEGY>::opcode0xFF)
					},{
						/* 0x00 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x04 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x08 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x0C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x10 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x14 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x18 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x1C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x20 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x24 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x28 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x2C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x30 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x34 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x38 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x3C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x40 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x44 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x48 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x4C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x50 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x54 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x58 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x5C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x60 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x64 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x68 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x6C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x70 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x74 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x78 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x7C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0x80 */ OPCODE_JXX(4u), OPCODE_JXX(4u), OPCODE_JXX(4u), OPCODE_JXX(4u),
						/* 0x84 */ OPCODE_JXX(4u), OPCODE_JXX(4u), OPCODE_JXX(4u), OPCODE_JXX(4u),
						/* 0x88 */ OPCODE_JXX(4u), OPCODE_JXX(4u), OPCODE_JXX(4u), OPCODE_JXX(4u),
						/* 0x8C */ OPCODE_JXX(4u), OPCODE_JXX(4u), OPCODE_JXX(4u), OPCODE_JXX(4u),

						/* 0x90 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x94 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x98 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0x9C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xA0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xA4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xA8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xAC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xB0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xB4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xB8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xBC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xC0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xC4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xC8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xCC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xD0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xD4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xD8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xDC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xE0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xE4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xE8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xEC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

						/* 0xF0 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xF4 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xF8 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
						/* 0xFC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL
					}
				},
				{ {}, {} }
			)
		)
	{ }


#undef OPCODE_NULL
#undef OPCODE_ERROR
#undef OPCODE_DEFAULT
#undef OPCODE_EXT
#undef OPCODE_JMP
#undef OPCODE_JXX
#undef OPCODE_CALL
#undef OPCODE_RET
#undef OPCODE_RETN
#undef OPCODE_CALLMODRM
#undef OPCODE_JMPMODRM
#undef OPCODE_FARJMP

#undef OPERAND_NULL

};

#endif
