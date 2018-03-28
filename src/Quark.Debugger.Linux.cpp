#include "Quark.Debugger.h"

namespace Quark {

    class Debugger::DbgImpl {
    private:
        std::string processName;

    public:
        DbgImpl(std::string procName) : processName(procName) {	
		}

        bool Perform(std::string &crashName) {
            return true;
        }
    };

    Debugger::Debugger(std::string procName) : pImpl(new Debugger::DbgImpl(procName)) { }

	Debugger::~Debugger() { }

	bool Debugger::Perform(std::string &crashName) {
		return pImpl->Perform(crashName);
	}

};