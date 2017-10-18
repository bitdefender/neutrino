#include "Neutrino.Plugin.h"

#include <vector>
#include <string>

namespace Neutrino {

	class PluginManager {
	private :
		enum class PluginState {
			LISTED, // only module name is valid
			IDENTIFIED, // module name and PluginInfo are valid
			LOADED, // all information is valid
			ERRORED // some error occured durring any loading stage
		};

		struct Plg {
			std::string moduleName;
			PluginState state;
			PluginInfo info;
			// os dependent
			//HANDLE lib;

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