#include "../Neutrino.Plugin.h"
#include "../Neutrino.Logger.Plugin.h"

#include <cstdio>

#include <io.h>
#include <fcntl.h>


extern "C" {
	__declspec(dllexport) Neutrino::PluginInfo NeutrinoModuleInfo = {
		{ 0, 0, 1 },
		Neutrino::PluginType::LOGGING,
		"consolelogger",
		"Neutrino console logging plugin."
	};
};

class ConsoleLoggerPlugin : public Neutrino::LoggerPlugin {
private:
	FILE *fOut;

	struct RunTime {
		int hours;
		int minutes;
		int seconds;
		int millisec;
	};

	void SplitTime(const Neutrino::Status &status, RunTime &rt) {
		unsigned long long r = 1000ull * (status.currentTime - status.startTime) / CLOCKS_PER_SEC;

		rt.millisec = r % 1000; r /= 1000;
		rt.seconds = r % 60; r /= 60;
		rt.minutes = r % 60; r /= 60;
		rt.hours = (int)r;
	}

	struct MemSize {
		static const char sfx[][3];

		const char *suffix;
		double size;
	};

	void ParseMemSize(const Neutrino::Status &status, MemSize &ms) {
		int idx = 0;
		ms.size = (double)status.vmUsed;

		while (ms.size > 1024.0) {
			idx++;
			ms.size /= 1024.0;
		}

		ms.suffix = ms.sfx[idx];
	}

	void LogStatus(const char *verb, const Neutrino::Status &status, bool nl = true);

public:
	ConsoleLoggerPlugin();

	virtual bool SetConfig(const nlohmann::json &cfg);
	virtual void ReleaseInstance();

	virtual bool NewTest(const Neutrino::Status &status, const Neutrino::Test &test);
	virtual bool Ping(const Neutrino::Status &status);
	virtual bool Finish(const Neutrino::Status &status);
};

const char ConsoleLoggerPlugin::MemSize::sfx[][3] = {
	"", "KB", "MB", "GB"
};

void ConsoleLoggerPlugin::LogStatus(const char *verb, const Neutrino::Status &status, bool nl) {
	RunTime rt;
	SplitTime(status, rt);

	MemSize ms;
	ParseMemSize(status, ms);

	fprintf(fOut, "[%02d:%02d:%02d.%03d] %-6s queued:%6d, traced:%8d, corpus:%6d, coverage:%7d, memory:%8.2f%s%c",
		rt.hours, rt.minutes, rt.seconds, rt.millisec,
		verb,
		status.tests.queued,
		status.tests.traced,
		status.tests.corpus,
		status.coverage,
		ms.size, ms.suffix,
		nl ? '\n' : ' '
	);
	fflush(fOut);
}

ConsoleLoggerPlugin::ConsoleLoggerPlugin() {
	int hn = _dup(_fileno(stdout));
	fOut = _fdopen(hn, "a+");

	freopen("output.txt", "wt", stdout);
}

bool ConsoleLoggerPlugin::SetConfig(const nlohmann::json &cfg) {
	return true;
}

void ConsoleLoggerPlugin::ReleaseInstance() {
	delete this;
}

bool ConsoleLoggerPlugin::NewTest(const Neutrino::Status &status, const Neutrino::Test &test) {
	LogStatus("NEW", status, false);
	fprintf(fOut, "test: %08x...\n", test.name.digest32[0]);
	fflush(fOut);

	return true;
}

bool ConsoleLoggerPlugin::Ping(const Neutrino::Status &status) {
	LogStatus("PING", status);
	return true;
}


bool ConsoleLoggerPlugin::Finish(const Neutrino::Status &status) {
	LogStatus("DONE", status);
	return true;
}


extern "C" __declspec(dllexport) Neutrino::LoggerPlugin* GetInstance() {
	return new ConsoleLoggerPlugin();
}



#ifdef _MSC_VER
#include <Windows.h>
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
) {
	return TRUE;
}
#endif