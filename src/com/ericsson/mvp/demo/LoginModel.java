/*------------------------------------------------------------------------------
 * COPYRIGHT Ericsson 2017
 *
 * The copyright to the computer program(s) herein is the property of
 * Ericsson Inc. The programs may be used and/or copied only with written
 * permission from Ericsson Inc. or in accordance with the terms and
 * conditions stipulated in the agreement/contract under which the
 * program(s) have been supplied.
 *----------------------------------------------------------------------------*/
package com.ericsson.mvp.demo;

import android.os.Handler;
import android.text.TextUtils;
import android.util.Log;

import com.ericsson.mvp.model.Imodel;

public class LoginModel implements Imodel, ILoginModel {

    private final String TAG = "LoginModel";

    //    interface OnLoginFinishedListener {
    //        void onUsernameError();
    //
    //        void onPasswordError();
    //
    //        void onSuccess();
    //    }

    @Override
    public void login(final String username, final String password, final OnLoginFinishedListener listener) {
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "try to login, username: " + username + " ,password: " + password);
                boolean error = false;
                if (TextUtils.isEmpty(username)) {
                    listener.onUsernameError();
                    error = true;
                    return;
                }
                if (TextUtils.isEmpty(password)) {
                    listener.onPasswordError();
                    error = true;
                    return;
                }
                if (!error) {
                    listener.onSuccess();
                }
            }
        }, 2000);

    }
}
