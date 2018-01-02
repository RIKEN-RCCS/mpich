/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef LOGFORMAT_TRACE_INPUTLOG_H_INCLUDED
#define LOGFORMAT_TRACE_INPUTLOG_H_INCLUDED

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class logformat_trace_InputLog */

#ifndef _Included_logformat_trace_InputLog
#define _Included_logformat_trace_InputLog
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     logformat_trace_InputLog
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_logformat_trace_InputLog_initIDs
  (JNIEnv *, jclass);

/*
 * Class:     logformat_trace_InputLog
 * Method:    open
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_logformat_trace_InputLog_open
  (JNIEnv *, jobject);

/*
 * Class:     logformat_trace_InputLog
 * Method:    close
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_logformat_trace_InputLog_close
  (JNIEnv *, jobject);

/*
 * Class:     logformat_trace_InputLog
 * Method:    peekNextKindIndex
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_logformat_trace_InputLog_peekNextKindIndex
  (JNIEnv *, jobject);

/*
 * Class:     logformat_trace_InputLog
 * Method:    getNextCategory
 * Signature: ()Lbase/drawable/Category;
 */
JNIEXPORT jobject JNICALL Java_logformat_trace_InputLog_getNextCategory
  (JNIEnv *, jobject);

/*
 * Class:     logformat_trace_InputLog
 * Method:    getNextYCoordMap
 * Signature: ()Lbase/drawable/YCoordMap;
 */
JNIEXPORT jobject JNICALL Java_logformat_trace_InputLog_getNextYCoordMap
  (JNIEnv *, jobject);

/*
 * Class:     logformat_trace_InputLog
 * Method:    getNextPrimitive
 * Signature: ()Lbase/drawable/Primitive;
 */
JNIEXPORT jobject JNICALL Java_logformat_trace_InputLog_getNextPrimitive
  (JNIEnv *, jobject);

/*
 * Class:     logformat_trace_InputLog
 * Method:    getNextComposite
 * Signature: ()Lbase/drawable/Composite;
 */
JNIEXPORT jobject JNICALL Java_logformat_trace_InputLog_getNextComposite
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif

#endif /* LOGFORMAT_TRACE_INPUTLOG_H_INCLUDED */
