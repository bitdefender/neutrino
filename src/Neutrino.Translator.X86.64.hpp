#include "Neutrino.Translator.X86.64.h"
#ifndef _NEUTRINO_TRANSLATOR_X86_64_HPP_
#define _NEUTRINO_TRANSLATOR_X86_64_HPP_

namespace Neutrino {

	DWORD GetRegUsage(const TranslationState &state);

	template<typename STRATEGY>
	DWORD TranslationTableX8664<STRATEGY>::GetCodeMemSize() {
		return 2 * sizeof(UINTPTR);
	}

	template <typename STRATEGY>
	template<BYTE destSize>
	int TranslationTableX8664<STRATEGY>::OpcodeJmp(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE code[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
		};

		const BYTE *pCode = code;
		UINTPTR offset;

		switch (destSize) {
		case 1:
			offset = *(CHAR *)(&pIn[1]);
			break;
		case 4:
			offset = (UINTPTR)(*(INT *)(&pIn[1]));
			break;
		default:
			offset = 0;
		};

		const void *dest = &pIn[1] + offset + destSize;

		TouchStatic(pOut, szOut, state, (UINTPTR)dest);

		state.Patch(PATCH_TYPE_DIRECT_64, (UINTPTR)dest, (UINTPTR *)&pOut[2]);
		state.flags |= FLAG_JUMP;

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		pIn += 1 + destSize;
		return PARSED_OPCODE;
	}

	/**
	 * Due to the lagre trampoline table the code for the jxx instruction translates to the following structure
	 *    jxx taken ( jxx 5 )
	 * falltrough:
	 *    jmp not_taken
     * taken:
	 *    <taken code>
     * not_taken:
	 *     <not taken code>
	 */
	template <typename STRATEGY>
	template<BYTE destSize>
	int TranslationTableX8664<STRATEGY>::OpcodeJxx(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		UINTPTR offset;

		switch (destSize) {
		case 1:
			offset = *(CHAR *)(&pIn[1]);
			break;
		case 4:
			offset = (UINTPTR)(*(INT *)(&pIn[1]));
			break;
		default:
			offset = 0;
		};

		const void *fallthrough = &pIn[1] + destSize;
		const void *taken = (const BYTE *)fallthrough + offset;

		static const BYTE jmpNotTaken[] = {
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		/* jxx taken */
		BYTE *pStart = pOut;
		CopyBytes<1 + destSize>(pIn, pOut, szOut);
		
		switch (destSize) {
		case 1:
			*(BYTE *)&pStart[1] = sizeof(jmpNotTaken);
			break;
		case 4:
			*(DWORD *)&pStart[1] = sizeof(jmpNotTaken);
			break;
		default:
			offset = 0;
		};

		/* jmp not_taken */
		const BYTE *pCode = jmpNotTaken;
		BYTE *pJump = pOut;
		CopyBytes<sizeof(jmpNotTaken)>(pCode, pOut, szOut);

		/* <taken code> */

		static const BYTE code[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
		};

		pCode = code;
		TouchStatic(pOut, szOut, state, (UINTPTR)taken);
		state.Patch(PATCH_TYPE_DIRECT_64, (UINTPTR)taken, (UINTPTR *)&pOut[2]);

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		*(DWORD *)(&pJump[1]) = (DWORD)(pOut - (pJump + sizeof(jmpNotTaken)));

		/* <not taken code> */

		pCode = code;
		TouchStatic(pOut, szOut, state, (UINTPTR)fallthrough);
		state.Patch(PATCH_TYPE_DIRECT_64, (UINTPTR)fallthrough, (UINTPTR *)&pOut[2]);

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);


		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	int TranslationTableX8664<STRATEGY>::OpcodeCall(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		UINTPTR offset;

		static const BYTE code[] = {
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 0x00 - mov [raxSave], rax
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 0x0A - mov rax, imm64
			0x50,															// 0x14 - push rax
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 0x15 - mov rax, [raxSave]
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00								// 0x1F - jmp [dest]
		};

		const BYTE *pCode = code;

		offset = *(INT *)(&pIn[1]);
		const void *dest = &pIn[1] + offset + 4;

		TouchStatic(pOut, szOut, state, (UINTPTR)dest);

		state.Patch(PATCH_TYPE_TRANSLATOR_SLOT, 0, (UINTPTR *)&pOut[0x02]);
		state.Patch(PATCH_TYPE_TRANSLATOR_SLOT, 0, (UINTPTR *)&pOut[0x17]);
		state.Patch(PATCH_TYPE_DIRECT_64, (UINTPTR)dest, (UINTPTR *)&pOut[0x21]);


		state.flags |= FLAG_JUMP;

		BYTE *pStart = pOut;
		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		*(QWORD *)(&pStart[0x0C]) = ((UINTPTR)pIn + 5);

		pIn += 5;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	int TranslationTableX8664<STRATEGY>::OpcodeRet(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codePfx[] = {
			0x48, 0x93,													// 0x00 - xchg rax, rbx
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x02 - mov [rbxSave], rbx
			0x48, 0x93,													// 0x0C - xchg rax, rbx
			0x5B														// 0x0E - pop rbx
		};

		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_TRANSLATOR_SLOT, 0, (UINTPTR *)&pStart[0x04]);

		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x48, 0x93,													// 0x00 - xchg rax, rbx
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x02 - mov rbx, [rbxSave]
			0x48, 0x93,													// 0x0C - xchg rax, rbx
			0xE9, 0x00, 0x00, 0x00, 0x00								// 0x0E - jump SolveIndirect
		};
		
		pCode = codeSfx;
		pStart = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_TRANSLATOR_SLOT, 0, (UINTPTR *)&pStart[0x04]);
		state.Patch(PATCH_TYPE_INDIRECT_64, 0, (UINTPTR *)&pStart[0x0F]);

