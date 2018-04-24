#include "Neutrino.System.h"

#include "Neutrino.Util.h"

#include <dlfcn.h>
#include <link.h>
#include <sys/auxv.h>

namespace Neutrino {

    void *GetVdsoSyscallAddress() {
        link_map *lm;
        Dl_info info;

        void *vdso = (void *) (uintptr_t) getauxval(AT_SYSINFO_EHDR);
        dladdr1(vdso, &info, (void **)&lm, RTLD_DL_LINKMAP);

        void *vsyscall = dlvsym(lm, "__kernel_vsyscall", "LINUX_2.5");
        return vsyscall;
    }

    bool System::MakeSystem(BYTE *&pOut, int &szOut, TranslationState &state, AbstractTranslator &translator, std::vector<std::pair<UINTPTR, UINTPTR> > &detours) {
        const BYTE pJmp[] = { 
            0x8F, 0x05, 0x00, 0x00, 0x00, 0x00,     // 0x00 - pop <retaddr>
            0xE8, 0x00, 0x00, 0x00, 0x00,           // 0x06 - call __kernel_vsyscall
            0xFF, 0x35, 0x00, 0x00, 0x00, 0x00      // 0x0B - push <retaddr>
        };
        
        const BYTE pRet[] = {
            0xC3
        };
        
        BYTE *pStart = pOut;
        UINTPTR pSrc = (UINTPTR)GetVdsoSyscallAddress();
        UINTPTR pDst = (UINTPTR)pOut;


        const BYTE *pCode = pJmp;
        CopyBytes<sizeof(pJmp)>(pCode, pOut, szOut);

        *(DWORD *)(&pStart[7]) = (DWORD)((int)pSrc - (int)&pStart[0x0B]);
        state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);
        state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x0D]);

        pCode = pRet;
        translator.Translate(pCode, pOut, szOut, state);

        detours.clear();
        detours.push_back(std::pair<UINTPTR, UINTPTR>(pSrc, pDst));

        return true;
    }

};