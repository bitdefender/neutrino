#include <cstdio>
#include <cstdint>
#include <ctime>

#include <vector>

#include <memory>

#include "ezOptionParser/ezOptionParser.h"
#include "json/json.hpp"

#include "Neutrino.Module.h"
#include "Neutrino.Plugin.Manager.h"
#include "Neutrino.Environment.h"

#include "Neutrino.Simulation.Trace.h"

#include "Neutrino.Strategy.Trace.h"
#include "Neutrino.Strategy.Tuple.h"
#include "Neutrino.Translator.h"

#include "Neutrino.Test.h"

#include "Neutrino.Fair.Queue.h"
#include "Neutrino.Priority.Queue.h"

#include "Neutrino.Corpus.h"

#include "Neutrino.Input.Plugin.h"
#include "Neutrino.Output.Plugin.h"
#include "Neutrino.Evaluator.Plugin.h"
#include "Neutrino.Mutator.Plugin.h"
#include "Neutrino.Logger.Plugin.h"

#include "Payload.h"
#include "Buffers.h"

#define MAX_CFG_SIZE (1 << 20)

#ifdef _MSC_VER
#include "psapi.h"
#endif

#ifdef _MSC_VER

typedef LARGE_INTEGER TIME_T;
typedef LARGE_INTEGER TIME_FREQ_T;
typedef double TIME_RES_T;
#define START_COUNTER(starttime, freq) { QueryPerformanceCounter(&(starttime)); }
#define GET_COUNTER_AGGREGATE(starttime, endtime, freq, total) { QueryPerformanceCounter(&(endtime)); \
	total += (endtime).QuadPart - (starttime).QuadPart; }
#define GET_COUNTER(starttime, endtime, freq, total) { QueryPerformanceCounter(&(endtime)); \
	total = 1000.0 * ((endtime).QuadPart - (starttime).QuadPart) / freq.QuadPart; }
#define GET_AGGREGATE_RESULT(total, freq, result) { \
	result = 1000.0 * (total) / (freq).QuadPart; }
#define GET_FREQ(freq) { QueryPerformanceFrequency(&(freq)); }

#else

typedef struct timespec TIME_T;
typedef double TIME_FREQ_T;
typedef double TIME_RES_T;
#define START_COUNTER(starttime, freq) { clock_gettime(CLOCK_REALTIME, &(starttime)); }
#define GET_COUNTER(starttime, endtime, freq, res) { clock_gettime(CLOCK_REALTIME, &(endtime)); \
	res = ((endtime).tv_sec - (starttime).tv_sec) + ((endtime).tv_nsec - (starttime).tv_nsec) / freq; }
#define GET_COUNTER_AGGREGATE(starttime, endtime, freq, res) { clock_gettime(CLOCK_REALTIME, &(endtime)); \
	res += ((endtime).tv_sec - (starttime).tv_sec) + ((endtime).tv_nsec - (starttime).tv_nsec) / freq; }
#define GET_AGGREGATE_RESULT(total, freq, result) { result = total; }
#define GET_FREQ(freq) { (freq) = 1E9; }

#endif

using namespace nlohmann;

Neutrino::Status processStatus;
ez::ezOptionParser opt;

void ParseCmdLine(int argc, const char *argv[]) {
	opt.overview = "Neutrino (ultra)fast fuzzer.";
	opt.syntax = "neutrino [OPTIONS]";
	opt.example = "neutrino -c <config_file> -p <payload> [--inprocess|--extern]\n";

	opt.add(
		"", // Default.
		0, // Required?
		0, // Number of args expected.
		0, // Delimiter if expecting multiple args.
		"Display usage instructions.", // Help description.
		"-h",     // Flag token. 
		"--help", // Flag token.
		"--usage" // Flag token.
	);

	opt.add(
		"", // Default.
		0, // Required?
		0, // Number of args expected.
		0, // Delimiter if expecting multiple args.
		"Display plugin list.", // Help description.
		"-l",     // Flag token. 
		"--list", // Flag token.
		"--plugins" // Flag token.
	);

	opt.add(
		"", 
		0, 
		1, 
		0, 
		"Configuration file", 
		"-c", 
		"--config"
	);

	opt.add(
		"",
		0,
		1,
		0,
		"Payload binary",
		"-p",
		"--payload"
	);

	opt.add(
		"",
		0,
		0,
		0,
		"Use inprocess execution.",
		"--inprocess"
	);

	opt.add(
		"",
		0,
		0,
		0,
		"Use extern execution.",
		"--extern"
	);

	opt.parse(argc, argv);
}

nlohmann::json config;

