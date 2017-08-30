/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include <android/native_window.h>

#include <binder/IMemory.h>

#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <private/gui/ComposerService.h>

#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkImage.h>
#include <SkRect.h>
#include <GrPaint.h>
#include <SkXfermode.h>

#include <utils/String8.h>
#include <ui/DisplayInfo.h>

#include <android/looper.h>
#include <gui/DisplayEventReceiver.h>
#include <utils/Looper.h>

#undef LOG_TAG
#define LOG_TAG "bp_native"

#define GET_DISPLAY_EVENT
//#define USE_SKIA_LEGACY

#include <cutils/log.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>

#include <keystore/IKeystoreService.h>
#include <keystore/keystore.h> // for error codes


#ifdef GET_DISPLAY_EVENT
#include "Remote.h"
#endif

using namespace android;

namespace android {

// Fill an RGBA_8888 formatted surface with a single color.
static void fillSurfaceRGBA8(const sp<SurfaceControl>& sc,
        uint8_t r, uint8_t g, uint8_t b) {

	uint32_t __width = 0;
	uint32_t __height = 0;

    ANativeWindow_Buffer outBuffer;
    sp<Surface> s = sc->getSurface();
    //ASSERT_TRUE(s != NULL);
    s->lock(&outBuffer, NULL);
	__width = outBuffer.width;
	__height = outBuffer.height;

    uint8_t* img = reinterpret_cast<uint8_t*>(outBuffer.bits);
    for (uint32_t y = 0; y < __height; y++) {
        for (uint32_t x = 0; x < __width; x++) {
            uint8_t* pixel = img + (4 * (y*outBuffer.stride + x));
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            pixel[3] = 0;
        }
    }
    s->unlockAndPost();
}

const static uint32_t diameter = 30;

static inline bool isPointOutOfCircle(uint32_t x, uint32_t y)
{
    return (x*x + y*y > diameter * diameter);
}

static void flipSurfaceRGBA8(const sp<SurfaceControl>& sc) {
    uint32_t dx = 0;
    uint32_t dy = 0;

    uint32_t __width = 0;
    uint32_t __height = 0;

    ANativeWindow_Buffer outBuffer;
    sp<Surface> s = sc->getSurface();
    //ASSERT_TRUE(s != NULL);
    s->lock(&outBuffer, NULL);
    printf("width=%d height=%d\n", outBuffer.width, outBuffer.height);
	__width = outBuffer.width;
	__height = outBuffer.height;
    uint8_t* img = reinterpret_cast<uint8_t*>(outBuffer.bits);
    for (uint32_t y = 0; y < __height; y++) {
        for (uint32_t x = 0; x < __width; x++) {
            uint8_t* pixel = img + (4 * (y*outBuffer.stride + x));
            if(x <= diameter && y <= diameter) {
                dx = diameter - x;
                dy = diameter - y;
            } else if(x >= __width - diameter && y <= diameter) {
                dx = x - (__width - diameter);
                dy = diameter - y;
            } else if(x <= diameter && y >= __height - diameter) {
                dx = diameter - x;
                dy = y - (__height - diameter);
            } else if(x >= __width - diameter && y >= __height - diameter) {
                dx = x - (__width - diameter);
                dy = y - (__height - diameter);
            } else {
                continue;
            }

            if(isPointOutOfCircle(dx, dy)) {
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 0;
                pixel[3] = 255;
            }
        }
    }
    s->unlockAndPost();
}

//-------------------------------------------------------------
#ifndef USE_SKIA_LEGACY
static inline SkImageInfo convertPixelFormat(const ANativeWindow_Buffer& buffer) {
    int                 fWidth;
    int                 fHeight;
    SkColorType         fColorType;
    SkAlphaType         fAlphaType;
    //SkColorProfileType  fProfileType;

    fWidth = buffer.width;
    fHeight = buffer.height;
    switch (buffer.format) {
        case WINDOW_FORMAT_RGBA_8888:
            fColorType = kN32_SkColorType;
            fAlphaType = kPremul_SkAlphaType;
            break;
        case WINDOW_FORMAT_RGBX_8888:
            fColorType = kN32_SkColorType;
            fAlphaType = kOpaque_SkAlphaType;
            break;
        case WINDOW_FORMAT_RGB_565:
            fColorType = kRGB_565_SkColorType;
            fAlphaType = kOpaque_SkAlphaType;
            break;
        default:
            fColorType = kUnknown_SkColorType;
            // switch to kUnknown_SkAlphaType when its in skia
            fAlphaType = kOpaque_SkAlphaType;
            break;
    }

    return SkImageInfo::Make(fWidth, fHeight, fColorType, fAlphaType);
}
#endif

static void flipSurfaceBySkia(const sp<SurfaceControl>& sc) {
    uint32_t mProtectImageTexName;

    ANativeWindow_Buffer outBuffer;
    sp<Surface> s = sc->getSurface();
    //ASSERT_TRUE(s != NULL);
    s->lock(&outBuffer, NULL);

    ssize_t bytesCount = outBuffer.stride * bytesPerPixel(outBuffer.format);

    SkBitmap bitmap;
#ifdef USE_SKIA_LEGACY
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, outBuffer.width, outBuffer.height, bytesCount);
#else
    bitmap.setInfo(convertPixelFormat(outBuffer), bytesCount);
#endif

    if (outBuffer.width > 0 && outBuffer.height > 0) {
        bitmap.setPixels(outBuffer.bits);
    } else {
        bitmap.setPixels(NULL);
    }

    SkCanvas canvas(bitmap);

