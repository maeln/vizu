#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#endif
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>
#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"

#define PI 3.14159265

void errorcb(int error, const char *desc)
{
    printf("GLFW error %d: %s\n", error, desc);
}

static void key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    NVG_NOTUSED(scancode);
    NVG_NOTUSED(mods);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// sin that goes from 0..1
float msin(float x)
{
    return (sinf(x) + 1.0) / 2.0;
}

void zeVoid(NVGcontext *ctx, float width, float height, float t, float skew)
{
    float middleHeight = height / 2.0;
    float middleWidth = width / 2.0;
    nvgMoveTo(ctx, 0.f, height / 2.0);
    nvgBezierTo(ctx, middleWidth / 2.0 - skew,
                (height / 2.0) * (msin(t) * 2.0),
                middleWidth / 2.0 + skew,
                (height / 2.0) * (msin(t) * 2.0),
                middleWidth, middleHeight);
    nvgBezierTo(ctx, middleWidth + (middleWidth / 2.0) + skew,
                (height / 2.0) * (msin(t + (PI)) * 2.0),
                middleWidth + (middleWidth / 2.0) - skew,
                (height / 2.0) * (msin(t + (PI)) * 2.0),
                width, middleHeight);
}

void aTri(NVGcontext *ctx, float s)
{
    nvgBeginPath(ctx);
    nvgMoveTo(ctx, -(s / 1.5), -s);
    nvgLineTo(ctx, (s / 1.5), -s);
    nvgLineTo(ctx, 0, -(s * 3.0));
    nvgLineTo(ctx, -(s / 1.5), -s);
    nvgClosePath(ctx);
}

void aBird(NVGcontext *ctx, float x, float y, float heading, float skew)
{
    float bSize = 15.0;

    nvgResetTransform(ctx);
    nvgTranslate(ctx, x + skew, y - skew);
    nvgRotate(ctx, PI / 2.0 + skew * 0.02 + heading);
    aTri(ctx, bSize);
    nvgStrokeColor(ctx, nvgRGBA(255, 0, 0, 150));
    nvgStrokeWidth(ctx, 2.0);
    nvgStroke(ctx);

    nvgResetTransform(ctx);
    nvgTranslate(ctx, x - skew, y + skew);
    nvgRotate(ctx, PI / 2.0 + skew * -0.02 + heading);
    aTri(ctx, bSize);
    nvgStrokeColor(ctx, nvgRGBA(0, 255, 0, 150));
    nvgStrokeWidth(ctx, 2.0);
    nvgStroke(ctx);

    nvgResetTransform(ctx);
    nvgTranslate(ctx, x + skew, y + skew);
    nvgRotate(ctx, PI / 2.0 + skew * 0.05 + heading);
    aTri(ctx, bSize);
    nvgStrokeColor(ctx, nvgRGBA(0, 0, 255, 150));
    nvgStrokeWidth(ctx, 2.0);
    nvgStroke(ctx);

    nvgResetTransform(ctx);
    nvgTranslate(ctx, x, y);
    nvgRotate(ctx, PI / 2.0 + heading);
    aTri(ctx, bSize);
    nvgStrokeColor(ctx, nvgRGBA(255, 255, 255, 170));
    nvgStrokeWidth(ctx, 1.0);
    nvgStroke(ctx);
}

typedef struct
{
    float heading;
    float speed;
    float x;
    float y;
} birdy;

typedef struct
{
    float sizex;
    float sizey;
    float ratiox;
    float ratioy;
} world;

float randf()
{
    return (float)rand() / RAND_MAX;
}

birdy makeMeABirdy(world *world)
{
    birdy b = {
        .heading = randf() * (PI * 2.0),
        .speed = 100.0 * randf(),
        .x = randf() * world->sizex,
        .y = randf() * world->sizey,
    };
    return b;
}

void renderBirdy(NVGcontext *ctx, world *world, birdy *bird)
{
    aBird(ctx, bird->x * world->ratiox, bird->y * world->ratioy, bird->heading, 0.5);
}

