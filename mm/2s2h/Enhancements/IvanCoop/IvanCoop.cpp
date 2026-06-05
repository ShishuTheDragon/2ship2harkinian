#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenGui/BenMenu.h"

extern "C" {
#include "variables.h"
#include "z64player.h"
}
#include "en_ivan.h"

using namespace UIWidgets;

#define CVAR_NAME "gModes.IvanCoop"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

namespace BenGui {

void BenMenu::AddIvanSettings(WidgetPath& path) {
    AddWidget(path, "Ivan (Co-op Mode)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_NAME)
        .Options(CheckboxOptions().Tooltip(
            "Enables Ivan the Fairy. Player 2 controls a fairy companion using the second controller. "
            "Takes effect on the next scene load."));
}

} // namespace BenGui

void RegisterIvanCoop() {
    COND_HOOK(OnSceneInit, CVAR, [](s8 sceneId, s8 spawnNum) {
        Player* player = GET_PLAYER(gPlayState);
        Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_IVAN, player->actor.world.pos.x,
                    player->actor.world.pos.y + Player_GetHeight(player) + 5.0f, player->actor.world.pos.z, 0, 0, 0,
                    1); // params=1 → reads controller 2
    });
}

static RegisterShipInitFunc initFunc(RegisterIvanCoop, { CVAR_NAME });
