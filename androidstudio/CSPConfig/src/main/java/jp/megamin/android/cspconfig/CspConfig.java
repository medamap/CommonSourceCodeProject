package jp.megamin.android.cspconfig;

import android.app.Activity;
import android.content.pm.PackageManager;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.os.Environment;

import java.io.File;

public class CspConfig {
    private static final int STORAGE_PERMISSION_CODE = 1;

    public static boolean checkAndRequestPermissions(Activity activity) {
        // 読み取りと書き込みの両方の権限をチェック
        if (ContextCompat.checkSelfPermission(activity, android.Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(activity, android.Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            // 両方の権限を要求
            ActivityCompat.requestPermissions(activity, new String[]{
                    android.Manifest.permission.READ_EXTERNAL_STORAGE,
                    android.Manifest.permission.WRITE_EXTERNAL_STORAGE
            }, STORAGE_PERMISSION_CODE);
            return false;
        }
        return true;
    }

    public static String getSdcardDownloadPath(Activity activity) {
        File externalStorageDirectory = Environment.getExternalStorageDirectory();
        String externalStoragePath = externalStorageDirectory.getPath();
        String downloadPathSuffix = "/Download";
        return externalStoragePath + downloadPathSuffix;
    }
}
