#ifndef _NEUTRINO_TRANSLATOR_H_
#define _NEUTRINO_TRANSLATOR_H_

#include "Neutrino.Abstract.Translator.h"
#include "Neutrino.Result.h"

namespace Neutrino {
	template <int count> inline bool CopyBytes(const BYTE *&bIn, BYTE *&bOut, int &szOut);

	template <typename STRATEGY> 
	class Translator : public STRATEGY {
		using STRATEGY::TouchStatic;
		using STRATEGY::TouchDynamic;
	private :
		typedef DWORD (Translator<STRATEGY>::*TranslateOpcodeFunc)(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		typedef void (Translator<STRATEGY>::*TranslateOperandFunc)(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		static TranslateOpcodeFunc translateOpcode0xFF[8];
		static TranslateOpcodeFunc translateOpcode[2][0x100];
		
		static TranslateOperandFunc translateOperand0xF6[8];
		static TranslateOperandFunc translateOperand0xF7[8];
		static TranslateOperandFunc translateOperand0xFF[8];
		static TranslateOperandFunc translateOperand[2][0x100];
	public :
		using STRATEGY::TouchDeferred;
		using STRATEGY::PushBasicBlock;
		using STRATEGY::LastBasicBlock;
		using STRATEGY::Reset;
		using STRATEGY::GetResult;

		virtual void Translate(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		
		/* Helpers */
		//bool TraceWrite(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest);

		/* Opcodes */
		DWORD TranslateOpcodeErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		DWORD TranslateOpcode(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		template <DWORD flag> 
		DWORD TranslatePrefix(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		template <BYTE destSize> 
		DWORD TranslateJmp(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		template <BYTE destSize> 
		DWORD TranslateJxx(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		template <TranslateOpcodeFunc *funcs>
		DWORD TranslateExtOpcode(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
			BYTE ext = (pIn[1] >> 3) & 0x07;

			return (this->*funcs[ext])(pIn, pOut, szOut, state);
		}

		DWORD TranslateCallModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		DWORD TranslateJumpModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		DWORD TranslateCall(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		DWORD TranslateRet(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		DWORD TranslateRetn(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		/* Operands */
		void TranslateOperandErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void TranslateNoOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void TranslateModRMOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void TranslateImm8Operand(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void TranslateImm1632Operand(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void TranslateImm32Operand(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		template <TranslateOperandFunc f1, TranslateOperandFunc f2>
		void TranslateAggOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
			(this->*f1)(pIn, pOut, szOut, state);
			(this->*f2)(pIn, pOut, szOut, state);
		}

		template <TranslateOperandFunc *funcs>
		void TranslateExtOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
			BYTE ext = (pIn[0] >> 3) & 0x07;

			(this->*funcs[ext])(pIn, pOut, szOut, state);
		}


	};
};

#include "Neutrino.Translator.hpp"

#endif