#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "overlays/actors/ovl_Elf_Msg6/z_elf_msg6.h"

void RegisterSkipLongestCutscene() {
    // On the way to Woodfall, when Tatl reminisces about Skull Kid
    COND_VB_SHOULD(VB_TATL_INTERRUPT_MSG6, true, {
        Actor* actor = va_arg(args, Actor*);
        if (*should && actor->csId == 9) {
            *should = false;
            Flags_SetSwitch(gPlayState, ELFMSG6_SWITCH_FLAG(actor));
            Actor_Kill(actor);

            auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
            gui->GetGameOverlay()->TextDrawNotification(3.0f, true, "Skipped longest cutscene");
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterSkipLongestCutscene);