		pIn++;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	int TranslationTableX8664<STRATEGY>::OpcodeRetn(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		DEBUG_BREAK;
		
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
	int TranslationTableX8664<STRATEGY>::OpcodeSyscall(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		CopyBytes<1>(pIn, pOut, szOut);

		static const BYTE codeSfx[] = {
			0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// 0x00 - mov rcx, nexinstr
		};

		const BYTE *pCode = codeSfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);

		*(QWORD *)(&pStart[0x02]) = (UINTPTR)pIn;

		return PARSED_OPCODE;
	}

	void JumpModRMPrefix(BYTE *&pOut, int &szOut, TranslationState &state);
	void JumpModRMSuffix(BYTE *&pOut, int &szOut, TranslationState &state);

	template <typename STRATEGY>
	int TranslationTableX8664<STRATEGY>::OpcodeCallModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		state.AddPrefix(JumpModRMPrefix);
		state.AddSuffix(JumpModRMSuffix);

		static const BYTE codePfx[] = {
			0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x00 - mov rbx, imm64
			0x48, 0x89, 0x5c, 0x24, 0xf8								// 0x0A - mov [rsp - 8], rbx
		};
				
		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);

		pOut[0] = 0x48 | ((state.flags >> 16) & 0x0F); // copy all rex prefixes
		pOut[1] = 0x8B;
		pOut += 2; szOut -= 2;
		
		BYTE *pModRm = pOut;
		pIn += 1;
		OperandModRm<0x83>(pIn, pOut, szOut, state);

		*(QWORD *)&pStart[0x02] = (UINTPTR)&pIn[0];

		static const BYTE codeSfx[] = {
			0x48, 0x8D, 0x64, 0x24, 0xF8								// 0x00 - lea rsp, [rsp - 8]
		};
		
		pCode = codeSfx;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);

		TouchDynamic(pOut, szOut, state);

		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}
		
	template <typename STRATEGY>
	int TranslationTableX8664<STRATEGY>::OpcodeJumpModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		state.AddPrefix(JumpModRMPrefix);
		state.AddSuffix(JumpModRMSuffix);

		pOut[0] = 0x48 | ((state.flags >> 16) & 0x0F); // copy all rex prefixes
		pOut[1] = 0x8B;
		pOut += 2; szOut -= 2;

		BYTE *pModRm = pOut;
		pIn += 1;
		OperandModRm<0x83>(pIn, pOut, szOut, state);

		TouchDynamic(pOut, szOut, state);

		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	int TranslationTableX8664<STRATEGY>::OpcodeFarJump(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		DEBUG_BREAK;
		
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

	DWORD BinLog2(DWORD x);

	extern CodePfxFunc ModRMPfx[];
	extern CodePfxFunc ModRMSfx[];

	template<typename STRATEGY>
	template<BYTE extOverride>
	inline void TranslationTableX8664<STRATEGY>::OperandModRmRip(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		BYTE modRM = pIn[0];
		BYTE mod = modRM & 0xC0;
		BYTE ext = (modRM >> 3) & 0x07;

		if (0x80 & extOverride) {
			ext = extOverride & 0x07;
		}

		BYTE rm = modRM & 0x07;

		DWORD offset = *(DWORD *)(&pIn[1]);

		/* Move the input cursor to the next instruction */
		pIn += 5;

		state.ripJumpDest = (QWORD)((INT)offset);

		DWORD regs = GetRegUsage(state);

		WORD inputRegs = regs & 0xFFFF;
		WORD outputRegs = regs >> 16;

		WORD extReg = 1 << (ext + ((state.flags & FLAG_64R) ? 8 : 0));
		inputRegs |= extReg;
		outputRegs |= extReg;

		WORD overWrittenRegs = (outputRegs) & (inputRegs ^ 0xFFFF);
		WORD unusedRegs = 0xFFFF ^ (inputRegs | outputRegs);

		WORD regSet = overWrittenRegs ? overWrittenRegs : unusedRegs;

		WORD tmpReg = regSet ^ (regSet & (regSet - 1));
		DWORD tmpRegPos = BinLog2(tmpReg);

		if (tmpRegPos >= 8) {
			DEBUG_BREAK;
		}

		pOut[0] = mod | (ext << 3) | tmpRegPos;
		state.AddPrefix(ModRMPfx[tmpRegPos]);

		if (0 == (overWrittenRegs & tmpReg)) {
			state.AddSuffix(ModRMSfx[tmpRegPos]);
		}

		pOut++;
		szOut--;
	}

	template <typename STRATEGY>
	template <BYTE extOverride>
	void TranslationTableX8664<STRATEGY>::OperandModRm(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		BYTE modRM = pIn[0];
		BYTE mod = modRM & 0xC0;

		if ((mod == 0) && (5 == (modRM & 0x7))) {
			OperandModRmRip<extOverride>(pIn, pOut, szOut, state);
			return;
		}

		if (0x80 & extOverride) {
			pOut[0] = (pIn[0] & 0xC7) | ((extOverride & 0x7F) << 3);
			pIn++;
			pOut++;
			szOut--;
		} else {
			CopyBytes<1>(pIn, pOut, szOut);
		}

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

	template<typename STRATEGY>
	void TranslationTableX8664<STRATEGY>::OperandImm163264(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		if (state.flags & FLAG_O16) {
			CopyBytes<2>(pIn, pOut, szOut);
		} else if (state.flags & FLAG_64W) {
			CopyBytes<8>(pIn, pOut, szOut);
		} else {
			CopyBytes<4>(pIn, pOut, szOut);
		}
	}

#define OPCODE_NULL &TranslationTableX86Base::OpcodeNull
#define OPCODE_ERROR &TranslationTableX86Base::OpcodeErr
#define OPCODE_DEFAULT &TranslationTableX86Base::OpcodeDefault
#define OPCODE_EXT(s) &TranslationTableX86Base::OpcodeExt< s >
#define OPCODE_PFX(flag) &TranslationTableX86Base::OpcodePrefix<(flag)>
#define OPCODE_JMP(idx) (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8664<STRATEGY>::OpcodeJmp<(idx)>
#define OPCODE_JXX(idx) (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8664<STRATEGY>::OpcodeJxx<(idx)>
#define OPCODE_CALL (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8664<STRATEGY>::OpcodeCall
#define OPCODE_RET (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8664<STRATEGY>::OpcodeRet
#define OPCODE_RETN (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8664<STRATEGY>::OpcodeRetn
#define OPCODE_CALLMODRM (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8664<STRATEGY>::OpcodeCallModRM
#define OPCODE_JMPMODRM (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8664<STRATEGY>::OpcodeJumpModRM
#define OPCODE_FARJMP (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8664<STRATEGY>::OpcodeFarJump
#define OPCODE_SYSCALL (TranslationTableX86Base::OpcodeFunc) &TranslationTableX8664<STRATEGY>::OpcodeSyscall

#define OPERAND_NULL &TranslationTableX86Base::OperandNull
#define OPERAND_NONE &TranslationTableX86Base::OperandNone
#define OPERAND_MODRM &TranslationTableX86Base::OperandModRm
#define OPERAND_IMM163264 (TranslationTableX86Base::OperandFunc) &TranslationTableX8664<STRATEGY>::OperandImm163264


	template<typename STRATEGY>
	TranslationTableX86Base::OpcodeFunc TranslationTableX8664<STRATEGY>::opcode0xFF[8] = {
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
	inline TranslationTableX8664<STRATEGY>::TranslationTableX8664(TranslationTableX86Base base) :
		TranslationTableX86Base(
			GetTableX86Base(),
			TranslationTableX86Base(
			(TranslationTableX86Base::OperandFunc)&TranslationTableX8664::OperandModRm<0>,
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

					/* 0x40 */ OPCODE_PFX(FLAG_REX), 
					/* 0x41 */ OPCODE_PFX(FLAG_REX |                                  FLAG_64B),
					/* 0x42 */ OPCODE_PFX(FLAG_REX |                       FLAG_64X),
					/* 0x43 */ OPCODE_PFX(FLAG_REX |                       FLAG_64X | FLAG_64B),
					/* 0x44 */ OPCODE_PFX(FLAG_REX |            FLAG_64R),
					/* 0x45 */ OPCODE_PFX(FLAG_REX |            FLAG_64R |            FLAG_64B),
					/* 0x46 */ OPCODE_PFX(FLAG_REX |            FLAG_64R | FLAG_64X),
					/* 0x47 */ OPCODE_PFX(FLAG_REX |            FLAG_64R | FLAG_64X | FLAG_64B),
					/* 0x48 */ OPCODE_PFX(FLAG_REX | FLAG_64W),
					/* 0x49 */ OPCODE_PFX(FLAG_REX | FLAG_64W |                       FLAG_64B),
					/* 0x4A */ OPCODE_PFX(FLAG_REX | FLAG_64W |            FLAG_64X),
					/* 0x4B */ OPCODE_PFX(FLAG_REX | FLAG_64W |            FLAG_64X | FLAG_64B),
					/* 0x4C */ OPCODE_PFX(FLAG_REX | FLAG_64W | FLAG_64R),
					/* 0x4D */ OPCODE_PFX(FLAG_REX | FLAG_64W | FLAG_64R |            FLAG_64B),
					/* 0x4E */ OPCODE_PFX(FLAG_REX | FLAG_64W | FLAG_64R | FLAG_64X),
					/* 0x4F */ OPCODE_PFX(FLAG_REX | FLAG_64W | FLAG_64R | FLAG_64X | FLAG_64B),

					/* 0x50 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
					/* 0x54 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
					/* 0x58 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
					/* 0x5C */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,

					/* 0x60 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_DEFAULT,
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
					/* 0xFC */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_EXT(TranslationTableX8664<STRATEGY>::opcode0xFF)
				},{
					/* 0x00 */ OPCODE_NULL, OPCODE_NULL, OPCODE_NULL, OPCODE_NULL,
					/* 0x04 */ OPCODE_NULL, OPCODE_SYSCALL, OPCODE_NULL, OPCODE_NULL,
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
			{ 
				{
					/* 0x00 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x04 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x08 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x0C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0x10 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x14 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x18 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x1C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0x20 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x24 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x28 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x2C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0x30 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x34 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x38 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x3C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0x40 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x44 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x48 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x4C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0x50 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x54 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x58 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x5C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0x60 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_MODRM,
					/* 0x64 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x68 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x6C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0x70 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x74 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x78 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x7C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0x80 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x84 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x88 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x8C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0x90 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x94 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x98 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x9C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0xA0 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xA4 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xA8 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xAC */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0xB0 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xB4 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xB8 */ OPERAND_IMM163264, OPERAND_IMM163264, OPERAND_IMM163264, OPERAND_IMM163264,
					/* 0xBC */ OPERAND_IMM163264, OPERAND_IMM163264, OPERAND_IMM163264, OPERAND_IMM163264,

					/* 0xC0 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xC4 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xC8 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xCC */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0xD0 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xD4 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xD8 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xDC */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0xE0 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xE4 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xE8 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xEC */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,

					/* 0xF0 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xF4 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xF8 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0xFC */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL
				},{
					/* 0x00 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x04 */ OPERAND_NULL, OPERAND_NONE, OPERAND_NULL, OPERAND_NULL,
					/* 0x08 */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL,
					/* 0x0C */ OPERAND_NULL, OPERAND_NULL, OPERAND_NULL, OPERAND_NULL
				} 
			}
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
#undef OPERAND_NONE 
#undef OPERAND_MODRM

};

#endif
