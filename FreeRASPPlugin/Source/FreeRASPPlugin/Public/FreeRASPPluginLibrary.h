#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

#include "FreeRASPPluginLibrary.generated.h"

/**
 * Delegate for handling threat detection callbacks from the FreeRASP library.
 * 
 * This delegate is called when the Android FreeRASP library detects a security threat.
 * It provides a message string describing the nature of the detected threat.
 * 
 * @param Message A string containing details about the detected security threat
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAndroidThreatDetectedCallback, const FString&, Message);

UCLASS()
class FREERASPPLUGIN_API UFreeRASPPluginLibrary : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    
    // Function to call Java method from Unreal
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
     * @example
     * // In Blueprint or C++:
     * // Create a custom event or function that takes a string parameter
     * // Then call SetOnAndroidThreatDetectedCallback and pass your function as the parameter
     */
    UFUNCTION(BlueprintCallable, Category = "FreeRASPPlugin")
    void SetOnAndroidThreatDetectedCallback(FOnAndroidThreatDetectedCallback Callback);
    
    // Static callback function that Java/JNI can call
    static void OnThreatDetected(const FString& Message);

private:
    // Static delegate instance
    static FOnAndroidThreatDetectedCallback OnAndroidThreatDetectedCallback;

private:
#if PLATFORM_ANDROID

    jobject ControllerInstance;

    // Helper function to get JNI environment
    static JNIEnv* GetJNIEnv();
    
    // Helper function to convert FString to jstring
    static jstring FStringToJString(JNIEnv* Env, const FString& String);
    
    // Helper function to convert jstring to FString
    static FString JStringToFString(JNIEnv* Env, jstring JavaString);
    
    // Helper function to convert TArray<FString> to jobjectArray
    static jobjectArray FStringArrayToJObjectArray(JNIEnv* Env, const TArray<FString>& StringArray);
#endif
};