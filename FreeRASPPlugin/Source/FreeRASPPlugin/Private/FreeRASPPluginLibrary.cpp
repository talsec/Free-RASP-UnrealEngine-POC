#include "FreeRASPPluginLibrary.h"
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Async/Async.h"  // for AsyncTask

#if PLATFORM_IOS
#import "FreeRASPPlugin-Swift.h"
#endif

#if PLATFORM_ANDROID
// JNI functions that Android Java code will call
extern "C" 
{
    JNIEXPORT void JNICALL Java_com_talsec_free_rasp_Controller_threatDetected(JNIEnv* env, jobject thiz, jstring message)
    {
        if (!message)
        {
            return;
        }
        const char* UTFString = env->GetStringUTFChars(message, nullptr);
        env->ReleaseStringUTFChars(message, UTFString);
        FString ThreatString(UTF8_TO_TCHAR(UTFString));
        // Call the static function to send the threat to UE
        UFreeRASPPluginLibrary::SendThreatToUE(ThreatString);
    }
}
#endif

#if PLATFORM_IOS
// C function that Swift code will call
extern "C" void NotifyUnrealSecurityThreat(const char* threatType)
{
    #if PLATFORM_IOS
        FString ThreatString(threatType);
        // Call the static function to send the threat to UE
        UFreeRASPPluginLibrary::SendThreatToUE(ThreatString);
    #endif
}
#endif

void UFreeRASPPluginLibrary::Initialize(FSubsystemCollectionBase& Collection)
{
    UE_LOG(LogTemp, Warning, TEXT("Initialize(FSubsystemCollectionBase& Collection) called"));   

    #if PLATFORM_ANDROID
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
    #endif
}

void UFreeRASPPluginLibrary::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Deinitialize() called"));

    #if PLATFORM_ANDROID
        // Clean up the instance
        JNIEnv* Env = FAndroidApplication::GetJavaEnv();
        if (!Env){
            return;
        }
        Env->DeleteLocalRef(ControllerInstance);
    #endif
}

void UFreeRASPPluginLibrary::InitializeTalsec(const TArray<FString>& appBundleIds, const FString& appTeamId,
    const FString& PackageName, const TArray<FString>& SigningCertificateBase64Hash, const TArray<FString>& SupportedAlternativeStores,  
    const FString& WatcherMailAddress, bool IsProd)
{
    #if PLATFORM_IOS
        // Convert C++ types to Objective-C types
        NSMutableArray<NSString*>* nsAppBundleIds = [[NSMutableArray alloc] init];
        for (const FString& bundleId : appBundleIds)
        {
            [nsAppBundleIds addObject:[NSString stringWithUTF8String:TCHAR_TO_UTF8(*bundleId)]];
        }
        
        NSString* nsAppTeamId = [NSString stringWithUTF8String:TCHAR_TO_UTF8(*appTeamId)];
        NSString* nsWatcherMailAddress = [NSString stringWithUTF8String:TCHAR_TO_UTF8(*WatcherMailAddress)];
        
        // Call the Swift method
        [FreeRASPSwiftHelper initTalsec:nsAppBundleIds :nsAppTeamId :nsWatcherMailAddress :IsProd];
    #endif

    #if PLATFORM_ANDROID
        // Initialize FreeRASP library with security configuration
        // This method sets up the Android FreeRASP library with the provided security parameters
        // including package name, certificate hashes, supported stores, and production mode flag
        
        // Get JNI environment for Java interop
        JNIEnv* Env = FAndroidApplication::GetJavaEnv();
        if (!Env)
        {
            return;
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
                    jstring WatcherEmailAddressJString = FStringToJString(Env, WatcherMailAddress);
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
                }
            }  
        }
    #endif
}

void UFreeRASPPluginLibrary::BroadcastSecurityThreat(const ThreatType& Threat)
{
    OnSecurityThreatDetected.Broadcast(Threat);
}

