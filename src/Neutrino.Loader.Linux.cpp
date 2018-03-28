#include "Neutrino.Loader.h"

#include <dlfcn.h>

namespace Neutrino {

    class Loader::LoaderImpl {
    private:
    	void *hMod;

        FnInitEx _init;
        FnUninit _uninit;
        FnSubmit _submit;

        int initRet;
    public:
        LoaderImpl(std::string libName) {
            hMod = dlopen(libName.c_str(), RTLD_LAZY);

            _init = (FnInitEx)dlsym(hMod, "FuzzerInit");
            _uninit = (FnUninit)dlsym(hMod, "FuzzerUninit");
            _submit = (FnSubmit)dlsym(hMod, "FuzzerSubmit");

            initRet = _init();
        }

        ~LoaderImpl() {
            _uninit();
		    dlclose(hMod);
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