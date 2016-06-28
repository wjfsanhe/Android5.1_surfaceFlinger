#define LOG_TAG "wangjfEGL"

#include <cutils/properties.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <utils/Log.h>
#include <utils/threads.h>

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <pthread.h>
#include <sys/select.h>

#include <cutils/properties.h>

#include <androidfw/AssetManager.h>
#include <binder/IPCThreadState.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>

#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#include <SkBitmap.h>
#include <SkStream.h>
#include <SkImageDecoder.h>

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/eglext.h>


#include <android/looper.h>
#include <gui/DisplayEventReceiverVR.h>
#include <utils/Looper.h>

#include <utils/Timers.h>
#include <time.h>
#include <cutils/sockets.h>
/*test*/
extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
		const struct timespec *request,
		struct timespec *remain);


#ifndef FBIO_WAITFORVSYNC 
#define FBIO_WAITFORVSYNC   _IOW('F', 0x20, __u32)
#endif

#if defined(HAVE_PTHREADS)
# include <pthread.h>
# include <sys/resource.h>
#endif



using namespace android;

sp<SurfaceComposerClient>       mSession;
int         mWidth;
int         mHeight;
static   struct timespec vsync_delay;
bool vsync_delay_enable;
EGLDisplay  mDisplay;
EGLDisplay  mContext;
EGLDisplay  mSurface;
sp<SurfaceControl> mFlingerSurfaceControl;
sp<Surface> mFlingerSurface;
status_t OpLine();
status_t OpScreen();
void*  TimerThread (void*);

int receiver(int fd, int events, void* data)
{
    DisplayEventReceiverVR* q = (DisplayEventReceiverVR*)data;

    ssize_t n;
    DisplayEventReceiverVR::Event buffer[1];
    static bool timerEnable=false;

    static nsecs_t oldTimeStamp = 0;

    while ((n = q->getEvents(buffer, 1)) > 0) {
        for (int i=0 ; i<n ; i++) {
            /*if (buffer[i].header.type == DisplayEventReceiverVR::DISPLAY_EVENT_VSYNC) {
                //printf("event vsync: count=%d\t", buffer[i].vsync.count);
            }
            if (oldTimeStamp) {
                float t = float(buffer[i].header.timestamp - oldTimeStamp) / s2ns(1);
                //printf("%f ms (%f Hz)\n", t*1000, 1.0/t);
            }*/
	    int64_t delay= systemTime()-buffer[i].header.timestamp ;
        //    oldTimeStamp = buffer[i].header.timestamp;
	    
	    OpScreen();
		ALOGD("delay %lld ns[%d]\n", delay,i);
	    //OpLine();
	    /*if(delay > 2000000) {
			ALOGD("delay %lld ns[%d]\n", delay,i);
	    }else {
		OpLine();
		OpLine();
		//TimerThread(NULL);
	    }*/
        }
    }
    if (n<0) {
        printf("error reading events (%s)\n", strerror(-n));
    }
    return 1;
}

void*  TimerThread (void*){

   // const nsecs_t period = 10000000;
	static int mNextFakeVSync=0;
	const nsecs_t period = 16777777;
	//printf("timer thread start \n");
while (1) {
	const nsecs_t now = systemTime(CLOCK_MONOTONIC);
	nsecs_t next_vsync = mNextFakeVSync;
	nsecs_t sleep = next_vsync - now;
	if (sleep < 0) {
		// we missed, find where the next vsync should be
		sleep = (period - ((now - next_vsync) % period));
		next_vsync = now + sleep;
	}
	mNextFakeVSync = next_vsync + period;

	struct timespec spec;
	spec.tv_sec  = next_vsync / 1000000000;
	spec.tv_nsec = next_vsync % 1000000000;

	int err;
	do {
		err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
	} while (err<0 && errno == EINTR);

	if (err == 0) {
		OpLine();
	}

}
	return NULL;

}




