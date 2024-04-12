package jp.matrix.shikarunochi.emulator;

import static android.content.ContentValues.TAG;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.NativeActivity;
import android.content.BroadcastReceiver;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Resources;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

public class EmulatorActivity extends NativeActivity {

    private static final int PERMISSIONS_REQUEST_CODE = 1;
    private volatile boolean permissionsGranted = false;
    private static final String[] PERMISSIONS_REQUIRED = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.BLUETOOTH,
            Manifest.permission.BLUETOOTH_ADMIN,
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_COARSE_LOCATION,
            // APIレベル 31 以降で必要な Bluetooth 接続権限
            "android.permission.BLUETOOTH_CONNECT"
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 画面を縦方向に固定
        //setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!hasPermissions()) {
                requestPermissions(PERMISSIONS_REQUIRED, PERMISSIONS_REQUEST_CODE);
            } else {
                initializeApp();
            }
        } else {
            initializeApp();
        }
        System.loadLibrary("native-activity");

        // 他のデバッグ時のみの初期化コード
        if (BuildConfig.DEBUG) {
            //System.loadLibrary("asan");
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        Log.i(TAG, "onRequestPermissionsResult called");
        if (requestCode == PERMISSIONS_REQUEST_CODE) {
            if (hasPermissions()) {
                Log.i(TAG, "All permissions granted");
                onPermissionsGranted();
            } else {
                Log.i(TAG, "Some permissions denied");
                onPermissionsDenied();
            }
        }
    }

    // ネイティブメソッドの宣言
    private native void nativeOnPermissionsGranted();
    private native void nativeOnPermissionsDenied();

    private boolean hasPermissions() {
        Log.i(TAG, "Checking permissions");
        // 基本的なファイルアクセス権限をチェック
        boolean hasFilePermissions = checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
                && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;

        // APIレベル 31 以降では、BLUETOOTH_CONNECT 権限もチェックする
        if (Build.VERSION.SDK_INT >= 31) {
            boolean bluetoothConnectGranted = checkSelfPermission("android.permission.BLUETOOTH_CONNECT") == PackageManager.PERMISSION_GRANTED;
            Log.i(TAG, "Checking BLUETOOTH_CONNECT permission " + (bluetoothConnectGranted ? "granted" : "denied"));

            return hasFilePermissions
                    && bluetoothConnectGranted
                    && checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED
                    && checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) == PackageManager.PERMISSION_GRANTED;
        } else {
            // Android 12未満では、BLUETOOTH_CONNECT権限は不要であり、位置情報権限も必要に応じてチェック
            return hasFilePermissions
                    && checkSelfPermission(Manifest.permission.BLUETOOTH) == PackageManager.PERMISSION_GRANTED
                    && checkSelfPermission(Manifest.permission.BLUETOOTH_ADMIN) == PackageManager.PERMISSION_GRANTED
                    && checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED
                    && checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) == PackageManager.PERMISSION_GRANTED;
        }
    }


    private void onPermissionsGranted() {
        Log.i(TAG, "Permissions granted");
        // JNIに権限付与成功を通知
        nativeOnPermissionsGranted();
    }

    private void onPermissionsDenied() {
        // JNIに権限拒否を通知
        nativeOnPermissionsDenied();
    }

    // JNIから呼び出す非同期メソッド
    public void checkPermissionsAsync() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "checkPermissionsAsync called");
                if (EmulatorActivity.this.hasPermissions()) {
                    Log.i(TAG, "Permissions already granted");
                    EmulatorActivity.this.onPermissionsGranted();
                } else {
                    EmulatorActivity.this.requestPermissions(PERMISSIONS_REQUIRED, PERMISSIONS_REQUEST_CODE);
                }
            }
        });
    }

    // C++ 側からアクセス可能なメソッド
    public boolean arePermissionsGranted() {
        return permissionsGranted;
    }

    private void initializeApp() {
        // 権限がある場合のアプリの初期化処理
        System.loadLibrary("native-activity");
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
    }

    // ネイティブメソッドの宣言
    public native void newFileCallback(String filename, String mediaInfo, String addPath);

    // コールバック用のインターフェースを定義
    interface NewFileCallback {
        void onNewFileSelected(String filename, String mediaInfo, String addPath);
    }

    // NDK から呼び出せるクリップボード取得メソッド
    public String getClipboardText() {
        final String[] result = new String[1];

        // UI スレッドで実行するための Handler
        Handler handler = new Handler(Looper.getMainLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                ClipboardManager clipboardManager = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
                if (clipboardManager.hasPrimaryClip() && clipboardManager.getPrimaryClip().getItemCount() > 0) {
                    CharSequence charSequence = clipboardManager.getPrimaryClip().getItemAt(0).getText();
                    result[0] = charSequence == null ? "" : charSequence.toString();
                } else {
                    result[0] = "";
                }
            }
        });

        // 非同期処理完了を待機
        try {
            Thread.sleep(100);  // 実際のアプリケーションでは、適切な同期処理を実装する必要がある
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        return result[0];
    }

    public byte[] getClipboardTextEncoded() {
        final byte[][] result = new byte[1][];

        Handler handler = new Handler(Looper.getMainLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                ClipboardManager clipboardManager = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
                if (clipboardManager.hasPrimaryClip() && clipboardManager.getPrimaryClip().getItemCount() > 0) {
                    CharSequence charSequence = clipboardManager.getPrimaryClip().getItemAt(0).getText();
                    if (charSequence != null) {
                        String text = charSequence.toString();
                        // 改行コードを CR に統一
                        text = text.replaceAll("\r\n|\n", "\r");
                        try {
                            result[0] = text.getBytes("Shift_JIS");
                        } catch (UnsupportedEncodingException e) {
                            try {
                                // Shift_JIS に変換できない場合は UTF-8 でエンコードする
                                result[0] = text.getBytes("UTF-8");
                            } catch (UnsupportedEncodingException ex) {
                                // UTF-8 での変換にも失敗した場合の処理
                                result[0] = null;
                            }
                        }
                    }
                }
            }
        });

        try {
            Thread.sleep(100); // 非同期処理完了を待機
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        return result[0];
    }

    // NDKから呼び出すためのメソッド
    public void showNewFileDialog(String message, String itemList, String extension, String mediaInfo, String addPath) {
        showNewFileDialogExecute(message, itemList, extension, mediaInfo, addPath, new NewFileCallback() {
            @Override
            public void onNewFileSelected(String filename, String mediaInfo, String addPath) {
                newFileCallback(filename, mediaInfo, addPath);
            }
        });
    }

    // ファイル選択ダイアログを表示する
    public void showNewFileDialogExecute(final String message, final String itemList, final String extension, final String mediaInfo, final String addPath, final NewFileCallback callback) {
        final Activity activity = this;
        activity.runOnUiThread(new Runnable() {
            final EditText input = new EditText(activity);
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(activity);
                builder.setTitle(message);

                input.setHint("ファイル名を入力してください（" + extension + "ファイル）");
                input.setSingleLine(true);
                builder.setView(input);

                builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        validateAndProcessInput((AlertDialog) dialog);
                    }
                });

                builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.cancel();
                        if (callback != null) {
                            callback.onNewFileSelected("", "", "");
                        }
                    }
                });

                final AlertDialog dialog = builder.create();

                input.setOnEditorActionListener(new TextView.OnEditorActionListener() {
                    @Override
                    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                        if (actionId == EditorInfo.IME_ACTION_DONE ||
                                (event != null && event.getKeyCode() == KeyEvent.KEYCODE_ENTER)) {
                            validateAndProcessInput(dialog);
                            return true;
                        }
                        return false;
                    }
                });

                dialog.show();
            }

            private void validateAndProcessInput(AlertDialog dialog) {
                String enteredText = input.getText().toString().trim();
                String fullFilename = enteredText.endsWith(extension) ? enteredText : enteredText + extension;

                List<String> existingFiles = Arrays.asList(itemList.toLowerCase().split(";"));
                if (!enteredText.isEmpty()) {
                    if (existingFiles.contains(fullFilename.toLowerCase())) {
                        Toast.makeText(activity, "ファイル名が重複しています。", Toast.LENGTH_SHORT).show();
                    } else {
                        dialog.dismiss();
                        if (callback != null) {
                            callback.onNewFileSelected(fullFilename, mediaInfo, addPath);
                        }
                    }
                }
            }
        });
    }

    //https://stackoverflow.com/questions/11730001/create-a-message-dialog-in-android-via-ndk-callback
    public int showAlert(final String message, final String itemList, boolean model, final int selectMode) {
        final AtomicInteger buttonId = new AtomicInteger();
        buttonId.set(-1);
        final long[] lastClickTime = {0}; // Shared last click time for all buttons
        final AlertDialog[] finalDialog = new AlertDialog[1]; // 配列のサイズを1に設定

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(EmulatorActivity.this);
                View dialogView = getLayoutInflater().inflate(R.layout.dialog_layout, null);
                builder.setView(dialogView);

                TextView title = dialogView.findViewById(R.id.dialog_title);
                title.setText(message);

                LinearLayout buttonLayout = dialogView.findViewById(R.id.button_layout);
                if (!itemList.isEmpty()) {
                    String[] items = itemList.split(";");
                    for (int i = 0; i < items.length; i++) {
                        Button button = new Button(EmulatorActivity.this);
                        button.setText(items[i]);
                        final int index = i;
                        button.setOnClickListener(new View.OnClickListener() {
                            @Override
                            public void onClick(View v) {
                                long clickTime = System.currentTimeMillis();
                                if (clickTime - lastClickTime[0] < 500) { // Double click
                                    buttonId.set(index);
                                    fileSelectCallback(index, selectMode);
                                    finalDialog[0].dismiss();
                                }
                                lastClickTime[0] = clickTime;
                            }
                        });
                        buttonLayout.addView(button);
                    }
                }

                Button cancelButton = dialogView.findViewById(R.id.cancel_button);
                cancelButton.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        fileSelectCallback(-1, selectMode);
                        finalDialog[0].dismiss();
                    }
                });

                AlertDialog dialog = builder.create();
                finalDialog[0] = dialog;
                dialog.show();
            }
        });
        return buttonId.get(); // 選択されたボタンのIDを返す
    }

    private void fileSelectCallback(int id, int selectMode) {
        switch (selectMode) {
            case 0: fileSelectCallback(id); break;
            case 1: bankSelectCallback(id); break;
            case 2: bootSelectCallback(id); break;
            case 3: exitSelectCallback(id); break;
        }
    }

    public int showExtendMenu(final String title, final String extendMenu) {
        final AtomicInteger buttonId = new AtomicInteger();
        final String[] nodes;
        buttonId.set(-1);
        if (!extendMenu.isEmpty()) {
            nodes = extendMenu.split(",");
        } else {
            return buttonId.get();
        }

        this.runOnUiThread(new Runnable() {
            public void run() {
                final Dialog dialog = new Dialog(EmulatorActivity.this);
                dialog.setTitle(title);
                dialog.setContentView(R.layout.custom_dialog_layout); // 事前に定義したカスタムレイアウトを使用

                LinearLayout layout = dialog.findViewById(R.id.custom_dialog_layout);
                for (int i = 0; i < nodes.length; i++) {
                    String[] node = nodes[i].split(";");
                    Button button = new Button(EmulatorActivity.this);
                    button.setText(node[1]);
                    // nodes[2] が "0" の時はフォルダ、"1" の時はファイル
                    //  フォルダの時はボタンの色を薄い黄色、ファイルの時は薄い青色にする
                    // 白に近い黄色の背景色を設定
                    // node 配列の数が足りない場合、可能なら配列の中の情報を logcat ダンプして continue する
                    if (node.length < 3) {
                        continue;
                    }
                    if (node[2].equals("0")) {
                        button.setBackgroundColor(Color.argb(255, 255, 255, 200)); // ARGBで白に近い黄色
                    } else {
                        button.setBackgroundColor(Color.argb(255, 200, 255, 255)); // ARGBで白に近い青
                    }
                    // テキスト色を設定（ここでは黒を例としています）
                    button.setTextColor(Color.BLACK);
                    final int index = i;
                    button.setOnClickListener(new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            buttonId.set(index);
                            Log.i("EmulatorActivity", "button clicked: " + nodes[index]);
                            extendMenuCallback(nodes[buttonId.get()]);
                            dialog.dismiss();
                        }
                    });
                    layout.addView(button);
                    // ボタンの下に少しマージンを空ける
                    View margin = new View(EmulatorActivity.this);
                    margin.setMinimumHeight(10);
                    layout.addView(margin);
                }

                Button cancelButton = dialog.findViewById(R.id.cancelButton);
                cancelButton.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Log.i("EmulatorActivity", "cancelButton clicked");
                        extendMenuCallback("");
                        dialog.dismiss();
                    }
                });

                Button backButton = dialog.findViewById(R.id.backButton);
                backButton.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (nodes.length > 0) {
                            String[] node = nodes[0].split(";");
                            Log.i("EmulatorActivity", "backButton clicked: " + node[4]);
                            extendMenuCallback(node[4]); // 親IDを渡す
                        } else {
                            Log.i("EmulatorActivity", "backButton clicked: empty");
                            extendMenuCallback("");
                        }
                        dialog.dismiss();
                    }
                });

                dialog.setCancelable(false);
                dialog.show();
            }
        });

        Log.i("EmulatorActivity", "showExtendMenu end");
        return buttonId.get();
    }

    public native void extendMenuCallback(String extendMenu);
    public native void fileSelectCallback(int id);
    public native void bankSelectCallback(int id);
    public native void bootSelectCallback(int id);
    public native void exitSelectCallback(int id);

    private int iconResouceId(int iconType, int iconId){
        switch(iconType){
            case 0://systemIcon
                switch(iconId) {
                    case 0:
                        return R.drawable.reset;
                    case 1:
                        return R.drawable.screen;
                    case 2:
                        return R.drawable.sound;
                    case 3:
                        return R.drawable.pcg;
                    case 4:
                        return R.drawable.config;

             }
            case 1://mediaIcon
                switch(iconId) {
                    case 0:
                        return R.drawable.floppy;
                    case 1:
                        return R.drawable.tape;
                    case 2:
                        return R.drawable.cart;
                    case 3:
                        return R.drawable.qd;
                    case 4:
                        return R.drawable.hdd;
                    case 5:
                        return R.drawable.cd;
                    case 6:
                        return R.drawable.bubble;
                    case 7:
                        return R.drawable.binary;
                }
        }
        return R.drawable.floppy;
    }

    //http://blog.livedoor.jp/itahidamito/archives/51661332.html
    public int[] loadBitmap(int iconType, int iconId){
        //画像を取得する
        Resources r = getResources();
        int id  = iconResouceId(iconType, iconId);

        //ビットマップ読み込みオプションの設定
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = true;    //端末スケールに応じたサイズで取得
        options.inPreferredConfig = Bitmap.Config.RGB_565;
        Bitmap bitmap = BitmapFactory.decodeResource(r, id, options);

        //リサイズ
        int size = 30;//端末スケールで取得すると、サイズ大きすぎるので、30%に縮小する。
        Bitmap afterResizeBitmap = Bitmap.createScaledBitmap(bitmap,
                (int) (bitmap.getWidth() * size / 100),
                (int) (bitmap.getHeight() * size / 100),
                false);

        Bitmap b = afterResizeBitmap.copy(Bitmap.Config.RGB_565, true);

        //Bitmapをint型の配列取得する
        int width = b.getWidth();
        int height = b.getHeight();
        int pixels[] = new int[width * height];
        b.getPixels(pixels, 0, width, 0, 0, width, height);

        //縦横サイズを最初に入れる
        int returnData[] = new int[width * height + 2];
        returnData[0] = width;
        returnData[1] = height;
        System.arraycopy(pixels, 0, returnData, 2, pixels.length);

        return returnData;
    }

    //http://android-note.open-memo.net/sub/image__draw_string_to_canvas.html
    public int[] createBitmapFromString(final String text, int textSize) {
        Paint objPaint = new Paint();
        Bitmap objBitmap;
        Canvas objCanvas;
        int textWidth = textSize * text.length(), textHeight = textSize;

        objPaint.setAntiAlias(true);
        objPaint.setColor(Color.WHITE);
        objPaint.setTextSize(textSize);
        Paint.FontMetrics fm = objPaint.getFontMetrics();
        objPaint.getTextBounds(text, 0, text.length(), new Rect(0, 0, textWidth, textHeight));
        //テキストの表示範囲を設定

        textWidth = (int) objPaint.measureText(text);
        textHeight = (int) (Math.abs(fm.top) + fm.bottom);
        objBitmap = Bitmap.createBitmap(textWidth, textHeight, Bitmap.Config.RGB_565);
        //キャンバスからビットマップを取得
        objCanvas = new Canvas(objBitmap);
        objCanvas.drawText(text, 0, Math.abs(fm.top), objPaint);

        Bitmap b = objBitmap.copy(Bitmap.Config.RGB_565, true);

        //Bitmapをint型の配列取得する
        int width = b.getWidth();
        int height = b.getHeight();
        int pixels[] = new int[width * height];
        b.getPixels(pixels, 0, width, 0, 0, width, height);

        //縦横サイズを最初に入れる
        int returnData[] = new int[width * height + 2];
        returnData[0] = width;
        returnData[1] = height;
        System.arraycopy(pixels, 0, returnData, 2, pixels.length);

        return returnData;
    }
}
