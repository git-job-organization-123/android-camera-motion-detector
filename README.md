# Android camera motion detector


<img src="https://github.com/git-job-organization-123/android-camera-motion-detector/blob/main/motion-123.png" width="800" />


## Install on Windows



**Download and install Android Studio**

https://deveoper.android.com/studio/

---

**Android SDK install**

Install "SDK Tools" and "SDK Platform-tools" in Android Studio SDK Manager

Add to path:

`%USERPROFILE%\AppData\Local\Android\Sdk\platform-tools`

Create `local.properties` file in android-camera-motion-detector root

Add sdk.dir to local.properties: 

`sdk.dir=C\:\\Users\\<user>\\AppData\\Local\\Android\\Sdk`

---

**Android NDK install**

Install "NDK (Native Development Kit)" in Android Studio SDK Tools

Add to path:

`%USERPROFILE%\AppData\Local\Android\Sdk\ndk\<version>`

Create `local.properties` file in android-camera-motion-detector root

Add ndk.dir to local.properties:

`ndk.dir=C\:\\Users\\<user>\\AppData\\Local\\Android\\Sdk\\ndk\\<version>`

---

**Gradle install**

Download and install:

https://gradle.org/install/

Add to path:

`C:\Gradle\gradle-7.6\bin`

---

**OpenCV Android SDK install**

Download:

https://sourceforge.net/projects/opencvlibrary/files/4.4.0/opencv-4.4.0-android-sdk.zip/download

Extract opencv-4.4.0-android-sdk.zip to C:\

---

**Debug build**

`cd C:\Users\<user>\android-camera-motion-detector`

`gradle assembleDebug`

Install to Android device:

`adb install app/build/outputs/apk/debug/app-debug.apk`

app-debug.apk size: ~28 MB

---

**Production build**

`cd C:\Users\<user>\android-camera-motion-detector`

`gradle assemble`

Install to Android device:

`adb install app/build/outputs/apk/debug/app-release-unsigned.apk`

app-release-unsigned.apk size: ~21 MB
