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
	PLUGIN_EXTERN Neutrino::PluginInfo NeutrinoModuleInfo = {
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

	bool GetNextTest(Neutrino::ExternalTest &out) {
		if (cItr == std::experimental::filesystem::end(cItr)) {
			return false;
		}

		
		do {

			std::experimental::filesystem::file_status st = cItr->status();
			if (std::experimental::filesystem::is_regular_file(st)) {
				unsigned long long sz = std::experimental::filesystem::file_size(cItr->path());
				std::ifstream tStream(cItr->path(), std::ios::binary);

				out.SetSize((unsigned int)sz);

				tStream.read((char *)out.GetBuffer(), sz);
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
	static const Neutrino::ExternalTestSource states[];

	std::vector<DirectoryMonitor> dirs[2];
	std::vector<DirectoryMonitor>::iterator cDirs[2];
	int cSel;

	bool SetDirs(const nlohmann::json &cfg, const char *key, std::vector<DirectoryMonitor>& dirs);

	bool HasNextTestDir(int idx);
	bool GetNextTestDir(int idx, Neutrino::ExternalTest &out);

public :
	FileInputPlugin();

	virtual bool SetConfig(const nlohmann::json &cfg);
	virtual void ReleaseInstance();

	virtual bool IsPersistent() const;

	virtual bool HasNextTest();
	virtual bool GetNextTest(Neutrino::ExternalTest &out, Neutrino::ExternalTestSource &state);
};

const Neutrino::ExternalTestSource FileInputPlugin::states[] = { Neutrino::ExternalTestSource::NEW, Neutrino::ExternalTestSource::EXCEPTED };

bool FileInputPlugin::SetDirs(const nlohmann::json &cfg, const char *key, std::vector<DirectoryMonitor> &dirs) {
	if (cfg.find(key) == cfg.end()) {
		printf("File input plugin needs dir config\n");
		return false;
	}

	if (!cfg[key].is_array()) {
		printf("File input plugin: srcDirs field must be an array\n");
		return false;
	}

	for (auto &it : cfg[key]) {
		if (it.is_string()) {
			dirs.push_back(DirectoryMonitor(it.get<std::string>()));
		}
	}

	return true;
}

FileInputPlugin::FileInputPlugin() {
	cSel = 0;
}

bool FileInputPlugin::SetConfig(const nlohmann::json &cfg) {
	
	const char keys[][12] = { "dirs", "exceptions" };

	for (int i = 0; i < 2; ++i) {
		SetDirs(cfg, keys[i], dirs[i]);
		cDirs[i] = dirs[i].begin();
	}

	//cItr->replace_filename(cSrcDir);// = std::experimental::filesystem::directory_iterator(cSrcDir);

	return true;
}

void FileInputPlugin::ReleaseInstance() {
	delete this;
}

bool FileInputPlugin::IsPersistent() const {
	return false;
}

bool FileInputPlugin::HasNextTestDir(int idx) {
	return (cDirs[idx] != dirs[idx].end()) && (cDirs[idx]->HasNextTest());
}

bool FileInputPlugin::HasNextTest() {
	return HasNextTestDir(0) || HasNextTestDir(1);
}

bool FileInputPlugin::GetNextTestDir(int idx, Neutrino::ExternalTest &out) {
	if (cDirs[idx] == dirs[idx].end()) {
		return false;
	}

	while (!cDirs[idx]->HasNextTest()) {
		cDirs[idx]++;

		if (cDirs[idx] == dirs[idx].end()) {
			return false;
		}

		cDirs[idx]->Rewind();
	}

	return cDirs[idx]->GetNextTest(out);
}

bool FileInputPlugin::GetNextTest(Neutrino::ExternalTest &out, Neutrino::ExternalTestSource &state) {
	bool ret = GetNextTestDir(cSel, out);

	if (!ret) {
		cSel ^= 1;

		ret = GetNextTestDir(cSel, out);
	}

	state = states[cSel];

	cSel ^= 1;
	return ret;
}



extern "C" PLUGIN_EXTERN Neutrino::InputPlugin* GetInstance() {
	return new FileInputPlugin();
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
