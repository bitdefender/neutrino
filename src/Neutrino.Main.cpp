#include <cstdio>
#include <cstdint>

#include "ezOptionParser/ezOptionParser.h"
#include "json/json.hpp"

#include "Neutrino.Module.h"
#include "Neutrino.Plugin.Manager.h"
#include "Neutrino.Environment.h"
#include "Neutrino.Translator.h"

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
	FILE *fCfg = fopen(fName, "rt");

	if (nullptr == fCfg) {
		printf("Config file \"%s\" not found!\n", fName);
		return false;
	}

	fseek(fCfg, 0, SEEK_END);
	long sz = ftell(fCfg);

	if (sz > (MAX_CFG_SIZE)) {
		fclose(fCfg);
		printf("Config file too large!\n");
		return false;
	}

	char *cfgTxt = new char[sz + 1];
	fseek(fCfg, 0, SEEK_SET);
	long srd = fread(cfgTxt, 1, sz, fCfg);
	cfgTxt[srd] = 0;

	config = nlohmann::json::parse(cfgTxt);
	return true;
}

Neutrino::PluginManager plugins;
Neutrino::Translator translator; 
Neutrino::Environment environment(translator);



int main(int argc, const char *argv[]) {
	TIME_FREQ_T liFreq;
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


	// Parse the command line
	ParseCmdLine(argc, argv);

	// Exit fast for the simple cmdline options
	if (opt.isSet("-h")) {
		std::string usage;
		opt.getUsage(usage);
		printf("%s", usage.c_str());
		return 0;
	}

	if (opt.isSet("-l")) {
		plugins.Scan();
		plugins.IdentifyAll();
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

	Neutrino::BYTE buffer[40], *bOut = buffer, *bIn = (Neutrino::BYTE *)Payload;
	Neutrino::InstructionState state;

	environment.InitExec((Neutrino::UINTPTR)Payload);

	START_COUNTER(liStart, liFreq);
	for (int n = 0; n < 1000; ++n) {
		for (int i = 0; i < testCount; ++i) {
			environment.Go((Neutrino::UINTPTR)Payload, tests[i].length, tests[i].test);
		}
	}
	GET_COUNTER(liStart, liStop, liFreq, liTotal);
    printf("Translated runtime: %.6lf\n", liTotal);


	return 0;
}
