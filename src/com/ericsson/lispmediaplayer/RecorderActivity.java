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

import android.app.Activity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

/**
 * The purpose of this class is ... TODO javadoc for class Recorder
 */
public class RecorderActivity extends Activity implements SurfaceHolder.Callback {

    private SurfaceView preview = null;
    private SurfaceHolder previewHolder = null;
    private Recorder recorder = null;
    private Button startBt;
    private Button stopBt;
    private int videoWidth = 640;
    private int videoHeight = 480;
    private EditText remoteIP;
    private EditText remotePort;
    private EditText localIP;
    private EditText localPort;
    private static boolean isFirstTime = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.recoder_layout);

        preview = (SurfaceView) findViewById(R.id.recorder_Preview);
        startBt = (Button) findViewById(R.id.recorder_start);
        stopBt = (Button) findViewById(R.id.recorder_stop);
        previewHolder = preview.getHolder();
        previewHolder.addCallback(this);
        previewHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        remoteIP = (EditText) findViewById(R.id.recorder_Remote_IP);
        remotePort = (EditText) findViewById(R.id.recorder_Remote_Port);
        localIP = (EditText) findViewById(R.id.recorder_Local_IP);
        localPort = (EditText) findViewById(R.id.recorder_Local_Port);

        recorder = new Recorder(videoWidth, videoHeight);

        startBt.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                String localip = localIP.getText().toString();
                String localport = localPort.getText().toString();
                String remoteip = remoteIP.getText().toString();
                String remoteport = remotePort.getText().toString();
                recorder.setLocalIpAndPort(localip, (short) Integer.parseInt(localport));
                recorder.setRemoteIpAndPort(remoteip, (short) Integer.parseInt(remoteport));

                try {
                    if (isFirstTime) {
                        recorder.setSurfaceView(preview);
                        recorder.initCamera();
                        isFirstTime = false;
                    }
                    recorder.start();
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                startBt.setEnabled(false);
                stopBt.setEnabled(true);
            }

        });

        stopBt.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                recorder.stop();
                startBt.setEnabled(true);
                stopBt.setEnabled(false);
            }

        });
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        try {
            recorder.stop();
            recorder.destroy();
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        super.onDestroy();
    }

    @Override
    public void surfaceCreated(SurfaceHolder paramSurfaceHolder) {
        // TODO Auto-generated method stub
    }

    @Override
    public void surfaceChanged(SurfaceHolder paramSurfaceHolder, int paramInt1, int paramInt2, int paramInt3) {
        // TODO Auto-generated method stub

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder paramSurfaceHolder) {
        // TODO Auto-generated method stub
    }

}
