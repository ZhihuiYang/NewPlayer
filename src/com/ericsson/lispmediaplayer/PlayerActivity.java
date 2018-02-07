package com.ericsson.lispmediaplayer;

import android.app.Activity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class PlayerActivity extends Activity implements SurfaceHolder.Callback {

    private SurfaceView renderView = null;
    private SurfaceHolder renderViewHolder = null;
    private Player player = null;
    private Button startBt;
    private Button pauseBt;
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
        super.onCreate(savedInstanceState);
        setContentView(R.layout.player_layout);

        renderView = (SurfaceView) findViewById(R.id.player_renderView);
        startBt = (Button) findViewById(R.id.player_start);
        pauseBt = (Button) findViewById(R.id.player_pause);
        stopBt = (Button) findViewById(R.id.player_stop);
        renderViewHolder = renderView.getHolder();
        renderViewHolder.addCallback(this);
        renderViewHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        remoteIP = (EditText) findViewById(R.id.player_Remote_IP);
        remotePort = (EditText) findViewById(R.id.player_Remote_Port);
        localIP = (EditText) findViewById(R.id.player_Local_IP);
        localPort = (EditText) findViewById(R.id.player_Local_Port);

        player = new Player(videoWidth, videoHeight, getAssets(), this);

        startBt.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                String localip = localIP.getText().toString();
                String localport = localPort.getText().toString();
                String remoteip = remoteIP.getText().toString();
                String remoteport = remotePort.getText().toString();
                player.setLocalIpAndPort(localip, (short) Integer.parseInt(localport));
                player.setRemoteIpAndPort(remoteip, (short) Integer.parseInt(remoteport));

                try {
                    if (isFirstTime) {
                        player.setSurfaceView(renderView);
                        player.init();
                        player.open();
                        isFirstTime = false;
                    }
                    player.play();
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                startBt.setEnabled(false);
                pauseBt.setEnabled(true);
                stopBt.setEnabled(true);
            }

        });

        pauseBt.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                player.pause();
                startBt.setEnabled(true);
                pauseBt.setEnabled(false);
                stopBt.setEnabled(true);
            }

        });

        stopBt.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                player.stop();
                startBt.setEnabled(true);
                pauseBt.setEnabled(true);
                stopBt.setEnabled(false);
            }

        });
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        player.close();
        player.destroy();
        super.onDestroy();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // TODO Auto-generated method stub

    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // TODO Auto-generated method stub

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        // TODO Auto-generated method stub

    }

}
