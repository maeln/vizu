#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#endif
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>
#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "math_utils.h"

#define PI 3.14159265

typedef struct
{
    float heading;
    float speed;
    vec2 position;
} birdy;

typedef struct
{
    vec2 size;
} world;

typedef struct
{
    vec2 viewport_size;
    vec2 viewport; // in world size ?
    vec2 position; // in world size too ?
} camera;

typedef struct
{
    vec2 viewport;
} phy_view;

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

// NOTE: for now: phy -> world is 1:1 ratio
// Should depend on a zoom

void updatePhyViewAndCamera(GLFWwindow *window, phy_view *view, camera *cam)
{
    int winWidth, winHeight;
    int fbWidth, fbHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    float pxRatio = (float)fbWidth / (float)winWidth;
    fbWidth /= pxRatio;
    fbHeight /= pxRatio;
    view->viewport.x = fbWidth;
    view->viewport.y = fbHeight;

    float vratio = (float)fbWidth / (float)fbHeight;
    cam->viewport_size.x = fbWidth;
    cam->viewport_size.y = fbHeight;
}

void updateCameraPostion(camera *cam, vec2 delta)
{
    cam->position = vec2_add(cam->position, delta);
    cam->viewport = vec2_add(cam->position, cam->viewport_size);
}

bool isInView(camera *cam, vec2 p)
{
    return p.x >= cam->position.x &&
           p.x < (cam->position.x + cam->viewport.x) &&
           p.y >= cam->position.y &&
           p.y < (cam->position.y + cam->viewport.y);
}

vec2 worldToPhy(camera *cam, phy_view *phy, vec2 p)
{
    // assume that you are within the viewport
    return new_vec2(
        map(p.x, cam->position.x, cam->viewport.x, 0.0, phy->viewport.x),
        map(p.y, cam->position.y, cam->viewport.y, 0.0, phy->viewport.y));
}

birdy makeMeABirdy(world *world)
{
    birdy b = {
        .heading = randf() * (PI * 2.0),
        .speed = 100.0 * randf(),
        .position = new_vec2(randf() * world->size.x, randf() * world->size.y),
    };
    return b;
}

void updateBirdy(NVGcontext *ctx, world *world, birdy *bird, double dt)
{
    bird->position.x += cosf(bird->heading) * bird->speed * dt;
    if (bird->position.x > world->size.x)
    {
        bird->position.x = 0;
    }
    else if (bird->position.x < 0)
    {
        bird->position.x = world->size.x;
    }

    bird->position.y += sinf(bird->heading) * bird->speed * dt;
    if (bird->position.y > world->size.y)
    {
        bird->position.y = 0;
    }
    else if (bird->position.y < 0)
    {
        bird->position.y = world->size.y;
    }
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

void worldEdges(NVGcontext *ctx, world *world, camera *cam, phy_view *view)
{
    nvgResetTransform(ctx);
    // top
    if (cam->position.y <= 0)
    {
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, -cam->position.x, -cam->position.y);
        nvgLineTo(ctx, world->size.x - cam->position.x, -cam->position.y);
        nvgClosePath(ctx);
        nvgStrokeColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgStrokeWidth(ctx, 2.0);
        nvgStroke(ctx);
    }
    // left
    if (cam->position.x <= 0)
    {
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, -cam->position.x, -cam->position.y);
        nvgLineTo(ctx, -cam->position.x, world->size.y - cam->position.y);
        nvgClosePath(ctx);
        nvgStrokeColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgStrokeWidth(ctx, 2.0);
        nvgStroke(ctx);
    }
    // bottom
    if (cam->viewport.y >= world->size.y)
    {
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, -cam->position.x,
                  view->viewport.y - (cam->viewport.y - world->size.y));
        nvgLineTo(ctx, world->size.x - cam->position.x,
                  view->viewport.y - (cam->viewport.y - world->size.y));
        nvgClosePath(ctx);
        nvgStrokeColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgStrokeWidth(ctx, 2.0);
        nvgStroke(ctx);
    }
    // right
    if (cam->viewport.x >= world->size.x)
    {
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, view->viewport.x - (cam->viewport.x - world->size.x),
                  -cam->position.y);
        nvgLineTo(ctx, view->viewport.x - (cam->viewport.x - world->size.x),
                  world->size.y - cam->position.y);
        nvgClosePath(ctx);
        nvgStrokeColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgStrokeWidth(ctx, 2.0);
        nvgStroke(ctx);
    }
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

