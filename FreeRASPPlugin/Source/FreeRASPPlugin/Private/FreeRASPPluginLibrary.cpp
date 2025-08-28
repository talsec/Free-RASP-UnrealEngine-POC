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
                // Static map for threat type conversion - initialized once
                static const TMap<FString, ThreatType> ThreatTypeMap = {
                    {TEXT("onPrivilegedAccess"), ThreatType::OnPrivilegedAccess},
                    {TEXT("onAppIntegrity"), ThreatType::OnAppIntegrity},
                    {TEXT("onDebug"), ThreatType::OnDebug}, // iOS & Android
                    {TEXT("onHooks"), ThreatType::OnHookDetected}, // runtime manipulation (ios) + Android
                    {TEXT("onSimulator"), ThreatType::OnSimulator}, // iOS & Android
                    {TEXT("onUnofficialStore"), ThreatType::OnUnofficialStore}, // iOS & Android
                    {TEXT("onDeviceBinding"), ThreatType::OnDeviceBinding}, // device change (ios)
                    {TEXT("onDeviceID"), ThreatType::OnDeviceID}, // device id (ios)
                    {TEXT("onObfuscationIssues"), ThreatType::OnObfuscationIssues},
                    {TEXT("onScreenshot"), ThreatType::OnScreenshot},
                    {TEXT("onScreenRecording"), ThreatType::OnScreenRecording},
                    {TEXT("onPasscode"), ThreatType::OnPasscode}, // onPasscodeChange is removed as its no longer used
                    {TEXT("onSecureHardwareNotAvailable,"), ThreatType::OnSecureHardwareNotAvailable},
                    {TEXT("onDevMode"), ThreatType::OnDevMode},
                    {TEXT("onADBEnabled"), ThreatType::OnADBEnabled},
                    {TEXT("onSystemVPN"), ThreatType::OnSystemVPN},
                };

                ThreatType ThreatEnum = ThreatType::Unknown;
                if (const ThreatType* FoundThreat = ThreatTypeMap.Find(threatType))
                {
                    ThreatEnum = *FoundThreat;
                }
                
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

