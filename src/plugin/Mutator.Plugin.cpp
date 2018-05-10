#include "../Neutrino.Plugin.h"
#include "../Neutrino.Mutator.Plugin.h"

extern "C" {
	PLUGIN_EXTERN Neutrino::PluginInfo NeutrinoModuleInfo = {
		{ 0, 0, 1 },
		Neutrino::PluginType::MUTATOR,
		"mutator",
		"Neutrino basic test mutator."
	};
};

class MutatorPlugin : public Neutrino::MutatorPlugin {
private:
	Neutrino::TestSource *source;
	Neutrino::TestDestination *destination;
public:
	MutatorPlugin() {
		source = nullptr;
		destination = nullptr;
	}

	virtual bool SetConfig(const nlohmann::json &cfg) {
		return true;
	}

	virtual void ReleaseInstance() {
		delete this;
	}

	virtual void SetSource(Neutrino::TestSource *src) {
		source = src;
	}

	virtual void SetDestination(Neutrino::TestDestination *dst) {
		destination = dst;
	}

	void PerformBitFlip(Neutrino::ExternalTest &n) {
		static const unsigned char xMask[] = {
			0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
			0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0,
			0x0F, 0x1E, 0x3C, 0x78, 0xF0
		};

		unsigned int sz = n.GetSize();
		unsigned char *buffer = n.GetBuffer();

		for (unsigned int i = 0; i < sz; ++i) {
			for (int j = 0; j < sizeof(xMask) / sizeof(xMask[0]); ++j) {
				buffer[i] ^= xMask[j];
				destination->EnqueueTest(n);
				buffer[i] ^= xMask[j];
			}
		}
	}

	void PerformByteSet(Neutrino::ExternalTest &n) {
		static const unsigned char xVal[] = {
			0x00, 0x20, 0x30, 0x41, 0x61, 0xFF
		};

		unsigned int sz = n.GetSize();
		unsigned char *buffer = n.GetBuffer();

		for (unsigned int i = 0; i < sz; ++i) {
			unsigned char c = buffer[i];
			for (int j = 0; j < sizeof(xVal) / sizeof(xVal[0]); ++j) {
				buffer[i] = xVal[j];
				destination->EnqueueTest(n);
			}
			buffer[i] = c;
		}
	}

	void PerformDwordSet(Neutrino::ExternalTest &n) {
		static const unsigned int xVal[] = {
			0x00000000, 0x41414141, 0xFFFFFFFF
		};

		unsigned int sz = n.GetSize();
		unsigned char *buffer = n.GetBuffer();

		if (sz >= 4) {
			for (unsigned int i = 0; i < sz - 3; ++i) {
				unsigned int c = *((unsigned int *)&buffer[i]);
				for (int j = 0; j < sizeof(xVal) / sizeof(xVal[0]); ++j) {
					*((unsigned int *)&buffer[i]) = xVal[j];
					destination->EnqueueTest(n);
				}
				*((unsigned int *)&buffer[i]) = c;
			}
		}
	}

	void PerformShorten(Neutrino::ExternalTest &n) {
		int sz = n.GetSize();

		for (int i = 1; i < sz; ++i) {
			n.SetSize(i);
			destination->EnqueueTest(n);
		}

		n.SetSize(sz);
	}
	
	virtual bool Perform() {
		std::shared_ptr<Neutrino::Test> c;
		
		static Neutrino::ExternalTest n;
			
		if (!source->GetSingleTest(Neutrino::TestSource::ExtractSingleType::EXTRACT_BEST, c)) {
			return false;
		}

		n.SetSize(c->size);
		memcpy(n.GetBuffer(), c->buffer, c->size);
		
		PerformBitFlip(n);

		PerformByteSet(n);

		PerformDwordSet(n);

		PerformShorten(n);
		
		return true;
	}

};


extern "C" PLUGIN_EXTERN Neutrino::MutatorPlugin *GetInstance() {
	return new MutatorPlugin();
}



#ifdef _BUILD_WINDOWS
#include <Windows.h>
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
) {
	return TRUE;
}
#endif