status_t initDisplay(){
	sp<IBinder> dtoken(SurfaceComposerClient::getBuiltInDisplay(
				ISurfaceComposer::eDisplayIdMain));
	DisplayInfo dinfo;
	status_t status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
	if (status)
		return -1;
	printf("display info: %d x%d\n",dinfo.w,dinfo.h) ;

	// create the native surface
	sp<SurfaceControl> control = mSession->createSurface(String8("wangjfEGL"),
			dinfo.w, dinfo.h*2,PIXEL_FORMAT_RGBX_8888,0x40000);

	SurfaceComposerClient::openGlobalTransaction();
	control->setLayer(0x40000000);
	SurfaceComposerClient::closeGlobalTransaction();

	sp<Surface> s = control->getSurface();

	// initialize opengl and egl
	const EGLint attribs[] = {
		EGL_RED_SIZE,   8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE,  8,
		EGL_DEPTH_SIZE, 0,
		EGL_NONE
	};
	EGLint w, h, dummy;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint contextAttributes[] = {  
#ifdef EGL_IMG_context_priority  
#if 1 
#warning "+++++++ using EGL_IMG_context_priority"  
		        EGL_CONTEXT_PRIORITY_LEVEL_IMG, EGL_CONTEXT_PRIORITY_HIGH_IMG,  
#endif  
#warning "------- no EGL_IMG_context_priority"  
#endif  
				        EGL_NONE, EGL_NONE  }; 
	eglInitialize(display, 0, 0);
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	surface = eglCreateWindowSurface(display, config, s.get(), NULL);
	context = eglCreateContext(display, config, NULL,contextAttributes);

	EGLint actualPriorityLevel;
	eglQueryContext( display, context, EGL_CONTEXT_PRIORITY_LEVEL_IMG, &actualPriorityLevel );
	switch ( actualPriorityLevel )
	{
		case EGL_CONTEXT_PRIORITY_HIGH_IMG: ALOGD( "Context is EGL_CONTEXT_PRIORITY_HIGH_IMG" ); break;
		case EGL_CONTEXT_PRIORITY_MEDIUM_IMG: ALOGD( "Context is EGL_CONTEXT_PRIORITY_MEDIUM_IMG" ); break;
		case EGL_CONTEXT_PRIORITY_LOW_IMG: ALOGD( "Context is EGL_CONTEXT_PRIORITY_LOW_IMG" ); break;
		default: ALOGD( "Context has unknown priority level" ); break;
	}





	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h) ;
	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
		return NO_INIT;

	mDisplay = display;
	mContext = context;
	mSurface = surface;
	mWidth = w;
	mHeight = h;
	mFlingerSurfaceControl = control;
	mFlingerSurface = s;

	printf("window info: %d x%d\n",mWidth,mHeight) ;

	return 0;
}
status_t OpScreen(){

	static bool UpDown=true;
	static bool oddEven=false ;
	int64_t  start,end,end2;


	start = systemTime();
	if(UpDown){ // up screen
		property_set("vr.updown", "0");
		ALOGD("-----  app up");
		glScissor(0,2560,1440,2560); //left
	//	glClearColor(1,0,0,1);
	//	glClear(GL_COLOR_BUFFER_BIT);
	//	glFinish();
		//glScissor(0,2560,1440,1280); //left

		if(oddEven)
			//if(1)
			glClearColor(1,0,0,1);
		else
			glClearColor(0,1,0,1); 

		glClear(GL_COLOR_BUFFER_BIT);
		//glFlush();       
		glFinish();       
		
		/*glScissor(0,3840,1440,1280); //right
		if(oddEven)
			//if(0)
			glClearColor(0,0,1,1);
		else
			glClearColor(1,0,0,1); 

		glClear(GL_COLOR_BUFFER_BIT);
		glFinish();*/
		//property_set("vr.updown", "1");
		ALOGD ("----- %lldns", systemTime()-start);

	} else { //down screen
		
		property_set("vr.updown", "1");
		ALOGD("----- app down");
		glScissor(0,0,1440,2560); //left
		//glClearColor(0,0,1,1);
		//glClear(GL_COLOR_BUFFER_BIT);
		//glFinish();
		//glScissor(0,0,1440,1280); //left

		if(oddEven)
			//if(1)
			glClearColor(0,0,1,1);
		else
			glClearColor(1,0,1,1); 

		glClear(GL_COLOR_BUFFER_BIT);
		//glFlush();       
		glFinish();       
		
		//glScissor(0,1280,1440,1280); //right
		/*if(oddEven)
			//if(0)
			glClearColor(1,0,1,1);
		else
			glClearColor(1,0,0,1); 

		glClear(GL_COLOR_BUFFER_BIT);
		glFinish();*/
		//property_set("vr.updown", "0");
		oddEven =!oddEven;
		ALOGD ("----- %lldns", systemTime()-start);

	}
	UpDown =!UpDown ;
	return 0;
	
}

