#ifndef _NEUTRINO_RESULT_H_
#define _NEUTRINO_RESULT_H_

namespace Neutrino {
	enum class ResultType : int {
		TRACE, // classic execution trace
		TUPLE, // AFL-inspired transition pairs
		ENUM_LAST
	};

	class AbstractResult {
	public:
		int type;
	};

};

#endif
