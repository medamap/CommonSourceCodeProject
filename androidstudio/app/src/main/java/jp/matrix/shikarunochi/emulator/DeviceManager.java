package jp.matrix.shikarunochi.emulator;

import android.content.Context;
import android.hardware.input.InputManager;
import android.view.InputDevice;

public class DeviceManager implements InputManager.InputDeviceListener {
    private InputManager inputManager;

    public DeviceManager(Context context) {
        inputManager = (InputManager) context.getSystemService(Context.INPUT_SERVICE);
    }

    public void registerInputDeviceListener() {
        inputManager.registerInputDeviceListener(this, null);
    }

    public void unregisterInputDeviceListener() {
        inputManager.unregisterInputDeviceListener(this);
    }

    @Override
    public void onInputDeviceAdded(int deviceId) {
        InputDevice device = InputDevice.getDevice(deviceId);
        if (isGameController(device)) {
            // ジョイスティックが接続された場合の処理
        }
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) {
        // ジョイスティックが切断された場合の処理
    }

    @Override
    public void onInputDeviceChanged(int deviceId) {
        // ジョイスティックの設定が変更された場合の処理
    }

    private boolean isGameController(InputDevice device) {
        int sources = device.getSources();
        return ((sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK) ||
                ((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD);
    }
}
