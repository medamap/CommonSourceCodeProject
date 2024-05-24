package jp.matrix.shikarunochi.emulator;

import static android.hardware.usb.UsbConstants.USB_CLASS_VENDOR_SPEC;

import android.Manifest;
import android.app.PendingIntent;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiInputPort;
import android.media.midi.MidiManager;
import android.media.midi.MidiReceiver;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import java.io.IOException;
import java.util.HashMap;

public class MidiManagerActivity {
    private static final String ACTION_USB_PERMISSION = "jp.matrix.shikarunochi.emulator.USB_PERMISSION";
    private static MidiManagerActivity instance;
    private MidiManager midiManager;
    private UsbManager usbManager;
    private BluetoothAdapter bluetoothAdapter;
    private MidiDevice midiDevice;
    private MidiReceiver midiReceiver;
    private Context context;
    private Handler uiHandler;


    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i("MIDI", "Received USB broadcast: " + action);
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if (device != null) {
                            Log.i("MIDI", "Permission granted for device " + device.getProductName());
                            openUsbDevice(device);
                        }
                    } else {
                        Log.d("MIDI", "Permission denied for device " + device);
                    }
                }
            } else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action) ||
                    UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                checkForUsbDevices();
            }
        }
    };

    private final BroadcastReceiver bluetoothReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i("MIDI", "Received Bluetooth broadcast: " + action);
            if (BluetoothDevice.ACTION_ACL_CONNECTED.equals(action) ||
                    BluetoothDevice.ACTION_ACL_DISCONNECTED.equals(action)) {
                startBleScan();
            }
        }
    };

    // アクティビティの終了時に呼び出されるクリーンアップメソッド
    public void close() {
        Log.i("MIDI", "Closing MidiManagerActivity");

        // ブロードキャストレシーバーの登録解除
        context.unregisterReceiver(usbReceiver);
        context.unregisterReceiver(bluetoothReceiver);

        // MIDIデバイスのクローズ
        if (midiDevice != null) {
            try {
                midiDevice.close();
                Log.i("MIDI", "Closed MIDI device successfully");
            } catch (IOException e) {
                Log.e("MIDI", "Error closing MIDI device", e);
            }
            midiDevice = null;
        }

        // その他のリソース解放が必要な場合はここに追加
    }

    private MidiManagerActivity(Context context) {
        this.context = context.getApplicationContext(); // Use application context to avoid leaks
        uiHandler = new Handler(Looper.getMainLooper());

        // Log the context class to ensure it's the correct type
        Log.i("MIDI", "Context class: " + this.context.getClass().getName());

        // Attempt to retrieve the MidiManager
        Object service = this.context.getSystemService(Context.MIDI_SERVICE);
        if (service == null) {
            Log.e("MIDI", "MIDI_SERVICE is not available");
        } else if (!(service instanceof MidiManager)) {
            Log.e("MIDI", "Service is not an instance of MidiManager: " + service.getClass().getName());
        } else {
            midiManager = (MidiManager) service;
            Log.i("MIDI", "MidiManager initialized successfully");
        }

        usbManager = (UsbManager) this.context.getSystemService(Context.USB_SERVICE);
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        initializeMidiDevices();

        // Register the BroadcastReceiver for USB permission
        IntentFilter usbFilter = new IntentFilter();
        usbFilter.addAction(ACTION_USB_PERMISSION);
        usbFilter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        usbFilter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        context.registerReceiver(usbReceiver, usbFilter);

        // Register the BroadcastReceiver for Bluetooth events
        IntentFilter bluetoothFilter = new IntentFilter();
        bluetoothFilter.addAction(BluetoothDevice.ACTION_ACL_CONNECTED);
        bluetoothFilter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED);
        context.registerReceiver(bluetoothReceiver, bluetoothFilter);
    }

    private void initializeMidiDevices() {
        if (midiManager != null) {
            MidiDeviceInfo[] infos = midiManager.getDevices();
            if (infos.length > 0) {
                openMidiDevice(infos[0]);
            }
        } else {
            Log.e("MIDI", "MidiManager is null");
        }
    }

    private void openMidiDevice(final MidiDeviceInfo info) {
        if (midiManager == null) {
            Log.e("MIDI", "MidiManager is null, cannot open MIDI device");
            return;
        }
        Log.i("MIDI", "Opening MIDI device " + info.getProperties().toString());
        midiManager.openDevice(info, new MidiManager.OnDeviceOpenedListener() {
            @Override
            public void onDeviceOpened(MidiDevice device) {
                if (device == null) {
                    Log.e("MIDI", "Could not open device");
                } else {
                    Log.i("MIDI", "onDeviceOpened " + device.getInfo().getProperties().toString());
                    if (midiDevice != null) {
                        try {
                            Log.i("MIDI", "Closed existing midiDevice " + midiDevice.getInfo().getProperties().toString());
                            midiDevice.close();
                        } catch (IOException e) {
                            Log.e("MIDI", "Error closing existing midiDevice", e);
                        }
                    }
                    midiDevice = device;
                    MidiInputPort inputPort = device.openInputPort(0); // inputPort を開く
                    if (inputPort == null) {
                        Log.e("MIDI", "Could not open input port " + device.getInfo().getProperties().toString());
                        // ポートがオープンできなければデバイスをクローズして null にする
                        try {
                            device.close();
                            midiDevice = null;
                        } catch (IOException e) {
                            Log.e("MIDI", "Error closing device", e);
                        }
                    } else {
                        midiReceiver = inputPort;
                        Log.i("MIDI", "Opened MIDI device successfully " + device.getInfo().getProperties().toString());
                        // UIスレッドでToastを表示
                        uiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                final Toast toast = Toast.makeText(context, "Open " + info.getProperties().getString(MidiDeviceInfo.PROPERTY_PRODUCT), Toast.LENGTH_LONG);
                                toast.show();
                                // 5秒後にトーストをキャンセルするためのハンドラーを設定
                                uiHandler.postDelayed(new Runnable() {
                                    @Override
                                    public void run() {
                                        toast.cancel();
                                    }
                                }, 5000); // 5秒後にトーストをキャンセル
                            }
                        });
                    }
                }
            }
        }, null);
    }

    public static synchronized MidiManagerActivity getInstance(Context context) {
        if (instance == null) {
            instance = new MidiManagerActivity(context);
        }
        return instance;
    }

    public void checkForUsbDevices() {
        HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();
        Log.i("MIDI", "Found " + deviceList.size() + " USB devices" + deviceList.toString());
        // もしデバイス数が0 なら
        if (deviceList.isEmpty()) {
            Log.i("MIDI", "No USB devices found");
            closeMidiResources();
            return;
        }
        for (UsbDevice device : deviceList.values()) {
            Log.i("MIDI", "Device: " + device.getDeviceName() + " - " + device.getDeviceId() + " - " + device.getVendorId() + " - " + device.getProductId() + " - " + device.getDeviceClass() + " - " + device.getDeviceSubclass() + " - " + device.getDeviceProtocol() + " - " + device.getInterfaceCount() + " - " + device.getConfigurationCount() + " - " + device.getSerialNumber() + " - " + device.getManufacturerName() + " - " + device.getProductName() + " - " + device.getVersion());

            boolean isMidiDevice = false;

            for (int i = 0; i < device.getInterfaceCount(); i++) {
                UsbInterface usbInterface = device.getInterface(i);
                Log.i("MIDI", "Interface: " + usbInterface.getId() + " - " + usbInterface.getInterfaceClass() + " - " + usbInterface.getInterfaceSubclass());
                if (usbInterface.getInterfaceClass() == UsbConstants.USB_CLASS_AUDIO &&
                        usbInterface.getInterfaceSubclass() == 3) {
                    Log.i("MIDI", "Found USB MIDI interface " + device.getProductName());
                    isMidiDevice = true;
                    break;
                }
            }

            if (isMidiDevice) {
                Log.i("MIDI", "Found USB MIDI device " + device.getProductName());
                if (!usbManager.hasPermission(device)) {
                    Log.i("MIDI", "Requesting permission for USB device " + device.getProductName());
                    PendingIntent permissionIntent = PendingIntent.getBroadcast(context, 0, new Intent(ACTION_USB_PERMISSION), PendingIntent.FLAG_UPDATE_CURRENT);
                    usbManager.requestPermission(device, permissionIntent);
                } else {
                    Log.i("MIDI", "USB device already has permission " + device.getProductName());
                    openUsbDevice(device);
                }
            } else {
                Log.i("MIDI", "USB device is not a MIDI device " + device.getProductName());
            }
        }
    }

    private void openUsbDevice(UsbDevice device) {
        Log.i("MIDI", "Opening USB device " + device.getProductName());
        if (midiManager == null) {
            Log.e("MIDI", "MidiManager is null");
            return;
        }

        MidiDeviceInfo[] infos = midiManager.getDevices();
        Log.i("MIDI", "Found " + infos.length + " MIDI devices");
        for (MidiDeviceInfo info : infos) {
            Log.i("MIDI", "MIDI device: " + info.getProperties().toString());
            Log.i("MIDI", "MIDI device type: " + info.getType() + " - " + MidiDeviceInfo.TYPE_USB + " - " + MidiDeviceInfo.TYPE_BLUETOOTH);
            if (info.getType() == MidiDeviceInfo.TYPE_USB) {
                // USBデバイスの一致を確認する
                String usbManufacturer = info.getProperties().getString(MidiDeviceInfo.PROPERTY_MANUFACTURER);
                final String usbProduct = info.getProperties().getString(MidiDeviceInfo.PROPERTY_PRODUCT);
                if (usbManufacturer != null && usbProduct != null) {
                    if (usbManufacturer.equals(device.getManufacturerName()) && usbProduct.equals(device.getProductName())) {
                        Log.i("MIDI", "Found matching MIDI device " + info.getProperties().toString());
                        openMidiDevice(info);
                        // デバイスがオープンできたかチェック
                        if (midiDevice == null) {
                            Log.e("MIDI", "Could not open USB device");
                        } else {
                            Log.i("MIDI", "Opened USB device successfully " + midiDevice.getInfo().getProperties().toString());
                        }
                    }
                }
            }
        }
    }

    private boolean isMatchingDevice(MidiDeviceInfo info, UsbDevice device) {
        String usbManufacturer = info.getProperties().getString(MidiDeviceInfo.PROPERTY_MANUFACTURER);
        String usbProduct = info.getProperties().getString(MidiDeviceInfo.PROPERTY_PRODUCT);
        Log.i("MIDI", "Checking if device matches: info_manufacturer = " + usbManufacturer + " and info_product = " + usbProduct);
        Log.i("MIDI", "Checking if device matches: device_manufacturer = " + device.getManufacturerName() + " and device_product = " + device.getProductName());
        return usbManufacturer != null && usbProduct != null &&
                usbManufacturer.equals(device.getManufacturerName()) &&
                usbProduct.equals(device.getProductName());
    }

    public void startBleScan() {
        if (midiManager == null) {
            Log.e("MIDI", "MidiManager is null, cannot start BLE scan");
            return;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M &&
                context.checkSelfPermission("android.permission.BLUETOOTH_SCAN") != PackageManager.PERMISSION_GRANTED) {
            Log.e("MIDI", "Bluetooth scan permission not granted");
            return;
        }

        BluetoothLeScanner scanner = bluetoothAdapter.getBluetoothLeScanner();
        scanner.startScan(new ScanCallback() {
            @Override
            public void onScanResult(int callbackType, ScanResult result) {
                BluetoothDevice device = result.getDevice();
                openBleDevice(device);
            }

            @Override
            public void onScanFailed(int errorCode) {
                Log.e("MIDI", "BLE scan failed with error code: " + errorCode);
            }
        });
    }

    private void openBleDevice(BluetoothDevice device) {
        if (midiManager == null) {
            Log.e("MIDI", "MidiManager is null, cannot open BLE device");
            return;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M &&
                context.checkSelfPermission("android.permission.BLUETOOTH_CONNECT") != PackageManager.PERMISSION_GRANTED) {
            Log.e("MIDI", "Bluetooth connect permission not granted");
            return;
        }

        midiManager.openBluetoothDevice(device, new MidiManager.OnDeviceOpenedListener() {
            @Override
            public void onDeviceOpened(MidiDevice midiDevice) {
                if (midiDevice == null) {
                    Log.e("MIDI", "Could not open BLE device");
                } else {
                    if (MidiManagerActivity.this.midiDevice != null) {
                        try {
                            MidiManagerActivity.this.midiDevice.close();
                        } catch (IOException e) {
                            Log.e("MIDI", "Error closing existing midiDevice", e);
                        }
                    }
                    MidiManagerActivity.this.midiDevice = midiDevice;
                    MidiInputPort inputPort = midiDevice.openInputPort(0);
                    if (inputPort == null) {
                        Log.e("MIDI", "Could not open input port");
                    } else {
                        midiReceiver = inputPort;
                        Log.i("MIDI", "Opened BLE device successfully " + midiDevice.getInfo().getProperties().toString());
                    }
                }
            }
        }, null);
    }

    MidiReceiver oldMidiReceiver;
    int counter = 0;

    public void sendMidiMessage(byte[] message) {
        if (midiReceiver != null) {
            if (++counter % 65536 == 0) {
                counter = 0;
                Log.i("MIDI", "SendMidiMessage called");
            }
            if (oldMidiReceiver != midiReceiver) {
                oldMidiReceiver = midiReceiver;
                Log.i("MIDI", "MidiReceiver changed and SendMidiMessage called");
            }
            try {
                long now = System.nanoTime();
                midiReceiver.send(message, 0, message.length, now);
            } catch (IOException e) {
                Log.e("MIDI", "Error sending MIDI message", e);
                // 例外が発生した場合のリソースクリーンアップ
                closeMidiResources();
            }
        }
    }

    // 例外発生時にリソースをクリーンアップするメソッド
    private void closeMidiResources() {
        Log.i("MIDI", "Closing MIDI resources due to exception");
        if (midiDevice != null) {
            try {
                midiDevice.close();
                Log.i("MIDI", "Closed MIDI device successfully");
            } catch (IOException e) {
                Log.e("MIDI", "Error closing MIDI device", e);
            }
        }
        midiDevice = null;
        midiReceiver = null;
    }

    // JNI経由でC++側にデータを渡すメソッド
    public native void passMidiMessageToNative(byte[] buffer, int length);

    public String getMidiDeviceInfo(int index) {
        if (midiManager == null) {
            Log.e("MIDI", "MidiManager is null, cannot get MIDI device info");
            return null;
        }

        MidiDeviceInfo[] midiDevices = midiManager.getDevices();
        if (midiDevices != null && index >= 0 && index < midiDevices.length) {
            return midiDevices[index].toString();
        }
        return null;
    }
}
