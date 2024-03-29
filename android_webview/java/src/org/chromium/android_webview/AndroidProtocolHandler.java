// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview;

import android.content.Context;
import android.content.res.AssetManager;
import android.net.Uri;
import android.util.Log;
import android.util.TypedValue;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;

import java.io.IOException;
import java.io.InputStream;
import java.net.URLConnection;
import java.util.List;

/**
 * Implements the Java side of Android URL protocol jobs.
 * See android_protocol_handler.cc.
 */
@JNINamespace("android_webview")
public class AndroidProtocolHandler {
    private static final String TAG = "AndroidProtocolHandler";

    // Supported URL schemes. This needs to be kept in sync with
    // clank/native/framework/chrome/url_request_android_job.cc.
    private static final String FILE_SCHEME = "file";
    private static final String CONTENT_SCHEME = "content";

    /**
     * Open an InputStream for an Android resource.
     * @param context The context manager.
     * @param url The url to load.
     * @return An InputStream to the Android resource.
     */
    @CalledByNative
    public static InputStream open(Context context, String url) {
        Uri uri = verifyUrl(url);
        if (uri == null) {
            return null;
        }
        try {
            String path = uri.getPath();
            if (uri.getScheme().equals(FILE_SCHEME)) {
                if (path.startsWith(nativeGetAndroidAssetPath())) {
                    return openAsset(context, uri);
                } else if (path.startsWith(nativeGetAndroidResourcePath())) {
                    return openResource(context, uri);
                }
            } else if (uri.getScheme().equals(CONTENT_SCHEME)) {
                return openContent(context, uri);
            }
        } catch (Exception ex) {
            Log.e(TAG, "Error opening inputstream: " + url);
        }
        return null;
    }

    // Assumes input string is in the format "foo.bar.baz" and strips out the last component.
    // Returns null on failure.
    private static String removeOneSuffix(String input) {
        if (input == null) return null;
        int lastDotIndex = input.lastIndexOf('.');
        if (lastDotIndex == -1) return null;
        return input.substring(0, lastDotIndex);
    }

    private static Class<?> getClazz(Context context, String packageName, String assetType)
            throws ClassNotFoundException {
        return context.getClassLoader().loadClass(packageName + ".R$" + assetType);
    }

    private static int getFieldId(Context context, String assetType, String assetName)
        throws ClassNotFoundException, NoSuchFieldException, IllegalAccessException {
        Class<?> clazz = null;
        try {
            clazz = getClazz(context, context.getPackageName(), assetType);
        } catch (ClassNotFoundException e) {
            // Strip out components from the end so gradle generated application suffix such as
            // com.example.my.pkg.pro works. This is by no means bulletproof.
            String packageName = context.getPackageName();
            while (clazz == null) {
                packageName = removeOneSuffix(packageName);
                // Throw original exception which contains the entire package id.
                if (packageName == null) throw e;
                try {
                    clazz = getClazz(context, packageName, assetType);
                } catch (ClassNotFoundException ignored) {
                    // Strip and try again.
                }
            }
        }

        java.lang.reflect.Field field = clazz.getField(assetName);
        int id = field.getInt(null);
        return id;
    }

    private static int getValueType(Context context, int fieldId) {
        TypedValue value = new TypedValue();
        context.getResources().getValue(fieldId, value, true);
        return value.type;
    }

