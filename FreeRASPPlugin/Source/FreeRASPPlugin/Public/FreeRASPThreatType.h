#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"

UENUM(BlueprintType)
enum ThreatType
{
    OnPrivilegedAccess,
    OnAppIntegrity,
    OnDebug, 
    OnSimulator, 
    OnUnofficialStore,
    OnHookDetected,
    OnDeviceID,
    OnDeviceBinding,
    OnObfuscationIssues,
    OnScreenshot,
    OnScreenRecording,
    OnPasscode,
    OnPasscodeChange,
    OnSecureHardwareNotAvailable,
    OnDevMode,
    OnADBEnabled,
    OnSystemVPN, 
    // UnlockedDeviceDetected,

    // DeviceIdDetected, 
    // DeviceChangeDetected, 
    Unknown,
};