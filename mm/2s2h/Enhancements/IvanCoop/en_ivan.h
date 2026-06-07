#ifndef EN_IVAN_H
#define EN_IVAN_H

#include "global.h"
#include "objects/gameplay_keep/gameplay_keep.h"

struct EnIvan;

typedef struct EnIvan {
    /* 0x000 */ Actor actor;
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ Vec3s jointTable[FAIRY_LIMB_MAX];
    /* 0x1B2 */ Vec3s morphTable[FAIRY_LIMB_MAX];
    /* 0x1DC */ ColliderCylinder collider;
    /* 0x228 */ Color_RGBAf innerColor;
    /* 0x238 */ Color_RGBAf outerColor;
    /* 0x248 */ LightInfo lightInfoGlow;
    /* 0x258 */ LightNode* lightNodeGlow;
    /* 0x25C */ LightInfo lightInfoNoGlow;
    /* 0x26C */ LightNode* lightNodeNoGlow;
    /* 0x270 */ f32 yVelocity;
    /* 0x274 */ u8 shouldDraw;
    /* 0x276 */ s16 shotTimer;
} EnIvan;

void EnIvan_Init(Actor* thisx, PlayState* play);
void EnIvan_Destroy(Actor* thisx, PlayState* play);
void EnIvan_Update(Actor* thisx, PlayState* play);
void EnIvan_Draw(Actor* thisx, PlayState* play);

#endif // EN_IVAN_H
