package com.fimi.gh2.tracker;

import android.graphics.Bitmap;
import android.support.annotation.NonNull;

import java.io.File;

public class ImagesStitchUtil {
    public final static int OK = 0;
    public final static int ERR_NEED_MORE_IMGS = 1;
    public final static int ERR_HOMOGRAPHY_EST_FAIL = 2;
    public final static int ERR_CAMERA_PARAMS_ADJUST_FAIL = 3;

    static {
        System.loadLibrary("Stitcher");
    }


    public static void StitchImages(String paths[], @NonNull onStitchResultListener listener) {
        for (String path : paths) {
            if (!new File(path).exists()) {
                listener.onError("无法读取文件或文件不存在:" + path);
                return;
            }
        }
        int wh[] = stitchImages(paths);
        switch (wh[0]) {
            case OK: {
                Bitmap bitmap = Bitmap.createBitmap(wh[1], wh[2], Bitmap.Config.ARGB_8888);
                int result = getBitmap(bitmap);
                if (result == OK && bitmap != null){
                    listener.onSuccess(bitmap);
                }else{
                    listener.onError("图片合成失败");
                }
            }
            break;
            case ERR_NEED_MORE_IMGS: {
                listener.onError("需要更多图片");
                return;
            }
            case ERR_HOMOGRAPHY_EST_FAIL: {
                listener.onError("图片对应不上");
                return;
            }
            case ERR_CAMERA_PARAMS_ADJUST_FAIL: {
                listener.onError("图片参数处理失败");
                return;
            }
        }
    }


    private native static int[] stitchImages(String path[]);

    private native static void getMat(long mat);

    private native static int getBitmap(Bitmap bitmap);


    public interface onStitchResultListener {

        void onSuccess(Bitmap bitmap);

        void onError(String errorMsg);
    }


}
