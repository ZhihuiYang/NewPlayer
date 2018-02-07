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

import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.widget.Toast;

import com.ericsson.lispmediaplayer.R;
import com.ericsson.mvp.presenter.ActivityPresenter;

public class LoginActivity extends ActivityPresenter<LoginView> implements View.OnClickListener,
        ILoginModel.OnLoginFinishedListener {

    private final String TAG = "LoginActivity";
    private LoginModel loginModel;

    private LoginView loginview;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        loginModel = new LoginModel();
        loginview = this.view;
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.button) {
            validateCredentials(loginview.getUsername(), loginview.getPassword());
        }
    }

    private void validateCredentials(String username, String password) {
        if (loginview != null) {
            loginview.showProgress();
        }

        loginModel.login(username, password, this);
    }

    @Override
    public void onUsernameError() {
        if (loginview != null) {
            loginview.setUsernameError();
            loginview.hideProgress();
        }
    }

    @Override
    public void onPasswordError() {
        if (loginview != null) {
            loginview.setPasswordError();
            loginview.hideProgress();
        }
    }

    @Override
    public void onSuccess() {
        if (loginview != null) {
            loginview.hideProgress();
            Log.d(TAG, "login success");
            Toast toast = Toast.makeText(getApplicationContext(), "login success", Toast.LENGTH_LONG);
            toast.setGravity(Gravity.CENTER, 0, 0);
            toast.show();
        }
    }
}
