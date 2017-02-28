Android App Uninstall Monitor Sample
------
This sample using jni to fork a detached process to monitor/detect 
the app uninstall by user.
By checking the app data root directory's existence, to know the app
is uninstall by user.
After that, it will sent an intent with a specified URL(usually a feedback
web page of your app) to open in the browser.

Note:
######
This solution didn't test on all Android platfrom and device, it might not
work on your device.

##### Update
This solution won't work after Android 5.0 (Lollipop), the forked native process
will be killed after main process stopped or killed.
