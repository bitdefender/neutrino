#ifndef _NEUTRINO_TRANSLATOR_H_
#define _NEUTRINO_TRANSLATOR_H_

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

	const DWORD PATCH_TYPE_UNUSED      = 0x00000000;
	const DWORD PATCH_TYPE_DIRECT      = 0x00000001;
	const DWORD PATCH_TYPE_INDIRECT    = 0x00000002;
	const DWORD PATCH_TYPE_TRACE_BASE  = 0x00000003;
	const DWORD PATCH_TYPE_TRACE_INDEX = 0x00000004;
	const DWORD PATCH_TYPE_REG1_BACKUP = 0x00000005;
	const DWORD PATCH_TYPE_REG2_BACKUP = 0x00000006;

	struct CodePatch {
		DWORD jumpType;
		UINTPTR destination;
		UINTPTR *patch;
	};

	struct InstructionState {
		DWORD flags;
		BYTE opCode;
		BYTE pfxCount;

		CodePatch patch[16];
		DWORD patchCount;

		InstructionState() {
			flags = 0;
			opCode = 0;
			pfxCount = 0;
			patchCount = 0;

			for (int i = 0; i < 8; ++i) {
				patch[i].jumpType = PATCH_TYPE_UNUSED;
			}
		}
	};

	class Translator {
	private :
		typedef DWORD (Translator::*TranslateOpcodeFunc)(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		typedef void(Translator::*TranslateOperandFunc)(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		static TranslateOpcodeFunc translateOpcode0xFF[8];
		static TranslateOpcodeFunc translateOpcode[2][0x100];
		
		static TranslateOperandFunc translateOperand0xFF[8];
		static TranslateOperandFunc translateOperand0xF6[8];
		static TranslateOperandFunc translateOperand0xF7[8];
		static TranslateOperandFunc translateOperand[2][0x100];
	public :
		void Translate(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);


		/* Helpers */
		template <int count> inline bool CopyBytes(const BYTE *&bIn, BYTE *&bOut, int &szOut) {
			if (count >= szOut) {
				return false;
			}

			for (int i = 0; i < count; ++i) {
				bOut[i] = bIn[i];
			}

			bOut += count;
			bIn += count;
			szOut -= count;
			return true;
		}

		bool TraceWrite(BYTE *&pOut, int &szOut, InstructionState &state, UINTPTR dest);

		/* Opcodes */
		DWORD TranslateOpcodeErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		DWORD TranslateOpcode(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		template <DWORD flag> DWORD TranslatePrefix(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
			CopyBytes<1>(pIn, pOut, szOut);
			
			state.flags |= flag;
			state.pfxCount++;
			return PARSED_PREFIX;
		}

		template <BYTE destSize> DWORD TranslateJmp(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
			static const BYTE code[] = {
				//0xE9, 0x00, 0x00, 0x00, 0x00
				0xff, 0x25, 0x00, 0x00, 0x00, 0x00
			};

			const BYTE *pCode = code;
			UINTPTR offset;

			switch (destSize) {
				case 1 :
					offset = *(int8_t *)(&pIn[1]);
					break;
				case 4 :
					offset = (UINTPTR)(*(DWORD *)(&pIn[1]));
					break;
				default :
					offset = 0;
			};
			
			const void *dest = &pIn[1] + offset + destSize;
			
			TraceWrite(pOut, szOut, state, (UINTPTR)dest);
			
			state.patch[state.patchCount].jumpType = PATCH_TYPE_DIRECT;
			state.patch[state.patchCount].destination = (UINTPTR)dest;
			state.patch[state.patchCount].patch = (UINTPTR *)&pOut[2];
			state.patchCount++;
			state.flags |= FLAG_JUMP;

			CopyBytes<sizeof(code)>(pCode, pOut, szOut);

			pIn += 1 + destSize;
			return PARSED_OPCODE;
		}

		template <BYTE destSize> DWORD TranslateJxx(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
			static const BYTE code[] = {
				//0xE9, 0x00, 0x00, 0x00, 0x00
				0xff, 0x25, 0x00, 0x00, 0x00, 0x00
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

			TraceWrite(pOut, szOut, state, (UINTPTR)fallthrough);

			state.patch[state.patchCount].jumpType = PATCH_TYPE_DIRECT;
			state.patch[state.patchCount].destination = (UINTPTR)fallthrough;
			state.patch[state.patchCount].patch = (UINTPTR *)&pOut[2];
			state.patchCount++;

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

			TraceWrite(pOut, szOut, state, (UINTPTR)taken);

			state.patch[state.patchCount].jumpType = PATCH_TYPE_DIRECT;
			state.patch[state.patchCount].destination = (UINTPTR)taken;
			state.patch[state.patchCount].patch = (UINTPTR *)&pOut[2];
			state.patchCount++;
			state.flags |= FLAG_JUMP;

			pCode = code;
			CopyBytes<sizeof(code)>(pCode, pOut, szOut);
			return PARSED_OPCODE;
		}

		template <Translator::TranslateOpcodeFunc *funcs>
		DWORD TranslateExtOpcode(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
			BYTE ext = (pIn[1] >> 3) & 0x07;

			return (this->*funcs[ext])(pIn, pOut, szOut, state);
		}


		DWORD TranslateCallModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		DWORD TranslateJumpModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		DWORD TranslateCall(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		DWORD TranslateRet(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		DWORD TranslateRetn(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		/* Operands */
		void TranslateOperandErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		void TranslateNoOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		void TranslateModRMOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		inline void TranslateImm8Operand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
			CopyBytes<1>(pIn, pOut, szOut);
		}

		inline void TranslateImm1632Operand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
			if (state.flags & FLAG_O16) {
				CopyBytes<2>(pIn, pOut, szOut);
			} else {
				CopyBytes<4>(pIn, pOut, szOut);
			}
		}

		inline void TranslateImm32Operand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
			CopyBytes<4>(pIn, pOut, szOut);
		}

		template <Translator::TranslateOperandFunc f1, Translator::TranslateOperandFunc f2>
		void TranslateAggOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
			(this->*f1)(pIn, pOut, szOut, state);
			(this->*f2)(pIn, pOut, szOut, state);
		}

		template <Translator::TranslateOperandFunc *funcs>
		void TranslateExtOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
			BYTE ext = (pIn[0] >> 3) & 0x07;

			(this->*funcs[ext])(pIn, pOut, szOut, state);
		}
	};
};


#endif