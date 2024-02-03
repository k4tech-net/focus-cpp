//
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#define GL_SILENCE_DEPRECATION

#include <stdio.h>
#ifdef NANOVG_GLEW
#	include <GL/glew.h>
#endif
#ifdef __APPLE__
#	define GLFW_INCLUDE_GLCOREARB
#endif
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>
#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"
#include "./demo.h"
#include "./perf.h"


struct AppState
{
    bool blowup = false;
    bool screenshot = false;
    bool premult = false;
};
AppState gAppState;


struct TimeHelper
{
    double previousTime;

    TimeHelper()
    {
        previousTime = glfwGetTime();
    }

    double now()
    {
        return glfwGetTime();
    }

    double getDeltaTime()
    {
        double now = glfwGetTime();
        double dt = now - previousTime;
        previousTime = now;
        return dt;
    }
};


struct PerfLogs
{
    GPUtimer gpuTimer;
    PerfGraph fps, cpuGraph, gpuGraph;
    float gpuTimes[3];

    void init()
    {
        initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
        initGraph(&cpuGraph, GRAPH_RENDER_MS, "CPU Time");
        initGraph(&gpuGraph, GRAPH_RENDER_MS, "GPU Time");
        initGPUTimer(&gpuTimer);
    }

    void onFrameStart()
    {
        startGPUTimer(&gpuTimer);
    }

    void myRenderGraph(NVGcontext* vg)
    {
        renderGraph(vg, 5,5, &fps);
        renderGraph(vg, 5+200+5,5, &cpuGraph);
        if (gpuTimer.supported)
            renderGraph(vg, 5+200+5+200+5,5, &gpuGraph);
    }

    void onFrameEnd(float dt, float cpuTime)
    {
        updateGraph(&fps, dt);
        updateGraph(&cpuGraph, cpuTime);

        // We may get multiple results.
        int n = stopGPUTimer(&gpuTimer, gpuTimes, 3);
        for (int i = 0; i < n; i++)
            updateGraph(&gpuGraph, gpuTimes[i]);
    }

    void myPrintf()
    {
        printf("Average Frame Time: %.2f ms\n", getGraphAverage(&fps) * 1000.0f);
        printf("          CPU Time: %.2f ms\n", getGraphAverage(&cpuGraph) * 1000.0f);
        printf("          GPU Time: %.2f ms\n", getGraphAverage(&gpuGraph) * 1000.0f);
    }
};


void errorcb(int error, const char* desc)
{
	printf("GLFW error %d: %s\n", error, desc);
}



static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	NVG_NOTUSED(scancode);
	NVG_NOTUSED(mods);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        gAppState.blowup = !gAppState.blowup;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
        gAppState.screenshot = true;
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
        gAppState.premult = !gAppState.premult;
}


struct MyGlfwBackendInfo
{
    double mouseX, mouseY;
    int winWidth, winHeight;
    int fbWidth, fbHeight;
};

struct MyGlfwContext
{
    GLFWwindow* window;

    bool initGlfw()
    {
        if (!glfwInit()) {
            printf("Failed to init GLFW.");
            return false;
        }
        glfwSetErrorCallback(errorcb);
#ifndef _WIN32 // don't require this on win32, and works with more cards
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

#ifdef DEMO_MSAA
        glfwWindowHint(GLFW_SAMPLES, 4);
#endif
        window = glfwCreateWindow(1000, 600, "NanoVG", NULL, NULL);
//	window = glfwCreateWindow(1000, 600, "NanoVG", glfwGetPrimaryMonitor(), NULL);
        if (!window) {
            glfwTerminate();
            return false;
        }

        glfwSetKeyCallback(window, key);

        glfwMakeContextCurrent(window);
#ifdef NANOVG_GLEW
        glewExperimental = GL_TRUE;
        if(glewInit() != GLEW_OK) {
            printf("Could not init glew.\n");
            return false;
        }
        // GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
        glGetError();
#endif

        glfwSwapInterval(0);
        glfwSetTime(0);

        return true;
    }

    bool shouldClose()
    {
        return glfwWindowShouldClose(window);
    }

