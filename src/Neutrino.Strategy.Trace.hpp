#include "Neutrino.Strategy.Trace.h"

#include "Neutrino.Util.h"

namespace Neutrino {

	template <typename CODEGEN>
	TraceStrategy<CODEGEN>::TraceStrategy() {
		Reset();
	}

	template <typename CODEGEN>
	void TraceStrategy<CODEGEN>::Reset() {
		out.traceIndex = -1;
	}

	template <typename CODEGEN>
	bool TraceStrategy<CODEGEN>::TouchStatic(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest) {
		return static_cast<CODEGEN *>(this)->TouchStatic(pOut, szOut, state, dest);
	}

	template <typename CODEGEN>
	bool TraceStrategy<CODEGEN>::TouchDynamic(BYTE *&pOut, int &szOut, TranslationState &state) {
		return static_cast<CODEGEN *>(this)->TouchDynamic(pOut, szOut, state);
	}

	template <typename CODEGEN>
	void TraceStrategy<CODEGEN>::TouchDeferred() { }

	template <typename CODEGEN>
	AbstractResult *TraceStrategy<CODEGEN>::GetResult() {
		return &out;
	}

	template <typename CODEGEN>
	void TraceStrategy<CODEGEN>::PushBasicBlock(UINTPTR bb) {
		out.trace[++out.traceIndex] = bb;
	}

	template <typename CODEGEN>
	UINTPTR TraceStrategy<CODEGEN>::LastBasicBlock() const {
		return out.trace[out.traceIndex];
	}

};