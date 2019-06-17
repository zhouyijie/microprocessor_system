package com.example.android.bluetoothlegatt;

import com.amazonaws.mobileconnectors.lambdainvoker.LambdaFunction;


public interface LambdaInterface {

    /**
     * Invoke the Lambda function "AndroidBackendLambdaFunction".
     * The function name is the method name.
     */
    @LambdaFunction
    ResponseClass testLambda(RequestClass request);

}
