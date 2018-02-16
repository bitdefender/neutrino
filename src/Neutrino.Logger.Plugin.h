#ifndef _NEUTRINO_LOGGER_PLUGIN_H_
#define _NEUTRINO_LOGGER_PLUGIN_H_

#include "Neutrino.Plugin.h"
#include "Neutrino.Test.h"

#include <ctime>

namespace Neutrino {
	struct Status {
		clock_t startTime;
		clock_t currentTime;
		int coverage;

		struct {
			int queued;
			int traced;
			int corpus;
		} tests;

		unsigned long long vmUsed;
	};

	class LoggerPlugin : public Plugin {
	public:
		virtual bool NewTest(const Status &status, const Test &test) = 0;
		virtual bool Ping(const Status &status) = 0;
		virtual bool Finish(const Status &status) = 0;
	};
};

#endif
