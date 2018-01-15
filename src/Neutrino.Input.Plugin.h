#ifndef _NEUTRINO_INPUT_PLUGIN_H_
#define _NEUTRINO_INPUT_PLUGIN_H_

#include "Neutrino.Plugin.h"
#include "Neutrino.Test.h"

namespace Neutrino {
	class InputPlugin : public Plugin {
	public :
		virtual bool IsPersistent() const = 0;
		
		virtual bool HasNextTest() = 0;
		virtual bool GetNextTest(Test &out) = 0;
	};
};


#endif
