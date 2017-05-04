#pragma once

#include "Texture.h"
#include "Util.h"

struct Animation
{
    int currFrame;
    int numFrames;
    float animRate;
    float accum;
    ivec2 animStart;
};
