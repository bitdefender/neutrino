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
        LoaderImpl(std::string libName) {
            SetErrorMode(SEM_FAILCRITICALERRORS);
            _set_error_mode(_OUT_TO_STDERR); // disable assert dialog boxes
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
            _set_app_type(_crt_console_app);


            hMod = LoadLibraryA(libName.c_str());

            _init = (FnInitEx)GetProcAddress(hMod, "FuzzerInit");
            _uninit = (FnUninit)GetProcAddress(hMod, "FuzzerUninit");
            _submit = (FnSubmit)GetProcAddress(hMod, "FuzzerSubmit");

            initRet = _init();
        }

        ~LoaderImpl() {
            _uninit();
		    FreeLibrary(hMod);
        }

        bool IsReady() const {
            return (0 == initRet);
        }

        FnSubmit GetEntry() const {
            return _submit;
        }
    };

    Loader::Loader(std::string libName) : pImpl(new Loader::LoaderImpl(libName)) { }
    Loader::~Loader() {}

    bool Loader::IsReady() const {
        return pImpl->IsReady();
    }

    FnSubmit Loader::GetEntry() const {
        return pImpl->GetEntry();
    }

};