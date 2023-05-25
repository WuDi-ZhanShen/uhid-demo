#include <jni.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <linux/uinput.h>
#include <cerrno>
#include <fcntl.h>
#include <poll.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <termios.h>
#include <unistd.h>
#include <linux/uhid.h>
#include <linux/input.h>
#include <jni.h>
#include <android/log.h>

#define KEYBOARD_SCANCODE_LENGTH 8
#define MOUSE_SCANCODE_LENGTH 4


static unsigned char hidKeyboardDescription[] = {

        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x06,        // Usage (Keyboard)
        0xA1, 0x01,        // Collection (Application)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0xE0,        //   Usage Minimum (0xE0)
        0x29, 0xE7,        //   Usage Maximum (0xE7)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x08,        //   Report Count (8)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x08,        //   Report Size (8)
        0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x05,        //   Report Count (5)
        0x75, 0x01,        //   Report Size (1)
        0x05, 0x08,        //   Usage Page (LEDs)
        0x19, 0x01,        //   Usage Minimum (Num Lock)
        0x29, 0x05,        //   Usage Maximum (Kana)
        0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x03,        //   Report Size (3)
        0x91, 0x03,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x06,        //   Report Count (6)
        0x75, 0x08,        //   Report Size (8)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x65,        //   Logical Maximum (101)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0x00,        //   Usage Minimum (0x00)
        0x29, 0x65,        //   Usage Maximum (0x65)
        0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              // End Collection

// 63 bytes
};


static unsigned char hidMouseDescription[] = {

        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x02,        // Usage (Mouse)
        0xA1, 0x01,        // Collection (Application)
        0x09, 0x01,        //   Usage (Pointer)
        0xA1, 0x00,        //   Collection (Physical)
        0x05, 0x09,        //     Usage Page (Button)
        0x19, 0x01,        //     Usage Minimum (0x01)
        0x29, 0x05,        //     Usage Maximum (0x05)
        0x15, 0x00,        //     Logical Minimum (0)
        0x25, 0x01,        //     Logical Maximum (1)
        0x95, 0x05,        //     Report Count (5)
        0x75, 0x01,        //     Report Size (1)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x01,        //     Report Count (1)
        0x75, 0x03,        //     Report Size (3)
        0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
        0x09, 0x30,        //     Usage (X)
        0x09, 0x31,        //     Usage (Y)
        0x09, 0x38,        //     Usage (Wheel)
        0x15, 0x81,        //     Logical Minimum (-127)
        0x25, 0x7F,        //     Logical Maximum (127)
        0x75, 0x08,        //     Report Size (8)
        0x95, 0x03,        //     Report Count (3)
        0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //   End Collection
        0xC0,              // End Collection

// 52 bytes
};


static int uHidKeyboardFd,uHidMouseFd;
static struct uhid_event keyboardEvent,mouseEvent;


extern "C"
JNIEXPORT jboolean JNICALL
Java_UHid_UHidDemo_nativeCreateUHidKeyboard(JNIEnv *env, jclass clazz) {
    if ((uHidKeyboardFd = open("/dev/uhid", O_RDWR | O_NDELAY)) < 0) {
        return false;
    }

    struct uhid_event ev = {0};
    ev.type = UHID_CREATE;
    strcpy((char *) ev.u.create.name, "uhid-Keyboard");
    ev.u.create.rd_data = hidKeyboardDescription;
    ev.u.create.rd_size = sizeof(hidKeyboardDescription);
    ev.u.create.bus = BUS_VIRTUAL;
    ev.u.create.vendor = 0x1;
    ev.u.create.product = 0x1;
    ev.u.create.version = 0x1;
    ev.u.create.country = 0;
    if (write(uHidKeyboardFd, &ev, sizeof(ev)) != sizeof(uhid_event)) {
        return false;
    }

    memset(&keyboardEvent, 0, sizeof(uhid_event));
    keyboardEvent.type = UHID_INPUT;
    keyboardEvent.u.input.size = KEYBOARD_SCANCODE_LENGTH;

    return true;
}


extern "C"
JNIEXPORT void JNICALL
Java_UHid_UHidDemo_nativeUHidKeyboardEvent(JNIEnv *env, jclass clazz, jbyteArray scan_code) {
    jbyte *elements = env->GetByteArrayElements(scan_code, NULL);
    for (int i = 0; i < KEYBOARD_SCANCODE_LENGTH; ++i) {
        keyboardEvent.u.input.data[i]=elements[i];
    }
    write(uHidKeyboardFd, &keyboardEvent, sizeof(uhid_event));
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_UHid_UHidDemo_nativeCloseUHidKeyboard(JNIEnv *env, jclass clazz) {
    struct uhid_event ev = {0};
    ev.type = UHID_DESTROY;
    return write(uHidKeyboardFd, &ev, sizeof(uhid_event)) > 0;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_UHid_UHidDemo_nativeCreateUHidMouse(JNIEnv *env, jclass clazz) {
    if ((uHidMouseFd = open("/dev/uhid", O_RDWR | O_NDELAY)) < 0) {
        return false;//error process.
    }

    struct uhid_event ev = {0};
    ev.type = UHID_CREATE;
    strcpy((char *) ev.u.create.name, "uhid-Mouse");
    ev.u.create.rd_data = hidMouseDescription;
    ev.u.create.rd_size = sizeof(hidMouseDescription);
    ev.u.create.bus = BUS_VIRTUAL;
    ev.u.create.vendor = 0x1;
    ev.u.create.product = 0x1;
    ev.u.create.version = 0x1;
    ev.u.create.country = 0;
    if (write(uHidMouseFd, &ev, sizeof(ev)) != sizeof(uhid_event)) {
        return false;
    }

    memset(&mouseEvent, 0, sizeof(uhid_event));
    mouseEvent.type = UHID_INPUT;
    mouseEvent.u.input.size = MOUSE_SCANCODE_LENGTH;
    return true;
}


extern "C"
JNIEXPORT void JNICALL
Java_UHid_UHidDemo_nativeUHidMouseEvent(JNIEnv *env, jclass clazz, jbyteArray scan_code) {
    jbyte *elements = env->GetByteArrayElements(scan_code, NULL);
    for (int i = 0; i < MOUSE_SCANCODE_LENGTH; ++i) {
        mouseEvent.u.input.data[i]=elements[i];
    }
    write(uHidMouseFd, &mouseEvent, sizeof(uhid_event));
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_UHid_UHidDemo_nativeCloseUHidMouse(JNIEnv *env, jclass clazz) {
    struct uhid_event ev = {0};
    ev.type = UHID_DESTROY;
    return write(uHidMouseFd, &ev, sizeof(uhid_event)) > 0;
}