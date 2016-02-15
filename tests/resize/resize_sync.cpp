/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <cutils/memory.h>

#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
/*tw add begin*/
#include <gui/CpuConsumer.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>
#include <android/looper.h>
#include <gui/DisplayEventReceiver.h>
#include <utils/Looper.h>

/*test*/

using namespace android;

//namespace android 
//we will post memory at receiver.
int receiver(int fd, int events, void* data)
{
    DisplayEventReceiver* q = (DisplayEventReceiver*)data;

    ssize_t n;
    DisplayEventReceiver::Event buffer[1];

    static nsecs_t oldTimeStamp = 0;

    while ((n = q->getEvents(buffer, 1)) > 0) {
        for (int i=0 ; i<n ; i++) {
            if (buffer[i].header.type == DisplayEventReceiver::DISPLAY_EVENT_VSYNC) {
                printf("event vsync: count=%d\t", buffer[i].vsync.count);
            }
            if (oldTimeStamp) {
                float t = float(buffer[i].header.timestamp - oldTimeStamp) / s2ns(1);
                printf("%f ms (%f Hz)\n", t*1000, 1.0/t);
            }
            oldTimeStamp = buffer[i].header.timestamp;
        }
    }
    if (n<0) {
        printf("error reading events (%s)\n", strerror(-n));
    }
    return 1;
}


int main(int argc, char** argv)
{
    // set up the thread-pool
    DisplayEventReceiver myDisplayEvent;
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();
    int vsyncRate=1;
    if(argc > 1) {
    	vsyncRate=atoi(argv[1]);
    }

    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    ALOGE(" Begin resize work\n"); 
    sp<SurfaceControl> surfaceControl = client->createSurface(String8("resize"),
            1440, 2560, PIXEL_FORMAT_RGB_565, 0x40000);
    sp<Surface> surface = surfaceControl->getSurface();

    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setLayer(300000);
    SurfaceComposerClient::closeGlobalTransaction();
    ANativeWindow_Buffer outBuffer;
    surface->lock(&outBuffer, NULL);
   
    //create looper relative component
    sp<Looper> loop = new Looper(false);
    loop->addFd(myDisplayEvent.getFd(), 0, ALOOPER_EVENT_INPUT, receiver,
            &myDisplayEvent);

    myDisplayEvent.setVsyncRate(vsyncRate);


    ALOGD("change surface Content  %s %d",__func__,__LINE__);
    ssize_t bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
    bool oddEven=true;
   do {
        //printf("about to poll...\n");
        int32_t ret = loop->pollOnce(-1);
        switch (ret) {
            case ALOOPER_POLL_WAKE:
                //("ALOOPER_POLL_WAKE\n");
                break;
            case ALOOPER_POLL_CALLBACK:
		{
			if(oddEven){
				android_memset16((uint16_t*)outBuffer.bits, 0xF800, bpr*outBuffer.height);
				surface->Post();
			} else {
				android_memset16((uint16_t*)outBuffer.bits, 0x07E0, bpr*outBuffer.height);
				surface->Post();
			}
			oddEven=!oddEven;
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

    surface->unlockAndPost();
	    ALOGD("change surface Content");
    sleep(1);
    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setSize(1440, 2560);
    SurfaceComposerClient::closeGlobalTransaction();

    
    IPCThreadState::self()->joinThreadPool();
    
    return 0;
}
