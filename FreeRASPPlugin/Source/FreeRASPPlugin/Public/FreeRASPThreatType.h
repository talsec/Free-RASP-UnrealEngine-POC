#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"

UENUM(BlueprintType)
enum ThreatType
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
    RuntimeManipulationDetected,
    PasscodeDetected,
    PasscodeChangeDetected, 
    JailbreakDetected,
    SignatureDetected, 
    MissingSecureEnclaveDetected, 
    DeviceIdDetected, 
    UofficialStoreDetected,
    DeviceChangeDetected, 
    Unknown,
};