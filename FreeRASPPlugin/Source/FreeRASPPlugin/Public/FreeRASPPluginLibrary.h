#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

#include "FreeRASPPluginLibrary.generated.h"

UENUM(BlueprintType)
enum class EThreatType : uint8
{
    RootDetected,
    TamperDetected,
    DebuggerDetected,
    EmulatorDetected,
    UntrustedInstallationSourceDetected,
    HookDetected,
    DeviceBindingDetected,
    ObfuscationIssuesDetected,
    ScreenshotDetected,
    ScreenRecordingDetected,
    UnlockedDeviceDetected,
    HardwareBackedKeystoreNotAvailableDetected,
    DeveloperModeDetected,
    ADBEnabledDetected,
    SystemVPNDetected,
    Unknown,
};

/**
 * Delegate for handling threat detection callbacks from the FreeRASP library.
 * 
 * This delegate is called when the Android FreeRASP library detects a security threat.
 * It provides an enum value describing the nature of the detected threat.
 * 
 * @param ThreatType An enum value indicating the type of security threat detected
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAndroidThreatDetectedCallback, EThreatType, ThreatType);

// Forward declaration for JNI function
#if PLATFORM_ANDROID
extern "C" JNIEXPORT void JNICALL Java_com_talsec_free_rasp_Controller_threatDetected(JNIEnv* env, jobject thiz, jstring message);
#endif

UCLASS()
class FREERASPPLUGIN_API UFreeRASPPluginLibrary : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    
    /**
     * Initializes the FreeRASP library with security configuration parameters.
     * 
     * This method initializes the Android FreeRASP library
     * with the specified security parameters. It sets up threat detection capabilities including
     * root detection, emulator detection, debugger detection, and other security checks.
     * 
     * @param PackageName The package name of your Android application (e.g., "com.yourcompany.yourapp")
     * @param SigningCertificateBase64Hash Array of base64-encoded SHA-256 hashes of your app's signing certificates.
     *                                     This is used to verify the app's authenticity and detect tampering.
     * @param SupportedAlternativeStores Array of supported alternative app store names (e.g., "Google Play Store").
     *                                   This helps FreeRASP distinguish between legitimate and unauthorized app sources.
     * @param WatcherEmailAddress Email address where security alerts and threat notifications will be sent. 
     * @param IsProd Boolean flag indicating whether the app is running in production mode (true) or development mode (false).
     *               In development mode, some security checks may be relaxed for debugging purposes.
     * 
     * @return true if initialization was successful, false otherwise.
     * 
     * @note This method is only functional on Android platforms. On other platforms, it will return false.
     * 
     * @note The method requires a valid Android application context and JNI environment to function properly.
     *       Make sure the Android application is properly initialized before calling this method.
     * 
     * @note This method should be called after the FreeRASPPluginLibrary subsystem has been initialized
     *       (typically in your GameInstance or early in the application lifecycle).
     * 
     * @warning The signing certificate hashes must match exactly with your app's actual signing certificates.
     *          Incorrect hashes will cause the security checks to fail and may trigger false threat detections.
     * 
     */
    UFUNCTION(BlueprintCallable, Category = "FreeRASPPlugin")
    bool InitializeTalsec(const FString& PackageName, 
        const TArray<FString>& SigningCertificateBase64Hash, 
        const TArray<FString>& SupportedAlternativeStores, 
        const FString& WatcherEmailAddress, bool IsProd);

    /**
     * Sets the callback function that will be executed when a threat is detected by the FreeRASP library.
     * 
     * This method allows you to register a callback function that will be called whenever the Android
     * FreeRASP library detects a security threat. The callback receives a message string describing
     * the detected threat.
     * 
     * @param Callback The delegate function to be called when a threat is detected. The function should
     *                 accept a single FString parameter containing the threat message.
     * 
     * @note This method is only functional on Android platforms. On other platforms, the callback
     *       will be set but never triggered since FreeRASP only operates on Android.
     * 
     * @note The callback is stored as a static delegate, so only one callback can be active at a time.
     *       Calling this method multiple times will replace the previous callback.
     * 
     */
    UFUNCTION(BlueprintCallable, Category = "FreeRASPPlugin")
    void SetOnAndroidThreatDetectedCallback(FOnAndroidThreatDetectedCallback Callback);
    
    // Static callback function that Java/JNI can call
    static void OnThreatDetected(EThreatType ThreatType);

    // Helper function to convert string to enum
    static EThreatType StringToThreatType(FString Message);

private:
    // Static delegate instance
    static FOnAndroidThreatDetectedCallback OnAndroidThreatDetectedCallback;

private:
#if PLATFORM_ANDROID

    jobject ControllerInstance;
    
    // Helper function to convert FString to jstring
    static jstring FStringToJString(JNIEnv* Env, const FString& String);
    
    // Helper function to convert TArray<FString> to jobjectArray
    static jobjectArray FStringArrayToJObjectArray(JNIEnv* Env, const TArray<FString>& StringArray);
#endif
};