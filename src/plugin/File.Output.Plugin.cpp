#include "../Neutrino.Plugin.h"
#include "../Neutrino.Output.Plugin.h"

#ifdef _MSC_VER
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

#include <vector>
#include <string>
#include <fstream>

extern "C" {
	__declspec(dllexport) Neutrino::PluginInfo NeutrinoModuleInfo = {
		{ 0, 0, 1 },
		Neutrino::PluginType::OUTPUT,
		"fileoutput",
		"Neutrino file output plugin."
	};
};

class FileOutputPlugin : public Neutrino::OutputPlugin {
private:
	std::string dir;

public:
	virtual bool SetConfig(const nlohmann::json &cfg);
	virtual void ReleaseInstance();

	virtual bool WriteTest(const Neutrino::Test &test);
};

bool FileOutputPlugin::SetConfig(const nlohmann::json &cfg) {
	if (cfg.find("dir") == cfg.end()) {
		printf("File output plugin needs dir config\n");
		return false;
	}

	if (!cfg["dir"].is_string()) {
		printf("File input plugin: srcDirs field must be an array\n");
		return false;
	}

	dir = cfg["dir"].get<std::string>();
	return true;
}

void FileOutputPlugin::ReleaseInstance() {
	delete this;
}

bool FileOutputPlugin::WriteTest(const Neutrino::Test &cfg) {
	static const char digits[] = {
		'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'a', 'b',
		'c', 'd', 'e', 'f',
	};
	
	std::string fname = dir + "/";


	for (int i = 0; i < 20; i += 4) {
		for (int j = 3; j >= 0; --j) {
			fname += digits[cfg.name.digest8[i | j] >> 0x04];
			fname += digits[cfg.name.digest8[i | j] & 0x0F];
		}
	}

	std::ofstream tStream(fname.c_str(), std::ios::binary);
	tStream.write((char *)cfg.buffer, cfg.size);

	return true;
}

extern "C" __declspec(dllexport) Neutrino::OutputPlugin* GetInstance() {
	return new FileOutputPlugin();
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