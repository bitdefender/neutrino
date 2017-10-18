#ifndef _NEUTRINO_TRANSLATOR_H_
#define _NEUTRINO_TRANSLATOR_H_


namespace Neutrino {
	/* FOR NOW! LATER: build platform independant type system */
	typedef unsigned char BYTE;
	typedef unsigned int DWORD;

	const DWORD FLAG_EXT = 0x00000001;
	const DWORD FLAG_O16 = 0x00000020;
	const DWORD FLAG_A16 = 0x00000040;
	const DWORD FLAG_LOCK = 0x00000080;
	const DWORD FLAG_REP = 0x00000100;
	const DWORD FLAG_REPZ = 0x00000200;
	const DWORD FLAG_REPNZ = 0x00000400;


	const DWORD PARSED_OPCODE = 0x00000000;
	const DWORD PARSED_PREFIX = 0x00000001;

	struct InstructionState {
		DWORD flags;
		BYTE opCode;
		BYTE pfxcount;
	};

	class Translator {
	private :
		typedef DWORD (Translator::*TranslateOpcodeFunc)(BYTE *&pIn, BYTE *&pOut, InstructionState &state);
		typedef void(Translator::*TranslateOperandFunc)(BYTE *&pIn, BYTE *&pOut, InstructionState &state);

		static TranslateOpcodeFunc translateOpcode[2][0x100];
		static TranslateOperandFunc translateOperand[2][0x100];
	public :
		void Translate(BYTE *&pIn, BYTE *&pOut, InstructionState &state);

		DWORD TranslateOpcodeErr(BYTE *&pIn, BYTE *&pOut, InstructionState &state);
		DWORD TranslateOpcode(BYTE *&pIn, BYTE *&pOut, InstructionState &state);

		template <DWORD flag> DWORD TranslatePrefix(BYTE *&pIn, BYTE *&pOut, InstructionState &state) {
			pOut[0] = pIn[0];
			pOut++; pIn++;
			state.flags |= flag;
			state.pfxcount++;
			return PARSED_PREFIX;
		}


		void TranslateOperandErr(BYTE *&pIn, BYTE *&pOut, InstructionState &state);
		void TranslateNoOperand(BYTE *&pIn, BYTE *&pOut, InstructionState &state);
	};
};


#endif