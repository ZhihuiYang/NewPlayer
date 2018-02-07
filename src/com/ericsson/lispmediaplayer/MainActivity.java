package com.ericsson.lispmediaplayer;

import android.app.Activity;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;

public class MainActivity extends Activity implements SurfaceHolder.Callback {

    private EditText m_UrlText;
    private SurfaceView m_RenderSurfaceView;
    private SurfaceHolder m_RenderSurfaceHolder;
    private Button m_StartBtn;
    private Button m_StopBtn;
    private CheckBox m_VRPlayBtn;
    private String m_StreamingUrl = null;
    private boolean m_isPlaying = false;
    private boolean m_isVRPlaying = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main_ffmpeg_demo);

        m_UrlText = (EditText) findViewById(R.id.url);
        m_RenderSurfaceView = (SurfaceView) findViewById(R.id.renderSurfaceView);
        m_RenderSurfaceHolder = m_RenderSurfaceView.getHolder();
        m_RenderSurfaceHolder.addCallback(this);
        m_StartBtn = (Button) findViewById(R.id.startButton);
        m_StopBtn = (Button) findViewById(R.id.stopButton);
        m_VRPlayBtn = (CheckBox) findViewById(R.id.vrPlayBtn);
        setAssetManager(getAssets());
        m_StartBtn.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                //Start streaming play
                if (!m_isPlaying) {

                    m_StreamingUrl = m_UrlText.getText().toString();
                    if (m_StreamingUrl.length() > 0) {
                        setSurfaceView(m_RenderSurfaceHolder.getSurface());
                        startPlay(m_StreamingUrl, m_isVRPlaying);
                        m_isPlaying = true;
                    }
                }
            }
        });

        m_VRPlayBtn.setOnCheckedChangeListener(new OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                // TODO Auto-generated method stub
                if (m_VRPlayBtn.isChecked()) {
                    m_isVRPlaying = true;
                    Log.d("MainActivity", "VR Playing");
                } else {
                    m_isVRPlaying = false;
                    Log.d("MainActivity", "2D Playing");
                }

            }

        });

        m_StopBtn.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                //Stop streaming play
                if (m_isPlaying) {
                    stopPlay();
                    m_isPlaying = false;
                }
            }
        });
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        if (m_isPlaying) {
            stopPlay();
            m_isPlaying = false;
        }
    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();
        if (m_isPlaying) {
            stopPlay();
            m_isPlaying = false;
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // TODO Auto-generated method stub

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // TODO Auto-generated method stub

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        // TODO Auto-generated method stub
        super.onConfigurationChanged(newConfig);
    }

    private native void setAssetManager(AssetManager assetManager);

    private native int setSurfaceView(Surface surface);

    private native int startPlay(String url, boolean isVRPlaying);

    private native int stopPlay();

    static {
        System.loadLibrary("ffmpegPlayerDemo");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
    }
}