// static function that is called by Swift/Objective-C/Java code to send the threat to UE
void UFreeRASPPluginLibrary::SendThreatToUE(const FString& threatType) {
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* World = GEngine->GetWorldContexts()[0].World();
        if (World && World->GetGameInstance())
        {
            if (UFreeRASPPluginLibrary* Library = World->GetGameInstance()->GetSubsystem<UFreeRASPPluginLibrary>())
            {
                // Convert threat type string to the correct ThreatType enum
                UE_LOG(LogTemp, Warning, TEXT("Threat type: %s"), *threatType);
                ThreatType ThreatEnum = ThreatType::Unknown;
                if (threatType == TEXT("root"))
                    ThreatEnum = ThreatType::RootDetected;
                else if (threatType == TEXT("jailbreak"))
                    ThreatEnum = ThreatType::JailbreakDetected;
                else if (threatType == TEXT("signature"))
                    ThreatEnum = ThreatType::SignatureDetected;
                else if (threatType == TEXT("runtimeManipulation"))
                    ThreatEnum = ThreatType::RuntimeManipulationDetected;
                else if (threatType == TEXT("passcode"))
                    ThreatEnum = ThreatType::PasscodeDetected;
                else if (threatType == TEXT("passcodeChange"))
                    ThreatEnum = ThreatType::PasscodeChangeDetected;
                else if (threatType == TEXT("tamper"))
                    ThreatEnum = ThreatType::TamperDetected;
                else if (threatType == TEXT("debugger"))
                    ThreatEnum = ThreatType::DebuggerDetected;
                else if (threatType == TEXT("emulator"))
                    ThreatEnum = ThreatType::EmulatorDetected;
                else if (threatType == TEXT("simulator"))
                    ThreatEnum = ThreatType::EmulatorDetected;
                else if (threatType == TEXT("missingSecureEnclave"))
                    ThreatEnum = ThreatType::MissingSecureEnclaveDetected;
                else if (threatType == TEXT("deviceChange"))
                    ThreatEnum = ThreatType::DeviceChangeDetected;
                else if (threatType == TEXT("deviceID"))
                    ThreatEnum = ThreatType::DeviceIdDetected;
                else if (threatType == TEXT("unofficialStore"))
                    ThreatEnum = ThreatType::UofficialStoreDetected;
                else if (threatType == TEXT("untrustedInstallationSource"))
                    ThreatEnum = ThreatType::UntrustedInstallationSourceDetected;
                else if (threatType == TEXT("hook"))
                    ThreatEnum = ThreatType::HookDetected;
                else if (threatType == TEXT("deviceBinding"))
                    ThreatEnum = ThreatType::DeviceBindingDetected;
                else if (threatType == TEXT("obfuscationIssues"))
                    ThreatEnum = ThreatType::ObfuscationIssuesDetected;
                else if (threatType == TEXT("screenshot"))
                    ThreatEnum = ThreatType::ScreenshotDetected;
                else if (threatType == TEXT("screenRecording"))
                    ThreatEnum = ThreatType::ScreenRecordingDetected;
                else if (threatType == TEXT("unlockedDevice"))
                    ThreatEnum = ThreatType::UnlockedDeviceDetected;
                else if (threatType == TEXT("hardwareBackedKeystoreNotAvailable"))
                    ThreatEnum = ThreatType::HardwareBackedKeystoreNotAvailableDetected;
                else if (threatType == TEXT("developerMode"))
                    ThreatEnum = ThreatType::DeveloperModeDetected;
                else if (threatType == TEXT("adbEnabled"))
                    ThreatEnum = ThreatType::ADBEnabledDetected;
                else if (threatType == TEXT("systemVPN"))
                    ThreatEnum = ThreatType::SystemVPNDetected;
                else if (threatType == TEXT("deviceChange"))
                    ThreatEnum = ThreatType::DeviceChangeDetected;
                // Use AsyncTask to ensure this runs on game thread 
                // this is important as Talsec uses background threads to do threat detection
                AsyncTask(ENamedThreads::GameThread, [Library, ThreatEnum]()
                {
                    Library->BroadcastSecurityThreat(ThreatEnum);
                });
            }
        }
    }
}

#if PLATFORM_ANDROID
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

