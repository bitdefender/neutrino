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

		void Patch(DWORD jump, UINTPTR dest, UINTPTR *addr) {
			patch[patchCount].jumpType = jump;
			patch[patchCount].destination = dest;
			patch[patchCount].patch = addr;
			patchCount++;
		}
	};

	class Translator {
	private :
		typedef DWORD (Translator::*TranslateOpcodeFunc)(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		typedef void(Translator::*TranslateOperandFunc)(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		static TranslateOpcodeFunc translateOpcode0xFF[8];
		static TranslateOpcodeFunc translateOpcode[2][0x100];
		
		static TranslateOperandFunc translateOperand0xF6[8];
		static TranslateOperandFunc translateOperand0xF7[8];
		static TranslateOperandFunc translateOperand0xFF[8];
		static TranslateOperandFunc translateOperand[2][0x100];
	public :
		void Translate(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);


		/* Helpers */
		template <int count> 
		inline bool CopyBytes(const BYTE *&bIn, BYTE *&bOut, int &szOut);

		bool TraceWrite(BYTE *&pOut, int &szOut, InstructionState &state, UINTPTR dest);

		/* Opcodes */
		DWORD TranslateOpcodeErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		DWORD TranslateOpcode(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		template <DWORD flag> 
		DWORD TranslatePrefix(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		template <BYTE destSize> 
		DWORD TranslateJmp(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		template <BYTE destSize> 
		DWORD TranslateJxx(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		template <Translator::TranslateOpcodeFunc *funcs>
		DWORD TranslateExtOpcode(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);


		DWORD TranslateCallModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		DWORD TranslateJumpModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		DWORD TranslateCall(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		DWORD TranslateRet(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		DWORD TranslateRetn(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		/* Operands */
		void TranslateOperandErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		void TranslateNoOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		void TranslateModRMOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		void TranslateImm8Operand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		void TranslateImm1632Operand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
		void TranslateImm32Operand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		template <Translator::TranslateOperandFunc f1, Translator::TranslateOperandFunc f2>
		void TranslateAggOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);

		template <Translator::TranslateOperandFunc *funcs>
		void TranslateExtOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state);
	};
};

#include "Neutrino.Translator.hpp"

#endif