    private static InputStream openResource(Context context, Uri uri) {
        assert uri.getScheme().equals(FILE_SCHEME);
        assert uri.getPath() != null;
        assert uri.getPath().startsWith(nativeGetAndroidResourcePath());
        // The path must be of the form "/android_res/asset_type/asset_name.ext".
        List<String> pathSegments = uri.getPathSegments();
        if (pathSegments.size() != 3) {
            Log.e(TAG, "Incorrect resource path: " + uri);
            return null;
        }
        String assetPath = pathSegments.get(0);
        String assetType = pathSegments.get(1);
        String assetName = pathSegments.get(2);
        if (!("/" + assetPath + "/").equals(nativeGetAndroidResourcePath())) {
            Log.e(TAG, "Resource path does not start with " + nativeGetAndroidResourcePath()
                    + ": " + uri);
            return null;
        }
        // Drop the file extension.
        assetName = assetName.split("\\.")[0];
        try {
            // Use the application context for resolving the resource package name so that we do
            // not use the browser's own resources. Note that if 'context' here belongs to the
            // test suite, it does not have a separate application context. In that case we use
            // the original context object directly.
            if (context.getApplicationContext() != null) {
                context = context.getApplicationContext();
            }
            int fieldId = getFieldId(context, assetType, assetName);
            int valueType = getValueType(context, fieldId);
            if (valueType == TypedValue.TYPE_STRING) {
                return context.getResources().openRawResource(fieldId);
            } else {
                Log.e(TAG, "Asset not of type string: " + uri);
                return null;
            }
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "Unable to open resource URL: " + uri, e);
            return null;
        } catch (NoSuchFieldException e) {
            Log.e(TAG, "Unable to open resource URL: " + uri, e);
            return null;
        } catch (IllegalAccessException e) {
            Log.e(TAG, "Unable to open resource URL: " + uri, e);
            return null;
        }
    }

    private static InputStream openAsset(Context context, Uri uri) {
        assert uri.getScheme().equals(FILE_SCHEME);
        assert uri.getPath() != null;
        assert uri.getPath().startsWith(nativeGetAndroidAssetPath());
        String path = uri.getPath().replaceFirst(nativeGetAndroidAssetPath(), "");
        try {
            AssetManager assets = context.getAssets();
            return assets.open(path, AssetManager.ACCESS_STREAMING);
        } catch (IOException e) {
            Log.e(TAG, "Unable to open asset URL: " + uri);
            return null;
        }
    }

    private static InputStream openContent(Context context, Uri uri) {
        assert uri.getScheme().equals(CONTENT_SCHEME);
        try {
            return context.getContentResolver().openInputStream(uri);
        } catch (Exception e) {
            Log.e(TAG, "Unable to open content URL: " + uri);
            return null;
        }
    }

    /**
     * Determine the mime type for an Android resource.
     * @param context The context manager.
     * @param stream The opened input stream which to examine.
     * @param url The url from which the stream was opened.
     * @return The mime type or null if the type is unknown.
     */
    @CalledByNative
    public static String getMimeType(Context context, InputStream stream, String url) {
        Uri uri = verifyUrl(url);
        if (uri == null) {
            return null;
        }
        try {
            String path = uri.getPath();
            // The content URL type can be queried directly.
            if (uri.getScheme().equals(CONTENT_SCHEME)) {
                return context.getContentResolver().getType(uri);
                // Asset files may have a known extension.
            } else if (uri.getScheme().equals(FILE_SCHEME)
                       && path.startsWith(nativeGetAndroidAssetPath())) {
                String mimeType = URLConnection.guessContentTypeFromName(path);
                if (mimeType != null) {
                    return mimeType;
                }
            }
        } catch (Exception ex) {
            Log.e(TAG, "Unable to get mime type" + url);
            return null;
        }
        // Fall back to sniffing the type from the stream.
        try {
            return URLConnection.guessContentTypeFromStream(stream);
        } catch (IOException e) {
            return null;
        }
    }

    /**
     * Make sure the given string URL is correctly formed and parse it into a Uri.
     * @return a Uri instance, or null if the URL was invalid.
     */
    private static Uri verifyUrl(String url) {
        if (url == null) {
            return null;
        }
        Uri uri = Uri.parse(url);
        if (uri == null) {
            Log.e(TAG, "Malformed URL: " + url);
            return null;
        }
        String path = uri.getPath();
        if (path == null || path.length() == 0) {
            Log.e(TAG, "URL does not have a path: " + url);
            return null;
        }
        return uri;
    }

    /**
     * Set the context to be used for resolving resource queries.
     * @param context Context to be used, or null for the default application
     *                context.
     */
    public static void setResourceContextForTesting(Context context) {
        nativeSetResourceContextForTesting(context);
    }

    private static native void nativeSetResourceContextForTesting(Context context);
    private static native String nativeGetAndroidAssetPath();
    private static native String nativeGetAndroidResourcePath();
}
