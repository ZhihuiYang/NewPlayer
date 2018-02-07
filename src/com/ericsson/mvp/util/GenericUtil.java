/*------------------------------------------------------------------------------
 * COPYRIGHT Ericsson 2017
 *
 * The copyright to the computer program(s) herein is the property of
 * Ericsson Inc. The programs may be used and/or copied only with written
 * permission from Ericsson Inc. or in accordance with the terms and
 * conditions stipulated in the agreement/contract under which the
 * program(s) have been supplied.
 *----------------------------------------------------------------------------*/
package com.ericsson.mvp.util;

import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;

public class GenericUtil {
    public static <T> Class<T> getViewClass(Class<?> klass) {
        Type type = klass.getGenericSuperclass();
        if ((type == null) || !(type instanceof ParameterizedType)) {
            return null;
        }
        ParameterizedType parameterizedType = (ParameterizedType) type;
        Type[] types = parameterizedType.getActualTypeArguments();
        if ((types == null) || (types.length == 0)) {
            return null;
        }
        return (Class<T>) types[0];
    }
}
