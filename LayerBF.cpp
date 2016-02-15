/*
 * Copyright (C) 2007 The Android Open Source Project
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

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/Log.h>

#include <ui/GraphicBuffer.h>

#include "LayerBF.h"
#include "SurfaceFlinger.h"
#include "DisplayDevice.h"
#include "RenderEngine/RenderEngine.h"

namespace android {
// ---------------------------------------------------------------------------

LayerBF::LayerBF(SurfaceFlinger* flinger, const sp<Client>& client,
        const String8& name, uint32_t w, uint32_t h, uint32_t flags)
    : Layer(flinger, client, name, w, h, flags) {
 	mFirstCall=1;
}

LayerBF::~LayerBF() {
}


void LayerBF::onFirstRef() {
    // Creates a custom BufferQueue for SurfaceFlingerConsumer to use
    //first we use old style producer and consumer.
    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    BufferQueueBF::createBufferQueue(&producer, &consumer);
    mProducer = new MonitoredProducer(producer, mFlinger);//auto remove mechamism
    ALOGD(" new producer %p",mProducer.get());
    mSurfaceFlingerConsumer = new SurfaceFlingerConsumer(consumer, mTextureName);
    mSurfaceFlingerConsumer->setConsumerUsageBits(getEffectiveUsage(0));
    mSurfaceFlingerConsumer->setContentsChangedListener(this);
    mSurfaceFlingerConsumer->setName(mName);
    mSurfaceFlingerConsumer->setDefaultMaxBufferCount(1); //only one buffer

    const sp<const DisplayDevice> hw(mFlinger->getDefaultDisplayDevice());
    updateTransformHint(hw);
    ALOGE("LayerBF:**on first REF\n") ;
}
void LayerBF::onFrameAvailable(const BufferItem& item) {
    // Add this buffer from our internal queue tracker
    { // Autolock scope
        Mutex::Autolock lock(mQueueItemLock);
	mQueueItems.clear();
	mQueueItems.push_back(item);
    }
    ALOGD("LayerBF new frame coming");
    android_atomic_inc(&mQueuedFrames);
    mFlinger->signalLayerUpdate();
}

Region LayerBF::latchBuffer(bool& recomputeVisibleRegions)
{
	ALOGD("LayerBF:latchBuffer");
	if (mFirstCall == 1){
		mDirtyRegion=Layer::latchBuffer(recomputeVisibleRegions);//we use Layer::LatchBuffer to update Texture.
		mFirstCall=0;	
	}
	return mDirtyRegion;
}


// ---------------------------------------------------------------------------

}; // namespace android
