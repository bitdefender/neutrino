#include <cstdio>
#include <cstdint>

#include "ezOptionParser/ezOptionParser.h"
#include "json/json.hpp"

#include "Neutrino.Module.h"
#include "Neutrino.Plugin.Manager.h"

#define MAX_CFG_SIZE (1 << 20)

using namespace nlohmann;

int Payload(int in) {
	if (in < 0) {
		return  100 - in;
	} else {
		return in * in - 50;
	}
}

struct JumpTable {
	uintptr_t orig; // original jump destination
	uintptr_t hook; // new destination
};

class Translator {
private :
public :
	uintptr_t nextBb;
};


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

	Neutrino::MODULE_T OpenModule();


	return 0;
}