status_t  OpLine(){

	static bool oddEven=true;
	int64_t  start,end,end2;
	DisplayEventReceiverVR myDisplayEvent;
	Rect *leftViewPort,*rightViewPort;	


	
	float t = float(systemTime() - start) / s2ns(1);
	start=systemTime();
	//we use left right eye mode .
	//right eye first then left eye. every 8ms
	//printf("Right[%d-%d-%d-%d]\n",rightViewPort->left,rightViewPort->top,rightViewPort->getWidth(),rightViewPort->getHeight());
	//glViewport(rightViewPort->left,rightViewPort->top,rightViewPort->getWidth(),rightViewPort->getHeight());
	//glScissor(rightViewPort->left,rightViewPort->right,rightViewPort->getWidth(),rightViewPort->getHeight());
	glScissor(0,0,1440,1280);
	
	if(oddEven)
	//if(1)
		glClearColor(1,0,0,1);
	else
		glClearColor(0,1,0,1); 

	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();       
	end2=systemTime();
	nsecs_t sleep= 8000000 - (end2 - start);
	if (sleep > 0){
		struct timespec spec;
		spec.tv_sec = 0;
		spec.tv_nsec = sleep % 8000000 ;
		int err;
		do {
			err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
		} while (err<0 && errno == EINTR);
	}
	if(sleep < 0){
		printf("sleep %lld ns, span %lld ns\n",sleep, end2 - start);
		//return  0;                        
	}
	//sleep then left eye ;
	//printf("Left[%d-%d-%d-%d]\n",leftViewPort->left,leftViewPort->top,leftViewPort->getWidth(),leftViewPort->getHeight());
	//glViewport(leftViewPort->left,leftViewPort->top,leftViewPort->getWidth(),leftViewPort->getHeight());
	//glScissor(leftViewPort->left,leftViewPort->right,leftViewPort->getWidth(),leftViewPort->getHeight());
	start=systemTime();
	glScissor(0,1280,1440,1280);
	if(oddEven)
	//if(0)
		glClearColor(0,0,1,1);
	else
		glClearColor(1,0,0,1); 

	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();
	//eglSwapBuffers(mDisplay, mSurface);  //post the first frame.
	end=systemTime();
	sleep= 8000000 - (end - start);
	float cost=float(end-start)/ s2ns(1);
	float cost2=float(end2-start)/ s2ns(1);

	oddEven =!oddEven;
	if(sleep < 0)
		ALOGD("sleep %lld ns\n",sleep);

	return 0;
}

