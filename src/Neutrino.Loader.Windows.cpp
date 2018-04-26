#include "Neutrino.Loader.h"

#include <Windows.h>
#include <process.h>

namespace Neutrino {

    class Loader::LoaderImpl {
    private:
    	HMODULE hMod;

        FnInitEx _init;
        FnUninit _uninit;
        FnSubmit _submit;

        int initRet;
    public:
		LoaderImpl(std::string libName, bool libFuzzerCompatible) {
			SetErrorMode(SEM_FAILCRITICALERRORS);
			_set_error_mode(_OUT_TO_STDERR); // disable assert dialog boxes
			_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
			_set_app_type(_crt_console_app);

			initRet = -1;

			UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
			SetErrorMode(oldErrorMode | SEM_FAILCRITICALERRORS);

			hMod = LoadLibraryA(libName.c_str());

			SetErrorMode(oldErrorMode);

			if (libFuzzerCompatible) {
				_submit = (FnSubmit)GetProcAddress(hMod, "LLVMFuzzerTestOneInput");

				if ((nullptr == _submit)) {
					return;
				}

				initRet = 0;
			} else {
				_init = (FnInitEx)GetProcAddress(hMod, "FuzzerInit");
				_uninit = (FnUninit)GetProcAddress(hMod, "FuzzerUninit");
				_submit = (FnSubmit)GetProcAddress(hMod, "FuzzerSubmit");

				if ((nullptr == _submit)) {
					return;
				}

				if ((nullptr != _init) && (nullptr != _uninit)) {
					initRet = _init();
				}
				else {
					initRet = 0;
				}
			}
		}

        ~LoaderImpl() {
			if (_uninit) {
				_uninit();
			}
		    FreeLibrary(hMod);
        }

        bool IsReady() const {
            return (0 == initRet);
        }

        FnSubmit GetEntry() const {
            return _submit;
        }
    };

    Loader::Loader(std::string libName, bool libFuzzerCompatible) : pImpl(new Loader::LoaderImpl(libName, libFuzzerCompatible)) { }
    Loader::~Loader() {}

    bool Loader::IsReady() const {
        return pImpl->IsReady();
    }

    FnSubmit Loader::GetEntry() const {
        return pImpl->GetEntry();
    }

};