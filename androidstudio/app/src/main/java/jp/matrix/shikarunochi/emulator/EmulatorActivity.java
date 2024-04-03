package jp.matrix.shikarunochi.emulator;

import static android.content.ContentValues.TAG;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.NativeActivity;
import android.content.Context;
import android.content.DialogInterface;
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
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Toast;

import androidx.annotation.NonNull;

import java.io.File;
import java.util.concurrent.atomic.AtomicInteger;

public class EmulatorActivity extends NativeActivity {

    private static final int PERMISSIONS_REQUEST_CODE = 1;
    private volatile boolean permissionsGranted = false;
    private static final String[] PERMISSIONS_REQUIRED = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
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
        // 権限チェックの実装
        for (String permission : PERMISSIONS_REQUIRED) {
            if (checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
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
                if (EmulatorActivity.this.hasPermissions()) {
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

    //https://stackoverflow.com/questions/11730001/create-a-message-dialog-in-android-via-ndk-callback
    public int showAlert(final String message, final String itemList, boolean model, final int selectMode) {
        final AtomicInteger buttonId = new AtomicInteger();
        buttonId.set(-1);

        this.runOnUiThread(new Runnable() {
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(EmulatorActivity.this);
                // メッセージをタイトルと詳細に分割
                String[] parts = message.split("\n", 2); // 2つに分割
                String title = parts[0];
                String detail = parts.length > 1 ? parts[1] : "";
                builder.setTitle(title);
                // 詳細テキストをメッセージとして設定
                if (!detail.isEmpty()) {
                    builder.setMessage(detail);
                }
                if(itemList.length() > 0) {
                    String itemLabels[] = itemList.split(";");
                    builder.setSingleChoiceItems(itemLabels, -1, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            buttonId.set(which);
                        }
                    });
                }else{
                    buttonId.set(0);//選択無しの場合。OKは 0で返す。
                }
                builder.setPositiveButton("OK",new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        if(selectMode == 0) {
                            fileSelectCallback(buttonId.get());
                        }else if(selectMode == 1){
                            bankSelectCallback(buttonId.get());
                        }else if(selectMode == 2){
                            bootSelectCallback(buttonId.get());
                        }else {
                            exitSelectCallback(buttonId.get());
                        }
                        dialog.dismiss();
                    }
                });
                builder.setNegativeButton("CANCEL",new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        if(selectMode == 0) {
                            fileSelectCallback(-1);
                        }else if(selectMode == 1){
                            bankSelectCallback(-1);
                        }else if(selectMode == 2){
                            bootSelectCallback(-1);
                        }else{
                            exitSelectCallback(-1);
                        }
                        dialog.dismiss();
                    }
                });

                builder.setCancelable(false);
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
        return buttonId.get();
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
                            extendMenuCallback(node[4]); // 親IDを渡す
                        } else {
                            extendMenuCallback("");
                        }
                        dialog.dismiss();
                    }
                });

                dialog.setCancelable(false);
                dialog.show();
            }
        });

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