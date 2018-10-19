#include <jni.h>
#include <vector>
#include <android/log.h>

#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <android/log.h>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <android/bitmap.h>


using namespace cv;
using namespace std;

char filepath1[100] = "/storage/emulated/0/Download/MOAAP/Chapter6/panorama_stitched.jpg";

#define  LOG_TAG    "DDLOG_JNI"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


void MatToBitmap(JNIEnv *env, Mat &mat, jobject &bitmap, jboolean needPremultiplyAlpha) {
    AndroidBitmapInfo info;
    void *pixels = 0;
    Mat &src = mat;

    try {

        LOGD("nMatToBitmap");
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        LOGD("nMatToBitmap1");

        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        LOGD("nMatToBitmap2 :%d  : %d  :%d", src.dims, src.rows, src.cols);

        CV_Assert(src.dims == 2 && info.height == (uint32_t) src.rows &&
                  info.width == (uint32_t) src.cols);
        LOGD("nMatToBitmap3");
        CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4);
        LOGD("nMatToBitmap4");
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        LOGD("nMatToBitmap5");
        CV_Assert(pixels);
        LOGD("nMatToBitmap6");
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
//            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            Mat tmp(info.height, info.width, CV_8UC3, pixels);
            if (src.type() == CV_8UC1) {
                LOGD("nMatToBitmap: CV_8UC1 -> RGBA_8888");
                cvtColor(src, tmp, COLOR_GRAY2RGBA);
            } else if (src.type() == CV_8UC3) {
                LOGD("nMatToBitmap: CV_8UC3 -> RGBA_8888");
                cvtColor(src, tmp, COLOR_RGB2RGBA);
            } else if (src.type() == CV_8UC4) {
                LOGD("nMatToBitmap: CV_8UC4 -> RGBA_8888");
                if (needPremultiplyAlpha)
                    cvtColor(src, tmp, COLOR_RGBA2mRGBA);
                else
                    src.copyTo(tmp);
            }
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            if (src.type() == CV_8UC1) {
                LOGD("nMatToBitmap: CV_8UC1 -> RGB_565");
                cvtColor(src, tmp, COLOR_GRAY2BGR565);
            } else if (src.type() == CV_8UC3) {
                LOGD("nMatToBitmap: CV_8UC3 -> RGB_565");
                cvtColor(src, tmp, COLOR_RGB2BGR565);
            } else if (src.type() == CV_8UC4) {
                LOGD("nMatToBitmap: CV_8UC4 -> RGB_565");
                cvtColor(src, tmp, COLOR_RGBA2BGR565);
            }
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch (const cv::Exception &e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nMatToBitmap catched cv::Exception: %s", e.what());
        jclass je = env->FindClass("org/opencv/core/CvException");
        if (!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nMatToBitmap catched unknown exception (...)");
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nMatToBitmap}");
        return;
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_fimi_gh2_tracker_ImagesStitchUtil_StitchPanorama(JNIEnv *env, jobject,
                                                          jlongArray imageAddressArray,
                                                          jlong outputAddress) {
       jsize a_len = env->GetArrayLength(imageAddressArray);
       jlong *imgAddressArr = env->GetLongArrayElements(imageAddressArray, 0);
       vector<Mat> imgVec;
       for (int k = 0; k < a_len; k++) {
           Mat &curimage = *(Mat *) imgAddressArr[k];
           Mat newimage;
           curimage.copyTo(newimage);
           float scale = 500.0f / curimage.rows;
           resize(newimage, newimage,
                  Size((int) (scale * curimage.cols), (int) (scale * curimage.rows)));
           LOGD("Image height %d width %d", newimage.rows, newimage.cols);

           imgVec.push_back(newimage);
       }
       Mat &result = *(Mat *) outputAddress;

       Stitcher stitcher = Stitcher::createDefault();
       LOGD("Stitching..........");
       Stitcher::Status status1 = stitcher.stitch(imgVec, result);

       LOGD("Result height %d width %d", result.rows, result.cols);

   //    imwrite(filepath1, result);

       return status1;
}
