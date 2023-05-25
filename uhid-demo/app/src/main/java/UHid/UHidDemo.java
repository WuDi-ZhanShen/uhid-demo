package UHid;

import java.util.Scanner;

// Usage: 1.Build this project and you will get an apk file.
// Usage: 2.unzip classes.dex and libUHid.so from apk, then move classes.dex to /sdcard, and move libUHid.so to /data/local/tmp/, and chmod 777 /data/local/tmp/libUHid.so
// Usage: 3.Run UHidDemo by this command: export CLASSPATH=/sdcard/classes.dex;app_process -Djava.library.path="/data/local/tmp/" / UHid.UHidDemo


public class UHidDemo {

    // JNI functions to create a uhid-Keyboard

    static native boolean nativeCreateUHidKeyboard();// create keyboard device

    static native void nativeUHidKeyboardEvent(byte[] scanCode); // send keyboard scanCode. scanCode should have the length of 8.

    static native boolean nativeCloseUHidKeyboard();// close keyboard device




    // JNI functions to create a uhid-Mouse
    static native boolean nativeCreateUHidMouse();// create mouse device

    static native void nativeUHidMouseEvent(byte[] scanCode);// send mouse scanCode. scanCode should have the length of 4.

    static native boolean nativeCloseUHidMouse();// close mouse device





    // Store whether the keyboard and mouse have been created
    static boolean isUHidKeyboardCreated = false, isUHidMouseCreated = false;



    public static void main(String[] args) {


        //check permission
        int uid = android.os.Process.myUid();
        if (uid != 0 && uid != 2000) {
            System.err.printf("Insufficient permission! Need to be launched by adb (uid 2000) or root (uid 0), but your uid is %d \n", uid);
            System.exit(-1);
            return;
        }

        //load library
        System.loadLibrary("UHid");


        // create uhid-Keyboard and uhid-Mouse
        isUHidKeyboardCreated = nativeCreateUHidKeyboard();
        isUHidMouseCreated = nativeCreateUHidMouse();


        // Check the result of device creation.
        if (!isUHidMouseCreated || !isUHidKeyboardCreated) {
            System.err.println("Failed to create uhid device!");
            System.exit(-1);
            return;
        }


        System.out.println("UHid keyboard and mouse have been created. You can type \"exit\" at any time to exit the program.");


        // Add a shutdown hook to close the uhid device in case of program failure.
        Runtime.getRuntime().addShutdownHook(new Thread() {
            @Override
            public void run() {
                if (isUHidKeyboardCreated) isUHidKeyboardCreated = !nativeCloseUHidKeyboard();
                if (isUHidMouseCreated) isUHidMouseCreated = !nativeCloseUHidMouse();
            }
        });


        // Keep the Java program running, otherwise the uhid device will be automatically closed when the program ends.
        try {
            Scanner scanner = new Scanner(System.in);
            String inline;
            label:
            while ((inline = scanner.nextLine()) != null) {
                switch (inline) {
                    case "exit":
                        break label;
                    case "k":
                        nativeUHidKeyboardEvent(new byte[]{0, 0, 4, 0, 0, 0, 0, 0});
                        break;
                    case "m":
                        nativeUHidMouseEvent(new byte[]{0, 0, 2, -1});
                        break;
                }
            }
            scanner.close();
        } catch (Exception unused) {
            // If this java program was launched using the "nohup" command, the Scanner will throw an exception because System.in is null.
            while (true) ;
        }


        if (isUHidKeyboardCreated) isUHidKeyboardCreated = !nativeCloseUHidKeyboard();
        if (isUHidMouseCreated) isUHidMouseCreated = !nativeCloseUHidMouse();
        System.out.println("Exit.\n");
    }



}
