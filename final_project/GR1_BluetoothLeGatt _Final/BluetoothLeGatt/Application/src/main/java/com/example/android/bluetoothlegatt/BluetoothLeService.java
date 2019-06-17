/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.android.bluetoothlegatt;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;


import java.nio.ByteBuffer;
import java.util.List;
import java.util.UUID;

/**
 * Service for managing connection and data communication with a GATT server hosted on a
 * given Bluetooth LE device.
 */
public class BluetoothLeService extends Service {
    private final static String TAG = BluetoothLeService.class.getSimpleName();

    private BluetoothManager mBluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;
    private String mBluetoothDeviceAddress;
    public BluetoothGatt mBluetoothGatt;
    private int mConnectionState = STATE_DISCONNECTED;

    private static final int STATE_DISCONNECTED = 0;
    private static final int STATE_CONNECTING = 1;
    private static final int STATE_CONNECTED = 2;

    public final static String ACTION_GATT_CONNECTED =
            "com.example.bluetooth.le.ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED =
            "com.example.bluetooth.le.ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED =
            "com.example.bluetooth.le.ACTION_GATT_SERVICES_DISCOVERED";
    public final static String ACTION_DATA_AVAILABLE =
            "com.example.bluetooth.le.ACTION_DATA_AVAILABLE";
    public final static String EXTRA_DATA =
            "com.example.bluetooth.le.EXTRA_DATA";

    public final static UUID UUID_HEART_RATE_MEASUREMENT =
            UUID.fromString(SampleGattAttributes.HEART_RATE_MEASUREMENT);

    public final static UUID UUID_DATA_MEASUREMENT =
            UUID.fromString(ProjectGattAttributes.INFORMATION);
    public final static UUID UUID_Service =
            UUID.fromString(ProjectGattAttributes.SERVICE);
    public final static UUID Handshake =
            UUID.fromString(ProjectGattAttributes.Handshake);
    public final static UUID UUID_WRITE =
            UUID.fromString(ProjectGattAttributes.WRITE);

    public static String output_data;

    public static String[] input_data;

    public static int data_iterator;


    //public static String x_value;

    //public static String y_value;

    //public static String z_value;

    private static int board_iterator;

    // Implements callback methods for GATT events that the app cares about.  For example,
    // connection change and services discovered.
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            String intentAction;
            output_data = "";
            data_iterator = 0;
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                intentAction = ACTION_GATT_CONNECTED;
                mConnectionState = STATE_CONNECTED;
                broadcastUpdate(intentAction);


