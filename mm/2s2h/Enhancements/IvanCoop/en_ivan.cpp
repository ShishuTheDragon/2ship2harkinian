#include "en_ivan.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

ActorProfile En_Ivan_Profile = {
    /**/ ACTOR_EN_IVAN,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnIvan),
    /**/ NULL,
    /**/ NULL,
    /**/ NULL,
    /**/ NULL,
};
