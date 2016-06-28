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
#include <time.h>
#include <cutils/sockets.h>
#include <private/android_filesystem_config.h>
/*test*/

using namespace android;
//global varible
sp<Surface> surface ;
ssize_t bpr=0 ;
ANativeWindow_Buffer outBuffer;
int32_t mSock ;
//namespace android 
//we will post memory at receiver.
int64_t getSystemTime()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)(t.tv_sec)*1000000000LL + t.tv_nsec;
}

void handleConnection(int32_t fd,uint16_t *pt){
	
	fd_set read_fds;
        struct timeval to;
        int rc = 0;
	bool oddEven=true;
    	int64_t  start,end,end2;

    	char buffer[4096];
        //surface->Post();
	//send one commit message here.
	char cmd[25];
	//fprintf(stderr,"Send one commit message"); 
	/*while(1){
		sprintf(cmd,"SignalR:%d:1440x2560III",oddEven);
		write(fd,cmd,strlen(cmd));
		usleep(10000);
		fprintf(stderr,cmd);
		fprintf(stderr,"\n");
	}*/
		android_memset16(pt, 0x00, bpr*outBuffer.height);
	while(1){
		to.tv_sec = 0;
		to.tv_usec = 16000;

		FD_ZERO(&read_fds);
		FD_SET(fd, &read_fds);

		if ((rc = select(fd + 1, &read_fds, NULL, NULL, &to)) < 0) {
			fprintf(stderr, "Error in select (%s)\n", strerror(errno));
			break;
		} else if (!rc) {
			//fprintf(stderr, "[TIMEOUT]\n");

			//write message once we got one signal.
                        oddEven=!oddEven;
                        fprintf(stderr,"time odd=%d\n",oddEven);
		//	android_memset16(pt, 0x00, bpr*outBuffer.height);
                        if(!oddEven){
                                //fprintf(stderr,"..,%d\n",outBuffer.height);
                                android_memset16(pt, 0x07E0, bpr*outBuffer.height/4);
                        } else {
                                //fprintf(stderr,"**,%d\n",outBuffer.height);
                                android_memset16(pt+bpr*outBuffer.height/4, 0xF800, bpr*outBuffer.height/4);
                        }
                        sprintf(cmd,"SignalR:%d:1440x2560",oddEven);
                        write(fd,cmd,strlen(cmd));
			continue;
		} else if (FD_ISSET(fd, &read_fds)) {
			
                	//float t = float(systemTime() - start) / s2ns(1);
			//start=systemTime();
			//printf("%f ms(%f HZ)\n",t*1000,1.0/t);

			memset(buffer, 0, 4096);
			if ((rc = read(fd, buffer, 4096)) <= 0) {
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
			//write message once we got one signal.
			char cmd[25];
			if(NULL==pt) continue ;
			oddEven=!oddEven;
			fprintf(stderr,"odd=%d\n",oddEven);
			if(oddEven){
				//fprintf(stderr,"..,%d\n",outBuffer.height);
				android_memset16(pt, 0x07E0, bpr*outBuffer.height);
			} else {
				//fprintf(stderr,"**,%d\n",outBuffer.height);
				android_memset16(pt, 0xF800, bpr*outBuffer.height);
			}
			sprintf(cmd,"SignalR:%d:1440x2560",oddEven);
			write(fd,cmd,strlen(cmd));			
			//usleep(500000);
			//surface->Post();

		}
	}

}

void* threadSignal(uint16_t* pt){

		
	surface->lock(&outBuffer, NULL);  
	bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);

        printf("----------------outBuffer.height=%d\n",outBuffer.height);
	android_memset16((uint16_t*)outBuffer.bits, 0xF800, bpr*outBuffer.height);
	surface->Post();
	//left eye and right eye.

	

	return NULL;

	if(mSock < 0) {    
		printf("resize: create local socket fail, %s\n",strerror(errno));
		return NULL;
	} else {
		printf("resize: cretae local socket success\n");
	}
	for (;;) {
		sockaddr addr;
		socklen_t alen = sizeof(addr);

		printf("waiting for connection\n");
		int fd = accept(mSock, &addr, &alen);
		if (fd < 0) {
			ALOGV("accept failed: %s\n", strerror(errno));
			continue;
		}
		fcntl(fd, F_SETFD, FD_CLOEXEC);
		printf("new client coming \n");
		handleConnection(fd,(uint16_t*)outBuffer.bits);

	}
	return NULL;
}

void *threadEnv(void*){

	
    // set up the thread-pool
    

    //create server socket
    mSock = socket_local_server("signalBF",
                                   /*ANDROID_SOCKET_NAMESPACE_FILESYSTEM,*/ANDROID_SOCKET_NAMESPACE_ABSTRACT,
                                   SOCK_STREAM);
    //usleep(5000);

    DisplayEventReceiver myDisplayEvent;
	

    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    printf(" Begin resize work\n"); 
    sp<SurfaceControl> surfaceControl = client->createSurface(String8("resize"),
            1440, 5120, PIXEL_FORMAT_RGB_565, 0x40000);
    surface = surfaceControl->getSurface();

    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setLayer(300000);
    SurfaceComposerClient::closeGlobalTransaction();
   

    printf("change surface Content  %s %d",__func__,__LINE__);
    

    //surface->lock(&outBuffer, NULL);  
    //ssize_t bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
    bool oddEven=true;
    int64_t  start,end,end2;
    uint16_t *region1Start=(uint16_t*)(outBuffer.bits) ;
    uint16_t *region2Start=(uint16_t*)((outBuffer.bits) + bpr*outBuffer.height/3);
    uint16_t *region3Start=(uint16_t*)((outBuffer.bits) + bpr*outBuffer.height*2/3);

    threadSignal((uint16_t*)outBuffer.bits);

    printf("press enter to exit\n");
    getchar();
    surface->unlockAndPost();
	    ALOGD("change surface Content");
    sleep(1);
    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setSize(1440, 2560);
    SurfaceComposerClient::closeGlobalTransaction();
    return NULL;

}
int main(int argc, char** argv)
{
   
    if (setgid(AID_SYSTEM) != 0) {
	    printf("set gid fail\n");
            exit(1);
    }
    if (setuid(AID_SYSTEM) != 0) {
	    printf("set uid fail\n");
            exit(1);
    }

    pthread_t thr;
    pthread_attr_t attr;
    
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();
	
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thr, &attr, threadEnv, NULL);
    //threadEnv(0);
    pthread_attr_destroy(&attr);
    
    //threadSignal(0);



    
    IPCThreadState::self()->joinThreadPool();
    
    return 0;
}
