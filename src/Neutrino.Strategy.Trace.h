#ifndef _NEUTRINO_STRATEGY_TRACE_H_
#define _NEUTRINO_STRATEGY_TRACE_H_

#include "Neutrino.Types.h"
#include "Neutrino.Translator.h"
#include "Neutrino.Result.h"

namespace Neutrino {

	class TraceOutput : public AbstractResult {
	public :
		int traceIndex;
		UINTPTR trace[1 << 26];
	};

	class TraceStrategy {
	private:
		TraceOutput out;
		UINTPTR regBackup;
	public:
		TraceStrategy();

		void Reset();
		bool TouchStatic(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest);
		bool TouchDynamic(BYTE *&pOut, int &szOut, TranslationState &state);
		void TouchDeferred();

		AbstractResult *GetResult();

		void PushBasicBlock(UINTPTR bb);
		UINTPTR LastBasicBlock() const;
	};
};

#endif