bool ParseConfigFile(const char *fName) {
	std::ifstream sCfg(fName);

	if (!sCfg) {
		printf("Config file \"%s\" not found!\n", fName);
		return false;
	}

	try {
		sCfg >> config;
	} catch (std::invalid_argument msg) {
		printf("%s error: %s\n", fName, msg.what());
		return false;
	}
	
	return true;
}

Neutrino::Corpus corpus;
typedef Neutrino::FairQueue<std::shared_ptr<Neutrino::Test>, 1 << 15> InputQueue;
InputQueue *testInputQueue;
int getTestBucket = 0;

extern "C" __declspec(dllexport) Neutrino::Test *currentTest = nullptr;

class MutatorDestinationAdapter : public Neutrino::TestDestination {
private :
	InputQueue &queue;
public :
	MutatorDestinationAdapter(InputQueue &q) : queue(q) { }

	virtual bool EnqueueTest(Neutrino::Test &test) {
		auto itm = corpus.AddTest(test);

		if (itm->state == Neutrino::TestState::NEW) {
			processStatus.tests.queued++;
			return queue.Enqueue(getTestBucket, itm);
		}

		return true;
	}

	virtual bool RequeueTest(double priority, Neutrino::Test &test) {
		return false;
	}
} *dstAdapter;

typedef Neutrino::PriorityQueue<std::shared_ptr<Neutrino::Test>, 1 << 16> MutationQueue;
MutationQueue testMutationQueue;

class MutatorSourceAdapter : public Neutrino::TestSource {
private :
	MutationQueue &queue;
public :
	MutatorSourceAdapter(MutationQueue &q) : queue(q) { }

	virtual int GetAvailableTestCount() const {
		return queue.Count();
	}
	
	virtual bool GetSingleTest(ExtractSingleType eType, std::shared_ptr<Neutrino::Test> &test) {
		double priority;
		switch (eType) {
			case ExtractSingleType::EXTRACT_BEST :
				return queue.Dequeue(priority, test);
			case ExtractSingleType::EXTRACT_PROBABILISTIC :
				return false;
			case ExtractSingleType::EXTRACT_RANDOM :
				return false;
			default :
				return false;
		}
	}
} srcAdapter(testMutationQueue);


Neutrino::PluginManager plugins;
Neutrino::AbstractEnvironment *environment;

#ifdef NEUTRINO_SIMULATION
Neutrino::SimulationTraceEnvironment traceEnvironment;
Neutrino::SimulationTupleEnvironment tupleEnvironment;
#else
Neutrino::Environment<Neutrino::TraceStrategy> traceEnvironment;
Neutrino::Environment<Neutrino::TupleStrategy> tupleEnvironment;
#endif

std::vector<Neutrino::InputPlugin *> inputPlugins;
std::vector<Neutrino::OutputPlugin *> outputPlugins;
std::vector<Neutrino::LoggerPlugin *> loggerPlugins;
Neutrino::EvaluatorPlugin *evaluatorPlugin = nullptr;
Neutrino::MutatorPlugin *mutatorPlugin = nullptr;

bool InitializeInputs(const std::string &cfgFile) {
	if (config.find("inputs") == config.end()) {
		printf("No input plugins specified in %s\n", cfgFile.c_str());
		return false;
	}

	if (!config["inputs"].is_object()) {
		printf("%s: inputs field must be an object\n", cfgFile.c_str());
		return false;
	}

	nlohmann::json iCfg = config["inputs"];

	for (json::iterator it = iCfg.begin(); it != iCfg.end(); ++it) {
		Neutrino::InputPlugin *tmp = plugins.GetInstance<Neutrino::InputPlugin>(it.key().c_str());

		if (!tmp->SetConfig(it.value())) {
			tmp->ReleaseInstance();
		} else {
			inputPlugins.push_back(tmp);
		}
	}

	getTestBucket = inputPlugins.size();
	testInputQueue = new InputQueue(getTestBucket + 1);
	dstAdapter = new MutatorDestinationAdapter(*testInputQueue);
	return true;
}

bool RunInputs(bool onlyPersistent) {
	for (unsigned int i = 0; i < inputPlugins.size(); ++i) {
		if (!onlyPersistent || inputPlugins[i]->IsPersistent()) {
			if (inputPlugins[i]->HasNextTest()) {
				Neutrino::Test tst;
				while (inputPlugins[i]->GetNextTest(tst)) {
					auto itm = corpus.AddTest(tst);

					if (itm->state == Neutrino::TestState::NEW) {
						testInputQueue->Enqueue(i, itm);
						processStatus.tests.queued++;
					}
				}
			}
		}
	}

	return true;
}

