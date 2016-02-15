#define LOG_TAG "eglTest"
//#define LOG_NDEBUG 0

#include <EGL/egl.h>
#include <GLES2/gl2.h>

//#include <gtest/gtest.h>
#include <gui/GLConsumer.h>
#include <gui/Surface.h>
#include <system/graphics.h>
#include <utils/Log.h>
#include <utils/Thread.h>

int  main(int argc ,char** argv){

	EGLDisplay mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	EGLint majorVersion, minorVersion;
	eglInitialize(mEglDisplay, &majorVersion, &minorVersion);
	EGLConfig myConfig = {0};
	EGLContext mEglContext;
    EGLint numConfigs = 0;
    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE };
    eglChooseConfig(mEglDisplay, configAttribs, &myConfig, 1,&numConfigs);
	mEglContext = eglCreateContext(mEglDisplay, myConfig, EGL_NO_CONTEXT, 0);
	
	Surface* mSTC;
	ANativeWindow mANW;
	sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    BufferQueue::createBufferQueue(&producer, &consumer);
	mSTC = new Surface(producer);
    mANW = mSTC;

	EGLSurface eglSurface = eglCreateWindowSurface(mEglDisplay, myConfig,mANW.get(), NULL);	
	EGLBoolean success = eglMakeCurrent(mEglDisplay, eglSurface, eglSurface, mEglContext);
	glClear(GL_COLOR_BUFFER_BIT);
    success = eglSwapBuffers(mEglDisplay, eglSurface);

	mSTC.clear();
    mANW.clear();

    eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(mEglDisplay, mEglContext);
    eglDestroySurface(mEglDisplay, eglSurface);
    eglTerminate(mEglDisplay);
	
}

