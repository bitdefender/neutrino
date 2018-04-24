#ifndef _NEUTRINO_SYSTEM_H_
#define _NEUTRINO_SYSTEM_H_

#include "Neutrino.Types.h"
#include "Neutrino.Abstract.Translator.h"

#include <vector>

namespace Neutrino {

    class System {
    public :
        static bool MakeSystem(BYTE *&pOut, int &szOut, TranslationState &state, AbstractTranslator &translator, std::vector<std::pair<UINTPTR, UINTPTR> > &detours);
    };

};


#endif
