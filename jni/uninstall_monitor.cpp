/*
 * Copyright (C) 2014 mutter
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

#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <sys/inotify.h>
#include "com_mutter_uninstallmonitor_MainApp.h"

#define DEBUG_TAG	"UninstallMonitor"
#define LOGI(...)	__android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, __VA_ARGS__)
#define LOGE(...)	__android_log_print(ANDROID_LOG_ERROR, DEBUG_TAG, __VA_ARGS__)
#define EVENT_SIZE	( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

void block_if_exists(const char*);

JNIEXPORT void JNICALL Java_com_mutter_uninstallmonitor_MainApp_forkUninstallMonitorProcess(
		JNIEnv *env, jobject object, jstring dirName, jstring intentUrl) {

	jboolean isCopy = true;
	const char* monitor_dir = env->GetStringUTFChars(dirName, &isCopy);
	const char* intent_url = env->GetStringUTFChars(intentUrl, &isCopy);
	LOGI("monitor directory %s\n", monitor_dir);
	LOGI("intent data uri %s\n", intent_url);

	pid_t pid;

	usleep(100);

	if ((pid = fork()) < 0) {
		LOGE("fork error");
	} else if (pid == 0) { /* first child */

		if ((pid = fork()) < 0) {
			LOGI("fork error");
		} else if (pid > 0) {
			exit(0);
		}

		// second child process
		setsid();
		char cwd[64] = { '0' };
		daemon(0, 0);
		getcwd(cwd, sizeof(cwd));
		LOGI("second child, pid = %d, cwd = %s\n", getpid(), cwd);

		int ret = -1;
		if ((ret = access(monitor_dir, F_OK | R_OK)) != 0) {
			LOGE("access error");
		}

		int length, i = 0;
		int fd;
		int wd;
		char buffer[EVENT_BUF_LEN];

		/* creating the INOTIFY instance */
		fd = inotify_init();

		if (fd < 0) {
			LOGE("inotify_init");
		}

		/**
		 * add to watched list
		 */
		wd = inotify_add_watch(fd, monitor_dir, IN_DELETE_SELF);
		LOGI("add to watched %s, fd = %d\n", monitor_dir, wd);

		/*
		 * Read to determine the event change happen on data directory.
		 * Actually this read blocks until the change event occurs
		 */

		struct stat sb;
		if (stat(monitor_dir, &sb) == -1) {
			LOGE("stat");
		}

		if (S_ISDIR(sb.st_mode)) {
			LOGI("blocked on read call");
			length = read(fd, buffer, EVENT_BUF_LEN);

			if (length < 0) {
				LOGE("read");
			}
		} else {
			LOGI("blocked on if exists call");
			block_if_exists(dirname(dirname(monitor_dir)));
		}

		LOGI("monitor file %s removed\n", monitor_dir);

		pid_t intent_pid;
		if ((intent_pid = fork()) < 0) {
			LOGE("fork error");
		} else if (intent_pid == 0) {
			LOGI("exec process");
			// Note: this won't work on Android 5.0, which require the system level permission
			// android.permission.INTERACT_ACROSS_USERS_FULL
			const char* envp[5] = {
					"SHELL=/system/bin/sh",
					"PATH=/sbin:/vendor/bin:/system/sbin:/system/bin:/system/xbin",
					"CLASSPATH=/system/framework/am.jar"
			};
			char bootclasspath[8192] = { '0' };
			sprintf(bootclasspath, "BOOTCLASSPATH=%s", getenv("BOOTCLASSPATH"));
			envp[3] = (const char*) bootclasspath;
			envp[4] = NULL;
			if (execle("/system/bin/app_process", "app_process",
					"/system/bin", "com.android.commands.am.Am",
					"start", "-a", "android.intent.action.VIEW",
					"-d", intent_url,
					"--activity-single-top", (char*) NULL, envp) == -1) {
				LOGI("execl error");
			} else {
				// this will never going to happen
				LOGI("execle successfully");
			}
		}
		inotify_rm_watch(fd, wd);

		// this work on Android 2.3.3, tested on HTC Desire
		// system("/system/bin/am start -a android.intent.action.VIEW -d http://www.baidu.com --activity-single-top");
		/* closing the INOTIFY instance */
		close(fd);
		exit(0);
	}

	//if (waitpid(pid, NULL, 0) != pid) {
	//	LOGI("wait error");
	//}
}

void block_if_exists(const char *directory)
{
	if (directory == NULL) {
		return;
	}
	LOGI("while true monitor dir %s\n", directory);
	while (1) {
		int ret = -1;
		if ((ret = access(directory, F_OK | R_OK)) != 0) {
			break;
		}
		sleep(1);
	}
}
