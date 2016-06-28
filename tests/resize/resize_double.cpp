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
//#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#define ATRACE_TAG ATRACE_TAG_APP

#include <cutils/memory.h>

#include <utils/Log.h>
#include <utils/Trace.h>
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
#include <time.h>
#include <cutils/sockets.h>
/*test*/

using namespace android;

//global varible
sp<Surface> surface ;


//namespace android 
//we will post memory at receiver.
int64_t getSystemTime()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)(t.tv_sec)*1000000000LL + t.tv_nsec;
}


int receiver(int fd, int events, void* data)
{
    DisplayEventReceiver* q = (DisplayEventReceiver*)data;

    ssize_t n;
    DisplayEventReceiver::Event buffer[1];

    static nsecs_t oldTimeStamp = 0;

    while ((n = q->getEvents(buffer, 1)) > 0) {
        for (int i=0 ; i<n ; i++) {
            if (buffer[i].header.type == DisplayEventReceiver::DISPLAY_EVENT_VSYNC) {
                //printf("event vsync: count=%d\t", buffer[i].vsync.count);
            }
            if (oldTimeStamp) {
                float t = float(buffer[i].header.timestamp - oldTimeStamp) / s2ns(1);
                //printf("%f ms (%f Hz)\n", t*1000, 1.0/t);
            }
            oldTimeStamp = buffer[i].header.timestamp;
        }
    }
    if (n<0) {
        printf("error reading events (%s)\n", strerror(-n));
    }
    return 1;
}

int testCall(){
ATRACE_CALL();
printf("testfunc");
return 0;
}
int main(int argc, char** argv)
{
	
    int32_t  mSock ;
    mSock = socket_local_server("signalBF",
                                   ANDROID_SOCKET_NAMESPACE_RESERVED,
                                   SOCK_STREAM);

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
    printf(" Begin resize work\n"); 
    sp<SurfaceControl> surfaceControl = client->createSurface(String8("resize"),
            1440, 2560, PIXEL_FORMAT_RGB_565, 0x0000);
    surface = surfaceControl->getSurface();

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


    printf("change surface Content  %s %d",__func__,__LINE__);
    ssize_t bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
    bool oddEven=true;
    int64_t  start,end,end2;
    uint16_t *region1Start=(uint16_t*)(outBuffer.bits) ;
    uint16_t *region2Start=(uint16_t*)((outBuffer.bits) + bpr*outBuffer.height/3);
    uint16_t *region3Start=(uint16_t*)((outBuffer.bits) + bpr*outBuffer.height*2/3);
#if  1
   do {
        //printf("about to poll...\n");
        int32_t ret = loop->pollOnce(-1);
        switch (ret) {
            case ALOOPER_POLL_WAKE:
                //("ALOOPER_POLL_WAKE\n");
                break;
            case ALOOPER_POLL_CALLBACK:
		{
			ATRACE_CALL();

			testCall();
			//Trace.traceBegin(Trace.TRACE_TAG_GRAPHICS, "resize");

			
                	float t = float(systemTime() - start) / s2ns(1);
			start=systemTime();
			
			if(oddEven){
				android_memset16((uint16_t*)outBuffer.bits, 0xF800, bpr*outBuffer.height);
				
				/*android_memset16(region3Start, 0x0000, bpr*outBuffer.height/3);
				android_memset16(region1Start, 0xF800, bpr*outBuffer.height/3);
				//usleep(5000);
				android_memset16(region1Start, 0x0000, bpr*outBuffer.height/3);
				android_memset16(region2Start, 0xF800, bpr*outBuffer.height/3);
				//usleep(5000);
				
				android_memset16(region2Start, 0x0000, bpr*outBuffer.height/3);
				android_memset16(region3Start, 0xF800, bpr*outBuffer.height/3);*/
				end2=systemTime();
				surface->unlockAndPost();
			} else {
				android_memset16((uint16_t*)outBuffer.bits, 0x07E0, bpr*outBuffer.height);
				
				/*android_memset16(region3Start, 0x0000, bpr*outBuffer.height/3);
				android_memset16(region1Start, 0x07E0, bpr*outBuffer.height/3);
				//usleep(5000);
				android_memset16(region1Start, 0x0000, bpr*outBuffer.height/3);
				android_memset16(region2Start, 0x07E0, bpr*outBuffer.height/3);
				//usleep(5000);
				
				android_memset16(region2Start, 0x0000, bpr*outBuffer.height/3);
				android_memset16(region3Start, 0x07E0, bpr*outBuffer.height/3);*/
				
				end2=systemTime();
				surface->unlockAndPost();
			}
			end=systemTime();
			float cost=float(end-start)/ s2ns(1);
			float cost2=float(end2-start)/ s2ns(1);
			
			
                	//printf("%f ms (%f Hz), draw cost %f ms,simple cost %f ms\n", t*1000, 1.0/t,cost*1000
			//					,cost2*1000);
			oddEven=!oddEven;
			//Trace.traceEnd(Trace.TRACE_TAG_GRAPHICS, "resize");

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
#else
    
    if((mSock) /*= socket_local_client("signalBF",
                                 ANDROID_SOCKET_NAMESPACE_ABSTRACT,
                                 SOCK_DGRAM))*/ < 0) {    
    	printf("resize: create local socket fail, %s\n",strerror(errno));
	return -1;
    } else {
	printf("resize: cretae local socket success\n");
	return 0;
    }
    char *buffer = (char*)malloc(4096);

    //create the first post.
    android_memset16((uint16_t*)outBuffer.bits, 0xF800, bpr*outBuffer.height);
    surface->Post();
    
    while(1) {
        fd_set read_fds;
        struct timeval to;
        int rc = 0;

        to.tv_sec = 10;
        to.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(mSock, &read_fds);

        if ((rc = select(mSock + 1, &read_fds, NULL, NULL, &to)) < 0) {
            fprintf(stderr, "Error in select (%s)\n", strerror(errno));
            break;
        } else if (!rc) {
            fprintf(stderr, "[TIMEOUT]\n");
            continue;
        } else if (FD_ISSET(mSock, &read_fds)) {
            memset(buffer, 0, 4096);
            if ((rc = read(mSock, buffer, 4096)) <= 0) {
                if (rc == 0) {
                    fprintf(stderr, "Lost connection to LayerBF?\n");
		    continue ;
		}
                else {
                    fprintf(stderr, "Error reading data (%s)\n", strerror(errno));
		    break;
		}
            }
	    //commit new post
	    printf("got message:%s\n",buffer);
	    oddEven=!oddEven;
	    if(oddEven)
   	    android_memset16((uint16_t*)outBuffer.bits, 0x07E0, bpr*outBuffer.height);
	    else
   	    android_memset16((uint16_t*)outBuffer.bits, 0xF800, bpr*outBuffer.height);
    	    surface->Post();
	    
         }
    }
    free(buffer);    

#endif
    surface->unlockAndPost();
	    ALOGD("change surface Content");
    sleep(1);
    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setSize(1440, 2560);
    SurfaceComposerClient::closeGlobalTransaction();

    
    IPCThreadState::self()->joinThreadPool();
    
    return 0;
}