status_t syncFrame(){

	// clear screen
        glShadeModel(GL_FLAT);
        glDisable(GL_DITHER);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0,0,1,1);
        glClear(GL_COLOR_BUFFER_BIT);
        eglSwapBuffers(mDisplay, mSurface);  //post the first frame.
	glEnable(GL_SCISSOR_TEST);	
	

	int fd = open("/dev/graphics/fb0", O_RDWR);
	if (fd <0 ) printf("file open fail\n");
	//if (ioctl(fd, FBIOGET_VSCREENINFO, &info) == -1) {
	printf("cmd is %d\n",FBIO_WAITFORVSYNC) ;
	while (1) {
		if (ioctl(fd, FBIO_WAITFORVSYNC,0) == -1) {
			ALOGD("%s:Error in ioctl FBIOGET_VSCREENINFO: %s\n", __FUNCTION__,
					strerror(errno));
		}	
		ALOGD("get one vsync \n");
		if(vsync_delay_enable) {
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &vsync_delay, NULL);
		}
		OpScreen();
	}
	close(fd);



	eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(mDisplay, mContext);
        eglDestroySurface(mDisplay, mSurface);
        mFlingerSurface.clear();
        mFlingerSurfaceControl.clear();
        eglTerminate(mDisplay);
        IPCThreadState::self()->stopProcess();


	return 0;

} 

status_t drawFrame(){



	// clear screen
        glShadeModel(GL_FLAT);
        glDisable(GL_DITHER);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0,0,1,1);
        glClear(GL_COLOR_BUFFER_BIT);
        eglSwapBuffers(mDisplay, mSurface);  //post the first frame.
	

	//create looper relative component
	bool oddEven=true;
	int64_t  start,end,end2;
	DisplayEventReceiverVR myDisplayEvent;
	Rect *leftViewPort,*rightViewPort;	
	sp<Looper> loop = new Looper(false);
	loop->addFd(myDisplayEvent.getFd(), 0, ALOOPER_EVENT_INPUT, receiver,
			&myDisplayEvent);

	myDisplayEvent.setVsyncRate(1);
	//leftViewPort=new Rect(0,0,mWidth,mHeight>>1);
	//leftViewPort=new Rect(0,mHeight,mWidth,mHeight>>1);
	//rightViewPort=new Rect(0,mHeight>>1,mWidth,mHeight);
	//rightViewPort=new Rect(0,mHeight>>1,mWidth,0);
	glEnable(GL_SCISSOR_TEST);	
	do {
		//printf("about to poll...\n");
		int32_t ret = loop->pollOnce(-1);
		switch (ret) {
			case ALOOPER_POLL_WAKE:
				//("ALOOPER_POLL_WAKE\n");
				break;
			case ALOOPER_POLL_CALLBACK:
				{

				};
				break;
			case ALOOPER_POLL_TIMEOUT:
				printf("ALOOPER_POLL_TIMEOUT\n");
				break;
			case ALOOPER_POLL_ERROR:
				printf("ALOOPER_POLL_TIMEOUT\n");
				break;
			default:
				printf("ugh? poll returned %d\n", ret);
				break;
		}
	} while (1);
	glDisable(GL_SCISSOR_TEST);	

	//clear environment.
	eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(mDisplay, mContext);
	eglDestroySurface(mDisplay, mSurface);
	mFlingerSurface.clear();
	mFlingerSurfaceControl.clear();
	eglTerminate(mDisplay);
	IPCThreadState::self()->stopProcess();
	return 0;

}

int main(int argc, char** argv)
{
#if defined(HAVE_PTHREADS)
	setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_DISPLAY);
#endif
	sp<ProcessState> proc(ProcessState::self());
	ProcessState::self()->startThreadPool();
	mSession = new SurfaceComposerClient();

	if(argc == 2) {
		int delay=atoi(argv[1]);
		printf("delay %d ns\n",delay);
		vsync_delay.tv_sec = 0;
                vsync_delay.tv_nsec = delay;
                vsync_delay_enable = 1;
	}
	// create the boot animation object
	initDisplay();	
	
    
    drawFrame();	
    //syncFrame();	
    IPCThreadState::self()->joinThreadPool();

    return 0;
}





