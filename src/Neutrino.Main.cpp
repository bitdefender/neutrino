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

#include "Neutrino.Loader.h"

#include "Neutrino.Simulation.Trace.h"

#include "Neutrino.Strategy.Trace.h"
#include "Neutrino.Strategy.Trace.X86.32.h"
#include "Neutrino.Strategy.Trace.X86.64.h"

#include "Neutrino.Strategy.Tuple.h"

#include "Neutrino.Translator.X86.32.h"
#include "Neutrino.Translator.X86.64.h"

#include "Neutrino.Trampoline.X86.32.h"
#include "Neutrino.Trampoline.X86.64.h"

#include "Neutrino.System.h"

#include "Neutrino.Test.h"

#include "Neutrino.Fair.Queue.h"
#include "Neutrino.Priority.Queue.h"

#include "Neutrino.Corpus.h"

#include "Neutrino.Input.Plugin.h"
#include "Neutrino.Output.Plugin.h"
#include "Neutrino.Evaluator.Plugin.h"
#include "Neutrino.Mutator.Plugin.h"
#include "Neutrino.Logger.Plugin.h"

//#include "Payload.h"
//#include "Buffers.h"

#define MAX_CFG_SIZE (1 << 20)

#ifdef _BUILD_WINDOWS
#include "psapi.h"
#endif

#ifdef _BUILD_WINDOWS

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

#define PAYLOAD_EXTENSION "dll"

#endif

#ifdef _BUILD_LINUX

#include <sys/time.h>
#include <sys/resource.h> 

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

#define PAYLOAD_EXTENSION "so"

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

extern "C" PLUGIN_EXTERN Neutrino::Test *currentTest;

Neutrino::Test *currentTest = nullptr;

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

#ifdef _M_X64 
	extern Neutrino::Environment<Neutrino::Translator<Neutrino::TranslationTableX8664<Neutrino::TraceStrategy<Neutrino::TraceStrategyX8664> > >, Neutrino::TrampolineX8664, Neutrino::System > traceEnvironment;
	extern Neutrino::Environment<Neutrino::Translator<Neutrino::TranslationTableX8664<Neutrino::TupleStrategy> >, Neutrino::TrampolineX8664, Neutrino::System > tupleEnvironment;
#else
	extern Neutrino::Environment<Neutrino::Translator<Neutrino::TranslationTableX8632<Neutrino::TraceStrategy<Neutrino::TraceStrategyX8632> > >, Neutrino::TrampolineX8632, Neutrino::System > traceEnvironment;
	extern Neutrino::Environment<Neutrino::Translator<Neutrino::TranslationTableX8632<Neutrino::TupleStrategy> >, Neutrino::TrampolineX8632, Neutrino::System > tupleEnvironment;
#endif

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

	getTestBucket = (int)inputPlugins.size();
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

#ifdef _BUILD_WINDOWS
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc));
	
	processStatus.vmUsed = pmc.PrivateUsage;
#endif


#ifdef _BUILD_LINUX
	const char* statm_path = "/proc/self/statm";
	long pagesUsed;

	FILE *f = fopen(statm_path,"r");
	if(!f){
		return true;
	}

	if(1 != fscanf(f, "%ld", &pagesUsed)) {
		return true;
	}
	fclose(f);

	processStatus.vmUsed = pagesUsed << 12;
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
		environment->Go(test->buffer, test->size);
		currentTest = nullptr;

		test->state = Neutrino::TestState::EXECUTED;
		processStatus.tests.traced++;

		int testsSinceLastGood = processStatus.tests.traced - processStatus.tests.lastGood;
		if ((testsSinceLastGood >= (1 << 14)) && (0 == (testsSinceLastGood & (testsSinceLastGood - 1)))) { // report in increments of pow2
			Ping();
		}
		
		double ret = evaluatorPlugin->Evaluate(*test, environment->GetResult());
		
		if (0.0 < ret) {
			test->state = Neutrino::TestState::EVALUATED;
			processStatus.tests.lastGood = processStatus.tests.traced;
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

Neutrino::AbstractResult *result = nullptr;

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

	result = environment->GetResult();

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

volatile bool running = true;
Neutrino::Loader *loader = nullptr; //("./payload/fuzzer.so");

#ifdef _BUILD_WINDOWS
LRESULT CALLBACK AssertWindowMonitor(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
) {

	const char cName[] = "#32770";

	if (HSHELL_WINDOWCREATED == nCode) {
		
		char winClass[256];

		HWND hWin = (HWND)wParam;
		if (0 != GetClassName(hWin, winClass, 256)) {
			if (0 == strcmp(cName, winClass)) {
				
				EnumChildWindows(
					hWin,
					[](HWND hwnd, LPARAM lParam) -> BOOL {
						char cName[256];

						if (0 != GetWindowText(hwnd, cName, 256)) {
							if (0 == strcmp("&Retry", cName)) {
								SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
								SendMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));

								return FALSE;
							}
						}

						return TRUE;
					},
					0
				);

			}
		}
		
	}

	return 0;
}

BOOL WINAPI CtrlCHandler(DWORD dwCtrlType) {
	if (CTRL_C_EVENT == dwCtrlType) {
		fprintf(stderr, "Ctrl-C pressed... Shutting down...\n");
		fflush(stderr);
		running = false;
		return TRUE;
	}

	return FALSE;
}

void InitWindows() {
	SetConsoleCtrlHandler(CtrlCHandler, TRUE);
}
#endif

#ifdef _BUILD_LINUX

#include<signal.h>
#include<unistd.h>

void SigHandler (int signo) {
	if (signo == SIGINT) {
    	fprintf(stderr, "Ctrl-C pressed... Shutting down...\n");
		fflush(stderr);
		running = false;
	}
}

void InitLinux() {
	signal(SIGINT, SigHandler);
}
#endif

int main(int argc, const char *argv[]) {

#ifdef _BUILD_WINDOWS
	HHOOK wndHook = SetWindowsHookEx(
		WH_SHELL,
		AssertWindowMonitor,
		nullptr,
		GetCurrentThreadId()
	);

	InitWindows();
#endif

#ifdef _BUILD_LINUX
	InitLinux();
#endif

	processStatus.startTime = clock();

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

	bool isLibfuzzerCompatible = false;
	if ((config.find("libfuzzer") != config.end()) && (config["libfuzzer"].is_boolean())) {
		isLibfuzzerCompatible = config["libfuzzer"].get<bool>();
	}

	loader = new Neutrino::Loader("./payload/fuzzer." PAYLOAD_EXTENSION, isLibfuzzerCompatible);
	
	if (!loader->IsReady()) {
		printf("Payload initialization failed!\n");
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

	//environment->InitExec((Neutrino::UINTPTR) loader->GetEntry());

	environment->InitExec((Neutrino::UINTPTR) loader->GetEntry()  /*Payload*/);
	while (running) {
		ExecuteTests();
		RunInputs(true);
		if (!ExecuteMutation()) {
			break;
		}
	}

	Finish();

	corpus.Stats();

	delete loader;
	return 0;
}
