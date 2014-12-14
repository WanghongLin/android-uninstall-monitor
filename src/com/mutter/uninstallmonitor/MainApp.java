/*
 * Copyright (C) 2014 WanghongLin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mutter.uninstallmonitor;

import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;

import android.app.Application;
import android.os.AsyncTask;
import android.os.Build;

public class MainApp extends Application {

	static {
		Runtime.getRuntime().loadLibrary("uninstall_monitor");
	}
	
	/**
	 * use when IN_DELETE_SELF event on /data/data/com.example.appname can't not detected in some device<br/>
	 * self-created monitor file, it's not good idea, when the user clear data, it will
	 * trigger IN_DELETE_SELF event in native code which is not we want<br/>
	 * It might be the better idea by using a while true loop to detect the existence of 
	 * app's root data directory(/data/data/com.example.appname)
	 */
	public static final String UNINSTALL_MONITOR = "uninstall_monitor";

	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();
		new AsyncTask<Void, Void, String>() {

			@Override
			protected String doInBackground(Void... params) {
				// TODO Auto-generated method stub
				try {
					DataOutputStream outputStream = new DataOutputStream(openFileOutput(UNINSTALL_MONITOR, MODE_PRIVATE));
					outputStream.writeUTF("created for uninstall monitor");
					outputStream.flush();
					outputStream.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

				String path = getFilesDir().getAbsolutePath() + File.separator + UNINSTALL_MONITOR;
				if (new File(path).exists()) {
					return path;
				}
				
				return null;
			}

			@Override
			protected void onPostExecute(String result) {
				// TODO Auto-generated method stub
				super.onPostExecute(result);
				if (result != null) {
					if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.GINGERBREAD_MR1) {
						forkUninstallMonitorProcess(result, getString(R.string.uninstall_intent_url));
					} else {
						forkUninstallMonitorProcess(getApplicationInfo().dataDir, 
								getString(R.string.uninstall_intent_url));
					}
				}
			}
			
		}.execute(null, null, null);
	}
	
	private native void forkUninstallMonitorProcess(String monitorFile, String intentUrl);
}
