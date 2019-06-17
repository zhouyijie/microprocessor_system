package com.example.android.bluetoothlegatt;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

import com.amazonaws.auth.CognitoCachingCredentialsProvider;
import com.amazonaws.mobileconnectors.s3.transferutility.*;
import com.amazonaws.regions.*;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.AmazonS3Client;


/**
 * Created by alexrichardson on 3/31/17.
 */

public class AWS_services {
    //Make a new connection to S3 client
    private static Context c;
    private File path;
    private AmazonS3 s3;
    public TransferUtility transferUtility;



    public void file_upload( Context context, String information, String bucket_name, String object_key) {
        c = context;

        // callback method to call credentialsProvider method.
        CognitoCachingCredentialsProvider credentialsProvider = new CognitoCachingCredentialsProvider(
                c,
                "us-east-1:f9392b49-3b25-4c2d-b277-3a66c68a287e", // Identity Pool ID
                Regions.US_EAST_1 // Region
        );
        s3 = new AmazonS3Client(credentialsProvider);

        // Set the region of your S3 bucket
        //s3.setRegion(Region.getRegion(Regions.US_EAST_1));


        File file = new File(context.getFilesDir(),"mydir");
        //if(!file.exists()){
            file.mkdir();
        //}

        try{
            File gpxfile = new File(file, object_key);
            FileWriter writer = new FileWriter(gpxfile);
            writer.append(information);
            writer.flush();
            writer.close();

            transferUtility = new TransferUtility(s3, c);


            //Uploads our stored data to the S3 cloud. Need data parsing prior to this
            TransferObserver transferObserver = transferUtility.upload(
                    bucket_name,     /* The bucket to upload to */
                    object_key,    /* The key for the uploaded object */
                    gpxfile        /* The file where the data to upload exists */
            );
            transferObserverListener(transferObserver);

        }catch (Exception e){
            Log.e("Exception", "File write failed: " + e.toString());
        }

    }

    public void file_download(Context context, String bucket_name, String object_key) {
        c = context;

        // callback method to call credentialsProvider method.
        CognitoCachingCredentialsProvider credentialsProvider = new CognitoCachingCredentialsProvider(
                context,
                "us-east-1:f9392b49-3b25-4c2d-b277-3a66c68a287e", // Identity Pool ID
                Regions.US_EAST_1 // Region
        );

        s3 = new AmazonS3Client(credentialsProvider);

        // Set the region of your S3 bucket
        //s3.setRegion(Region.getRegion(Regions.US_EAST_1));


        File file = new File(context.getFilesDir(),"mydir");
        //if(!file.exists()){
        file.mkdir();
        //}

        try{
            File gpxfile = new File(file, object_key);
            FileWriter writer = new FileWriter(gpxfile);


            transferUtility = new TransferUtility(s3, c);


            TransferObserver transferObserver = transferUtility.download(
                    bucket_name,     /* The bucket to download from */
                    object_key,    /* The key for the object to download */
                    file        /* The file to download the object to */
            );
            transferObserverListener(transferObserver);
            readFromFile(c, file.getAbsolutePath());


        }catch (Exception e){
            Log.e("Exception", "File write failed: " + e.toString());
        }

        //Uploads our stored data to the S3 cloud. Need data parsing prior to this


    }

    public void credentialsProvider(Context context){

        // Initialize the Amazon Cognito credentials provider
        CognitoCachingCredentialsProvider credentialsProvider = new CognitoCachingCredentialsProvider(
                context,
                "us-east-1:f9392b49-3b25-4c2d-b277-3a66c68a287e", // Identity Pool ID unique to our AWS
                Regions.US_EAST_1 // Region
    );

        setAmazonS3Client(credentialsProvider);
    }


    /**
     *  Create a AmazonS3Client constructor and pass the credentialsProvider.
     * @param credentialsProvider
     */
    public void setAmazonS3Client(CognitoCachingCredentialsProvider credentialsProvider){

        // Create an S3 client
        s3 = new AmazonS3Client(credentialsProvider);

        // Set the region of your S3 bucket
        s3.setRegion(Region.getRegion(Regions.US_EAST_1));

    }

    /**
     * This is listener method of the TransferObserver
     * Within this listener method, we get status of uploading and downloading file,
     * to display percentage of the part of file to be uploaded or downloaded to S3
     * It displays an error, when there is a problem in  uploading or downloading file to or from S3.
     * @param transferObserver
     */
    public void transferObserverListener(TransferObserver transferObserver) {

        transferObserver.setTransferListener(new TransferListener() {

            @Override
            public void onStateChanged(int id, TransferState state) {
                //Log.e( & quot; statechange & quot;,state + & quot;&quot;);
            }

            @Override
            public void onProgressChanged(int id, long bytesCurrent, long bytesTotal) {
                int percentage = (int) (bytesCurrent / bytesTotal * 100);
                //Log.e( & quot; percentage & quot;,percentage + & quot;&quot;);
            }

            @Override
            public void onError(int id, Exception ex) {
               // Log.e( & quot; error & quot;,&quot; error & quot;);
            }

        });
    }
    private void writeToFile(String data,Context context, File  file_name) {
        try {
            FileOutputStream outputStream = new FileOutputStream(file_name);
            outputStream.write(data.getBytes());
            //TextView t = (TextView)findViewById(R.id.bottomMidText);
           // t.setText(file_name.getAbsolutePath().toString());
            outputStream.close();
        }
        catch (IOException e) {
            Log.e("Exception", "File write failed: " + e.toString());
        }
    }

    public void wrtieFileOnInternalStorage(Context mcoContext,String sFileName, String sBody){
        File file = new File(mcoContext.getFilesDir(),"mydir");
        if(!file.exists()){
            file.mkdir();
        }

        try{
            File gpxfile = new File(file, sFileName);
            FileWriter writer = new FileWriter(gpxfile);
            writer.append(sBody);
            writer.flush();
            writer.close();

        }catch (Exception e){
            Log.e("Exception", "File write failed: " + e.toString());
        }
    }


    private String readFromFile(Context context, String file_name) {

        String ret = "";

        try {
            InputStream inputStream = context.openFileInput(file_name);

            if ( inputStream != null ) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
                String receiveString = "";
                StringBuilder stringBuilder = new StringBuilder();

                while ( (receiveString = bufferedReader.readLine()) != null ) {
                    stringBuilder.append(receiveString);
                }

                inputStream.close();
                ret = stringBuilder.toString();
            }
        }
        catch (FileNotFoundException e) {
            Log.e("login activity", "File not found: " + e.toString());
        } catch (IOException e) {
            Log.e("login activity", "Can not read file: " + e.toString());
        }

        return ret;
    }
}
