#include "../Neutrino.Plugin.h"
#include "../Neutrino.Evaluator.Plugin.h"
#include "../Neutrino.Strategy.Trace.h"

#include "../modules/Module.Layout.h"

#include <unordered_set>
#include <unordered_map>
#include <utility>

extern "C" {
	PLUGIN_EXTERN Neutrino::PluginInfo NeutrinoModuleInfo = {
		{ 0, 0, 1 },
		Neutrino::PluginType::EVALUATOR,
		"traceevaluator",
		"Neutrino basic trace evaluator."
	};
};

class TraceEvaluatorPlugin : public Neutrino::EvaluatorPlugin {
private:
	const Neutrino::EnumSet<Neutrino::ResultType> iType;
	//Neutrino::ModuleLayout *memLayout;

	using BlockEntry = std::pair<Neutrino::UINTPTR, Neutrino::UINTPTR>;

	struct BlockEntryHash {
		inline std::size_t operator()(const BlockEntry & v) const {
			return v.first * 31 + v.second;
		}
	};

	std::unordered_map<BlockEntry, int, BlockEntryHash> knownBlocks;

public:
	TraceEvaluatorPlugin() : iType(1, Neutrino::ResultType::TRACE) {
		//memLayout = vmem::CreateMemoryLayout(GetCurrentProcess());
	}

	virtual bool SetConfig(const nlohmann::json &cfg) {
		return true;
	}

	virtual void ReleaseInstance() {
		delete this;
	}

	virtual const Neutrino::EnumSet<Neutrino::ResultType> *GetInputType() const {
		return &iType; //Neutrino::EvaluatorInputType::TRACE;
	}

	virtual double Evaluate(const Neutrino::Test &test, const Neutrino::AbstractResult *result) {
		const Neutrino::TraceOutput *trace = static_cast<const Neutrino::TraceOutput *>(result);

		std::unordered_set<BlockEntry, BlockEntryHash> localBlocks;

		int totalBlocks = 0, newBlocks = 0, betterBlocks = 0;

		for (int i = 0; i < trace->traceIndex; ++i) {
			Neutrino::UINTPTR base = 0, offset = trace->trace[i];
			/*vmem::MemoryRegionInfo mInfo;
			if (!memLayout->Query((void *)trace->trace[i], mInfo)) {

			}*/

			auto q = std::make_pair(base, offset);

			if (localBlocks.end() == localBlocks.find(q)) {
				localBlocks.insert(q);
			}
		}

		totalBlocks = localBlocks.size();

		for (auto &it : localBlocks) {
			auto entry = knownBlocks.find(it);
			if (knownBlocks.end() == entry) {
				newBlocks++;
				knownBlocks.insert(std::make_pair(it, test.size));
			} else if (entry->second > test.size) {
				betterBlocks++;
				entry->second = test.size;
			}
		}

		return 0.4 * newBlocks + 0.6 * betterBlocks;
	}

};


extern "C" PLUGIN_EXTERN Neutrino::EvaluatorPlugin* GetInstance() {
	return new TraceEvaluatorPlugin();
}



#ifdef _BUILD_WINDOWS
#include <Windows.h>
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
) {
	return TRUE;
}
#endif




