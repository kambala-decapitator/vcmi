#include "androidutils.h"
#include "../../lib/CAndroidVMHelper.h"

namespace android
{
void initClassloader()
{
    withJniEnv(&CAndroidVMHelper::initClassloader);
    /*withJniEnv([](JNIEnv * jniEnv)
    {
	    CAndroidVMHelper::initClassloader(jniEnv);
    });*/
}
}
