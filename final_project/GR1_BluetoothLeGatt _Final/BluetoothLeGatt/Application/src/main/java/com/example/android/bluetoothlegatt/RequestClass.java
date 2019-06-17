package com.example.android.bluetoothlegatt;

/**
 * Created by alexrichardson on 4/7/17. Derived from http://docs.aws.amazon.com/lambda/latest/dg/with-ondemand-android-mobile-create-app.html
 */

public class RequestClass {
    String reqInfo;

    public String getData() {
        return reqInfo;
    }

    public void setData(String reqInfo) {
        this.reqInfo = reqInfo;
    }

    public RequestClass( String reqInfo) {
        this.reqInfo = reqInfo;
    }

    public RequestClass() {
    }
}