#include "Neutrino.Plugin.h"

#include <vector>
#include <string>

#include "Neutrino.Module.h"

namespace Neutrino {

	class PluginManager {
	private :
		enum class PluginState {
			LISTED, // only module name is valid
			IDENTIFIED, // module name and PluginInfo are valid
			LOADED, // all information is valid
			ERRORED // some error occured durring any loading stage
		};

		typedef Plugin* (*PluginGetInstanceFunc)();

		struct Plg {
			std::string moduleName;
			PluginState state;
			PluginInfo info;
			module_t module;
			PluginGetInstanceFunc getInst;

			Plg(const std::string name);
			bool GetInfo();
		};

		std::vector<Plg> knownPlugins;
		Plg *FindPlugin(const char *name);
		void *GetInst(const char *name);
	public:
		bool Scan();
		void IdentifyAll();
		void PrintAll();
		
		template <typename T> T* GetInstance(const char *name) {
			return (T *)GetInst(name);
		}
	};

};