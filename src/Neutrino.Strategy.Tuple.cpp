#include "Neutrino.Strategy.Tuple.h"

#include "Neutrino.Util.h"

namespace Neutrino {
	WORD TupleStrategy::Hash(UINTPTR x) {
		x = ((x >> 16) ^ x) * 0x45d9f3b;
		x = ((x >> 16) ^ x) * 0x45d9f3b;
		x = (x >> 16) ^ x;
		return (WORD)x;
	}

	TupleStrategy::TupleStrategy() {
		Reset();
	}

	void TupleStrategy::Reset() {
		out.lastBlock = 0;
		out.lastHash = 0;
		memset(out.tuple, 0, sizeof(out.tuple));
	}

	bool TupleStrategy::TouchStatic(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest) {
		static const BYTE code[] = {
			0xA3, 0x00, 0x00, 0x00, 0x00,								// 0x00 - mov [eaxSave], eax
			0x89, 0x15, 0x00, 0x00, 0x00, 0x00,							// 0x05 - mov [edxSave], edx
			0xA1, 0x00, 0x00, 0x00, 0x00,								// 0x0B - mov eax, [lastHash]
			0x8D, 0x80, 0x00, 0x00, 0x00, 0x00,							// 0x10 - lea eax, [eax + hashDest]
			0x0F, 0xB7, 0xC0,											// 0x16 - movzx eax, ax
			0x8D, 0x80, 0x00, 0x00, 0x00, 0x00,							// 0x19 - lea eax, [eax + tupleBase] 
			0x8B, 0x10,													// 0x1F - mov edx, [eax]
			0x8D, 0x52, 0x01,											// 0x21 - lea edx, [edx + 1]
			0x89, 0x10,													// 0x24 - mov [eax], edx
			0xC7, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x26 - mov [lastHash], <hashDest >> 1>
			0x8B, 0x15, 0x00, 0x00, 0x00, 0x00,							// 0x30 - mov edx, [edxSave]
			0xA1, 0x00, 0x00, 0x00, 0x00,								// 0x36 - mov eax, [eaxSave]
			0xC7, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// 0x3B - mov [lastBB], dest
		};

		const BYTE *pCode = code;
		BYTE *pStart = pOut;

		if (!CopyBytes<sizeof(code)>(pCode, pOut, szOut)) {
			return false;
		}

		WORD hash = Hash(dest);
		*(UINTPTR *)&pStart[0x01] = (UINTPTR)&regPtr1;
		*(UINTPTR *)&pStart[0x07] = (UINTPTR)&regPtr2;
		*(UINTPTR *)&pStart[0x0C] = (UINTPTR)&out.lastHash;
		*(UINTPTR *)&pStart[0x12] = hash;
		*(UINTPTR *)&pStart[0x1B] = (UINTPTR)&out.tuple;
		*(UINTPTR *)&pStart[0x28] = (UINTPTR)&out.lastHash;
		*(UINTPTR *)&pStart[0x2C] = hash >> 1;
		*(UINTPTR *)&pStart[0x32] = (UINTPTR)&regPtr2;
		*(UINTPTR *)&pStart[0x37] = (UINTPTR)&regPtr1;
		*(UINTPTR *)&pStart[0x3D] = (UINTPTR)&out.lastBlock;
		*(UINTPTR *)&pStart[0x41] = dest;

		return true;
	}

	bool TupleStrategy::TouchDynamic(BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE code[] = {
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00						// 0x00 - mov [lastBB], ebx
		};

		const BYTE *pCode = code;
		BYTE *pStart = pOut;

		if (!CopyBytes<sizeof(code)>(pCode, pOut, szOut)) {
			return false;
		}

		*(UINTPTR *)&pStart[0x02] = (UINTPTR)&out.lastBlock;
		
		return true;
	}

	AbstractResult *TupleStrategy::GetResult() {
		return &out;
	}

	void TupleStrategy::PushBasicBlock(UINTPTR bb) {
		out.lastBlock = bb;
	}

	UINTPTR TupleStrategy::LastBasicBlock() const {
		return out.lastBlock;
	}

	void TupleStrategy::TouchDeferred() {
		WORD newHash = Hash(out.lastBlock);
		*(UINTPTR *)&out.tuple[(newHash + out.lastHash) & 0xFFFF] += 1;
		out.lastHash = newHash >> 1;
	}
};