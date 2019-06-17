package com.example.android.bluetoothlegatt;

/**
 * Created by alexrichardson on 4/7/17.
 */

public class ResponseClass {
    String resInfo;

    public String getData() {
        return resInfo;
    }

    public void setData(String resInfo) {
        this.resInfo = resInfo;
    }

    public ResponseClass(String resInfo) {
        this.resInfo = resInfo;
    }

    public ResponseClass() {
    }
}
