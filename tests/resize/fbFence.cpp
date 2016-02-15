#include <unistd.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include  "fbFence.h"

#define LOG_TAG   "FB_FENCE"

namespace android {
FBFence::FBFence(){
	mTimeLineFd=sw_sync_timeline_create();
	ALOGD("create new FrontBuffer TimeLineFd : %d",mTimeLineFd);
	mFenceFd=sw_sync_fence_create(mTimeLineFd,"fence-FBuffer",1); //initial value 1
	ALOGD("create new FrontBuffer FenceFd: %d",mFenceFd);		
}

FBFence::~FBFence() {
    if (mFenceFd != -1) {
        close(mFenceFd);
    }
}
status_t FBFence::signal(){
	if(mTimeLineFd>0){
		sw_sync_timeline_inc(mTimeLineFd,1);
		ALOGD("send one time line signal from FBFence");
	}	
	return status_t(NO_ERROR);
}

}   //android 