void aBird(NVGcontext *ctx, vec2 p, float heading, float skew)
{
    float bSize = 15.0;

    nvgResetTransform(ctx);
    nvgTranslate(ctx, p.x + skew, p.y - skew);
    nvgRotate(ctx, PI / 2.0 + skew * 0.02 + heading);
    aTri(ctx, bSize);
    nvgStrokeColor(ctx, nvgRGBA(255, 0, 0, 150));
    nvgStrokeWidth(ctx, 2.0);
    nvgStroke(ctx);

    nvgResetTransform(ctx);
    nvgTranslate(ctx, p.x - skew, p.y + skew);
    nvgRotate(ctx, PI / 2.0 + skew * -0.02 + heading);
    aTri(ctx, bSize);
    nvgStrokeColor(ctx, nvgRGBA(0, 255, 0, 150));
    nvgStrokeWidth(ctx, 2.0);
    nvgStroke(ctx);

    nvgResetTransform(ctx);
    nvgTranslate(ctx, p.x + skew, p.y + skew);
    nvgRotate(ctx, PI / 2.0 + skew * 0.05 + heading);
    aTri(ctx, bSize);
    nvgStrokeColor(ctx, nvgRGBA(0, 0, 255, 150));
    nvgStrokeWidth(ctx, 2.0);
    nvgStroke(ctx);

    nvgResetTransform(ctx);
    nvgTranslate(ctx, p.x, p.y);
    nvgRotate(ctx, PI / 2.0 + heading);
    aTri(ctx, bSize);
    nvgStrokeColor(ctx, nvgRGBA(255, 255, 255, 170));
    nvgStrokeWidth(ctx, 1.0);
    nvgStroke(ctx);
}

void renderBirdy(NVGcontext *ctx, phy_view *phy, camera *cam, birdy *bird)
{
    if (isInView(cam, bird->position))
        aBird(ctx, worldToPhy(cam, phy, bird->position), bird->heading, 0.5);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
        .size = new_vec2(1024.0, 1024.0),
    };

    camera cam = {
        .position = new_vec2(0.0, 0.0),
        .viewport = new_vec2(1000.0, 600.0)};

    phy_view view = {
        .viewport = new_vec2(1000.0, 600.0)};

    birdy birds[10];
    for (int i = 0; i < 10; ++i)
    {
        birds[i] = makeMeABirdy(&world);
    }

    double mx = -1.0;
    double my = -1.0;
    double pmx, pmy;
    int winWidth, winHeight;
    int fbWidth, fbHeight;
    float pxRatio;
    while (!glfwWindowShouldClose(window))
    {
        dt = glfwGetTime() - time;
        time = glfwGetTime();

        glfwGetWindowSize(window, &winWidth, &winHeight);
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        // Calculate pixel ration for hi-dpi devices.
        pxRatio = (float)fbWidth / (float)winWidth;

        // Update and render
        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

        updatePhyViewAndCamera(window, &view, &cam);

        pmx = mx;
        pmy = my;
        glfwGetCursorPos(window, &mx, &my);
        vec2 dm = new_vec2(mx - pmx, my - pmy);
        if (pmx >= 0 && pmy >= 0)
            updateCameraPostion(&cam, dm);

        // now use real dpi size
        fbWidth /= pxRatio;
        fbHeight /= pxRatio;

        float skew1 = 4.f + sinf(time * 0.2);
        float skew2 = -4.f + sinf(time * 0.3);
        float skew3 = 0.f + sinf(time * 0.5);
        for (int i = 0; i < 10; ++i)
        {
            updateBirdy(vg, &world, &birds[i], dt);
            updateBirdy(vg, &world, &birds[i], dt);
            renderBirdy(vg, &view, &cam, &birds[i]);
            renderBirdy(vg, &view, &cam, &birds[i]);
        }

        worldEdges(vg, &world, &cam, &view);

        // printf("cam, view: (%f, %f), pos: (%f, %f)\n", cam.viewport.x, cam.viewport.y, cam.position.x, cam.position.y);

        nvgEndFrame(vg);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    nvgDeleteGL3(vg);

    glfwTerminate();
    return 0;
}
