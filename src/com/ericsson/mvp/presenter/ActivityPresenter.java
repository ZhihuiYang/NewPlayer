/*------------------------------------------------------------------------------
 * COPYRIGHT Ericsson 2017
 *
 * The copyright to the computer program(s) herein is the property of
 * Ericsson Inc. The programs may be used and/or copied only with written
 * permission from Ericsson Inc. or in accordance with the terms and
 * conditions stipulated in the agreement/contract under which the
 * program(s) have been supplied.
 *----------------------------------------------------------------------------*/
package com.ericsson.mvp.presenter;

import android.app.Activity;
import android.os.Bundle;

import com.ericsson.mvp.util.GenericUtil;
import com.ericsson.mvp.view.IView;

public abstract class ActivityPresenter<T extends IView> extends Activity implements IPresenter<T> {
    protected T view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        create(savedInstanceState);

        try {
            view = getViewClass().newInstance();
            view.bindPresenter(this);
            setContentView(view.create(getLayoutInflater(), null));
            view.bindEvent();
            created(savedInstanceState);
        } catch (InstantiationException | IllegalAccessException e) {
            // TODO Auto-generated catch block
            throw new RuntimeException(e.getMessage());
        }

    }

    @Override
    public Class<T> getViewClass() {
        // TODO Auto-generated method stub
        return GenericUtil.getViewClass(getClass());
    }

    @Override
    public void create(Bundle savedInstance) {

    }

    @Override
    public void created(Bundle savedInstance) {
        // TODO Auto-generated method stub

    }

}
