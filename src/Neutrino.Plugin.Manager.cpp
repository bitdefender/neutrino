#include "Neutrino.Plugin.Manager.h"

#include <cstring>
#include <vector>

#ifdef _MSC_VER
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

#define PLUGIN_DIR "./plugins"


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

	module_t module = OpenModule(moduleName.c_str());

 	if (nullptr == module) {
		state = PluginState::ERRORED;
		return false;
	}

	PluginInfo *pInfo = (PluginInfo *)FindFunction(module, "NeutrinoModuleInfo");
	if (nullptr == pInfo) {
		state = PluginState::ERRORED;
		return false;
	}

	memcpy(&info, pInfo, sizeof(info));
	state = PluginState::IDENTIFIED;

	CloseModule(module);
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

	if (plg->state == PluginState::ERRORED) {
		return nullptr;
	}

	if (plg->state == PluginState::IDENTIFIED) {
		plg->module = OpenModule(plg->moduleName.c_str());
		// if (plg->module) not loaded

		plg->getInst = (PluginGetInstanceFunc)FindFunction(plg->module, "GetInstance");
		if (nullptr == plg->getInst) {
			CloseModule(plg->module);
			plg->state = PluginState::ERRORED;
			return nullptr;
		}

		plg->state = PluginState::LOADED;
	}

	return plg->getInst();
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
		"Unknown",
		"Input",
		"Output",
		"Evaluator",
		"Mutator"
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
