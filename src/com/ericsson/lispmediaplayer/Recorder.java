/*------------------------------------------------------------------------------
 * COPYRIGHT Ericsson 2016
 *
 * The copyright to the computer program(s) herein is the property of
 * Ericsson Inc. The programs may be used and/or copied only with written
 * permission from Ericsson Inc. or in accordance with the terms and
 * conditions stipulated in the agreement/contract under which the
 * program(s) have been supplied.
 *----------------------------------------------------------------------------*/
package com.ericsson.lispmediaplayer;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * The purpose of this class is providing capabilities of capture
 * video data, then encode it to H264 streaming and use RTP protocol
 * to send.
 */
public class Recorder {
    private static final String TAG = "JavaRecorder";
    private String localIp;
    private short localPort;
    private String remoteIp;
    private Short remotePort;
    private Camera camera = null;
    private AudioRecord audioRecord = null;
    private SurfaceView previewSurfaceView;
    private SurfaceHolder previewSurfaceHolder;
    private int surfaceViewWidth;
    private int surfaceViewHeight;
    private int videoWidth;
    private int videoHeight;

    private static final int AUDIO_DEFAULT_SAMPLE_RATE = 44100;
    private static final int AUDIO_DEFAULT_CHANNEL_CONFIG = AudioFormat.CHANNEL_IN_STEREO;

    /**
     * Creates a new instance of <code>Recorder</code>.
     * 
     * @param videoWidth the video width you want Recorder to capture.
     * @param videoHeight the video height you want Recorder to
     *            capture.
     */
    public Recorder(int videoWidth, int videoHeight) {
        this.videoHeight = videoHeight;
        this.videoWidth = videoWidth;
        recorderGetInstance();
    }

    /**
     * set preview <code>SurfaceView</code> to Recorder.
     * 
     * @param view the preview <code>SurfaceView</code>.
     */
    public void setSurfaceView(SurfaceView view) {
        this.previewSurfaceView = view;
        this.previewSurfaceHolder = previewSurfaceView.getHolder();
        this.surfaceViewWidth = previewSurfaceView.getWidth();
        this.surfaceViewHeight = previewSurfaceView.getHeight();
    }

    /**
     * set local IP address and port to Recorder.
     * 
     * @param ip the value is local IP address.
     * @param port the value is local port which used to send data.
     */
    public void setLocalIpAndPort(String ip, short port) {
        this.localIp = ip;
        this.localPort = port;
    }

    /**
     * set remote IP address and port to Recorder.
     * 
     * @param ip the value is remote IP address, where the application
     *            want to send data to.
     * @param port the value is remote port.
     */
    public void setRemoteIpAndPort(String ip, short port) {
        this.remoteIp = ip;
        this.remotePort = port;
    }

    /**
     * Initialize Recorder.
     * 
     * @return <code>true</code> means initialize success,
     *         <code>false</code> means failed.
     * @throws Exception Exceptione may poped up when initialize
     *             camera.
     */
    public int initCamera() throws Exception {
        int error = 0;
        if (camera == null) {
            try {
                this.camera = Camera.open(Camera.CameraInfo.CAMERA_FACING_BACK);
            } catch (Exception e) {
                Log.e(TAG, "open camera failed, Exception: " + e.getMessage() + " poped up");
                camera = null;
                throw e;
            }
        }
        if (camera != null) {
            try {
                camera.setPreviewCallback(new PreviewCallback() {

                    @Override
                    public void onPreviewFrame(byte[] data, Camera camera) {
                        nativeSendYuvData(data, videoWidth, videoHeight);
                    }

                });
            } catch (Exception e) {
                Log.e(TAG, "camera set preview callback failed, Exception: " + e.getMessage() + "poped up");
                camera.release();
                camera = null;
                throw e;
            }

            Parameters para = camera.getParameters();
            para.setPreviewFrameRate(30);
            para.setPreviewFormat(ImageFormat.NV21);
            para.setPreviewSize(videoWidth, videoHeight);
            camera.setParameters(para);
            camera.setPreviewDisplay(previewSurfaceHolder);
        }

        recorderSetLocalIpAndPort(this.localIp, this.localPort);
        recorderSetRemoteIpAndPort(this.remoteIp, this.remotePort);

        error = recorderInit();

        return error;
    }

    public void initAudio() {

    }

    /**
     * trigger camera to capture picture, and callback the picture
     * data as preview data.
     */
    public void startPreview() {
        if (camera != null) {
            camera.startPreview();
        }
    }

    /**
     * stop camera preview.
     * 
     * @throws Exception exceptions may be pop up during processing.
     */
    public synchronized void stopPreview() throws Exception {
        if (camera != null) {
            try {
                camera.setPreviewCallback(null);
            } catch (Exception e) {
                Log.e(TAG, "unbind camera preview callback failed, Exception: " + e.getMessage() + "poped up");
                throw e;
            }

            try {
                camera.stopPreview();
            } catch (Exception e) {
                Log.e(TAG, "stopPreview failed, Exception: " + e.getMessage() + "poped up");
                throw e;
            }
        }
    }

    /**
     * destroy Recorder
     * 
     * @throws Exception exceptions may pop up when release camera.
     */
    public void destroy() throws Exception {
        try {
            camera.release();
            camera = null;
        } catch (Exception e) {
            Log.e(TAG, "release camera failed, Exception: " + e.getMessage() + "poped up");
            throw e;
        } finally {
            recorderDestroy();
            //recorderReleaseInstance();
        }
    }

    /**
     * start recording.
     */
    public void start() {
        startPreview();
        recorderStart();
    }

    /**
     * stop recording.
     */
    public void stop() {
        try {
            stopPreview();
        } catch (Exception e) {
            // TODO Auto-generated catch block
            Log.e(TAG, "stop camera preview failed, Exception: " + e.getMessage() + "Poped up");
            e.printStackTrace();
        } finally {
            recorderStop();
        }
    }

    @Override
    protected void finalize() throws Throwable {
        // TODO Auto-generated method stub
        recorderReleaseInstance();
        super.finalize();
    }

    private native int recorderGetInstance();

    private native int recorderReleaseInstance();

    private native int recorderInit();

    private native int recorderDestroy();

    private native int recorderSetLocalIpAndPort(String ip, short port);

    private native int recorderSetRemoteIpAndPort(String ip, short port);

    private native int recorderStart();

    private native int recorderStop();

    private native void nativeSendYuvData(byte[] yuvArray, int width, int height);

    static {
        System.loadLibrary("openh264");
        System.loadLibrary("mediaPlayer");
    }

}
