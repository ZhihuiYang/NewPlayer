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

import android.content.Context;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;

import com.ericsson.lispmediaplayer.R;
import com.ericsson.mvp.presenter.IPresenter;
import com.ericsson.mvp.util.EventUtil;
import com.ericsson.mvp.view.MyView;

public class LoginView extends MyView implements ILoginView {

    private ProgressBar progressBar;
    private EditText username;
    private EditText password;
    private Button login;
    private Context context;

    @Override
    public int getLayoutId() {
        // TODO Auto-generated method stub
        return R.layout.activity_login;
    }

    @Override
    public void initWidget() {
        progressBar = (ProgressBar) findViewById(R.id.progress);
        username = (EditText) findViewById(R.id.username);
        password = (EditText) findViewById(R.id.password);
        login = (Button) findViewById(R.id.button);
        context = rootView.getContext();
    }

    @Override
    public void bindPresenter(IPresenter presenter) {
        // TODO Auto-generated method stub
        super.bindPresenter(presenter);
    }

    @Override
    public String getUsername() {
        return username.getText().toString();
    }

    @Override
    public String getPassword() {
        return password.getText().toString();
    }

    @Override
    public void bindEvent() {
        // TODO Auto-generated method stub
        EventUtil.click(presenter, login);
    }

    @Override
    public void showProgress() {
        progressBar.setVisibility(View.VISIBLE);
    }

    @Override
    public void hideProgress() {
        progressBar.setVisibility(View.GONE);
    }

    @Override
    public void setUsernameError() {
        username.setError(context.getString(R.string.username_error));
    }

    @Override
    public void setPasswordError() {
        password.setError(context.getString(R.string.password_error));
    }

}
