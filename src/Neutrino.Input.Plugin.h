#ifndef _NEUTRINO_INPUT_PLUGIN_H_
#define _NEUTRINO_INPUT_PLUGIN_H_

#include "Neutrino.Plugin.h"
#include "Neutrino.External.Test.h"

namespace Neutrino {

	class InputPlugin : public Plugin {
	public :
		virtual bool IsPersistent() const = 0;
		
		virtual bool HasNextTest() = 0;
		virtual bool GetNextTest(ExternalTest &buffer, ExternalTestSource &source) = 0;
	};
};


#endif
