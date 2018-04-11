#include "Neutrino.Loader.h"

#include <dlfcn.h>

namespace Neutrino {

    class Loader::LoaderImpl {
    private:
    	void *hMod;

        FnInitEx _init;
        FnUninit _uninit;
        FnSubmit _submit;

        FnLibfuzzerSubmit _libfuzzersubmit;

        int initRet;
    public:
        LoaderImpl(std::string libName, bool libFuzzerCompatible) {
            hMod = dlopen(libName.c_str(), RTLD_LAZY);

            if (nullptr == hMod) {
                printf("dlopen() error %s\n", dlerror());
                return;
            }

            if (libFuzzerCompatible) {
                _libfuzzersubmit = (FnLibfuzzerSubmit)dlsym(hMod, "LLVMFuzzerTestOneInput");
                if ((nullptr == _libfuzzersubmit)) {
                    return;
                }

		        initRet = 0;
            } else {

                _init = (FnInitEx)dlsym(hMod, "FuzzerInit");
                _uninit = (FnUninit)dlsym(hMod, "FuzzerUninit");
                _submit = (FnSubmit)dlsym(hMod, "FuzzerSubmit");

                if ((nullptr == _submit)) {
                    return;
                }

                if ((nullptr != _init) && (nullptr != _uninit)) {
                    initRet = _init();
                } else {
                    initRet = 0;
                }
            }
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

        FnLibfuzzerSubmit GetLibfuzzerEntry() const {
			return _libfuzzersubmit;
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

	FnLibfuzzerSubmit Loader::GetLibfuzzerEntry() const {
		return pImpl->GetLibfuzzerEntry();
	}

};