# FreeRASP Plugin for Unreal Engine

A comprehensive security plugin for Unreal Engine that integrates Talsec's FreeRASP (Runtime Application Self-Protection) solution to protect your mobile applications against various security threats.

## Overview

FreeRASP is a runtime application self-protection solution that helps detect and prevent:
- **Root/Jailbreak Detection**: Identifies if the device is rooted (Android) or jailbroken (iOS)
- **Emulator Detection**: Detects if the app is running in an emulator or simulator
- **Debugger Detection**: Identifies if the app is being debugged
- **Tampering Detection**: Detects if the app has been modified or tampered with
- **Screen Recording Detection**: Identifies if the screen is being recorded
- **VPN Detection**: Detects if a VPN is being used

## Features

- **Cross-Platform Support**: Works on both iOS and Android
- **Easy Integration**: Simple plugin-based integration with Unreal Engine
- **Real-time Protection**: Continuous monitoring during app runtime
- **Configurable**: Customizable detection settings and callbacks
- **Performance Optimized**: Minimal impact on app performance

## Supported Platforms

- **iOS**: iOS 15.0+ (arm64)
- **Android**: API level 21+ (arm64)

## Installation

### Prerequisites

- Unreal Engine 5.x
- Xcode 14+ (for iOS development)
- Android Studio (for Android development)
- iOS 15.0+ SDK
- Android API level 21+

### Setup Instructions

1. **Copy the Plugin**: Place the `FreeRASPPlugin` folder in your project's `Plugins/` directory
2. **Enable the Plugin**: The plugin is enabled by default, but you can verify in the Unreal Editor under Edit > Plugins > Security
3. **Rebuild the Project**: Close and reopen your project to ensure the plugin is properly loaded

## Configuration

### Android Configuration

The plugin automatically configures the following for Android:

- **Dependencies**: Adds TalsecSecurity-Community library (version 15.1.0)
- **Permissions**: Automatically adds required network state permission
- **Build Configuration**: Integrates with Gradle build system

### iOS Configuration

The plugin automatically configures the following for iOS:

- **Frameworks**: Links TalsecRuntime.xcframework
- **System Frameworks**: Add Foundation framework
- **Build Settings**: Configures proper linking and embedding

## Usage

### Basic Integration

1. **Include the Plugin**: The plugin is automatically included when enabled
2. **Initialize FreeRASP**: Call the initialization function in your game's startup code
3. **Handle Callbacks**: Implement callback functions to respond to security events

### Example Implementation

```cpp
// In your game's initialization code
#include "FreeRASPPluginLibrary.h"
#include "FreeRASPThreatType.h"

void AYourGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the FreeRASP plugin library instance
    UFreeRASPPluginLibrary* FreeRASPLib = GetGameInstance()->GetSubsystem<UFreeRASPPluginLibrary>();
    
    if (FreeRASPLib)
    {
        // Bind to security threat events
        FreeRASPLib->OnSecurityThreatDetected.AddDynamic(this, &AYourGameMode::HandleSecurityThreat);
        
        // Initialize FreeRASP with your configuration
        TArray<FString> AppBundleIds; // iOS: Add your app bundle IDs
        FString AppTeamId = TEXT(""); // iOS: Add your Apple Developer Team ID
        FString PackageName = TEXT(""); // Android: Add your package name
        TArray<FString> SigningCertificates; // Android: Add your signing certificate hashes
        TArray<FString> AlternativeStores; // Android: Add alternative store package names
        FString WatcherEmail = TEXT("your-email@example.com"); // Add your email for notifications
        bool IsProduction = false; // Set to true for production builds
        
        FreeRASPLib->InitializeTalsec(AppBundleIds, AppTeamId, PackageName, 
                                     SigningCertificates, AlternativeStores, 
                                     WatcherEmail, IsProduction);
    }
}

void AYourGameMode::HandleSecurityThreat(ThreatType Threat)
{
    switch (Threat)
    {
        case ThreatType::RootDetected:
            UE_LOG(LogTemp, Warning, TEXT("Root detected!"));
            // Handle root detection
            break;
            
        case ThreatType::JailbreakDetected:
            UE_LOG(LogTemp, Warning, TEXT("Jailbreak detected!"));
            // Handle jailbreak detection
            break;
            
        case ThreatType::EmulatorDetected:
            UE_LOG(LogTemp, Warning, TEXT("Emulator detected!"));
            // Handle emulator detection
            break;
            
        case ThreatType::DebuggerDetected:
            UE_LOG(LogTemp, Warning, TEXT("Debugger detected!"));
            // Handle debugger detection
            break;
            
        case ThreatType::TamperDetected:
            UE_LOG(LogTemp, Warning, TEXT("App tampering detected!"));
            // Handle tampering detection
            break;
            
        case ThreatType::ScreenRecordingDetected:
            UE_LOG(LogTemp, Warning, TEXT("Screen recording detected!"));
            // Handle screen recording detection
            break;
            
        case ThreatType::SystemVPNDetected:
            UE_LOG(LogTemp, Warning, TEXT("VPN detected!"));
            // Handle VPN detection
            break;
            
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown security threat detected!"));
            break;
    }
}
```