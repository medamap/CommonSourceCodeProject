package jp.matrix.shikarunochi.emulator;

import android.app.PendingIntent;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiInputPort;
import android.media.midi.MidiManager;
import android.util.Log;

import java.util.HashMap;

public class MidiManagerActivity {
    private static final String ACTION_USB_PERMISSION = "jp.matrix.shikarunochi.emulator.USB_PERMISSION";
    private static MidiManagerActivity instance;
    private MidiManager midiManager;
    private UsbManager usbManager;
    private BluetoothAdapter bluetoothAdapter;
    private MidiDevice midiDevice;
    private MidiInputPort inputPort;
    private Context context;

    private MidiManagerActivity(Context context) {
        this.context = context;
        midiManager = (MidiManager) context.getSystemService(Context.MIDI_SERVICE);
        usbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        initializeMidiDevices();
    }

    private void initializeMidiDevices() {
        if (midiManager != null) {
            MidiDeviceInfo[] infos = midiManager.getDevices();
            if (infos.length > 0) {
                midiManager.openDevice(infos[0], new MidiManager.OnDeviceOpenedListener() {
                    @Override
                    public void onDeviceOpened(MidiDevice device) {
                        if (device == null) {
                            Log.e("MIDI", "Could not open device");
                        } else {
                            midiDevice = device;
                            inputPort = device.openInputPort(0);
                            if (inputPort == null) {
                                Log.e("MIDI", "Could not open input port");
                                return;
                            }
                        }
                    }
                }, null);
            }
        }
    }

    public static synchronized MidiManagerActivity getInstance(Context context) {
        if (instance == null) {
            instance = new MidiManagerActivity(context);
        }
        return instance;
    }

    public void checkForUsbDevices() {
        HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();
        for (UsbDevice device : deviceList.values()) {
            if (device.getDeviceClass() == UsbConstants.USB_CLASS_AUDIO) {
                if (!usbManager.hasPermission(device)) {
                    PendingIntent permissionIntent =
                            PendingIntent.getBroadcast(context, 0, new Intent(ACTION_USB_PERMISSION), 0);
                    usbManager.requestPermission(device, permissionIntent);
                } else {
                    openUsbDevice(device);
                }
            }
        }
    }

    private void openUsbDevice(UsbDevice device) {
        MidiDeviceInfo[] infos = midiManager.getDevices();
        for (MidiDeviceInfo info : infos) {
            if (info.getType() == MidiDeviceInfo.TYPE_USB && isMatchingDevice(info, device)) {
                midiManager.openDevice(info, new MidiManager.OnDeviceOpenedListener() {
                    @Override
                    public void onDeviceOpened(MidiDevice midiDevice) {
                        if (midiDevice == null) {
                            Log.e("MIDI", "Could not open USB device");
                        } else {
                            Log.i("MIDI", "USB device opened successfully");
                            inputPort = midiDevice.openInputPort(0);
                            if (inputPort == null) {
                                Log.e("MIDI", "Could not open input port");
                            }
                        }
                    }
                }, null);
                break;
            }
        }
    }

    private boolean isMatchingDevice(MidiDeviceInfo info, UsbDevice device) {
        // PROPERTY_VENDOR_IDとPROPERTY_PRODUCT_IDの代わりに、USBデバイスの情報を直接比較
        return info.getProperties().getInt("usb_device_id") == device.getDeviceId() &&
                info.getProperties().getInt("usb_vendor_id") == device.getVendorId();
    }

    public void startBleScan() {
        if (bluetoothAdapter == null) {
            bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        }

        if (bluetoothAdapter == null) {
            Log.e("MIDI", "BluetoothAdapter is not available");
            return;
        }

        if (!bluetoothAdapter.isEnabled()) {
            Log.e("MIDI", "Bluetooth is not enabled");
            return;
        }

        BluetoothLeScanner scanner = bluetoothAdapter.getBluetoothLeScanner();
        if (scanner != null) {
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
        } else {
            Log.e("MIDI", "BluetoothLeScanner is not available");
        }
    }


    private void openBleDevice(BluetoothDevice device) {
        midiManager.openBluetoothDevice(device, new MidiManager.OnDeviceOpenedListener() {
            @Override
            public void onDeviceOpened(MidiDevice midiDevice) {
                if (midiDevice == null) {
                    Log.e("MIDI", "Could not open BLE device");
                } else {
                    Log.i("MIDI", "BLE device opened successfully");
                    inputPort = midiDevice.openInputPort(0);
                    if (inputPort == null) {
                        Log.e("MIDI", "Could not open input port");
                    }
                }
            }
        }, null);
    }

    public void sendMidiMessage(byte[] message) {
        if (inputPort != null) {
            try {
                inputPort.send(message, 0, message.length);
            } catch (Exception e) {
                Log.e("MIDI", "Error sending MIDI message", e);
            }
        }
    }

    // JNI経由でC++側にデータを渡すメソッド
    public native void passMidiMessageToNative(byte[] buffer, int length);

    public String getMidiDeviceInfo(int index) {
        MidiDeviceInfo[] midiDevices = midiManager.getDevices();
        if (midiDevices != null && index >= 0 && index < midiDevices.length) {
            return midiDevices[index].toString();
        }
        return null;
    }
}
