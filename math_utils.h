#pragma once

#include <time.h>
#include <stdlib.h>
#include <math.h>

float map(float value, float low1, float high1, float low2, float high2)
{
    return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

// sin that goes from 0..1
float msin(float x)
{
    return (sinf(x) + 1.0) / 2.0;
}

float randf()
{
    return (float)rand() / RAND_MAX;
}

typedef struct
{
    float x;
    float y;
} vec2;

vec2 new_vec2(float x, float y)
{
    vec2 v = {
        .x = x,
        .y = y};
    return v;
}

vec2 vec2_add(vec2 a, vec2 b)
{
    return new_vec2(a.x + b.x, a.y + b.y);
}