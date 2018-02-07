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

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.ericsson.mvp.presenter.IPresenter;

public interface IView {

    View create(LayoutInflater inflater, ViewGroup container);

    void initWidget();

    int getLayoutId();

    <V extends View> V findViewById(int id);

    void bindPresenter(IPresenter presenter);

    void bindEvent();
}
