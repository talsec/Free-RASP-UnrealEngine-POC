#include "FreeRASPPluginLibrary.h"
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Async/Async.h"  // for AsyncTask

#if PLATFORM_IOS
#import "FreeRASPPlugin-Swift.h"
#endif

/**
 * Global C function that serves as a bridge between Swift/Objective-C and Unreal Engine C++.
 * 
 * This function is called by the Talsec FreeRASP framework when a security threat is detected
 * on iOS devices. It acts as an entry point that allows the native iOS security monitoring
 * code to communicate detected threats back to the Unreal Engine application.
 * 
 * The function performs the following operations:
 * 1. Validates that the Unreal Engine is running and has an active world context
 * 2. Retrieves the FreeRASP plugin library subsystem from the game instance
 * 3. Converts the string-based threat type to the corresponding ThreatType enum value
 * 4. Schedules the threat broadcast on the game thread using AsyncTask to ensure thread safety
 * 
 * 
 * @param threatType A null-terminated C string containing the threat type identifier.
 *                   This string is provided by the Talsec framework and should match
 *                   one of the supported threat type strings listed above.
 *                   If the string doesn't match any known threat type, ThreatType::Unknown
 *                   will be used.
 * 
 * @note This function is only active on iOS platforms due to the PLATFORM_IOS preprocessor directive.
 *       On other platforms, this function will be a no-op.
 * @note The function uses AsyncTask to ensure the threat broadcast occurs on the game thread,
 *       which is necessary because Talsec performs threat detection on background threads.
 * @note This function is marked as extern "C" to ensure proper linkage with Swift/Objective-C code.
 * 
 * @see UFreeRASPPluginLibrary::BroadcastSecurityThreat
 * @see ThreatType
 * @see FOnSecurityThreatDetected
 */
