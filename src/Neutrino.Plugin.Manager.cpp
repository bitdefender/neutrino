#include "Neutrino.Plugin.Manager.h"

#include <vector>
#include <filesystem>

#define PLUGIN_DIR "./plugins"

// FOR NOW, replace with OS independent header
#include <Windows.h>

typedef void *(*GetInstanceFunc)();

inline Neutrino::PluginManager::Plg::Plg(const std::string name) : moduleName(name) {
	state = PluginState::LISTED;
}

inline bool Neutrino::PluginManager::Plg::GetInfo() {
	switch (state) {
		case PluginState::ERRORED :
			return false;
		case PluginState::IDENTIFIED :
		case PluginState::LOADED :
			return true;
	}

	HMODULE hModule;
	UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	SetErrorMode(oldErrorMode | SEM_FAILCRITICALERRORS); 
	
	hModule = LoadLibrary(moduleName.c_str());

	SetErrorMode(oldErrorMode);

 	if (nullptr == hModule) {
		state = PluginState::ERRORED;
		return false;
	}

	PluginInfo *pInfo = (PluginInfo *)GetProcAddress(hModule, "NeutrinoModuleInfo");
	if (nullptr == pInfo) {
		state = PluginState::ERRORED;
		return false;
	}

	memcpy(&info, pInfo, sizeof(info));
	state = PluginState::IDENTIFIED;

	FreeLibrary(hModule);
	return true;
}

Neutrino::PluginManager::Plg *Neutrino::PluginManager::FindPlugin(const char *name) {
	for (auto &p : knownPlugins) {
		if (((p.state == PluginState::IDENTIFIED) || (p.state == PluginState::LOADED)) &&
			(0 == strcmp(p.info.name, name))) 
		{
			return &p;
		}
	}
	return nullptr;
}

void *Neutrino::PluginManager::GetInst(const char *name) {
	Plg *plg = FindPlugin(name);

	if (nullptr == plg) {
		return nullptr;
	}

	HMODULE hMod;
	if (plg->state == PluginState::IDENTIFIED) {
		hMod = LoadLibrary(plg->moduleName.c_str());
	}

	GetInstanceFunc iFunc = (GetInstanceFunc)GetProcAddress(hMod, "GetInstance");

	return iFunc();
}

bool Neutrino::PluginManager::Scan() {
	for (auto &p : std::experimental::filesystem::directory_iterator(PLUGIN_DIR)) {
		knownPlugins.push_back(Plg(p.path().u8string()));
	}

	return true;
}

void Neutrino::PluginManager::IdentifyAll() {
	for (auto &p : knownPlugins) {
		p.GetInfo();
	}
}

void Neutrino::PluginManager::PrintAll() {
	char types[][16] = {
		"Unknown"
	};

	printf("\nNeutrino plugin list\n\n");
	printf("Name                 Version    Type      Description\n");
	printf("------------------------------------------------------------------------------\n");

	for (auto &p : knownPlugins) {
		if ((p.state == PluginState::IDENTIFIED) || (p.state == PluginState::LOADED)) {
			printf("%-20s %d.%d.%d\t%-10s%s\n",
				p.info.name,
				p.info.version.major, p.info.version.minor, p.info.version.patch,
				types[(unsigned int)p.info.type],
				p.info.description
			);
		}
	}
}
