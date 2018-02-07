/*------------------------------------------------------------------------------
 * COPYRIGHT Ericsson 2017
 *
 * The copyright to the computer program(s) herein is the property of
 * Ericsson Inc. The programs may be used and/or copied only with written
 * permission from Ericsson Inc. or in accordance with the terms and
 * conditions stipulated in the agreement/contract under which the
 * program(s) have been supplied.
 *----------------------------------------------------------------------------*/
package com.ericsson.lispmediaplayer;

import java.util.Arrays;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import android.content.Context;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraDevice.StateCallback;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.media.ImageReader;
import android.media.ImageReader.OnImageAvailableListener;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.util.Size;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class Camera2Recorder {

    private String TAG = "Camera2Recorder";
    private String cameraId;
    private CameraCaptureSession captureSession;
    private CameraDevice cameraDevice;
    private Size previewSize;
    private CameraManager cameraManger;
    private SurfaceView surfaceView;
    private SurfaceHolder surfaceHolder;
    private Context context;
    private Handler handler;
    private ImageReader imageReader;
    private Semaphore cameraOpenCloseLock = new Semaphore(1);
    private OnImageAvailableListener onImageAvailableListener;
    private HandlerThread backgroundThread;
    private Handler backgroundHandler;
    private Builder previewRequestBuilder;
    private CaptureRequest previewRequest;

    public Camera2Recorder(Context context) {
        this.context = context;
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        this.surfaceView = surfaceView;
        this.surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(new SurfaceHolder.Callback() {

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                // TODO Auto-generated method stub

            }

            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                initCamera();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                // TODO Auto-generated method stub

            }
        });
    }

    public void initCamera() {
        cameraManger = (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);

        HandlerThread handlerThread = new HandlerThread("Camera2");
        handlerThread.start();
        handler = new Handler(handlerThread.getLooper());

        try {
            cameraId = "" + CameraMetadata.LENS_FACING_FRONT;

            imageReader = ImageReader.newInstance(surfaceView.getWidth(), surfaceView.getHeight(),
                    ImageFormat.YUV_420_888, 1);
            imageReader.setOnImageAvailableListener(onImageAvailableListener, handler);

            if (!cameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening");
            }

            cameraManger.openCamera(cameraId, deviceStateCallback, handler);
        } catch (CameraAccessException e) {
            Log.e(TAG, "open camera failed." + e.getLocalizedMessage());
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }

    }

    private StateCallback deviceStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(CameraDevice camera) {
            Log.d(TAG, "deviceStateCallback, camera was opened");
            cameraOpenCloseLock.release();
            cameraDevice = camera;

        }

        @Override
        public void onError(CameraDevice camera, int error) {
            Log.d(TAG, "deviceStateCallback, camera pop error: " + error);
            cameraOpenCloseLock.release();
            cameraDevice.close();
            cameraDevice = null;

        }

        @Override
        public void onDisconnected(CameraDevice camera) {
            Log.d(TAG, "deviceStateCallback, camera disconnected");
            cameraOpenCloseLock.release();
            cameraDevice.close();
            cameraDevice = null;

        }
    };

    private void closeCamera() {
        try {
            cameraOpenCloseLock.acquire();
            if (captureSession != null) {
                captureSession.close();
                captureSession = null;
            }

            if (cameraDevice != null) {
                cameraDevice.close();
                cameraDevice = null;
            }

            if (imageReader != null) {
                imageReader.close();
                imageReader = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing: ", e);
        }
    }

    private void startBackgroundThread() {
        backgroundThread = new HandlerThread("Camera2Background");
        backgroundThread.start();
        backgroundHandler = new Handler(backgroundThread.getLooper());
    }

    private void stopBackgroundThread() {
        backgroundThread.quitSafely();
        try {
            backgroundThread.join();
            backgroundThread = null;
            backgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void createCameraPreviewSession() {
        try {
            previewRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            previewRequestBuilder.addTarget(surfaceHolder.getSurface());
            cameraDevice.createCaptureSession(Arrays.asList(surfaceHolder.getSurface(), imageReader.getSurface()),
                    captureSessionStateCallback, backgroundHandler);
        } catch (CameraAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

    private CameraCaptureSession.StateCallback captureSessionStateCallback = new CameraCaptureSession.StateCallback() {

        @Override
        public void onConfigured(CameraCaptureSession session) {
            if (cameraDevice == null) {
                return;
            }

            captureSession = session;
            previewRequestBuilder.set(CaptureRequest.CONTROL_AE_MODE, CameraMetadata.CONTROL_AF_MODE_CONTINUOUS_VIDEO);
            setAutoFlash(previewRequestBuilder);

            previewRequest = previewRequestBuilder.build();
            try {
                captureSession.setRepeatingRequest(previewRequest, captureCallback, backgroundHandler);
            } catch (CameraAccessException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }

        @Override
        public void onConfigureFailed(CameraCaptureSession session) {
            // TODO Auto-generated method stub

        }
    };

    private CameraCaptureSession.CaptureCallback captureCallback = new CameraCaptureSession.CaptureCallback() {

        private void process(CaptureResult result) {

        }

        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result) {
            // TODO Auto-generated method stub
            super.onCaptureCompleted(session, request, result);
        }

        @Override
        public void onCaptureProgressed(CameraCaptureSession session, CaptureRequest request,
                CaptureResult partialResult) {
            // TODO Auto-generated method stub
            super.onCaptureProgressed(session, request, partialResult);
        }

    };

    private void setAutoFlash(CaptureRequest.Builder requestBuilder) {

        requestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CameraMetadata.CONTROL_AE_MODE_ON_AUTO_FLASH);
    }
}
