#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Modules/ModuleManager.h"
#include "FreeRASPThreatType.h"

#include "FreeRASPPlugin-Swift.h"

#include "FreeRASPPluginLibrary.generated.h"

// Multicast delegate that can have multiple subscribers
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSecurityThreatDetected, ThreatType, Threat);


UCLASS()
class FREERASPPLUGIN_API UFreeRASPPluginLibrary : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Initializes the Talsec FreeRASP security framework for iOS applications.
     * 
     * This method sets up the FreeRASP security monitoring system that detects various security threats
     * such as jailbreak, debugger attachment, emulator detection, and other runtime security violations.
     * The method is only functional on iOS platforms and will be a no-op on other platforms.
     * 
     * @param appBundleIds Array of application bundle identifiers that are allowed to run this app.
     *                     These are the bundle IDs of apps that are considered trusted.
     * @param appTeamId The Apple Developer Team ID associated with your app's provisioning profile.
     *                  This is used for signature verification and app authenticity checks.
     * @param watcherMailAddress Email address where security threat notifications will be sent.
     *                           This is used by Talsec for monitoring and alerting purposes.
     * @param isProd Boolean flag indicating whether the app is running in production mode (true)
     *               or development/debug mode (false). This affects the sensitivity of threat detection.
     * 
     * @note This method should be called early in the application lifecycle, typically during
     *       app initialization or in the GameInstance's Initialize method.
     * @note Only available on iOS platforms. On other platforms, this method will have no effect.
     * @note After initialization, security threats will be broadcast through the OnSecurityThreatDetected
     *       delegate, which can be bound to in Blueprint or C++ to handle detected threats.
     * 
     * @see OnSecurityThreatDetected
     * @see ThreatType
     */
    void InitializeTalsec(const TArray<FString>& appBundleIds, const FString& appTeamId, const FString& watcherMailAddress, bool isProd);
    
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

};