bool Output(Neutrino::Test &test) {
	for (auto &it : outputPlugins) {
		it->WriteTest(test);
	}
	return true;
}

bool UpdateProcessStatus() {
	processStatus.currentTime = clock();

	processStatus.coverage = environment->GetCoverage();

#ifdef _MSC_VER
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc));
	
	processStatus.vmUsed = pmc.PrivateUsage;
#else
	processStatus.vmUsed = 0;
#endif

	return true;
}

bool Log(Neutrino::Test &test) {
	if (!UpdateProcessStatus()) {
		return false;
	}

	for (auto &it : loggerPlugins) {
		it->NewTest(processStatus, test);
	}
	return true;
}

bool Ping() {
	if (!UpdateProcessStatus()) {
		return false;
	}

	for (auto &it : loggerPlugins) {
		it->Ping(processStatus);
	}
	return true;
}

bool Finish() {
	if (!UpdateProcessStatus()) {
		return false;
	}

	for (auto &it : loggerPlugins) {
		it->Finish(processStatus);
	}
	return true;
}

bool ExecuteTests() {
	std::shared_ptr<Neutrino::Test> test;

	while (testInputQueue->Dequeue(test)) {
		processStatus.tests.queued--;

		currentTest = test.get();
		environment->Go(test->size, test->buffer);
		currentTest = nullptr;

		test->state = Neutrino::TestState::EXECUTED;
		processStatus.tests.traced++;

		if (0 == (processStatus.tests.traced % 10000)) {
			Ping();
		}
		
		double ret = evaluatorPlugin->Evaluate(*test, environment->GetResult());
		
		if (0.0 < ret) {
			test->state = Neutrino::TestState::EVALUATED;
			processStatus.tests.corpus++;
			Output(*test);
			Log(*test);
		} else {
			//test->state = Neutrino::TestState::DISCARDED;
		}

		testMutationQueue.Enqueue(ret, test);
	}

	return true;
}

bool ExecuteMutation() {
	return mutatorPlugin->Perform();
}

bool InitializeOutputs(const std::string &cfgFile) {
	if (config.find("outputs") == config.end()) {
		printf("No output plugins specified in %s\n", cfgFile.c_str());
		return false;
	}

	if (!config["outputs"].is_object()) {
		printf("%s: outputs field must be an object\n", cfgFile.c_str());
		return false;
	}

	nlohmann::json iCfg = config["outputs"];

	for (json::iterator it = iCfg.begin(); it != iCfg.end(); ++it) {
		Neutrino::OutputPlugin *tmp = plugins.GetInstance<Neutrino::OutputPlugin>(it.key().c_str());

		if (!tmp->SetConfig(it.value())) {
			tmp->ReleaseInstance();
		}
		else {
			outputPlugins.push_back(tmp);
		}
	}

	return true;
}

bool InitializeLoggers(const std::string &cfgFile) {
	if (config.find("loggers") == config.end()) {
		return true;
	}

	if (!config["loggers"].is_object()) {
		printf("%s: loggers field must be an object\n", cfgFile.c_str());
		return false;
	}

	nlohmann::json iCfg = config["loggers"];

	for (json::iterator it = iCfg.begin(); it != iCfg.end(); ++it) {
		Neutrino::LoggerPlugin *tmp = plugins.GetInstance<Neutrino::LoggerPlugin>(it.key().c_str());

		if (!tmp->SetConfig(it.value())) {
			tmp->ReleaseInstance();
		}
		else {
			loggerPlugins.push_back(tmp);
		}
	}

	return true;
}

bool InitializeEvaluator(const std::string &cfgFile) {
	if (config.find("evaluator") == config.end()) {
		printf("No evaluator plugin specified in %s\n", cfgFile.c_str());
		return false;
	}

	if (!config["evaluator"].is_object()) {
		printf("%s: evaluator field must be an object\n", cfgFile.c_str());
		return false;
	}

	nlohmann::json iCfg = config["evaluator"];

	if (iCfg.find("plugin") == iCfg.end()) {
		printf("evaluator.plugin must specify a plugin\n");
		return false;
	}

	nlohmann::json iPlg = iCfg["plugin"];

	Neutrino::EvaluatorPlugin *tmp = plugins.GetInstance<Neutrino::EvaluatorPlugin>(iPlg.get<std::string>().c_str());

	if (!tmp) {
		printf("Couldn't get instance of %s plugin\n", iPlg.get<std::string>().c_str());
		return false;
	}

	if (!tmp->SetConfig(iCfg)) {
		tmp->ReleaseInstance();
		printf("Couldn't configure %s plugin\n", iPlg.get<std::string>().c_str());
		return false;
	}

	evaluatorPlugin = tmp;

	const Neutrino::EnumSet<Neutrino::ResultType> *res = tmp->GetInputType();

	if (*res == Neutrino::ResultType::TRACE) {
		environment = &traceEnvironment;
	} else if (*res == Neutrino::ResultType::TUPLE) {
		environment = &tupleEnvironment;
	} else {
		printf("Unsupported evaluator input type\n");
		return false;
	}

	return true;
}

