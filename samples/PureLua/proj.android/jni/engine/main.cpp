#include <jni.h>
#include <android/log.h>
#include "LuaEngine.h"

#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

extern "C"
{
    
jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    return JNI_VERSION_1_4;
}

JNIEXPORT jstring JNICALL Java_org_cocos2dx_purelua_PureLua_startLuaEngine(JNIEnv * env,
		jobject obj) {
	LOGD("start Lua engine here!\n");
    LuaEngine* pEngine = LuaEngine::defaultEngine();
    pEngine->executeScriptFile("main.lua");
}
}