    void swapAndPoll()
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    MyGlfwBackendInfo getGlfwBackendInfo()
    {
        MyGlfwBackendInfo info;
        glfwGetCursorPos(window, &info.mouseX, &info.mouseY);
        glfwGetWindowSize(window, &info.winWidth, &info.winHeight);
        glfwGetFramebufferSize(window, &info.fbWidth, &info.fbHeight);
        return info;
    }

    void terminate()
    {
        glfwTerminate();
    }
};



struct MyNvgDemo
{
    NVGcontext* vg;
    DemoData data;
    TimeHelper timeHelper;
    PerfLogs perfLogs;

    bool init()
    {
        perfLogs.init();

#ifdef DEMO_MSAA
        vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
#else
        vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#endif
        if (vg == NULL) {
            printf("Could not init nanovg.\n");
            return false;
        }

        if (loadDemoData(vg, &data) == -1)
            return false;

        return true;
    }

    void myRender(MyGlfwBackendInfo info)
    {
        double now = timeHelper.now();

        // Calculate pixel ration for hi-dpi devices.
        float pxRatio = (float)info.fbWidth / (float)info.winWidth;
        nvgBeginFrame(vg, info.winWidth, info.winHeight, pxRatio);

        renderDemo(vg, info.mouseX, info.mouseY, info.winWidth, info.winHeight, now, gAppState.blowup, &data);
        perfLogs.myRenderGraph(vg);

        nvgEndFrame(vg);

        // Measure the CPU time taken excluding swap buffers (as the swap may wait for GPU)
        double cpuTime = glfwGetTime() - now;
        _perfOnFrameEnd(timeHelper.getDeltaTime(), cpuTime);
    }

    void perfOnFrameStart()
    {
        perfLogs.onFrameStart();
    }

    void _perfOnFrameEnd(float dt, float cpuTime)
    {
        perfLogs.onFrameEnd(dt, cpuTime);
    }


    void teardown()
    {
        freeDemoData(vg, &data);
        nvgDeleteGL3(vg);
    }

    void printPerf()
    {
        perfLogs.myPrintf();
    }
};


struct MyNvgDemo_NoPerf
{
    NVGcontext* vg;
    DemoData data;
    TimeHelper timeHelper;

    bool init()
    {

#ifdef DEMO_MSAA
        vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
#else
        vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#endif
        if (vg == NULL) {
            printf("Could not init nanovg.\n");
            return false;
        }

        if (loadDemoData(vg, &data) == -1)
            return false;

        return true;
    }

    void myRender(MyGlfwBackendInfo info)
    {
        // Calculate pixel ration for hi-dpi devices.
        float pxRatio = (float)info.fbWidth / (float)info.winWidth;
        nvgBeginFrame(vg, info.winWidth, info.winHeight, pxRatio);

        double now = timeHelper.now();
        renderDemo(vg, info.mouseX, info.mouseY, info.winWidth, info.winHeight, now, gAppState.blowup, &data);

        nvgEndFrame(vg);
    }


    void teardown()
    {
        freeDemoData(vg, &data);
        nvgDeleteGL3(vg);
    }
};



int main()
{
    MyGlfwContext myGlfwContext;
    if (!myGlfwContext.initGlfw())
        return -1;

    //MyNvgDemo myNvgDemo;
    MyNvgDemo_NoPerf myNvgDemo;
    myNvgDemo.init();


	while (!myGlfwContext.shouldClose())
	{
        //myNvgDemo.perfOnFrameStart();

        MyGlfwBackendInfo info = myGlfwContext.getGlfwBackendInfo();

		// Update and render
		glViewport(0, 0, info.fbWidth, info.fbHeight);
		if (gAppState.premult)
			glClearColor(0,0,0,0);
		else
			glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        myNvgDemo.myRender(info);

		if (gAppState.screenshot) {
			gAppState.screenshot = false;
			saveScreenShot(info.fbWidth, info.fbHeight, gAppState.premult, "dump.png");
		}

        myGlfwContext.swapAndPoll();
	}

    myNvgDemo.teardown();

    //myNvgDemo.printPerf();

	myGlfwContext.terminate();
	return 0;
}