void updateBirdy(NVGcontext *ctx, world *world, birdy *bird, double dt)
{
    bird->x += cosf(bird->heading) * bird->speed * dt;
    if (bird->x > world->sizex)
    {
        bird->x = 0;
    }
    else if (bird->x < 0)
    {
        bird->x = world->sizex;
    }

    bird->y += sinf(bird->heading) * bird->speed * dt;
    if (bird->y > world->sizey)
    {
        bird->y = 0;
    }
    else if (bird->y < 0)
    {
        bird->y = world->sizey;
    }
}

int main()
{
    srand(time(NULL));
    GLFWwindow *window;
    NVGcontext *vg = NULL;

    if (!glfwInit())
    {
        printf("Failed to init GLFW.");
        return -1;
    }

    glfwSetErrorCallback(errorcb);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    window = glfwCreateWindow(1000, 600, "NanoVG", NULL, NULL);
    //	window = glfwCreateWindow(1000, 600, "NanoVG", glfwGetPrimaryMonitor(), NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key);

    glfwMakeContextCurrent(window);

    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL)
    {
        printf("Could not init nanovg.\n");
        return -1;
    }
    // glfwSwapInterval(0);
    glfwSetTime(0);

    double time = glfwGetTime();
    double dt = 0.0;

    world world = {
        .sizex = 1000,
        .sizey = 600,
        .ratiox = 1.0,
        .ratioy = 1.0,
    };

    birdy birds[10];
    for (int i = 0; i < 10; ++i)
    {
        birds[i] = makeMeABirdy(&world);
    }

    while (!glfwWindowShouldClose(window))
    {
        double mx, my;
        int winWidth, winHeight;
        int fbWidth, fbHeight;
        float pxRatio;

        dt = glfwGetTime() - time;
        time = glfwGetTime();

        glfwGetCursorPos(window, &mx, &my);
        glfwGetWindowSize(window, &winWidth, &winHeight);
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        // Calculate pixel ration for hi-dpi devices.
        pxRatio = (float)fbWidth / (float)winWidth;

        // Update and render
        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // now use real dpi size
        fbWidth /= pxRatio;
        fbHeight /= pxRatio;

        world.ratiox = world.sizex / fbWidth;
        world.ratioy = world.sizey / fbHeight;

        float skew1 = 4.f + sinf(time * 0.2);
        float skew2 = -4.f + sinf(time * 0.3);
        float skew3 = 0.f + sinf(time * 0.5);

        nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

        nvgBeginPath(vg);
        zeVoid(vg, fbWidth, fbHeight, time, skew1);
        nvgStrokeColor(vg, nvgRGBA(255, 0, 0, 150));
        nvgStrokeWidth(vg, 4.0);
        nvgStroke(vg);

        nvgBeginPath(vg);
        zeVoid(vg, fbWidth, fbHeight, time, skew2);
        nvgStrokeColor(vg, nvgRGBA(0, 255, 0, 150));
        nvgStrokeWidth(vg, 4.0);
        nvgStroke(vg);

        nvgBeginPath(vg);
        zeVoid(vg, fbWidth, fbHeight, time, skew3);
        nvgStrokeColor(vg, nvgRGBA(0, 0, 255, 150));
        nvgStrokeWidth(vg, 4.0);
        nvgStroke(vg);

        nvgBeginPath(vg);
        zeVoid(vg, fbWidth, fbHeight, time, 0.0);
        nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 170));
        nvgStrokeWidth(vg, 1.0);
        nvgStroke(vg);

        for (int i = 0; i < 10; ++i)
        {
            updateBirdy(vg, &world, &birds[i], dt);
            updateBirdy(vg, &world, &birds[i], dt);
            renderBirdy(vg, &world, &birds[i]);
            renderBirdy(vg, &world, &birds[i]);
        }

        nvgEndFrame(vg);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    nvgDeleteGL3(vg);

    glfwTerminate();
    return 0;
}
