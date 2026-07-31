package com.sovworks.eds.android.helpers;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.ClipData;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapRegionDecoder;
import android.graphics.Rect;
import android.os.Build;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowManager;

import com.sovworks.eds.android.R;
import com.sovworks.eds.fs.Path;

import java.io.IOException;
import java.io.InputStream;
import java.util.WeakHashMap;

@SuppressLint("NewApi")
public class CompatHelperBase
{
	private static final WeakHashMap<View, int[]> LEGACY_PADDING = new WeakHashMap<>();

	/**
	 * Keeps legacy (pre edge-to-edge) screens below an ActionBar and above the
	 * navigation bar on Android 15+. This is deliberately geometry based so it
	 * also works with custom action-bar heights and non-edge-to-edge windows.
	 */
	public static void applyLegacyContentInsets(final Activity activity)
	{
		if (activity == null || activity.isFinishing())
			return;
		final Window window = activity.getWindow();
		final int flags = window.getDecorView().getSystemUiVisibility();
		if ((window.getAttributes().flags & WindowManager.LayoutParams.FLAG_FULLSCREEN) != 0 ||
				(flags & View.SYSTEM_UI_FLAG_FULLSCREEN) != 0 ||
				(flags & View.SYSTEM_UI_FLAG_LAYOUT_NO_LIMITS) != 0)
			return;
		final View root = activity.findViewById(android.R.id.content);
		if (root == null)
			return;
		synchronized (LEGACY_PADDING)
		{
			if (!LEGACY_PADDING.containsKey(root))
				LEGACY_PADDING.put(root, new int[] {root.getPaddingLeft(), root.getPaddingTop(), root.getPaddingRight(), root.getPaddingBottom()});
		}
		final Runnable apply = () -> updateLegacyContentInsets(activity, root);
		root.getViewTreeObserver().addOnGlobalLayoutListener(apply::run);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
		{
			window.getDecorView().setOnApplyWindowInsetsListener((v, insets) -> {
				apply.run();
				return insets;
			});
		}
		root.post(apply);
	}

	private static void updateLegacyContentInsets(Activity activity, View root)
	{
		if (!root.isAttachedToWindow())
			return;
		int[] base;
		synchronized (LEGACY_PADDING) { base = LEGACY_PADDING.get(root); }
		if (base == null)
			return;
		int[] location = new int[2];
		root.getLocationOnScreen(location);
		int topSystem = 0;
		int bottomSystem = 0;
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
		{
			WindowInsets wi = root.getRootWindowInsets();
			if (wi != null)
			{
				if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R)
				{
					android.graphics.Insets bars = wi.getInsets(WindowInsets.Type.systemBars());
					topSystem = bars.top;
					bottomSystem = bars.bottom;
				}
				else
				{
					topSystem = wi.getSystemWindowInsetTop();
					bottomSystem = wi.getSystemWindowInsetBottom();
				}
			}
		}
		int actionBarHeight = activity.getActionBar() == null ? 0 : activity.getActionBar().getHeight();
		int topOverlap = Math.max(0, topSystem + actionBarHeight - location[1]);
		int bottomPadding = Math.max(base[3], bottomSystem);
		root.setPadding(base[0], base[1] + topOverlap, base[2], bottomPadding);
	}
	public static void setWindowFlagSecure(Activity act)
	{
		if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
		    act.getWindow().setFlags(WindowManager.LayoutParams.FLAG_SECURE, WindowManager.LayoutParams.FLAG_SECURE);
	}

	public static void restartActivity(Activity activity)
	{
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
		    activity.recreate();
		else
		{
		    Intent intent = activity.getIntent();
		    activity.overridePendingTransition(0, 0);
		    intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
		    activity.finish();
		    activity.overridePendingTransition(0, 0);
		    activity.startActivity(intent);
		}
	}

	public static void storeTextInClipboard(Context context,String text)
	{
        if(Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB)
        {
            @SuppressWarnings("deprecation") android.text.ClipboardManager clipboard = (android.text.ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
            clipboard.setText(text);
        }
        else
        {
            android.content.ClipboardManager clipboard = (android.content.ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
            ClipData clip = ClipData.newPlainText("Text", text);
            clipboard.setPrimaryClip(clip);
        }
	}

	public static Bitmap loadBitmapRegion(Path path,int sampleSize,Rect regionRect) throws IOException
	{
		BitmapFactory.Options options = new BitmapFactory.Options();
	    options.inSampleSize = sampleSize;
	    InputStream data = path.getFile().getInputStream();
		try
		{
			if(Build.VERSION.SDK_INT>= Build.VERSION_CODES.GINGERBREAD_MR1)
			{
				BitmapRegionDecoder decoder = BitmapRegionDecoder.newInstance(data, true);
				try
				{
					return decoder.decodeRegion(regionRect, options);
				}
				finally
				{
					decoder.recycle();
				}
			}
			else
				return BitmapFactory.decodeStream(data, null, options);
		}
		finally
		{
			data.close();
		}
	}

	private static String serviceRunningNotificationsChannelId;
	public static synchronized String getServiceRunningNotificationsChannelId(Context context)
	{
		if (serviceRunningNotificationsChannelId == null)
		{
			serviceRunningNotificationsChannelId = "com.sovworks.eds.SERVICE_RUNNING_CHANNEL2";
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				NotificationChannel channel = new NotificationChannel(
					serviceRunningNotificationsChannelId,
					context.getString(R.string.service_notifications_channel_name),
					NotificationManager.IMPORTANCE_LOW
				);
				channel.enableLights(false);
				channel.enableVibration(false);
				NotificationManager notificationManager = context.getSystemService(NotificationManager.class);
				notificationManager.createNotificationChannel(channel);
			}
		}
		return serviceRunningNotificationsChannelId;
	}

	private static String fileOperationsNotificationsChannelId;
	public static synchronized String getFileOperationsNotificationsChannelId(Context context)
	{
		if (fileOperationsNotificationsChannelId == null)
		{
			fileOperationsNotificationsChannelId = "com.sovworks.eds.FILE_OPERATIONS_CHANNEL2";
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				NotificationChannel channel = new NotificationChannel(
					fileOperationsNotificationsChannelId,
					context.getString(R.string.file_operations_notifications_channel_name),
					NotificationManager.IMPORTANCE_LOW
				);
				channel.enableLights(false);
				channel.enableVibration(false);
				NotificationManager notificationManager = context.getSystemService(NotificationManager.class);
				notificationManager.createNotificationChannel(channel);
			}
		}
		return fileOperationsNotificationsChannelId;
	}
		
}
