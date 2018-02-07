package com.ericsson.lispmediaplayer;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MainActivityOpenSLES extends Activity {

    private String m_pcmUrl = null;
    private EditText m_UrlText;
    private Button m_PlayBtn;
    private Button m_StopBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main_activity_open_sles);
        m_UrlText = (EditText) findViewById(R.id.audioUrl);
        m_PlayBtn = (Button) findViewById(R.id.startAudioPlay);
        m_StopBtn = (Button) findViewById(R.id.stopAudioPlay);

        m_PlayBtn.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO start audio Play

            }

        });

        m_StopBtn.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO stop audio play

            }

        });

    }

    private native void createEngine();

    private native boolean createAudioPlayer(String url);

    private native void shutdown();
}
