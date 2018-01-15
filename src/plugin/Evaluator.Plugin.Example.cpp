#include "../Neutrino.Plugin.h"
#include "../Neutrino.Evaluator.Plugin.h"

extern "C" {
	__declspec(dllexport) Neutrino::PluginInfo NeutrinoModuleInfo = {
		{ 0, 0, 1 },
		Neutrino::PluginType::EVALUATOR,
		"evaluatorexample",
		"Neutrino evaluator plugin example."
	};
};

class EvaluatorPlugin : public Neutrino::EvaluatorPlugin {
private :
	const Neutrino::EnumSet<Neutrino::ResultType> iType;
public :
	EvaluatorPlugin() : iType(1, Neutrino::ResultType::TRACE) {
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

	virtual double Evaluate(Neutrino::AbstractResult *input) {
		return 1.0f;
	}

};


extern "C" __declspec(dllexport) Neutrino::EvaluatorPlugin* GetInstance() {
	return new EvaluatorPlugin();
}



#ifdef _MSC_VER
#include <Windows.h>
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
) {
	return TRUE;
}
#endif




