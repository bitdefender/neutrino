#include "Neutrino.Strategy.Trace.X86.32.h"

#include "Neutrino.Util.h"

namespace Neutrino {

	bool TraceStrategyX8632::TouchStatic(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest) {
		static const BYTE code[] = {
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x00 - mov [eaxSave], eax
			0xA1, 0x00, 0x00, 0x00, 0x00,						// 0x05 - mov eax, [traceIndex]
			0x8D, 0x40, 0x01,									// 0x0A - lea eax, [eax + 0x1]
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x0D - mov [traceIndex], eax
			0x8D, 0x04, 0x85, 0x00, 0x00, 0x00, 0x00,			// 0x12 - lea eax, [4 * eax + traceBase]
			0xC7, 0x00, 0x00, 0x00, 0x00, 0x00,					// 0x19 - mov [eax], dest
			0xA1, 0x00, 0x00, 0x00, 0x00						// 0x1F - mov eax, [eaxSave]
		};

		const BYTE *pCode = code;
		BYTE *pStart = pOut;

		if (!CopyBytes<sizeof(code)>(pCode, pOut, szOut)) {
			return false;
		}

		*(UINTPTR *)&pStart[0x01] = (UINTPTR)&regEax;
		*(UINTPTR *)&pStart[0x06] = (UINTPTR)&out.traceIndex;
		*(UINTPTR *)&pStart[0x0E] = (UINTPTR)&out.traceIndex;
		*(UINTPTR *)&pStart[0x15] = (UINTPTR)&out.trace;
		*(UINTPTR *)&pStart[0x1B] = dest;
		*(UINTPTR *)&pStart[0x20] = (UINTPTR)&regEax;


		return true;
	}

	bool TraceStrategyX8632::TouchDynamic(BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE code[] = {
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x00 - mov [eaxSave], eax
			0xA1, 0x00, 0x00, 0x00, 0x00,						// 0x05 - mov eax, [traceIndex]
			0x8D, 0x40, 0x01,									// 0x0A - lea eax, [eax + 0x1]
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x0D - mov [traceIndex], eax
			0x8D, 0x04, 0x85, 0x00, 0x00, 0x00, 0x00,			// 0x12 - lea eax, [4 * eax + traceBase]
			0x89, 0x18,											// 0x19 - mov [eax], ebx
			0xA1, 0x00, 0x00, 0x00, 0x00						// 0x1B - mov eax, [eaxSave]
		};

		const BYTE *pCode = code;
		BYTE *pStart = pOut;

		if (!CopyBytes<sizeof(code)>(pCode, pOut, szOut)) {
			return false;
		}

		*(UINTPTR *)&pStart[0x01] = (UINTPTR)&regEax;
		*(UINTPTR *)&pStart[0x06] = (UINTPTR)&out.traceIndex;
		*(UINTPTR *)&pStart[0x0E] = (UINTPTR)&out.traceIndex;
		*(UINTPTR *)&pStart[0x15] = (UINTPTR)&out.trace;
		*(UINTPTR *)&pStart[0x1C] = (UINTPTR)&regEax;

		return true;
	}

};