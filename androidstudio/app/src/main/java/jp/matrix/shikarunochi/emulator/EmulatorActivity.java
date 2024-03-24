package jp.matrix.shikarunochi.emulator;

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
import android.view.Window;
import android.view.WindowManager;

import java.io.File;
import java.util.concurrent.atomic.AtomicInteger;

public class EmulatorActivity extends NativeActivity {

    private static final int PERMISSIONS_REQUEST_CODE = 1;
    private static final int OPEN_DOCUMENT_REQUEST_CODE = 1;
    private static final String[] PERMISSIONS_REQUIRED = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // 画面を縦方向に固定
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!hasPermissions()) {
                requestPermissions(PERMISSIONS_REQUIRED, PERMISSIONS_REQUEST_CODE);
            }
        }
        // ユーザーにファイルピッカーを表示し、SDカード上のファイルへのアクセスを許可してもらう
        //Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        //intent.addCategory(Intent.CATEGORY_OPENABLE);
        //intent.setType("*/*");  // 必要に応じてファイルタイプを指定
        //startActivityForResult(intent, OPEN_DOCUMENT_REQUEST_CODE);

        System.loadLibrary("native-activity");
    }

    private boolean hasPermissions() {
        for (String permission : PERMISSIONS_REQUIRED) {
            if (checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
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