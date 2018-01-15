#include "../Neutrino.Plugin.h"
#include "../Neutrino.Input.Plugin.h"

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
		Neutrino::PluginType::INPUT,
		"fileinput",
		"Neutrino file input plugin."
	};
};

class DirectoryMonitor {
private :
	std::string dirName;
	std::experimental::filesystem::directory_iterator cItr;
public :
	DirectoryMonitor(const std::string &dir) : dirName(dir) {
		Rewind();
	}

	void Rewind() {
		cItr = std::experimental::filesystem::directory_iterator(dirName);
	}

	bool HasNextTest() {
		return cItr != std::experimental::filesystem::end(cItr);
	}

	bool GetNextTest(Neutrino::Test &out) {
		if (cItr == std::experimental::filesystem::end(cItr)) {
			return false;
		}

		
		do {

			std::experimental::filesystem::file_status st = cItr->status();
			if (std::experimental::filesystem::is_regular_file(st)) {
				unsigned long long sz = std::experimental::filesystem::file_size(cItr->path());
				std::ifstream tStream(cItr->path(), std::ios::binary);

				out.size = (int)sz;
				out.buffer = new unsigned char[out.size];

				tStream.read((char *)out.buffer, out.size);
				cItr++;

				return true;
			} else {
				cItr++;
			}
		} while (true);
	}
};

class FileInputPlugin : public Neutrino::InputPlugin {
private :
	std::vector<DirectoryMonitor> dirs;
	std::vector<DirectoryMonitor>::iterator cDir;

public :
	virtual bool SetConfig(const nlohmann::json &cfg);
	virtual void ReleaseInstance();

	virtual bool IsPersistent() const;

	virtual bool HasNextTest();
	virtual bool GetNextTest(Neutrino::Test &out);
};

bool FileInputPlugin::SetConfig(const nlohmann::json &cfg) {
	if (cfg.find("dirs") == cfg.end()) {
		printf("File input plugin needs dir config\n");
		return false;
	}

	if (!cfg["dirs"].is_array()) {
		printf("File input plugin: dirs field must be an array\n");
		return false;
	}

	for (auto &it : cfg["dirs"]) {
		if (it.is_string()) {
			dirs.push_back(DirectoryMonitor(it.get<std::string>()));
		}
	}

	cDir = dirs.begin();
	//cItr->replace_filename(cDir);// = std::experimental::filesystem::directory_iterator(cDir);

	return true;
}

void FileInputPlugin::ReleaseInstance() {
	delete this;
}

bool FileInputPlugin::IsPersistent() const {
	return false;
}

bool FileInputPlugin::HasNextTest() {
	return (cDir != dirs.end()) && (cDir->HasNextTest());
}

bool FileInputPlugin::GetNextTest(Neutrino::Test &out) {

	while (!cDir->HasNextTest()) {
		cDir++;

		if (cDir == dirs.end()) {
			return false;
		}

		cDir->Rewind();
	}

	return cDir->GetNextTest(out);
}



extern "C" __declspec(dllexport) Neutrino::InputPlugin* GetInstance() {
	return new FileInputPlugin();
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
