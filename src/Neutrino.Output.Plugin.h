#ifndef _NEUTRINO_OUTPUT_PLUGIN_H_
#define _NEUTRINO_OUTPUT_PLUGIN_H_

#include "Neutrino.Plugin.h"
#include "Neutrino.Test.h"

namespace Neutrino {
	class OutputPlugin : public Plugin {
	public:
		virtual bool WriteTest(const Neutrino::Test &test) = 0;
	};
};


#endif
