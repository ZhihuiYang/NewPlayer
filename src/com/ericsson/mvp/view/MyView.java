/*------------------------------------------------------------------------------
 * COPYRIGHT Ericsson 2017
 *
 * The copyright to the computer program(s) herein is the property of
 * Ericsson Inc. The programs may be used and/or copied only with written
 * permission from Ericsson Inc. or in accordance with the terms and
 * conditions stipulated in the agreement/contract under which the
 * program(s) have been supplied.
 *----------------------------------------------------------------------------*/
package com.ericsson.mvp.view;

import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.ericsson.mvp.presenter.IPresenter;

public abstract class MyView implements IView {

    protected final SparseArray<View> viewArray = new SparseArray<View>();

    protected View rootView;

    protected IPresenter presenter;

    @Override
    public View create(LayoutInflater inflater, ViewGroup container) {
        // TODO Auto-generated method stub
        int rootLayoutId = getLayoutId();
        rootView = inflater.inflate(rootLayoutId, container, false);
        initWidget();
        return rootView;
    }

    @Override
    public <V extends View> V findViewById(int id) {
        return (V) rootView.findViewById(id);
    }

    @Override
    public void initWidget() {

    }

    @Override
    public void bindPresenter(IPresenter presenter) {
        this.presenter = presenter;

    }

    @Override
    public void bindEvent() {

    }
}
