#include <iostream>
#include <stdlib.h>

#include "jni.h"

using namespace std;

//https://www3.ntu.edu.sg/home/ehchua/programming/java/JavaNativeInterface.html

extern "C" JNIEXPORT jbyteArray JNICALL Java_com_seaboat_bytecode_ByteCodeEncryptor_encrypt(
	JNIEnv * jni_env,
	jobject  arg,
	jbyteArray _buf) {
	jbyte * jbuf = jni_env->GetByteArrayElements(_buf, 0);
	jsize length = jni_env->GetArrayLength(_buf);

	jbyte *dbuf = (jbyte *)malloc(length);
	int index = 0;
	for (; index < length - 1; ) {
		dbuf[index] = jbuf[index + 1] ^ 0x07;
		dbuf[index + 1] = jbuf[index] ^ 0x08;
		index += 2;
	}
	if ((0 == index && 1 == length) || length - 1 == index) {	// size 1 || size 2(index-1) + 1
		dbuf[index] = jbuf[index] ^ 0x09;
	}
	jbyteArray dst = jni_env->NewByteArray(length);
	jni_env->SetByteArrayRegion(dst, 0, length, dbuf);
	free(dbuf);
	return dst;
}