                Log.i(TAG, "Connected to GATT server.");
                // Attempts to discover services after successful connection.
                Log.i(TAG, "Attempting to start service discovery:" +
                        mBluetoothGatt.discoverServices());

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                intentAction = ACTION_GATT_DISCONNECTED;
                mConnectionState = STATE_DISCONNECTED;
                Log.i(TAG, "Disconnected from GATT server.");
                broadcastUpdate(intentAction);
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                broadcastUpdate(ACTION_GATT_SERVICES_DISCOVERED);
            } else {
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
            }
         /*   else if(UUID_DATA_MEASUREMENT.equals(characteristic.getUuid())) {
                Toast.makeText(getApplicationContext(), "UUID found to be Data", Toast.LENGTH_SHORT).show();
                readCustomCharacteristic();

            }
            else if(Handshake.equals(characteristic.getUuid())) {
                Toast.makeText(getApplicationContext(), "UUID found to be handshake", Toast.LENGTH_SHORT).show();
                handshake(board_iterator);
            }*/
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            Log.d(TAG, "Bluetooth characteristic hass been updated");
            broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
            if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                Log.w(TAG, "BluetoothAdapter not initialized");
                return;
            }
            //Previous implementation. Has been moved to broadcastUpdate insteads
            /*else if(UUID_DATA_MEASUREMENT.equals(characteristic.getUuid())) {
                Toast.makeText(getApplicationContext(), "UUID found to be Data", Toast.LENGTH_SHORT).show();
                readCustomCharacteristic();

            }
            else if(Handshake.equals(characteristic.getUuid())) {
                Toast.makeText(getApplicationContext(), "UUID found to be handshake", Toast.LENGTH_SHORT).show();
                handshake(board_iterator);
            }*/

        }
    };

    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
        sendBroadcast(intent);
    }

    private void broadcastUpdate(final String action,
                                 final BluetoothGattCharacteristic characteristic) {
        final Intent intent = new Intent(action);

        // This is special handling for the Heart Rate Measurement profile.  Data parsing is
        // carried out as per profile specifications:
        // http://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.heart_rate_measurement.xml
        if (UUID_HEART_RATE_MEASUREMENT.equals(characteristic.getUuid())) {
            int flag = characteristic.getProperties();
            int format = -1;
            if ((flag & 0x01) != 0) {
                format = BluetoothGattCharacteristic.FORMAT_UINT16;
                Log.d(TAG, "Heart rate format UINT16.");
            } else {
                format = BluetoothGattCharacteristic.FORMAT_UINT8;
                Log.d(TAG, "Heart rate format UINT8.");
            }
            final int heartRate = characteristic.getIntValue(format, 1);
            Log.d(TAG, String.format("Received heart rate: %d", heartRate));
            intent.putExtra(EXTRA_DATA, String.valueOf(heartRate));
        }
        else if(UUID_DATA_MEASUREMENT.equals(characteristic.getUuid())) {
            Log.d(TAG, "data.");
            //readCustomCharacteristic();
            int flag = characteristic.getProperties();
            int format = -1;
            if ((flag & 0x01) != 0) {
                format = BluetoothGattCharacteristic.FORMAT_UINT16;
                Log.d(TAG, "Data format UINT16.");
            } else {
                format = BluetoothGattCharacteristic.FORMAT_UINT8;
                Log.d(TAG, "Data rate format UINT8.");
            }
            //loads data received by board into output data. It sends one data sample containing x,y and z values
            final byte[] data = characteristic.getValue();
            if (data != null && data.length > 0) {
                final StringBuilder stringBuilder = new StringBuilder(data.length);
                int i = 0;
                String x_value = "";
                String y_value = "";
                String z_value = "";
                int asInt = (data[0] & 0xFF)
                        | ((data[1] & 0xFF) << 8)
                        | ((data[2] & 0xFF) << 16)
                        | ((data[3] & 0xFF) << 24)
                        | ((data[4] & 0xFF) << 32)
                        | ((data[5] & 0xFF) << 40);

                /*for (byte byteChar : data) {
                    if (i < data.length / 3) {
                        x_value += String.format("%02X ", byteChar);
                    } else if (i < 2 * data.length / 3) {
                        y_value += String.format("%02X ", byteChar);
                    } else {
                        z_value += String.format("%02X ", byteChar);

                    }
                }*/
                //output_data += x_value + "," + y_value + "," + z_value + "\n";

                //Add return value at the end of each x,y,z transaction
                if(data_iterator % 3 == 2)
                    output_data += Float.intBitsToFloat(asInt) + "\n";

                else
                    output_data += Float.intBitsToFloat(asInt) + ",";
                data_iterator ++;
                Log.d(TAG, "output data is " + output_data + ", length of array is " + data.length);
            }
        }
        else if(Handshake.equals(characteristic.getUuid())) {
            Log.d(TAG, "Handshake.");
            int datapoint = 0;
            if(input_data[board_iterator] != null)
                datapoint = 100*Integer.parseInt(input_data[board_iterator]);

            BluetoothGattService mCustomService = mBluetoothGatt.getService(UUID_Service);

            BluetoothGattCharacteristic mWriteCharacteristic = mCustomService.getCharacteristic(UUID_WRITE);
            mWriteCharacteristic.setValue(datapoint,android.bluetooth.BluetoothGattCharacteristic.FORMAT_UINT8,0);
            if(mBluetoothGatt.writeCharacteristic(mWriteCharacteristic) == false){
                Log.w(TAG, "Failed to write characteristic");
            }
            else
                board_iterator = (board_iterator + 1) % (input_data.length);

        }


        sendBroadcast(intent);
    }

    public class LocalBinder extends Binder {
        BluetoothLeService getService() {
            return BluetoothLeService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        // After using a given device, you should make sure that BluetoothGatt.close() is called
        // such that resources are cleaned up properly.  In this particular example, close() is
        // invoked when the UI is disconnected from the Service.
        close();
        return super.onUnbind(intent);
    }

    private final IBinder mBinder = new LocalBinder();

    /**
     * Initializes a reference to the local Bluetooth adapter.
     *
     * @return Return true if the initialization is successful.
     */
    public boolean initialize() {
        // For API level 18 and above, get a reference to BluetoothAdapter through
        // BluetoothManager.
        if (mBluetoothManager == null) {
            mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            if (mBluetoothManager == null) {
                Log.e(TAG, "Unable to initialize BluetoothManager.");
                return false;
            }
        }

        mBluetoothAdapter = mBluetoothManager.getAdapter();
        if (mBluetoothAdapter == null) {
            Log.e(TAG, "Unable to obtain a BluetoothAdapter.");
            return false;
        }

        return true;
    }

    /**
     * Connects to the GATT server hosted on the Bluetooth LE device.
     *
     * @param address The device address of the destination device.
     *
     * @return Return true if the connection is initiated successfully. The connection result
     *         is reported asynchronously through the
     *         {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     *         callback.
     */
    public boolean connect(final String address) {
        if (mBluetoothAdapter == null || address == null) {
            Log.w(TAG, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }

        // Previously connected device.  Try to reconnect.
        if (mBluetoothDeviceAddress != null && address.equals(mBluetoothDeviceAddress)
                && mBluetoothGatt != null) {
            Log.d(TAG, "Trying to use an existing mBluetoothGatt for connection.");
            if (mBluetoothGatt.connect()) {
                mConnectionState = STATE_CONNECTING;
                return true;
            } else {
                return false;
            }
        }

        final BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
        if (device == null) {
            Log.w(TAG, "Device not found.  Unable to connect.");
            return false;
        }
        // We want to directly connect to the device, so we are setting the autoConnect
        // parameter to false.
        mBluetoothGatt = device.connectGatt(this, false, mGattCallback);
        Log.d(TAG, "Trying to create a new connection.");
        mBluetoothDeviceAddress = address;
        mConnectionState = STATE_CONNECTING;
        return true;
    }

    /**
     * Disconnects an existing connection or cancel a pending connection. The disconnection result
     * is reported asynchronously through the
     * {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     * callback.
     */
    public void disconnect() {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.disconnect();
    }

    /**
     * After using a given BLE device, the app must call this method to ensure resources are
     * released properly.
     */
    public void close() {
        if (mBluetoothGatt == null) {
            return;
        }
        mBluetoothGatt.close();
        mBluetoothGatt = null;
    }

    /**
     * Request a read on a given {@code BluetoothGattCharacteristic}. The read result is reported
     * asynchronously through the {@code BluetoothGattCallback#onCharacteristicRead(android.bluetooth.BluetoothGatt, android.bluetooth.BluetoothGattCharacteristic, int)}
     * callback.
     *
     * @param characteristic The characteristic to read from.
     */
    public void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.readCharacteristic(characteristic);
    }

    /**
     * Enables or disables notification on a give characteristic.
     *
     * @param characteristic Characteristic to act on.
     * @param enabled If true, enable notification.  False otherwise.
     */
    public void setCharacteristicNotification(BluetoothGattCharacteristic characteristic,
                                              boolean enabled) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, enabled);

    }

    //This method is used to confirm that the next piece of data should be sent
    public void handshake(int iteration) {

        /*check if the service is available on the device*/
        BluetoothGattService mHandshake = mBluetoothGatt.getService(UUID_Service);
        if (mHandshake == null) {
            Log.w(TAG, "Handshake BLE Service not found");
            return;
        }
        //Find the Characteristic ID. If it exist, try to send data to the board

        BluetoothGattCharacteristic mReadCharacteristic = mHandshake.getCharacteristic(Handshake);
        if (mBluetoothGatt.readCharacteristic(mReadCharacteristic) == false) {
            Log.w(TAG, "Failed to read characteristic");
        } else {
            int datapoint = 100*Integer.parseInt(input_data[board_iterator]);
            writeCustomCharacteristic(datapoint);
            board_iterator = (board_iterator + 1) % (input_data.length);
        }
    }

    public void readCustomCharacteristic() {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        /*check if the service is available on the device*/
        BluetoothGattService mCustomService = mBluetoothGatt.getService(UUID_Service);
        if(mCustomService == null){
            Log.w(TAG, "Data BLE Service not found");
            return;
        }

        /*get the read characteristic from the service*/
        BluetoothGattCharacteristic mReadCharacteristic = mCustomService.getCharacteristic(Handshake);
        if (mBluetoothGatt.readCharacteristic(mReadCharacteristic) == false) {
            Log.w(TAG, "Failed to read characteristic");
        } else {
            int flag = mReadCharacteristic.getProperties();
            int format = -1;
            if ((flag & 0x01) != 0) {
                format = BluetoothGattCharacteristic.FORMAT_UINT16;
                Log.d(TAG, "Data format UINT16.");
            } else {
                format = BluetoothGattCharacteristic.FORMAT_UINT8;
                Log.d(TAG, "Data rate format UINT8.");
            }
            //loads data received by board into output data. It sends one data sample containing x,y and z values
            final byte[] data = mReadCharacteristic.getValue();
            if (data != null && data.length > 0) {
                final StringBuilder stringBuilder = new StringBuilder(data.length);
                int i = 0;
                String x_value = "";
                String y_value = "";
                String z_value = "";
                for (byte byteChar : data) {
                    if (i < data.length / 3) {
                        x_value += String.format("%02X ", byteChar);
                    } else if (i < 2 * data.length / 3) {
                        y_value += String.format("%02X ", byteChar);
                    } else {
                        z_value += String.format("%02X ", byteChar);

                    }
                }
                output_data += x_value + "," + y_value + "," + z_value + "\n";
            }

        }
    }

    public void writeCustomCharacteristic(int value) {

        /*check if the service is available on the device*/
        BluetoothGattService mCustomService = mBluetoothGatt.getService(UUID_Service);
        if(mCustomService == null){
            Log.w(TAG, "Custom BLE Service not found");
            return;
        }
        /*get the read characteristic from the service*/
        BluetoothGattCharacteristic mWriteCharacteristic = mCustomService.getCharacteristic(UUID_WRITE);
        mWriteCharacteristic.setValue(value,android.bluetooth.BluetoothGattCharacteristic.FORMAT_UINT8,0);
        if(mBluetoothGatt.writeCharacteristic(mWriteCharacteristic) == false){
            Log.w(TAG, "Failed to write characteristic");
        }
    }

    /**
     * Retrieves a list of supported GATT services on the connected device. This should be
     * invoked only after {@code BluetoothGatt#discoverServices()} completes successfully.
     *
     * @return A {@code List} of supported services.
     */
    public List<BluetoothGattService> getSupportedGattServices() {
        if (mBluetoothGatt == null) return null;

        return mBluetoothGatt.getServices();
    }
}