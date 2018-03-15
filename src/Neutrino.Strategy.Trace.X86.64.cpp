#include "Neutrino.Strategy.Trace.X86.64.h"

#include "Neutrino.Util.h"

namespace Neutrino {

	bool TraceStrategyX8664::TouchStatic(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest) {
		static const BYTE code[] = {
			// backup registers rax, rcx
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x00 - mov [raxSave], rax
			0x48, 0x89, 0xC8,											// 0x0A - mov rax, rcx
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x0D - mov [rcxSave], rax
			
			// increment traceIndex (using rax)
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x17 - mov rax, [traceIndex]
			0x48, 0x8D, 0x40, 0x01,										// 0x21 - lea rax, [rax + 1]
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x25 - mov [traceIndex], rax
			
			// calculate buffer pointer (into rcx)
			0x48, 0x89, 0xC1,											// 0x2F - mov rcx, rax
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x32 - mov rax, <traceBase>
			0x48, 0x8D, 0x0C, 0xC8,										// 0x3C - lea rcx, [rax + 8 * rcx]
			
			// write basic block to the buffer
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x40 - mov rax, dest
			0x48, 0x89, 0x01,											// 0x4A - mov [rcx], rax
			
			// restore registers rax, rcx
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x4D - mov rax, [rcxSave]
			0x48, 0x89, 0xC1,											// 0x57 - mov rcx, rax
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// 0x5A - mov rax, [raxSave]
		};

		const BYTE *pCode = code;
		BYTE *pStart = pOut;

		if (!CopyBytes<sizeof(code)>(pCode, pOut, szOut)) {
			return false;
		}

		*(UINTPTR *)&pStart[0x02] = (UINTPTR)&regRax;
		*(UINTPTR *)&pStart[0x0F] = (UINTPTR)&regRcx;
		*(UINTPTR *)&pStart[0x19] = (UINTPTR)&out.traceIndex;
		*(UINTPTR *)&pStart[0x27] = (UINTPTR)&out.traceIndex;
		*(UINTPTR *)&pStart[0x34] = (UINTPTR)&out.trace;
		*(UINTPTR *)&pStart[0x42] = dest;
		*(UINTPTR *)&pStart[0x4F] = (UINTPTR)&regRcx; 
		*(UINTPTR *)&pStart[0x5C] = (UINTPTR)&regRax;

		return true;
	}

	bool TraceStrategyX8664::TouchDynamic(BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE code[] = {
			// backup registers rax, rcx
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x00 - mov [raxSave], rax
			0x48, 0x89, 0xC8,											// 0x0A - mov rax, rcx
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x0D - mov [rcxSave], rax

			// increment traceIndex (using rax)
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x17 - mov rax, [traceIndex]
			0x48, 0x8D, 0x40, 0x01,										// 0x21 - lea rax, [rax + 1]
			0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x25 - mov [traceIndex], rax

			// calculate buffer pointer (into rcx)
			0x48, 0x89, 0xC1,											// 0x2F - mov rcx, rax
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x32 - mov rax, <traceBase>
			0x48, 0x8D, 0x0C, 0xC8,										// 0x3C - lea rcx, [rax + 8 * rcx]

			// write basic block to the buffer
			0x48, 0x89, 0x19,											// 0x40 - mov [rcx], rbx

			// restore registers rax, rcx
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x43 - mov rax, [rcxSave]
			0x48, 0x89, 0xC1,											// 0x4D - mov rcx, rax
			0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// 0x50 - mov rax, [raxSave]
		};

		const BYTE *pCode = code;
		BYTE *pStart = pOut;

		if (!CopyBytes<sizeof(code)>(pCode, pOut, szOut)) {
			return false;
		}

		*(UINTPTR *)&pStart[0x02] = (UINTPTR)&regRax;
		*(UINTPTR *)&pStart[0x0F] = (UINTPTR)&regRcx;
		*(UINTPTR *)&pStart[0x19] = (UINTPTR)&out.traceIndex;
		*(UINTPTR *)&pStart[0x27] = (UINTPTR)&out.traceIndex;
		*(UINTPTR *)&pStart[0x34] = (UINTPTR)&out.trace;
		*(UINTPTR *)&pStart[0x45] = (UINTPTR)&regRcx;
		*(UINTPTR *)&pStart[0x52] = (UINTPTR)&regRax;

		return true;
	}

};