    SkPaint mPaint;
    mPaint.setAntiAlias(true);
    mPaint.setARGB(255, 0, 0, 0);
    SkRect mSkRect;
    mSkRect.set(0, 0, outBuffer.width, outBuffer.height);
    canvas.drawRect(mSkRect, mPaint);
    //canvas.drawCircle(600, 400, 200, mPaint);

    mPaint.setXfermodeMode(SkXfermode::kDstOut_Mode);
    mPaint.setARGB(255, 0, 0, 0);
    mSkRect.set(0, 0, outBuffer.width, outBuffer.height);
    canvas.drawRoundRect(mSkRect, 60, 60, mPaint);

    //mPaint.setXfermodeMode(SkXfermode::kDstOver_Mode);
    //mPaint.setARGB(130, 255, 0, 0);
    //mSkRect.set(100, 100, outBuffer.width - 200, outBuffer.height - 200);
    //canvas.drawRoundRect(mSkRect, 50, 50, mPaint);

    s->unlockAndPost();
}


//-------------------------------------------------------------

class LayerUpdate {
public:
    void SetComposerClient() {
        int ret = NO_ERROR;
        if(mComposerClient == NULL) {
            mComposerClient = new SurfaceComposerClient;
            ret = mComposerClient->initCheck();
            if(ret == NO_ERROR) {
                printf("ComposerClient initCheck ok\n");
            }
        }
    }

    void SetUp() {
        int ret = NO_ERROR;

        sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
                ISurfaceComposer::eDisplayIdMain));
        DisplayInfo info;
        SurfaceComposerClient::getDisplayInfo(display, &info);
        printf("orientation:%d width:%d height:%d\n", info.orientation, info.w, info.h);

        if (localOrientation != info.orientation) {
            localOrientation = info.orientation;
            if(mBGSurfaceControl != NULL) {
            }
        } else {
            return;
        }

        SetComposerClient();

        ssize_t displayWidth = info.w;
        ssize_t displayHeight = info.h;
        if (info.orientation != DISPLAY_ORIENTATION_0 &&
                info.orientation != DISPLAY_ORIENTATION_180) {
            displayWidth = info.h;
            displayHeight = info.w;
        }

        // Background surface
        mBGSurfaceControl = mComposerClient->createSurface(
                String8("BG Test Surface"), displayWidth, displayHeight,
                PIXEL_FORMAT_RGBA_8888, 0);
        if(mBGSurfaceControl == NULL) {
            printf("mSurfaceControl == NULL\n");
            goto setup_error;
        }
        if(!mBGSurfaceControl->isValid()) {
            printf("mSurfaceControl->isValid()\n");
            goto setup_error;
        }

        //flipSurfaceRGBA8(mBGSurfaceControl);
        flipSurfaceBySkia(mBGSurfaceControl);
        SurfaceComposerClient::openGlobalTransaction();

        //ret = mBGSurfaceControl->setLayer(INT_MAX);
        //(z=161000 name=StatusBar) (z=2000002 name=BlackSurface)
        ret = mBGSurfaceControl->setLayer(1000000);
        if(ret != NO_ERROR) {
            printf("SurfaceControl->setLayer ok\n");
            goto setup_error;
        }

        SurfaceComposerClient::closeGlobalTransaction(true);
        printf("closeGlobalTransaction ok\n");

        return;

setup_error:
        TearDown();
    }

    void TearDown() {
        mComposerClient->dispose();
        mBGSurfaceControl = 0;
        mFGSurfaceControl = 0;
        mSyncSurfaceControl = 0;
        mComposerClient = 0;
    }

    void waitForPostedBuffers() {
        // Since the sync surface is in synchronous mode (i.e. double buffered)
        // posting three buffers to it should ensure that at least two
        // SurfaceFlinger::handlePageFlip calls have been made, which should
        // guaranteed that a buffer posted to another Surface has been retired.
        fillSurfaceRGBA8(mSyncSurfaceControl, 31, 31, 31);
        fillSurfaceRGBA8(mSyncSurfaceControl, 31, 31, 31);
        fillSurfaceRGBA8(mSyncSurfaceControl, 31, 31, 31);
    }

    sp<SurfaceComposerClient> mComposerClient;
    sp<SurfaceControl> mBGSurfaceControl;
    sp<SurfaceControl> mFGSurfaceControl;

    uint8_t localOrientation = 0xFF;

    // This surface is used to ensure that the buffers posted to
    // mFGSurfaceControl have been picked up by SurfaceFlinger.
    sp<SurfaceControl> mSyncSurfaceControl;
};

}

LayerUpdate mLayerUpdate;

#ifdef GET_DISPLAY_EVENT
void onDisplayEvent(int displayId, int event)
{
    printf("onDisplayEvent displayId=%d event=%d\n", displayId, event);
    usleep(1000 * 100);
    mLayerUpdate.SetUp();
}
#endif

int main()
{
    printf("Starting " LOG_TAG "\n");
#ifdef GET_DISPLAY_EVENT
    sp<Remote> remote = Remote::getInstance();
    remote->setListener(&onDisplayEvent);
    printf("setListener Done\n");
#endif

    mLayerUpdate.SetUp();

#ifdef GET_DISPLAY_EVENT
    printf("\n\njoinThreadPool!\n");
    IPCThreadState::self()->joinThreadPool();
#else
    printf("\n\npress Enter Key to finish!\n");
    getchar();
#endif

    printf("\n\never get here!\n");
    mLayerUpdate.TearDown();

    return 0;
}
