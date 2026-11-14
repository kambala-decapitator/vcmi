#pragma once

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
# include <QCoreApplication>
# include <QJniObject>
# include <QtCore/private/qandroidextras_p.h>

# define QJNI_OBJECT_CLASS QJniObject
# define QT_ACTIVITY QNativeInterface::QAndroidApplication::context()
# define ANDROID_SDK_VERSION QNativeInterface::QAndroidApplication::sdkVersion()
# define RUN_ON_ANDROID_MAIN_THREAD QNativeInterface::QAndroidApplication::runOnAndroidMainThread
# define START_ACTIVITY QtAndroidPrivate::startActivity
#else // < 6.0 actually
# include <QAndroidJniObject>
# include <QtAndroid>

# define QJNI_OBJECT_CLASS QAndroidJniObject
# define QT_ACTIVITY QtAndroid::androidActivity()
# define ANDROID_SDK_VERSION QtAndroid::androidSdkVersion()
# define RUN_ON_ANDROID_MAIN_THREAD QtAndroid::runOnAndroidThread
# define START_ACTIVITY QtAndroid::startActivity
#endif

// https://bugreports.qt.io/browse/QTBUG-140208
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
# define JNI_CALL_METHOD(obj, retType, method, signature, ...) obj.callMethod<retType>(method __VA_OPT__(,) __VA_ARGS__)
#else
# define JNI_CALL_METHOD(obj, retType, method, signature, ...) obj.callMethod<retType>(method, signature, __VA_ARGS__)
#endif
