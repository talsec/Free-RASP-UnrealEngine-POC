#include "FreeRASPPluginLibrary.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"

#if PLATFORM_ANDROID

FOnAndroidThreatDetectedCallback UFreeRASPPluginLibrary::OnAndroidThreatDetectedCallback;   

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
       // Convert string to enum and call the static callback function
       EThreatType ThreatType = UFreeRASPPluginLibrary::StringToThreatType(MessageString);
       UFreeRASPPluginLibrary::OnThreatDetected(ThreatType);
    }
}

void UFreeRASPPluginLibrary::SetOnAndroidThreatDetectedCallback(FOnAndroidThreatDetectedCallback Callback)
{
    // Store the provided callback in the static delegate instance
    // This callback will be executed when OnThreatDetected is called from JNI
    OnAndroidThreatDetectedCallback = Callback;
}

void UFreeRASPPluginLibrary::Initialize(FSubsystemCollectionBase& Collection)
{

    UE_LOG(LogTemp, Warning, TEXT("Initialize(FSubsystemCollectionBase& Collection) called"));

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
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
}

void UFreeRASPPluginLibrary::Deinitialize()
{
    // Clean up the instance
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    if (!Env){
        return;
    }
    Env->DeleteLocalRef(ControllerInstance);
}

bool UFreeRASPPluginLibrary::InitializeTalsec(const FString& PackageName, 
    const TArray<FString>& SigningCertificateBase64Hash, 
    const TArray<FString>& SupportedAlternativeStores, 
    const FString& WatcherEmailAddress, bool IsProd)
{
    // Initialize FreeRASP library with security configuration
    // This method sets up the Android FreeRASP library with the provided security parameters
    // including package name, certificate hashes, supported stores, and production mode flag
    
    // Get JNI environment for Java interop
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    if (!Env)
    {
        return false;
    }

    // Get the Android application context required by FreeRASP
    jobject ApplicationContext = nullptr;
    
    // Retrieve context from the current Android activity
    if (FAndroidApplication::GetGameActivityThis())
    {
        // Get the activity class to access its methods
        jclass ActivityClass = Env->GetObjectClass(FAndroidApplication::GetGameActivityThis());
        if (ActivityClass)
        {
            // Get the getApplicationContext method to obtain the application context
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
        // Find the FreeRASP Controller Java class
        jclass ControllerClass = FAndroidApplication::FindJavaClass("com/talsec/free/rasp/Controller");
        if (ControllerClass)
        {
            // Get the initializeTalsec method ID with the correct signature
            // Signature: (Context, String, String[], String[], String, boolean) -> void
            jmethodID InitializeTalsecMethod = Env->GetMethodID(ControllerClass, "initializeTalsec",
                 "(Landroid/content/Context;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;Z)V");
            if (InitializeTalsecMethod)
            {
                // Convert Unreal Engine parameters to JNI types for Java interop
                jstring PackageNameJString = FStringToJString(Env, PackageName);
                jstring WatcherEmailAddressJString = FStringToJString(Env, WatcherEmailAddress);
                jboolean IsProdJBoolean = IsProd ? JNI_TRUE : JNI_FALSE;
                
                // Convert string arrays to Java object arrays
                jobjectArray SigningCertArray = FStringArrayToJObjectArray(Env, SigningCertificateBase64Hash);
                jobjectArray SupportedStoresArray = FStringArrayToJObjectArray(Env, SupportedAlternativeStores);

                // Call the Java FreeRASP initialization method with all parameters
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
    return false;
}


// Static callback methods that JNI will call
void UFreeRASPPluginLibrary::OnThreatDetected(EThreatType ThreatType)
{
    if (OnAndroidThreatDetectedCallback.IsBound())
    {
        OnAndroidThreatDetectedCallback.Execute(ThreatType);
    }
}

// Helper function to convert string message to enum
EThreatType UFreeRASPPluginLibrary::StringToThreatType(FString Message)
{
    if (Message == TEXT("onRootDetected"))
        return EThreatType::RootDetected;
    else if (Message == TEXT("onTamperDetected"))
        return EThreatType::TamperDetected;
    else if (Message == TEXT("onDebuggerDetected"))
        return EThreatType::DebuggerDetected;
    else if (Message == TEXT("onEmulatorDetected"))
        return EThreatType::EmulatorDetected;
    else if (Message == TEXT("onUntrustedInstallationSourceDetected"))
        return EThreatType::UntrustedInstallationSourceDetected;
    else if (Message == TEXT("onHookDetected"))
        return EThreatType::HookDetected;
    else if (Message == TEXT("onDeviceBindingDetected"))
        return EThreatType::DeviceBindingDetected;
    else if (Message == TEXT("onObfuscationIssuesDetected"))
        return EThreatType::ObfuscationIssuesDetected;
    else if (Message == TEXT("onScreenshotDetected"))
        return EThreatType::ScreenshotDetected;
    else if (Message == TEXT("onScreenRecordingDetected"))
        return EThreatType::ScreenRecordingDetected;
    else if (Message == TEXT("onUnlockedDeviceDetected"))
        return EThreatType::UnlockedDeviceDetected;
    else if (Message == TEXT("onHardwareBackedKeystoreNotAvailableDetected"))
        return EThreatType::HardwareBackedKeystoreNotAvailableDetected;
    else if (Message == TEXT("onDeveloperModeDetected"))
        return EThreatType::DeveloperModeDetected;
    else if (Message == TEXT("onADBEnabledDetected"))
        return EThreatType::ADBEnabledDetected;
    else if (Message == TEXT("onSystemVPNDetected"))
        return EThreatType::SystemVPNDetected;
    else
        return EThreatType::Unknown; // Default fallback
}

// static helper method
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

jstring UFreeRASPPluginLibrary::FStringToJString(JNIEnv* Env, const FString& String)
{
    FTCHARToUTF8 Converter(*String);
    return Env->NewStringUTF(Converter.Get());
}

#endif