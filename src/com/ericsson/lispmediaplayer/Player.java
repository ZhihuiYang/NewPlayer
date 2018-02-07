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

import static java.lang.Math.cos;
import static java.lang.Math.sin;
import static java.lang.Math.sqrt;
import android.content.Context;
import android.content.res.AssetManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * The purpose of this class is receiver H264 data via RTP protocol,
 * then decode it and render it to a certain <code>SurfaceView</code>.
 * and
 */
public class Player implements SensorEventListener {
    private static final String TAG = "JavaPlayer";
    private String localIp;
    private short localPort;
    private String remoteIp;
    private short remotePort;
    private SurfaceView renderView;
    private SurfaceHolder renderViewHolder;
    private int renderViewWidth;
    private int renderViewHeight;
    private int videoWidth;
    private int videoHeight;
    private SensorManager mSensorManager;
    private Sensor mGyroscope;

    /**
     * Creates a new instance of <code>Player</code>.
     * 
     * @param videoWidth the video width that want to play.
     * @param videoHeight the video height that want to play.
     */
    public Player(int videoWidth, int videoHeight, AssetManager assetManager, Context context) {
        this.videoWidth = videoWidth;
        this.videoHeight = videoHeight;
        setAssetManager(assetManager);
        playerGetInstance();
        mSensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
        mGyroscope = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        mSensorManager.registerListener(this, mGyroscope, SensorManager.SENSOR_DELAY_GAME);
    }

    private static final float NS2S = 1.0f / 1000000000.0f;
    private final float[] deltaRotationVector = new float[4];
    private float timestamp;
    public static final float EPSILON = 0.000000001f;

    @Override
    public void onSensorChanged(SensorEvent event) {
        // TODO Auto-generated method stub
        // This time step's delta rotation to be multiplied by the current rotation
        // after computing it from the gyro sample data.
        if (timestamp != 0) {
            final float dT = (event.timestamp - timestamp) * NS2S;
            // Axis of the rotation sample, not normalized yet.
            float axisX = event.values[0];
            float axisY = event.values[1];
            float axisZ = event.values[2];

            // Calculate the angular speed of the sample
            float omegaMagnitude = (float) sqrt((axisX * axisX) + (axisY * axisY) + (axisZ * axisZ));

            // Normalize the rotation vector if it's big enough to get the axis
            if (omegaMagnitude > EPSILON) {
                axisX /= omegaMagnitude;
                axisY /= omegaMagnitude;
                axisZ /= omegaMagnitude;
            }

            // Integrate around this axis with the angular speed by the time step
            // in order to get a delta rotation from this sample over the time step
            // We will convert this axis-angle representation of the delta rotation
            // into a quaternion before turning it into the rotation matrix.
            float thetaOverTwo = (omegaMagnitude * dT) / 2.0f;
            float sinThetaOverTwo = (float) sin(thetaOverTwo);
            float cosThetaOverTwo = (float) cos(thetaOverTwo);
            deltaRotationVector[0] = sinThetaOverTwo * axisX;
            deltaRotationVector[1] = sinThetaOverTwo * axisY;
            deltaRotationVector[2] = sinThetaOverTwo * axisZ;
            deltaRotationVector[3] = cosThetaOverTwo;
        }
        timestamp = event.timestamp;
        float[] deltaRotationMatrix = new float[9];
        SensorManager.getRotationMatrixFromVector(deltaRotationMatrix, deltaRotationVector);
        // User code should concatenate the delta rotation we computed with the current rotation
        // in order to get the updated rotation.
        //rotationCurrent = rotationCurrent * deltaRotationMatrix;

        playerUpdateOrientation(-deltaRotationVector[0] * 2.0f, deltaRotationVector[1] * 2.0f,
                deltaRotationVector[2] * 2.0f);
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO Auto-generated method stub

    }

    /**
     * set the rendering <code>SurfaceView</code> to player.
     * 
     * @param renderView SurfaceView where the video rendering in.
     */
    public void setSurfaceView(SurfaceView renderView) {
        this.renderView = renderView;
        this.renderViewHolder = this.renderView.getHolder();
        this.renderViewWidth = this.renderView.getWidth();
        this.renderViewHeight = this.renderView.getHeight();
    }

    /**
     * set local IP address and port to player.
     * 
     * @param ip the local IP address.
     * @param port the local port.
     */
    public void setLocalIpAndPort(String ip, short port) {
        this.localIp = ip;
        this.localPort = port;
    }

    /**
     * set remote IP address and port to player.
     * 
     * @param ip the remote IP address.
     * @param port the remote port.
     */
    public void setRemoteIpAndPort(String ip, short port) {
        this.remoteIp = ip;
        this.remotePort = port;
    }

    /**
     * Initialize Player.
     * 
     * @return <code>true</code> means initialize success,
     *         <code>false</code> means failed.
     */
    public int init() {

        playerSetSurfaceView(this.renderViewHolder.getSurface());

        playerSetLocalIpAndPort(this.localIp, this.localPort);

        playerSetRemoteIpAndPort(this.remoteIp, this.remotePort);

        return playerInit();
    }

    /**
     * open the palyer.
     * 
     * @return <code>true</code> means initialize success,
     *         <code>false</code> means failed.
     */
    public int open() {
        return playerOpen();
    }

    /**
     * trigger the player to play video.
     * 
     * @return <code>true</code> means initialize success,
     *         <code>false</code> means failed.
     */
    public int play() {
        return playerPlay();
    }

    /**
     * trigger the player to pause video playing.
     * 
     * @return <code>true</code> means initialize success,
     *         <code>false</code> means failed.
     */
    public int pause() {
        return playerPause();
    }

    /**
     * trigger the player to stop video playing.
     * 
     * @return <code>true</code> means initialize success,
     *         <code>false</code> means failed.
     */
    public int stop() {
        return playerStop();
    }

    /**
     * close player.
     * 
     * @return <code>true</code> means initialize success,
     *         <code>false</code> means failed.
     */
    public int close() {
        return playerClose();
    }

    /**
     * destroy player.
     * 
     * @return <code>true</code> means initialize success,
     *         <code>false</code> means failed.
     */
    public int destroy() {
        playerDestroy();
        //playerReleaseInstance();

        return 0;
    }

    @Override
    protected void finalize() throws Throwable {
        // TODO Auto-generated method stub
        playerReleaseInstance();
        super.finalize();
    }

    private native int playerUpdateOrientation(float diffx, float diffy, float diffz);

    private native int playerGetInstance();

    private native int playerReleaseInstance();

    private native int playerInit();

    private native int playerOpen();

    private native int playerPlay();

    private native int playerClose();

    private native int playerStop();

    private native int playerPause();

    private native int playerDestroy();

    private native int playerSetSurfaceView(Surface renderView);

    private native int playerSetLocalIpAndPort(String ip, short port);

    private native int playerSetRemoteIpAndPort(String ip, short port);

    private native void setAssetManager(AssetManager assetManager);

    static {
        System.loadLibrary("openh264");
        System.loadLibrary("mediaPlayer");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
    }

}
