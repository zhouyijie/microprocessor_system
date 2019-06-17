package com.example.android.bluetoothlegatt;

import java.util.HashMap;

/**
 * Created by alexrichardson on 3/26/17.
 */

public class ProjectGattAttributes {
    private static HashMap<String, String> attributes = new HashMap();
    //TODO: Jaeho, put the characteristics and services here
    public static String INFORMATION = "340a1b80-cf4b-11e1-ac36-0002a5d5c51b";
    public static String Handshake = "331a2b90-d05b-2101-bc46-1012b505d52b";
    public static String SERVICE = "02366e80-cf3a-11e1-9ab4-0002a5d5c51b";
    public static String WRITE = "432a3ba0-e06b-3111-cc56-2022c515e53b";
    public static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";

    static {
        // Sample Services.
        attributes.put("0000180a-0000-1000-8000-00805f9b34fb", "Device Information Service");
        // Sample Characteristics.
        attributes.put(INFORMATION, "Data");
        attributes.put(Handshake, "Handshake");
        attributes.put(SERVICE, "Service");
        attributes.put(WRITE, "Write");

        attributes.put("00002a29-0000-1000-8000-00805f9b34fb", "Manufacturer Name String");
    }

    public static String lookup(String uuid, String defaultName) {
        String name = attributes.get(uuid);
        return name == null ? defaultName : name;
    }
}
