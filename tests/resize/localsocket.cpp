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
//#define  SOCKET_NAME  "/data/data/baofeng/signalBF"
#define  SOCKET_NAME  "/tmp/baofeng/signalBF"
//#define  SOCKET_NAME  "signalBF"
int main(int argc, char** argv)
{
	
    int32_t  mSock ;


    if (setgid(AID_SYSTEM) != 0) {
            exit(1);
    }
    if (setuid(AID_SYSTEM) != 0) {
            exit(1);
    }
    printf("%s\n",SOCKET_NAME);    
    if (argc > 1){
	    unlink(SOCKET_NAME);
	    mSock = socket_local_server(SOCKET_NAME,
			    ANDROID_SOCKET_NAMESPACE_FILESYSTEM,//ANDROID_SOCKET_NAMESPACE_RESERVED,
			    SOCK_STREAM);
	    if(mSock < 0){
		    printf("create local socket server fail:%s\n",strerror(errno));
	    }else{
		    printf("create local socket server success:%d\n",mSock);
	    }

    } else{

	    mSock = socket_local_client(SOCKET_NAME,
			    ANDROID_SOCKET_NAMESPACE_FILESYSTEM,//ANDROID_SOCKET_NAMESPACE_RESERVED,
			    SOCK_STREAM);
	    if(mSock < 0){
		    printf("create local socket client fail:%s\n",strerror(errno));
	    }else{
		    printf("create local socket client success:%d\n",mSock);
	    }
    }
    sleep (100);
    return 0;
}
