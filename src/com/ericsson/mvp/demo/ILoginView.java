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

public interface ILoginView {

    public String getUsername();

    public String getPassword();

    public void showProgress();

    public void hideProgress();

    public void setUsernameError();

    public void setPasswordError();
}
