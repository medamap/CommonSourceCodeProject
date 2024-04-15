package jp.matrix.shikarunochi.emulator;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class BluetoothBroadcastReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        // ここにインテントを受け取ったときの処理を書く
        Log.i("CommonProject", "Intent received: " + intent.getAction());
    }
}
