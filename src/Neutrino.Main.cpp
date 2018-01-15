#include <cstdio>
#include <cstdint>

#include <vector>

#include "ezOptionParser/ezOptionParser.h"
#include "json/json.hpp"

#include "Neutrino.Module.h"
#include "Neutrino.Plugin.Manager.h"
#include "Neutrino.Environment.h"
#include "Neutrino.Strategy.Trace.h"
#include "Neutrino.Strategy.Tuple.h"
#include "Neutrino.Translator.h"

#include "Neutrino.Test.h"

#include "Neutrino.Fair.Queue.h"
#include "Neutrino.Priority.Queue.h"

#include "Neutrino.Input.Plugin.h"
#include "Neutrino.Output.Plugin.h"
#include "Neutrino.Evaluator.Plugin.h"

#include "Payload.h"
#include "Buffers.h"

#define MAX_CFG_SIZE (1 << 20)

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


typedef Neutrino::FairQueue<Neutrino::Test, 1 << 15> InputQueue;
InputQueue *testInputQueue;

Neutrino::PluginManager plugins;
Neutrino::Environment<Neutrino::TraceStrategy> traceEnvironment;
Neutrino::Environment<Neutrino::TupleStrategy> tupleEnvironment;
Neutrino::AbstractEnvironment *environment;

std::vector<Neutrino::InputPlugin *> inputPlugins;
std::vector<Neutrino::OutputPlugin *> outputPlugins;
Neutrino::EvaluatorPlugin *evaluatorPlugin = nullptr;
//Neutrino::MutatorPlugin *mutator = nullptr;

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

	testInputQueue = new InputQueue(inputPlugins.size() + 1);

	return true;
}

bool RunInputs(bool onlyPersistent) {
	for (unsigned int i = 0; i < inputPlugins.size(); ++i) {
		if (!onlyPersistent || inputPlugins[i]->IsPersistent()) {
			if (inputPlugins[i]->HasNextTest()) {
				Neutrino::Test tst;
				while (inputPlugins[i]->GetNextTest(tst)) {
					testInputQueue->Enqueue(i, tst);
				}
			}
		}
	}

	return true;
}

bool ExecuteTests() {
	Neutrino::Test test;

	environment->InitExec((Neutrino::UINTPTR)Payload);

	while (testInputQueue->Dequeue(test)) {
		environment->Go((Neutrino::UINTPTR)Payload, test.size, test.buffer);
		evaluatorPlugin->Evaluate(environment->GetResult());
	}


	return true;
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

bool Output(Neutrino::Test &test) {
	for (auto &it : outputPlugins) {
		it->WriteTest(test);
	}
	return true;
}



int main(int argc, const char *argv[]) {
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

	while (true) {
		ExecuteTests();
		RunInputs(true);
		break;
	}
	
	/*TIME_FREQ_T liFreq;
	TIME_T liStart, liStop;
	TIME_RES_T liTotal;

	GET_FREQ(liFreq);
	START_COUNTER(liStart, liFreq);
	for (int n = 0; n < 1000; ++n) {
		for (int i = 0; i < testCount; ++i) {
			Payload(tests[i].length, tests[i].test);
		}
	}
	GET_COUNTER(liStart, liStop, liFreq, liTotal);
	printf("Native runtime: %.6lf\n", liTotal);

	Neutrino::BYTE *bIn = (Neutrino::BYTE *)Payload;
	Neutrino::TranslationState state;

	traceEnvironment.InitExec((Neutrino::UINTPTR)Payload);

	START_COUNTER(liStart, liFreq);
	for (int n = 0; n < 1000; ++n) {
		for (int i = 0; i < testCount; ++i) {
			traceEnvironment.Go((Neutrino::UINTPTR)Payload, tests[i].length, tests[i].test);
		}
	}
	GET_COUNTER(liStart, liStop, liFreq, liTotal);
    printf("Translated<trace> runtime: %.6lf\n", liTotal);


	tupleEnvironment.InitExec((Neutrino::UINTPTR)Payload);
	START_COUNTER(liStart, liFreq);
	for (int n = 0; n < 1000; ++n) {
		for (int i = 0; i < testCount; ++i) {
			tupleEnvironment.Go((Neutrino::UINTPTR)Payload, tests[i].length, tests[i].test);
		}
	}
	GET_COUNTER(liStart, liStop, liFreq, liTotal);
	printf("Translated<tuple> runtime: %.6lf\n", liTotal);*/

	return 0;
}
