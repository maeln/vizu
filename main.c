#include <stdio.h>
#include <GL/glew.h>
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
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

int main()
{
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

    window = glfwCreateWindow(1000, 600, "NanoVG", NULL, NULL);
    //	window = glfwCreateWindow(1000, 600, "NanoVG", glfwGetPrimaryMonitor(), NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key);

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Could not init glew.\n");
        return -1;
    }
    // GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
    glGetError();

    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL)
    {
        printf("Could not init nanovg.\n");
        return -1;
    }
    glfwSwapInterval(0);
    glfwSetTime(0);

    double time = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        double mx, my;
        int winWidth, winHeight;
        int fbWidth, fbHeight;
        float pxRatio;

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

        float middleHeight = (float)fbHeight / 2.0;
        float middleWidth = (float)fbWidth / 2.0;

        nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, middleHeight);
        nvgBezierTo(vg, middleWidth / 2.0,
                    ((float)fbHeight / 2.0) * (msin(time) * 2.0),
                    middleWidth / 2.0,
                    ((float)fbHeight / 2.0) * (msin(time) * 2.0),
                    middleWidth, middleHeight);
        nvgBezierTo(vg, middleWidth + (middleWidth / 2.0),
                    ((float)fbHeight / 2.0) * (msin(time + (PI)) * 2.0),
                    middleWidth + (middleWidth / 2.0),
                    ((float)fbHeight / 2.0) * (msin(time + (PI)) * 2.0),
                    (float)fbWidth, middleHeight);
        // nvgCircle(vg, 50.f * sinf(time) + 150.f, 50.f * cosf(time) + 150.f, 8.f);
        // nvgFillColor(vg, nvgRGBA(123, 99, 34, 255));
        nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 255));
        nvgStrokeWidth(vg, 3.0);
        // nvgFill(vg);
        nvgStroke(vg);

        nvgEndFrame(vg);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    nvgDeleteGL3(vg);

    glfwTerminate();
    return 0;
}
