#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"

UENUM(BlueprintType)
enum ThreatType
{
    RootDetected,
    TamperDetected,
    DebuggerDetected, // both iOS and Android
    EmulatorDetected, // both iOS and Android
    UntrustedInstallationSourceDetected,
    HookDetected,
    DeviceBindingDetected, // both iOS and Android
    ObfuscationIssuesDetected,
    ScreenshotDetected,
    ScreenRecordingDetected,
    UnlockedDeviceDetected,
    HardwareBackedKeystoreNotAvailableDetected,
    DeveloperModeDetected,
    ADBEnabledDetected,
    SystemVPNDetected, // both iOS and Android
    RuntimeManipulationDetected, // iOS only
    PasscodeDetected, // iOS only
    PasscodeChangeDetected, // iOS only
    JailbreakDetected, // iOS only
    SignatureDetected, // iOS only
    MissingSecureEnclaveDetected, // iOS only
    DeviceIdDetected, // iOS only
    DeviceBindingChangeDetected, // iOS only
    UofficialStoreDetected, // iOS only
    Unknown,
};