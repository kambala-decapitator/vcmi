#include <jni.h>

#include <concepts>

#if __has_include(<QJniEnvironment>)
# include <QJniEnvironment>
# define HAS_QJNI_ENVIRONMENT 1
#else
# include <QAndroidJniEnvironment>
#endif

namespace android
{
void initClassloader();

void withJniEnv(std::invocable<JNIEnv *> auto&& f)
{
#if HAS_QJNI_ENVIRONMENT
	f(QJniEnvironment::getJniEnv());
#else
    f(QAndroidJniEnvironment{});
	// QAndroidJniEnvironment jniEnv;
	// f(static_cast<JNIEnv *>(jniEnv));
#endif
}
}
