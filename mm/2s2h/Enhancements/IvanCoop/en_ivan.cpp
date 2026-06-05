#include "en_ivan.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

void EnIvan_Init(Actor* thisx, PlayState* play);
void EnIvan_Destroy(Actor* thisx, PlayState* play);
void EnIvan_Update(Actor* thisx, PlayState* play);
void EnIvan_Draw(Actor* thisx, PlayState* play);

ActorProfile En_Ivan_Profile = {
    /**/ ACTOR_EN_IVAN,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnIvan),
    /**/ EnIvan_Init,
    /**/ EnIvan_Destroy,
    /**/ EnIvan_Update,
    /**/ EnIvan_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 8, ICHAIN_STOP),
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_NONE | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_ON,
    },
    { 12, 27, 0, { 0, 0, 0 } },
};

static Vec3f sSparkleVelocity = { 0.0f, -0.05f, 0.0f };
static Vec3f sSparkleAccel = { 0.0f, -0.025f, 0.0f };

static void EnIvan_SpawnSparkles(EnIvan* self, PlayState* play) {
    Vec3f pos;
    Color_RGBA8 primColor;
    Color_RGBA8 envColor;

    pos.x = Rand_CenteredFloat(6.0f) + self->actor.world.pos.x;
    pos.y = (Rand_ZeroOne() * 6.0f) + self->actor.world.pos.y + 5.0f;
    pos.z = Rand_CenteredFloat(6.0f) + self->actor.world.pos.z;

    primColor.r = (u8)self->innerColor.r;
    primColor.g = (u8)self->innerColor.g;
    primColor.b = (u8)self->innerColor.b;
    primColor.a = 255;

    envColor.r = (u8)self->outerColor.r;
    envColor.g = (u8)self->outerColor.g;
    envColor.b = (u8)self->outerColor.b;
    envColor.a = 255;

    EffectSsKirakira_SpawnDispersed(play, &pos, &sSparkleVelocity, &sSparkleAccel, &primColor, &envColor, 1500, 12);
}

static void EnIvan_UpdateLights(EnIvan* self, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 glowRadius = self->shouldDraw ? 100 : 0;
    s16 ambientRadius = self->shouldDraw ? 200 : 0;

    Lights_PointGlowSetInfo(&self->lightInfoGlow, self->actor.world.pos.x, self->actor.world.pos.y + 9.0f,
                            self->actor.world.pos.z, 200, 255, 200, glowRadius);
    Lights_PointNoGlowSetInfo(&self->lightInfoNoGlow, player->actor.world.pos.x,
                              (s16)(player->actor.world.pos.y) + 69, player->actor.world.pos.z, 200, 255, 200,
                              ambientRadius);
}

void EnIvan_Init(Actor* thisx, PlayState* play) {
    EnIvan* self = (EnIvan*)thisx;

    Actor_ProcessInitChain(thisx, sInitChain);

    Collider_InitCylinder(play, &self->collider);
    Collider_SetCylinder(play, &self->collider, thisx, &sCylinderInit);
    Collider_UpdateCylinder(thisx, &self->collider);

    SkelAnime_Init(play, &self->skelAnime, (SkeletonHeader*)gameplay_keep_Skel_02AF58,
                   (AnimationHeader*)gameplay_keep_Anim_029140, self->jointTable, self->morphTable, FAIRY_LIMB_MAX);
    ActorShape_Init(&thisx->shape, 0.0f, NULL, 15.0f);

    Lights_PointGlowSetInfo(&self->lightInfoGlow, thisx->world.pos.x, thisx->world.pos.y, thisx->world.pos.z, 200, 255,
                            200, 0);
    self->lightNodeGlow = LightContext_InsertLight(play, &play->lightCtx, &self->lightInfoGlow);
    Lights_PointNoGlowSetInfo(&self->lightInfoNoGlow, thisx->world.pos.x, thisx->world.pos.y, thisx->world.pos.z, 200,
                              255, 200, 0);
    self->lightNodeNoGlow = LightContext_InsertLight(play, &play->lightCtx, &self->lightInfoNoGlow);

    self->innerColor.r = 255.0f;
    self->innerColor.g = 255.0f;
    self->innerColor.b = 255.0f;
    self->innerColor.a = 255.0f;

    self->outerColor.r = 0.0f;
    self->outerColor.g = 255.0f;
    self->outerColor.b = 0.0f;
    self->outerColor.a = 255.0f;

    self->yVelocity = 0.0f;
    self->shouldDraw = true;

    thisx->room = -1; // persist through room transitions
    thisx->terminalVelocity = -20.0f;
    thisx->gravity = 0.0f;
}

