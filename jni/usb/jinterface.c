 /*
 *
 * Copyright (C) 2014 Shachar Gritzman <gritzman@outlook.com>
 * 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include <jni.h>
#include <string.h>
#include <android/log.h>
#include "libusb.h"
#define LOG_TAG "USBHelp" // text for log tag

#define TIMEOUT_VIN		-1
#define TIMEOUT_COUT	-1
#define ENDPOINT_BULK_IN 0x61
#define ENDPOINT_BULK_OUT 0x12
#define USB_CHUNC_SIZE 512

libusb_context * ctx = NULL;
libusb_device_handle *dev_handle = NULL;

JNIEnv * callback_env;
jobject callback_obj;
jmethodID mid;
jclass cls;
struct libusb_transfer *transfer_in;

int do_exit = 1;
int grab_en = 1;

jint length;

void cb_in()
{
	 (*callback_env)->CallVoidMethod(callback_env, callback_obj, mid, 0);
	do_exit = 1;
}

jint
Java_nok_pack_Device_DataRequest(JNIEnv * env, jobject this,
		jbyteArray buffer_1, jbyteArray buffer_2, jint length_loc)
{
	unsigned char* bufferPtr_1 = (unsigned char* )((*env)->GetByteArrayElements(env, buffer_1,    NULL));
	unsigned char* bufferPtr_2 = (unsigned char* )((*env)->GetByteArrayElements(env, buffer_2,    NULL));
	unsigned char* tmp;


	length = length_loc;
	transfer_in =  libusb_alloc_transfer(0);

	callback_env = env;
	callback_obj = this;

	cls = (*callback_env)->GetObjectClass(callback_env, callback_obj);
	mid = (*callback_env)->GetMethodID(callback_env, cls, "RecCallback", "(I)V");

	grab_en = 1;
	do_exit = 0;

	while(grab_en)
	{

		libusb_fill_bulk_transfer( transfer_in, dev_handle, ENDPOINT_BULK_IN,
				bufferPtr_1,  length,  // Note: in_buffer is where input data written.
				cb_in, NULL, // no user data
				TIMEOUT_VIN); // wait untill data available

		libusb_submit_transfer(transfer_in);

		while ( !do_exit )
		{
			libusb_handle_events_completed(ctx, NULL);
		}

		tmp = bufferPtr_1;
		bufferPtr_1 = bufferPtr_2;
		bufferPtr_2 = tmp;

		do_exit = 0;
	}

	if(NULL != dev_handle) libusb_close(dev_handle);
	if(NULL != ctx) libusb_exit(ctx);

	(*env)->ReleaseByteArrayElements(env,buffer_1,  bufferPtr_1,  JNI_ABORT);
	(*env)->ReleaseByteArrayElements(env,buffer_2,  bufferPtr_2,  JNI_ABORT);
}

jint
Java_nok_pack_Device_SendData(JNIEnv * env, jobject this,
		jbyteArray cnfg, jint length)
{
	int actual_length = 0;

	jbyte* cnfgPtr = (*env)->GetByteArrayElements(env, cnfg, NULL);

	int ret = libusb_bulk_transfer(dev_handle, ENDPOINT_BULK_OUT, cnfgPtr, length, &actual_length, TIMEOUT_COUT);

	(*env)->ReleaseByteArrayElements(env,cnfg,cnfgPtr,JNI_ABORT);

    return actual_length;
}

jstring
Java_nok_pack_initUSB( JNIEnv* env, jobject thiz )
{
  int r;
  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "entering iniUSB");
  r = libusb_init(NULL);
  if(r < 0) {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "failed to initialize libusb");
    return  (*env)->NewStringUTF(env, "Failed to initialize libusb");
  } else {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "successfully initialized libusb");
    return (*env)->NewStringUTF(env, "libusb successfully enabled");
  }
}

jint
Java_nok_pack_USBcheckDevice( JNIEnv* env, jobject this, jint vid, jint pid )
{
	 int i=0,ret = 0;

	 dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);

	 if(NULL == dev_handle) return -1;

	 // get interface
	 ret = libusb_claim_interface(dev_handle, 0);

  return ret;
}
