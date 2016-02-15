/*****************************************************************
* we create this Fence object which used by FB relative machamism
*
*
*****************************************************************/
#ifndef ANDROID_FB_FENCE_H_
#define ANDROID_FB_FENCE_H_
#include <ui/Fence.h>

namespace android {
class FBFence:public Fence {
public:
	FBFence();
	status_t signal();
	~FBFence();
private:
	friend class Fence;
	// Only allow instantiation using ref counting.
	friend class LightRefBase<FBFence>;
	int mTimeLineFd;
};
};
#endif // ANDROID_FB_FENCE_H