bool InitializeMutator(const std::string &cfgFile) {
	if (config.find("mutator") == config.end()) {
		printf("No mutator plugin specified in %s\n", cfgFile.c_str());
		return false;
	}

	if (!config["mutator"].is_object()) {
		printf("%s: mutator field must be an object\n", cfgFile.c_str());
		return false;
	}

	nlohmann::json iCfg = config["mutator"];

	if (iCfg.find("plugin") == iCfg.end()) {
		printf("mutator.plugin must specify a plugin\n");
		return false;
	}

	nlohmann::json iPlg = iCfg["plugin"];

	Neutrino::MutatorPlugin *tmp = plugins.GetInstance<Neutrino::MutatorPlugin>(iPlg.get<std::string>().c_str());

	if (!tmp) {
		printf("Couldn't get instance of %s plugin\n", iPlg.get<std::string>().c_str());
		return false;
	}

	if (!tmp->SetConfig(iCfg)) {
		tmp->ReleaseInstance();
		printf("Couldn't configure %s plugin\n", iPlg.get<std::string>().c_str());
		return false;
	}

	tmp->SetSource(&srcAdapter);
	tmp->SetDestination(dstAdapter);

	mutatorPlugin = tmp;
	return true;
}

#define FUZZER_CALL	__cdecl

typedef int(FUZZER_CALL *FnInitEx)();
typedef	int(FUZZER_CALL *FnUninit)();
typedef	int(FUZZER_CALL *FnSubmit)(const unsigned int buffSize, const unsigned char *buffer);


#include <process.h>

class LibLoader {
private :
	HMODULE hMod;

	FnInitEx _init;
	FnUninit _uninit;
	FnSubmit _submit;

	int initRet;
public :
	LibLoader(const char *libName) {
		SetErrorMode(SEM_FAILCRITICALERRORS);
		_set_error_mode(_OUT_TO_STDERR); // disable assert dialog boxes
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
		_set_app_type(_crt_console_app);


		hMod = LoadLibraryA(libName);

		_init = (FnInitEx)GetProcAddress(hMod, "FuzzerInit");
		_uninit = (FnUninit)GetProcAddress(hMod, "FuzzerUninit");
		_submit = (FnSubmit)GetProcAddress(hMod, "FuzzerSubmit");

		initRet = _init();
	}

	bool IsReady() const {
		return (0 == initRet);
	}

	FnSubmit GetEntry() const {
		return _submit;
	}

	~LibLoader() {
		_uninit();
		FreeLibrary(hMod);
	}
};

LibLoader loader("./payload/fuzzer.dll");

int main(int argc, const char *argv[]) {
	processStatus.startTime = clock();


	if (!loader.IsReady()) {
		printf("Payload initialization failed!\n");
		return 0;
	}

	// Parse the command line
	ParseCmdLine(argc, argv);

	// Exit fast for the simple cmdline options
	if (opt.isSet("-h")) {
		std::string usage;
		opt.getUsage(usage);
		printf("%s", usage.c_str());
		return 0;
	}


	plugins.Scan();
	plugins.IdentifyAll();
	if (opt.isSet("-l")) {
		plugins.PrintAll();
		return 0;
	}

	// Parse the config file

	std::string cfgFile;
	if (opt.isSet("-c")) {
		opt.get("-c")->getString(cfgFile);
	} else {
		cfgFile = "neutrino.json";
	}

	if (!ParseConfigFile(cfgFile.c_str())) {
		return 0;
	}

	// Do some meaningful work
	if (!InitializeInputs(cfgFile)) {
		return 0;
	}

	if (!InitializeEvaluator(cfgFile)) {
		return 0;
	}

	if (!RunInputs(false)) {
		return 0;
	}

	if (!InitializeOutputs(cfgFile)) {
		return 0;
	}

	if (!InitializeLoggers(cfgFile)) {
		return 0;
	}

	if (!InitializeMutator(cfgFile)) {
		return 0;
	}

	int c = 0;

	environment->InitExec((Neutrino::UINTPTR) loader.GetEntry()  /*Payload*/);
	while (true) {
		ExecuteTests();
		RunInputs(true);
		if (!ExecuteMutation()) {
			break;
		}
	}

	Finish();

	corpus.Stats();

	return 0;
}