extern "C" void NotifyUnrealSecurityThreat(const char* threatType)
{
    #if PLATFORM_IOS
        if (GEngine && GEngine->GetWorldContexts().Num() > 0)
        {
            UWorld* World = GEngine->GetWorldContexts()[0].World();
            if (World && World->GetGameInstance())
            {
                if (UFreeRASPPluginLibrary* Library = World->GetGameInstance()->GetSubsystem<UFreeRASPPluginLibrary>())
                {
                    FString ThreatString(threatType);
                    // Convert threat type string to the correct ThreatType enum
                    ThreatType ThreatEnum = ThreatType::Unknown;
                    
                    if (ThreatString.Equals("debugger", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::DebuggerDetected;
                    }
                    else if (ThreatString.Equals("jailbreak", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::JailbreakDetected;
                    }
                    else if (ThreatString.Equals("signature", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::SignatureDetected;
                    }
                    else if (ThreatString.Equals("runtimeManipulation", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::RuntimeManipulationDetected;
                    }
                    else if (ThreatString.Equals("passcode", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::PasscodeDetected;
                    }
                    else if (ThreatString.Equals("passcodeChange", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::PasscodeChangeDetected;
                    }
                    else if (ThreatString.Equals("simulator", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::EmulatorDetected;
                    }
                    else if (ThreatString.Equals("missingSecureEnclave", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::MissingSecureEnclaveDetected;
                    }
                    else if (ThreatString.Equals("deviceChange", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::DeviceBindingDetected;
                    }
                    else if (ThreatString.Equals("deviceID", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::DeviceIdDetected;
                    }
                    else if (ThreatString.Equals("unofficialStore", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::UofficialStoreDetected;
                    }
                    else if (ThreatString.Equals("systemVPN", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::SystemVPNDetected;
                    }
                    else if (ThreatString.Equals("screenshot", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::ScreenshotDetected;
                    }
                    else if (ThreatString.Equals("screenRecording", ESearchCase::IgnoreCase))
                    {
                        ThreatEnum = ThreatType::ScreenRecordingDetected;
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
    #endif
}

void UFreeRASPPluginLibrary::Initialize(FSubsystemCollectionBase& Collection)
{
    UE_LOG(LogTemp, Warning, TEXT("Initialize(FSubsystemCollectionBase& Collection) called"));   
}

void UFreeRASPPluginLibrary::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Deinitialize() called"));
}

/**
 * Initializes the Talsec FreeRASP security framework for iOS applications.
 * 
 * This method sets up the FreeRASP security monitoring system that detects various security threats
 * such as jailbreak, debugger attachment, emulator detection, and other runtime security violations.
 * The method is only functional on iOS platforms and will be a no-op on other platforms.
 * 
 * The initialization process involves:
 * 1. Converting Unreal Engine C++ string types to Objective-C NSString types
 * 2. Creating an NSMutableArray for the bundle IDs
 * 3. Calling the Swift helper method to initialize the Talsec framework
 * 
 * @param appBundleIds Array of application bundle identifiers that are allowed to run this app.
 *                     These are the bundle IDs of apps that are considered trusted.
 *                     For example: ["com.yourcompany.yourapp", "com.trustedpartner.app"]
 * @param appTeamId The Apple Developer Team ID associated with your app's provisioning profile.
 *                  This is used for signature verification and app authenticity checks.
 *                  Format: "XXXXXXXXXX" (10-character alphanumeric string)
 * @param watcherMailAddress Email address where security threat notifications will be sent.
 *                           This is used by Talsec for monitoring and alerting purposes.
 * @param isProd Boolean flag indicating whether the app is running in production mode (true)
 *               or development/debug mode (false).
 * 
 * @note This method should be called early in the application lifecycle, typically during
 *       app initialization or in the GameInstance's Initialize method.
 * @note Only available on iOS platforms. On other platforms, this method will have no effect
 *       due to the PLATFORM_IOS preprocessor directive.
 * @note After initialization, security threats will be broadcast through the OnSecurityThreatDetected
 *       delegate, which can be bound to in Blueprint or C++ to handle detected threats.
 * @note The method performs automatic string conversion from Unreal Engine's FString to
 *       Objective-C's NSString for proper integration with the iOS Talsec framework.
 * 
 * @see OnSecurityThreatDetected
 * @see ThreatType
 * @see NotifyUnrealSecurityThreat
 * 
 * @example
 * // Example usage in GameInstance::Initialize()
 * if (UFreeRASPPluginLibrary* FreeRASP = GetSubsystem<UFreeRASPPluginLibrary>())
 * {
 *     TArray<FString> BundleIds = {"com.yourcompany.yourapp"};
 *     FreeRASP->InitializeTalsec(BundleIds, "TEAM123456", "security@yourcompany.com", true);
 * }
 */
void UFreeRASPPluginLibrary::InitializeTalsec(const TArray<FString>& appBundleIds, const FString& appTeamId, const FString& watcherMailAddress, bool isProd)
{
    #if PLATFORM_IOS
        // Convert C++ types to Objective-C types
        NSMutableArray<NSString*>* nsAppBundleIds = [[NSMutableArray alloc] init];
        for (const FString& bundleId : appBundleIds)
        {
            [nsAppBundleIds addObject:[NSString stringWithUTF8String:TCHAR_TO_UTF8(*bundleId)]];
        }
        
        NSString* nsAppTeamId = [NSString stringWithUTF8String:TCHAR_TO_UTF8(*appTeamId)];
        NSString* nsWatcherMailAddress = [NSString stringWithUTF8String:TCHAR_TO_UTF8(*watcherMailAddress)];
        
        // Call the Swift method
        [FreeRASPSwiftHelper initTalsec:nsAppBundleIds :nsAppTeamId :nsWatcherMailAddress :isProd];
    #endif
}

void UFreeRASPPluginLibrary::BroadcastSecurityThreat(const ThreatType& Threat)
{
    OnSecurityThreatDetected.Broadcast(Threat);
}


