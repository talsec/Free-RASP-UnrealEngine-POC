// SimpleSwiftHelper.swift
import UIKit
import Foundation
import os.log

import TalsecRuntime

// Declare the external C function from Unreal Engine
@_silgen_name("NotifyUnrealSecurityThreat")
func NotifyUnrealSecurityThreat(_ threatType: UnsafePointer<CChar>)

@objc(FreeRASPSwiftHelper)
@objcMembers
public class FreeRASPSwiftHelper: NSObject {

    // Create a logger for better console output
    private static let logger = Logger(subsystem: "com.freerasp.plugin", category: "security")
    
    @objc public override init() {
        super.init()
    }

    @objc public static func initTalsec(_ appBundleIds: [String], _ appTeamId: String, _ watcherMailAddress: String, _ isProd: Bool) {
        let config = TalsecConfig(
            appBundleIds: appBundleIds,
            appTeamId: appTeamId,
            watcherMailAddress: watcherMailAddress,
            isProd: isProd
        )
        Talsec.start(config: config)
    }
}

extension SecurityThreatCenter: SecurityThreatHandler {
    public func threatDetected(_ securityThreat: TalsecRuntime.SecurityThreat) {
        var message = "unknown";
        // Handle each threat type individually
        switch securityThreat {
            case .signature:
                message = "onAppIntegrity"
            case .jailbreak:
                message = "onPrivilegedAccess"
            case .debugger:
                message = "onDebug"
            case .runtimeManipulation:
                message = "onHooks"
            case .passcode:
                message = "onPasscode"
            case .passcodeChange:
                message = "onPasscodeChange"
            case .simulator:
                message = "onSimulator"
            case .missingSecureEnclave:
                message = "onSecureHardwareNotAvailable"
            case .deviceChange:
                message = "onDeviceBinding"
            case .deviceID:
                message = "onDeviceID"
            case .unofficialStore:
                message = "onUnofficialStore"
            case .systemVPN:
                message = "onSystemVPN"
            case .screenshot:
                message = "onScreenshot"
            case .screenRecording:
                message = "onScreenRecording"
        }

        // Notify Unreal Engine
        message.withCString { cString in
            NotifyUnrealSecurityThreat(cString)
        }
    }
}
