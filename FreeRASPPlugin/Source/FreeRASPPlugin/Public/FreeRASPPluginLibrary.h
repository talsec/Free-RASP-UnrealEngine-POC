#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Modules/ModuleManager.h"
#include "FreeRASPThreatType.h"

#include "FreeRASPPlugin-Swift.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "Android/AndroidJava.h"

// unreal will define this for us
// we are going to use this to get the application context
// previousely we used to get it as follows
// FAndroidApplication::GetGameActivityThis()
// and then calling getApplicationContext() using jni apis
extern jobject GGameActivityThis;
#endif

#include "FreeRASPPluginLibrary.generated.h"

// Multicast delegate that can have multiple subscribers
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSecurityThreatDetected, ThreatType, Threat);


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
     * Initializes the Talsec FreeRASP security framework for mobile applications.
     * 
     * This method sets up the FreeRASP security monitoring system that detects various security threats
     * such as jailbreak/root detection, debugger attachment, emulator detection, tampering detection,
     * and other runtime security violations. The method adapts its behavior based on the target platform.
     * 
     * @param appBundleIds Array of application bundle identifiers that are allowed to run this app (iOS).
     *                     These are the bundle IDs of apps that are considered trusted.
     *                     For Android, this parameter is not used and can be empty.
     * @param appTeamId The Apple Developer Team ID associated with your app's provisioning profile (iOS).
     *                  This is used for signature verification and app authenticity checks.
     *                  For Android, this parameter is not used and can be empty.
     * @param PackageName The package name of your Android application (Android only).
     *                   This should match the package name in your AndroidManifest.xml.
     *                   For iOS, this parameter is not used and can be empty.
     * @param SigningCertificateBase64Hash Array of base64-encoded SHA-256 hashes of your app's signing certificates (Android).
     *                                     These are used to verify the authenticity of your app.
     *                                     For iOS, this parameter is not used and can be empty.
     * @param SupportedAlternativeStores Array of supported alternative app store package names (Android).
     *                                   This allows your app to run on alternative app stores while maintaining security.
     *                                   For iOS, this parameter is not used and can be empty.
     * @param watcherMailAddress Email address where security threat notifications will be sent.
     *                           This is used by Talsec for monitoring and alerting purposes.
     * @param isProd Boolean flag indicating whether the app is running in production mode (true)
     *               or development/debug mode (false). This affects the sensitivity of threat detection.
     * 
     * @note This method should be called early in the application lifecycle, typically during
     *       app initialization or in the GameInstance's Initialize method.
     * @note Platform-specific behavior:
     *       - iOS: Uses appBundleIds and appTeamId for signature verification
     *       - Android: Uses PackageName, SigningCertificateBase64Hash, and SupportedAlternativeStores
     *       - Other platforms: Method will have no effect
     * @note After initialization, security threats will be broadcast through the OnSecurityThreatDetected
     *       delegate, which can be bound to in Blueprint or C++ to handle detected threats.
     * 
     * @see OnSecurityThreatDetected
     * @see ThreatType
     */
    void InitializeTalsec(const TArray<FString>& appBundleIds, const FString& appTeamId, 
        const FString& PackageName, const TArray<FString>& SigningCertificateBase64Hash, const TArray<FString>& SupportedAlternativeStores, 
        const FString& watcherMailAddress, bool isProd);

    /**
     * Broadcasts a security threat event to the OnSecurityThreatDetected delegate.
     * 
     * This function is used to notify other parts of the application about detected security threats.
     * It should be called when a security threat is detected, and the delegate will be triggered
     * to notify any listeners.
     * 
     * @param Threat The type of security threat that was detected.
     * 
     * @see OnSecurityThreatDetected
     * @see ThreatType
     */
     UFUNCTION()
     void BroadcastSecurityThreat(const ThreatType& Threat);

    /**
     * The delegate that others can bind to
     * 
     * This delegate is used to notify other parts of the application about detected security threats.
     * It is triggered when a security threat is detected, and the delegate will be triggered
     * to notify any listeners.
     */
    UPROPERTY()
    FOnSecurityThreatDetected OnSecurityThreatDetected;

    /**
     * Sends a security threat notification from native code to Unreal Engine.
     * 
     * This method is called by the native FreeRASP library (Talsec) when a security threat is detected.
     * It converts the string-based threat type from the native library into the corresponding ThreatType enum
     * and broadcasts the threat through the OnSecurityThreatDetected delegate on the game thread.
     * 
     * 
     * @param threatType The string identifier of the detected threat type from the native FreeRASP library.
     *                   This parameter is provided by the Talsec native code and should match one of the
     *                   supported threat type strings listed above.
     * 
     * @note This method is designed to be called from native code (JNI on Android, native iOS code) and
     *       should not be called directly from Unreal Engine code. The native FreeRASP library automatically
     *       calls this method when threats are detected.
     * @note The method ensures thread safety by using AsyncTask to execute the threat broadcast on the
     *       game thread, as the native FreeRASP library operates on background threads.
     * @note If the threat type string is not recognized, the method will default to ThreatType::Unknown.
     * @note The method requires a valid game instance and world context to function properly.
     * 
     * @see OnSecurityThreatDetected
     * @see ThreatType
     * @see BroadcastSecurityThreat
     */
    UFUNCTION()
    static void SendThreatToUE(const FString& threatType);

private:
#if PLATFORM_ANDROID

    jobject ControllerInstance;
    
    // Helper function to convert FString to jstring
    static jstring FStringToJString(JNIEnv* Env, const FString& String);
    
    // Helper function to convert TArray<FString> to jobjectArray
    static jobjectArray FStringArrayToJObjectArray(JNIEnv* Env, const TArray<FString>& StringArray);
#endif
};