void EnIvan_Destroy(Actor* thisx, PlayState* play) {
    EnIvan* self = (EnIvan*)thisx;

    LightContext_RemoveLight(play, &play->lightCtx, self->lightNodeGlow);
    LightContext_RemoveLight(play, &play->lightCtx, self->lightNodeNoGlow);
    Collider_DestroyCylinder(play, &self->collider);
}

void EnIvan_Update(Actor* thisx, PlayState* play) {
    EnIvan* self = (EnIvan*)thisx;
    Input* input = &play->state.input[thisx->params]; // params=1 → controller 2

    f32 stickX = input->cur.stick_x;
    f32 stickY = input->cur.stick_y;
    f32 desiredSpeed = sqrtf(stickX * stickX + stickY * stickY) / 10.0f;

    // Rotate to face movement direction, camera-relative (matches player actor convention)
    if (desiredSpeed > 0.01f) {
        s16 stickAngle = Math_Atan2S_XY(stickY, -stickX);
        s16 worldYaw = Camera_GetInputDirYaw(GET_ACTIVE_CAM(play)) + stickAngle;
        Math_SmoothStepToS(&thisx->world.rot.y, worldYaw, 2, 10000, 0);
        thisx->shape.rot.y = thisx->world.rot.y;
    }

    Math_SmoothStepToF(&thisx->speed, desiredSpeed, 1.0f, 1.3f, 0.0f);

    // Vertical movement: A rises, B sinks, else float back to still
    if (CHECK_BTN_ALL(input->cur.button, BTN_A)) {
        Math_SmoothStepToF(&self->yVelocity, 6.0f, 1.0f, 1.5f, 0.0f);
    } else if (CHECK_BTN_ALL(input->cur.button, BTN_B)) {
        Math_SmoothStepToF(&self->yVelocity, -6.0f, 1.0f, 1.5f, 0.0f);
    } else {
        Math_SmoothStepToF(&self->yVelocity, 0.0f, 1.0f, 1.5f, 0.0f);
    }

    // Zero velocity.y each frame so gravity acts as direct Y velocity setter
    thisx->velocity.y = 0.0f;
    thisx->gravity = self->yVelocity;

    Actor_MoveWithGravity(thisx);
    Actor_UpdateBgCheckInfo(play, thisx, 19.0f, 20.0f, 0.0f, 5);

    // Z-trigger: snap back to Link
    if (CHECK_BTN_ALL(input->cur.button, BTN_Z)) {
        Player* player = GET_PLAYER(play);
        thisx->world.pos = player->actor.world.pos;
        thisx->world.pos.y += Player_GetHeight(player) + 5.0f;
        thisx->speed = 0.0f;
        self->yVelocity = 0.0f;
        thisx->velocity.y = 0.0f;
    } else {
        Collider_UpdateCylinder(thisx, &self->collider);
        CollisionCheck_SetOC(play, &play->colChkCtx, &self->collider.base);
    }

    if (self->shouldDraw) {
        EnIvan_SpawnSparkles(self, play);
    }

    SkelAnime_Update(&self->skelAnime);
    EnIvan_UpdateLights(self, play);
}

void EnIvan_Draw(Actor* thisx, PlayState* play) {
    EnIvan* self = (EnIvan*)thisx;

    if (!self->shouldDraw) {
        return;
    }

    Gfx* dListHead = (Gfx*)GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Gfx));

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL27_Xlu(play->state.gfxCtx);

    s32 envAlpha = (50) & 0x1FF;
    envAlpha = (envAlpha > 255) ? 511 - envAlpha : envAlpha;

    gSPSegment(POLY_XLU_DISP++, 0x08, (uintptr_t)dListHead);
    gDPPipeSync(dListHead++);
    gDPSetPrimColor(dListHead++, 0, 0x01, (u8)self->innerColor.r, (u8)self->innerColor.g, (u8)self->innerColor.b,
                    (u8)self->innerColor.a);
    gDPSetRenderMode(dListHead++, G_RM_PASS, G_RM_ZB_CLD_SURF2);
    gSPEndDisplayList(dListHead++);

    gDPSetEnvColor(POLY_XLU_DISP++, (u8)self->outerColor.r, (u8)self->outerColor.g, (u8)self->outerColor.b,
                   (u8)envAlpha);

    POLY_XLU_DISP = SkelAnime_Draw(play, self->skelAnime.skeleton, self->skelAnime.jointTable, NULL, NULL, thisx,
                                   POLY_XLU_DISP);

    CLOSE_DISPS(play->state.gfxCtx);
}
