#include "FreeRASPPluginLibrary.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"

FOnAndroidThreatDetectedCallback UFreeRASPPluginLibrary::OnAndroidThreatDetectedCallback;   

void UFreeRASPPluginLibrary::SetOnAndroidThreatDetectedCallback(FOnAndroidThreatDetectedCallback Callback)
{
    // Store the provided callback in the static delegate instance
    // This callback will be executed when OnThreatDetected is called from JNI
    OnAndroidThreatDetectedCallback = Callback;
}

void UFreeRASPPluginLibrary::Initialize(FSubsystemCollectionBase& Collection)
{
#if PLATFORM_ANDROID

    UE_LOG(LogTemp, Warning, TEXT("Initialize(FSubsystemCollectionBase& Collection) called"));

    JNIEnv* Env = GetJNIEnv();
    if (!Env)
    {
        return;
    }

    jclass ControllerClass = FAndroidApplication::FindJavaClass("com/talsec/free/rasp/Controller");
	// Get the constructor
    jmethodID Constructor = Env->GetMethodID(ControllerClass, "<init>", "()V");
    if (Constructor)
    {
        // Create an instance of the controller class
        ControllerInstance = Env->NewObject(ControllerClass, Constructor);
        UE_LOG(LogTemp, Warning, TEXT("ControllerInstance created"));
    }
#endif  // PLATFORM_ANDROID
}

void UFreeRASPPluginLibrary::Deinitialize()
{
#if PLATFORM_ANDROID
    // Clean up the instance
    JNIEnv* Env = GetJNIEnv();
    if (!Env)
    {
        return;
    }
    Env->DeleteLocalRef(ControllerInstance);
#endif  // SUPPORTED_PLATFORM
}

bool UFreeRASPPluginLibrary::InitializeTalsec(const FString& PackageName, 
    const TArray<FString>& SigningCertificateBase64Hash, 
    const TArray<FString>& SupportedAlternativeStores, 
    const FString& WatcherEmailAddress, bool IsProd)
{
#if PLATFORM_ANDROID
    JNIEnv* Env = GetJNIEnv();
    if (!Env)
    {
        return false;
    }

    // Get the application context
    jobject ApplicationContext = nullptr;
    
    // Get context from Android application
    if (FAndroidApplication::GetGameActivityThis())
    {
        // Get the activity class
        jclass ActivityClass = Env->GetObjectClass(FAndroidApplication::GetGameActivityThis());
        if (ActivityClass)
        {
            // Get the getApplicationContext method
            jmethodID GetContextMethod = Env->GetMethodID(ActivityClass, "getApplicationContext", "()Landroid/content/Context;");
            if (GetContextMethod)
            {
                ApplicationContext = Env->CallObjectMethod(FAndroidApplication::GetGameActivityThis(), GetContextMethod);
                UE_LOG(LogTemp, Warning, TEXT("ApplicationContext created"));
            }
            Env->DeleteLocalRef(ActivityClass);
        }
    }

    if (ApplicationContext)
    {
        // Get the activity class
        jclass ControllerClass = FAndroidApplication::FindJavaClass("com/talsec/free/rasp/Controller");
        if (ControllerClass)
        {
            // Get the method ID
            jmethodID InitializeTalsecMethod = Env->GetMethodID(ControllerClass, "initializeTalsec",
                 "(Landroid/content/Context;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;Z)V");
            if (InitializeTalsecMethod)
            {
                UE_LOG(LogTemp, Warning, TEXT("InitializeTalsecMethod found! Proceeding to call it"));
                // Convert all parameters to JNI types
                jstring PackageNameJString = FStringToJString(Env, PackageName);
                jstring WatcherEmailAddressJString = FStringToJString(Env, WatcherEmailAddress);
                jboolean IsProdJBoolean = IsProd ? JNI_TRUE : JNI_FALSE;
                // Convert arrays to jobjectArray
                jobjectArray SigningCertArray = FStringArrayToJObjectArray(Env, SigningCertificateBase64Hash);
                jobjectArray SupportedStoresArray = FStringArrayToJObjectArray(Env, SupportedAlternativeStores);

                // Call the Java method
                Env->CallVoidMethod(ControllerInstance, InitializeTalsecMethod, ApplicationContext, PackageNameJString, SigningCertArray, SupportedStoresArray, WatcherEmailAddressJString, IsProdJBoolean);

                // Clean up local references
                Env->DeleteLocalRef(PackageNameJString);
                Env->DeleteLocalRef(WatcherEmailAddressJString);
                Env->DeleteLocalRef(SigningCertArray);
                Env->DeleteLocalRef(SupportedStoresArray);
                Env->DeleteLocalRef(ControllerClass);

                return true;
            }
        } 
    }
#endif
    return false;
}


// Static callback methods that JNI will call
void UFreeRASPPluginLibrary::OnThreatDetected(const FString& Message)
{
    if (OnAndroidThreatDetectedCallback.IsBound())
    {
        OnAndroidThreatDetectedCallback.Execute(Message);
    }
}

// static helper method
#if PLATFORM_ANDROID
jobjectArray UFreeRASPPluginLibrary::FStringArrayToJObjectArray(JNIEnv* Env, const TArray<FString>& StringArray)
{
    if (!Env)
    {
        return nullptr;
    }

    // Get the String class
    jclass StringClass = Env->FindClass("java/lang/String");
    if (!StringClass)
    {
        return nullptr;
    }

    // Create a new object array
    jobjectArray Result = Env->NewObjectArray(StringArray.Num(), StringClass, nullptr);
    if (!Result)
    {
        Env->DeleteLocalRef(StringClass);
        return nullptr;
    }

    // Fill the array with converted strings
    for (int32 i = 0; i < StringArray.Num(); ++i)
    {
        jstring JString = FStringToJString(Env, StringArray[i]);
        Env->SetObjectArrayElement(Result, i, JString);
        Env->DeleteLocalRef(JString);
    }

    // Clean up
    Env->DeleteLocalRef(StringClass);
    return Result;
}
#endif


#if PLATFORM_ANDROID
JNIEnv* UFreeRASPPluginLibrary::GetJNIEnv()
{
    return FAndroidApplication::GetJavaEnv();
}

jstring UFreeRASPPluginLibrary::FStringToJString(JNIEnv* Env, const FString& String)
{
    FTCHARToUTF8 Converter(*String);
    return Env->NewStringUTF(Converter.Get());
}

FString UFreeRASPPluginLibrary::JStringToFString(JNIEnv* Env, jstring JavaString)
{
    if (!JavaString)
    {
        return FString();
    }

    const char* UTFString = Env->GetStringUTFChars(JavaString, nullptr);
    FString Result = FString(UTF8_TO_TCHAR(UTFString));
    Env->ReleaseStringUTFChars(JavaString, UTFString);
    return Result;
}

// JNI functions that Java will call
extern "C" 
{
    JNIEXPORT void JNICALL Java_com_talsec_free_rasp_Controller_threatDetected(JNIEnv* env, jobject thiz, jstring message)
    {
        if (!message)
        {
            return;
        }
        const char* UTFString = env->GetStringUTFChars(message, nullptr);
        FString MessageString = FString(UTF8_TO_TCHAR(UTFString));
        env->ReleaseStringUTFChars(message, UTFString);

        // Call the static callback function
        UFreeRASPPluginLibrary::OnThreatDetected(MessageString);
    }
}
#endif