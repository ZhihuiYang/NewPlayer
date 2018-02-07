package com.ericsson.lispmediaplayer;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;

public class MainActivityLispMP extends Activity {
    private CheckBox isSender;
    private EditText remoteIP;
    private EditText remotePort;
    private EditText localIP;
    private EditText localPort;
    private Button button;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main_lispmp);
        //test();
        isSender = (CheckBox) findViewById(R.id.isSender);
        remoteIP = (EditText) findViewById(R.id.Remote_IP);
        remotePort = (EditText) findViewById(R.id.Remote_Port);
        localIP = (EditText) findViewById(R.id.Local_IP);
        localPort = (EditText) findViewById(R.id.Local_Port);
        button = (Button) findViewById(R.id.button);
        button.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                if (isSender.isChecked()) {
                    //sender
                    String ip = remoteIP.getText().toString();
                    String port = remotePort.getText().toString();
                    String[] sendStrings = { ip, port };
                    send(sendStrings);
                } else {
                    String localip = localIP.getText().toString();
                    String localport = localPort.getText().toString();
                    String remoteip = remoteIP.getText().toString();
                    String remoteport = remotePort.getText().toString();
                    String[] recvStrings = { localip, localport, remoteip, remoteport };
                    receive(recvStrings);
                }
            }
        });

    }

    private native void test();

    private native int send(String[] argv);

    private native int receive(String[] argv);

    static {
        System.loadLibrary("mediaPlayer");
    }
}
