#ifndef _NEUTRINO_ENUM_SET_H_
#define _NEUTRINO_ENUM_SET_H_

#include <cstdarg>

namespace Neutrino {
	
	template <typename ENUM> class EnumSet {
		static const int max = (int)ENUM::ENUM_LAST;
		static const int space = (max + 0x1F) >> 5;
		unsigned int set[space];
	public:
		EnumSet() {
			memset(set, 0, sizeof(set));
		}

		EnumSet(int cnt, ...) {
			memset(set, 0, sizeof(set));
			va_list arg;

			va_start(arg, cnt);

			for (int i = 0; i < cnt; ++i) {
				ENUM e = va_arg(arg, ENUM);

				*this += e;
			}

			va_end(arg);
		}

		EnumSet<ENUM> &operator+=(ENUM e) {
			int ee = (int)e;
			set[ee >> 5] |= 1 << (ee & 0x1F);
			return *this;
		}

		EnumSet<ENUM> &operator-=(ENUM e) {
			int ee = (int)e;
			set[ee >> 5] &= ~(1 << (ee & 0x1F));
			return *this;
		}


		EnumSet<ENUM> &operator+=(const EnumSet<ENUM> &e) {
			for (int i = 0; i < space; ++i) {
				set[i] |= e.set[i];
			}
			return *this;
		}

		EnumSet<ENUM> &operator-=(const EnumSet<ENUM> &e) {
			for (int i = 0; i < space; ++i) {
				set[i] &= ~e.set[i];
			}
			return *this;
		}

		bool operator==(ENUM e) const {
			int ee = (int)e;
			return set[ee >> 5] & (1 << (ee & 0x1F));
		}

		bool operator!=(ENUM e) const {
			int ee = (int)e;
			return !(set[ee >> 5] & (1 << (ee & 0x1F)));
		}
	};

};

#endif
