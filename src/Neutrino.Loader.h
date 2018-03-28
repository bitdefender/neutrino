#ifndef _NEUTRINO_LOADER_H_
#define _NEUTRINO_LOADER_H_

#include <string>
#include <memory>

namespace Neutrino {

#ifdef _BUILD_WINDOWS
    #define FUZZER_CALL	__cdecl
#endif

#ifdef _BUILD_LINUX
    #define FUZZER_CALL __attribute__((__cdecl__))
#endif

    typedef int(FUZZER_CALL *FnInitEx)();
    typedef	int(FUZZER_CALL *FnUninit)();
    typedef	int(FUZZER_CALL *FnSubmit)(const unsigned int buffSize, const unsigned char *buffer);

    class Loader {
    private:
        class LoaderImpl;

        std::unique_ptr<LoaderImpl> pImpl;
    public:
        Loader(std::string libName);
    	~Loader();

        bool IsReady() const;
	    FnSubmit GetEntry() const;
    };

};

#endif
