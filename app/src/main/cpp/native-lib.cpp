#include <jni.h>
#include <string>
#include "Log.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_jnidemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

const char *APP_PACKAGE_NAME = "com.example.jnidemo";
// 验证是否通过
static jboolean auth = JNI_FALSE;

static jclass contextClass;
static jclass signatureClass;
static jclass packageNameClass;
static jclass packageInfoClass;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_jnidemo_MainActivity_test1(JNIEnv *env, jobject thiz) {
    
    //获取jclass对象
    jclass pJclass = env->GetObjectClass(thiz);
    //获取java 方法
    jmethodID idcode = env->GetMethodID(pJclass, "callBack", "(I)V");

    jmethodID id = env->GetMethodID(pJclass, "callBack", "([B)V");

    jmethodID pId = env->GetMethodID(pJclass, "callBack",
                                     "(Ljava/lang/String;I)Ljava/lang/String;");
    //反射调用方法
//    env->CallVoidMethod(thiz,idcode,200);
    jstring pJstring = env->NewStringUTF("200");
    env->CallObjectMethod(thiz,pId,pJstring,300);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_jnidemo_MainActivity_test2(JNIEnv *env, jobject thiz) {

    jclass pJclass = env->GetObjectClass(thiz);
    //    方法签名 String 对象1    基本类型
    jfieldID textStringid = env->GetFieldID(pJclass, "text", "Ljava/lang/String;");
    //创建C字符串
    jstring pJstring = env->NewStringUTF("native 层修改 代码");
    //执行修改
    env->SetObjectField(thiz,textStringid,pJstring);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_jnidemo_MainActivity_nativeAdd(JNIEnv *env, jobject thiz, jint x, jint y) {
    return x+y;
}


/*
 * 获取全局 Application
 */
jobject getApplicationContext(JNIEnv *env) {
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
    return env->CallObjectMethod(at, getApplication);
}

/**
 * 通过反射获取签名信息
 */
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_jnidemo_MainActivity_init(JNIEnv *env, jclass clazz) {

    jclass binderClass = env->FindClass("android/os/Binder");
    jclass contextClass = env->FindClass("android/content/Context");
    jclass signatureClass = env->FindClass("android/content/pm/Signature");
    jclass packageNameClass = env->FindClass("android/content/pm/PackageManager");
    jclass packageInfoClass = env->FindClass("android/content/pm/PackageInfo");

    jmethodID packageManager = env->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jmethodID packageName = env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
    jmethodID toCharsString = env->GetMethodID(signatureClass, "toCharsString", "()Ljava/lang/String;");
    jmethodID packageInfo = env->GetMethodID(packageNameClass, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jmethodID nameForUid = env->GetMethodID(packageNameClass, "getNameForUid", "(I)Ljava/lang/String;");
    jmethodID callingUid = env->GetStaticMethodID(binderClass, "getCallingUid", "()I");

    jint uid = env->CallStaticIntMethod(binderClass, callingUid);

    // 获取全局 Application
    jobject context = getApplicationContext(env);

    jobject packageManagerObject = env->CallObjectMethod(context, packageManager);
    jstring packNameString = (jstring) env->CallObjectMethod(context, packageName);
    jobject packageInfoObject = env->CallObjectMethod(packageManagerObject, packageInfo, packNameString, 64);
    jfieldID signaturefieldID = env->GetFieldID(packageInfoClass, "signatures", "[Landroid/content/pm/Signature;");
    jobjectArray signatureArray = (jobjectArray) env->GetObjectField(packageInfoObject, signaturefieldID);
    jobject signatureObject = env->GetObjectArrayElement(signatureArray, 0);
    jstring runningPackageName = (jstring) env->CallObjectMethod(packageManagerObject, nameForUid, uid);

    if (runningPackageName) {// 正在运行应用的包名
        const char *charPackageName = env->GetStringUTFChars(runningPackageName, 0);
        if (strcmp(charPackageName, APP_PACKAGE_NAME) != 0) {
            return JNI_FALSE;
        }
        env->ReleaseStringUTFChars(runningPackageName, charPackageName);
    } else {
        return JNI_FALSE;
    }

    jstring signatureStr = (jstring) env->CallObjectMethod(signatureObject, toCharsString);
    const char *signature = env->GetStringUTFChars(
            (jstring) env->CallObjectMethod(signatureObject, toCharsString), NULL);

    env->DeleteLocalRef(binderClass);
    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(signatureClass);
    env->DeleteLocalRef(packageNameClass);
    env->DeleteLocalRef(packageInfoClass);

    LOGE("current apk signature %s", signature);

    // 应用签名，通过 JNIDecryptKey.getSignature(getApplicationContext())
    //TODO  获取，注意开发版和发布版的区别，发布版需要使用正式签名打包后获取
    const char *SIGNATURE_KEY = "308202e4308201cc020101300d06092a864886f70d01010b050030373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b30090603550406130255533020170d3233303831303035323435315a180f32303533303830323035323435315a30373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b300906035504061302555330820122300d06092a864886f70d01010105000382010f003082010a0282010100df62a81709d1582d3f1b65207fa4f948c396c074ce487a736965071529992018b5b200b638a8b56fc328dd2fd96ba80918069b633b4cb6705fba561f64304c8888e3006feed03c3b3fffa4aa10cd6e9f4aa143ceaf3656bec317696692d6d850e7d8f420ccdd5f225b60625c9b31fb165ca4bdf8965163aa209b3ab90cfd2294a2af442fddb1af17e110ad43f17b302793f5434d9ed09a6bca1c97148af17d6dc8256e6d701aad2d89f388106140536cb5aa54ab37a34a3908e1d5da188be5c55416f357387ca8ec01e31b35a361fcb5266301071b9f30aa358baef67908a8415b4b8dea19932918002a3c156985e715bfa1adf21d110e4f07a2647265cbc3570203010001300d06092a864886f70d01010b0500038201010038046ec241527148e128eaeb0d14303732fd37f27819f30432590a2df3f4cc618e883846db71f854f6de1b13b7ce6512e2642e6cbf1fb24fa902a90671d891729eb79db2ef2f54aa301b45cb279b3247efaee093780d8363ecba9af60ab50f825b6428f915f9ed4914cc09bb78b11e8446bfe44ea641f1708d53e682c69346d00375a51b0e494408f3d41a54e50b97fb05fc57293edac1c0f5d5cb6f2a6176495609782bda3aab99bd21db7ef62b2ff993cc59d1d7a5d9553ca49aa11768eee6c9b5c28661e7790c817c53365fd6aec5034edefeb8deda672dd137fd61c78b991dce8f27a9056d3946359c55287251b5890bdeec384cc47daf9fa32cef2a4d12";
    if (strcmp(signature, SIGNATURE_KEY) == 0) {
        LOGE("verification passed");
        env->ReleaseStringUTFChars(signatureStr, signature);
        auth = JNI_TRUE;
        return JNI_TRUE;
    } else {
        LOGE("verification failed");
        auth = JNI_FALSE;
        return JNI_FALSE;
    }
    return auth;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_jnidemo_MainActivity_getKey(JNIEnv *env, jclass clazz) {
    const char  * APP_KEY = "qweqw12312asdasdasda231231asdasdasd";
    if (auth) {
        return env->NewStringUTF(APP_KEY);
    } else {// 你没有权限，验证没有通过。
        return env->NewStringUTF("You don't have permission, the verification didn't pass.");
    }
}

/*
    根据context对象,获取签名字符串
*/
const char* getSignString(JNIEnv *env,jobject contextObject) {
    jmethodID getPackageManagerId = (env)->GetMethodID(contextClass, "getPackageManager","()Landroid/content/pm/PackageManager;");
    jmethodID getPackageNameId = (env)->GetMethodID(contextClass, "getPackageName","()Ljava/lang/String;");
    jmethodID signToStringId = (env)->GetMethodID(signatureClass, "toCharsString","()Ljava/lang/String;");
    jmethodID getPackageInfoId = (env)->GetMethodID(packageNameClass, "getPackageInfo","(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jobject packageManagerObject =  (env)->CallObjectMethod(contextObject, getPackageManagerId);
    jstring packNameString =  (jstring)(env)->CallObjectMethod(contextObject, getPackageNameId);
    jobject packageInfoObject = (env)->CallObjectMethod(packageManagerObject, getPackageInfoId,packNameString, 64);
    jfieldID signaturefieldID =(env)->GetFieldID(packageInfoClass,"signatures", "[Landroid/content/pm/Signature;");
    jobjectArray signatureArray = (jobjectArray)(env)->GetObjectField(packageInfoObject, signaturefieldID);
    jobject signatureObject =  (env)->GetObjectArrayElement(signatureArray,0);
    return (env)->GetStringUTFChars((jstring)(env)->CallObjectMethod(signatureObject, signToStringId),0);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_jnidemo_MainActivity_nativeMethod_1key(JNIEnv *env, jobject thiz,
                                                        jobject context) {
    const char *RELEASE_SIGN = "308202e4308201cc020101300d06092a864886f70d01010b050030373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b30090603550406130255533020170d3233303831303035323435315a180f32303533303830323035323435315a30373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b300906035504061302555330820122300d06092a864886f70d01010105000382010f003082010a0282010100df62a81709d1582d3f1b65207fa4f948c396c074ce487a736965071529992018b5b200b638a8b56fc328dd2fd96ba80918069b633b4cb6705fba561f64304c8888e3006feed03c3b3fffa4aa10cd6e9f4aa143ceaf3656bec317696692d6d850e7d8f420ccdd5f225b60625c9b31fb165ca4bdf8965163aa209b3ab90cfd2294a2af442fddb1af17e110ad43f17b302793f5434d9ed09a6bca1c97148af17d6dc8256e6d701aad2d89f388106140536cb5aa54ab37a34a3908e1d5da188be5c55416f357387ca8ec01e31b35a361fcb5266301071b9f30aa358baef67908a8415b4b8dea19932918002a3c156985e715bfa1adf21d110e4f07a2647265cbc3570203010001300d06092a864886f70d01010b0500038201010038046ec241527148e128eaeb0d14303732fd37f27819f30432590a2df3f4cc618e883846db71f854f6de1b13b7ce6512e2642e6cbf1fb24fa902a90671d891729eb79db2ef2f54aa301b45cb279b3247efaee093780d8363ecba9af60ab50f825b6428f915f9ed4914cc09bb78b11e8446bfe44ea641f1708d53e682c69346d00375a51b0e494408f3d41a54e50b97fb05fc57293edac1c0f5d5cb6f2a6176495609782bda3aab99bd21db7ef62b2ff993cc59d1d7a5d9553ca49aa11768eee6c9b5c28661e7790c817c53365fd6aec5034edefeb8deda672dd137fd61c78b991dce8f27a9056d3946359c55287251b5890bdeec384cc47daf9fa32cef2a4d12";
    const char* signStrng =  getSignString(env,context);
    if(strcmp(signStrng,RELEASE_SIGN)==0)//签名一致  返回合法的 api key，否则返回错误
    {
        return (env)->NewStringUTF("dongtaizhucemiyao123123123123");
    }else
    {
        return (env)->NewStringUTF("error");
    }
}

//动态  效率高
//    反编译  安全性高
void regist(JNIEnv *env, jobject thiz, jobject jCallback) {

    LOGD("--动态注册调用成功-->");
    jstring pJstring = env->NewStringUTF("动态注册调用成功");
    jclass pJclass = env->GetObjectClass(thiz);
    jmethodID id = env->GetMethodID(pJclass, "beInjectedDebug", "(Ljava/lang/String;)V");
    //执行函数
    env->CallVoidMethod(thiz,id,pJstring);
}

jint RegisterNatives(JNIEnv *env) {
    //动态反射MainActivity
    jclass activityClass = env->FindClass("com/example/jnidemo/MainActivity");
    if (activityClass == NULL) {
        return JNI_ERR;
    }
    //注册MainActivity中setAntiBiBCallback方法
    JNINativeMethod method_MainActivity [] = {
        "setAntiBiBCallback",
        "(Lcom/example/jnidemo/IAntiDebugCallback;)V",
        (void *)regist
    };

    //注册  参数1 页面实体类
    //参数2 注册的方法
    //参数3 注册方法的数量
    return env->RegisterNatives(activityClass,method_MainActivity, sizeof(method_MainActivity)/sizeof(method_MainActivity[0]));
}

/**
 * 动态注册
 * @param vm
 * @param reserved
 * @return
 */
JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM* vm,void* reserved){
//    手机app   手机开机了
    JNIEnv *env = NULL;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    //初始化函数
    contextClass = (jclass)env->NewGlobalRef((env)->FindClass("android/content/Context"));
    signatureClass = (jclass)env->NewGlobalRef((env)->FindClass("android/content/pm/Signature"));
    packageNameClass = (jclass)env->NewGlobalRef((env)->FindClass("android/content/pm/PackageManager"));
    packageInfoClass = (jclass)env->NewGlobalRef((env)->FindClass("android/content/pm/PackageInfo"));

    jint result = RegisterNatives(env);
    //如果注册失败，打印日志
    if (result != JNI_OK) {
        LOGD("--动态注册失败-->");
        return -1;
    }

    return JNI_VERSION_1_6;

}
