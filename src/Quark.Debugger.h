#ifndef _QUARK_DEBUGGER_H_
#define _QUARK_DEBUGGER_H_

#include <string>
#include <memory>

namespace Quark {

	class Debugger {
	private:
		class DbgImpl;

		std::unique_ptr<DbgImpl> pImpl;
	public:
		Debugger(std::string procName);
		~Debugger();
		bool Perform(std::string &crashName);
	};

};

#endif
