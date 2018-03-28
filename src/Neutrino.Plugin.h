#ifndef _NEUTRINO_PLUGIN_H_
#define _NEUTRINO_PLUGIN_H_

#include "json/json.hpp"

#ifdef _BUILD_WINDOWS
#include "Neutrino.Plugin.Windows.h"
#endif

#ifdef _BUILD_LINUX
#include "Neutrino.Plugin.Linux.h"
#endif

namespace Neutrino {
	enum class PluginType : unsigned int {
		UNKNOWN, // Add plugin 
		INPUT,
		OUTPUT,
		EVALUATOR,
		MUTATOR,
		LOGGING
	};

	struct PluginVersion {
		unsigned char major;
		unsigned char minor;
		unsigned short patch;
	};

	class PluginInfo {
	public:
		PluginVersion version;
		PluginType type;
		char name[64];
		char description[256];
	};

	class Plugin {
	public:
		virtual bool SetConfig(const nlohmann::json &cfg) = 0;
		virtual void ReleaseInstance() = 0;
	};
};

#endif
