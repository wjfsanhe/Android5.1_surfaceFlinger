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

#ifndef ANDROID_LAYER_BF_H
#define ANDROID_LAYER_BF_H

#include <stdint.h>
#include <sys/types.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Timers.h>

#include <ui/FrameStats.h>
#include <ui/GraphicBuffer.h>
#include <ui/PixelFormat.h>
#include <ui/Region.h>
#include "Layer.h"
#include <gui/ISurfaceComposerClient.h>

#include <private/gui/LayerState.h>

#include "FrameTracker.h"
#include "Client.h"
#include "MonitoredProducer.h"
#include "SurfaceFlinger.h"
#include "SurfaceFlingerConsumer.h"
#include "Transform.h"

#include "DisplayHardware/HWComposer.h"
#include "DisplayHardware/FloatRect.h"
#include "RenderEngine/Mesh.h"
#include "RenderEngine/Texture.h"
#include <cutils/sockets.h>

namespace android {

// ---------------------------------------------------------------------------

class Client;
class Colorizer;
class DisplayDevice;
class GraphicBuffer;
class SurfaceFlinger;

// ---------------------------------------------------------------------------

/*
 * A new BufferQueue and a new SurfaceFlingerConsumer are created when the
 * Layer is first referenced.
 *
 * This also implements onFrameAvailable(), which notifies SurfaceFlinger
 * that new data has arrived.
 */
class LayerBF : public Layer {
    static int32_t sSequence;


public:

    // Layer serial number.  This gives layers an explicit ordering, so we
    // have a stable sort order when their layer stack and Z-order are
    // the same.

    // -----------------------------------------------------------------------

    LayerBF(SurfaceFlinger* flinger, const sp<Client>& client,
            const String8& name, uint32_t w, uint32_t h, uint32_t flags);

    virtual ~LayerBF();




public:

    /*
     * called after page-flip
     */
    virtual void onLayerDisplayed(const sp<const DisplayDevice>& hw,
            HWComposer::HWCLayerInterface* layer);


    /*
     * latchBuffer - called each time the screen is redrawn and returns whether
     * the visible regions need to be recomputed (this is a fairly heavy
     * operation, so this should be set only if needed). Typically this is used
     * to figure out if the content or size of a surface has changed.
     */
    virtual Region latchBuffer(bool& recomputeVisibleRegions);



protected:
    // constant
    //sp<SurfaceFlinger> mFlinger;

    virtual void onFirstRef();

    /*
     * Trivial class, used to ensure that mFlinger->onLayerDestroyed(mLayer)
     * is called.
     */


protected:
    // Interface implementation for SurfaceFlingerConsumer::ContentsChangedListener
    virtual void onFrameAvailable(const BufferItem& item);
    virtual void onFrameReplaced(const BufferItem& item);
    virtual void onSidebandStreamChanged();
    int32_t mSock;


};

// ---------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_LAYER_H
