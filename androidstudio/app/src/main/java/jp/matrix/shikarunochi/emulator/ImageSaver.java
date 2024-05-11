package jp.matrix.shikarunochi.emulator;

import android.content.ContentValues;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

public class ImageSaver {

    public static void saveImage(Context context, String imageName, byte[] imageData, int width, int height) {
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        ByteBuffer buffer = ByteBuffer.wrap(imageData);
        bitmap.copyPixelsFromBuffer(buffer);

        if (Build.VERSION.SDK_INT >= 29) {
            saveImageToGalleryApi29AndAbove(context, imageName, bitmap);
        } else {
            saveImageToGalleryApi28AndBelow(context, imageName, bitmap);
        }
    }

    private static void saveImageToGalleryApi29AndAbove(Context context, String imageName, Bitmap bitmap) {
        ContentValues values = new ContentValues();
        values.put(MediaStore.Images.Media.DISPLAY_NAME, imageName);
        values.put(MediaStore.Images.Media.MIME_TYPE, "image/png");
        // API 29以降の保存パス設定
        values.put("relative_path", Environment.DIRECTORY_PICTURES + File.separator + "YourAlbumName");  // 'YourAlbumName' を適宜替えてください

        Uri uri = context.getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values);
        try (OutputStream out = context.getContentResolver().openOutputStream(uri)) {
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, out);
        } catch (IOException e) {
            Log.e("ImageSaver", "Failed to save image", e);
        }
    }

    private static void saveImageToGalleryApi28AndBelow(Context context, String imageName, Bitmap bitmap) {
        // パブリックディレクトリにファイルを保存
        File picturesDirectory = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
        if (!picturesDirectory.exists() && !picturesDirectory.mkdirs()) {
            Log.e("ImageSaver", "Failed to create directory: " + picturesDirectory.getAbsolutePath());
            return;
        }
        File imageFile = new File(picturesDirectory, imageName + ".png");

        try (FileOutputStream out = new FileOutputStream(imageFile)) {
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, out);
            addImageToGallery(context, imageFile.getAbsolutePath()); // ギャラリーに追加
            Log.d("ImageSaver", "Image saved to: " + imageFile.getAbsolutePath());
        } catch (IOException e) {
            Log.e("ImageSaver", "Failed to save image", e);
        }
    }


    private static void addImageToGallery(Context context, String filePath) {
        ContentValues values = new ContentValues();
        values.put(MediaStore.Images.Media.DATE_TAKEN, System.currentTimeMillis());
        values.put(MediaStore.Images.Media.MIME_TYPE, "image/png");
        values.put(MediaStore.MediaColumns.DATA, filePath);

        context.getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values);
    }
}
