package com.talsec.free.rasp;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.telephony.TelephonyManager;
import android.widget.Toast;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.aheaditec.talsec_security.security.api.SuspiciousAppInfo;
import com.aheaditec.talsec_security.security.api.Talsec;
import com.aheaditec.talsec_security.security.api.TalsecConfig;
import com.aheaditec.talsec_security.security.api.ThreatListener;

import java.util.List;

public class Controller implements ThreatListener.ThreatDetected, ThreatListener.DeviceState {
    
    private static final String TAG = Controller.class.getSimpleName();

    // Native method declaration - implemented in C++
    public static native void threatDetected(String message);
    
    private boolean talSecInitialized;
    
    public Controller() {
        talSecInitialized = false;
    }

    public void initializeTalsec(Context context, String packageName,
                                   String [] signingCertificateBase64Hash,
                                   String [] supportedAlternativeStores,
                                   String watcherEmailAddress, boolean isProd) {
        Log.d(TAG, "initializeTalsec called");
        if(!talSecInitialized) {
            TalsecConfig config = new TalsecConfig.Builder(packageName,
                    signingCertificateBase64Hash)
                    .supportedAlternativeStores(supportedAlternativeStores)
                    .watcherMail(watcherEmailAddress)
                    .prod(isProd)
                    .build();
            ThreatListener threatListener = new ThreatListener(this, this);
            threatListener.registerListener(context);
            Talsec.start(context, config);
            talSecInitialized = true;
            Log.d(TAG, "freeRASP initialized");
        }
    }

    @Override
    public void onRootDetected() {
        Log.d(TAG, "Root detected");
        threatDetected("onRootDetected");
    }

    @Override
    public void onTamperDetected() {
        Log.d(TAG, "Tamper detected");
        threatDetected("onTamperDetected");
    }

    @Override
    public void onMalwareDetected(List<SuspiciousAppInfo> list) {
        // not implemented yet
    }

    @Override
    public void onDebuggerDetected() {
        Log.d(TAG, "Debugger detected");
        threatDetected("onDebuggerDetected");
    }

    @Override
    public void onEmulatorDetected() {
        Log.d(TAG, "Emulator detected");
        threatDetected("onEmulatorDetected");
    }

    @Override
    public void onUntrustedInstallationSourceDetected() {
        Log.d(TAG, "Untrusted installation source detected");
        threatDetected("onUntrustedInstallationSourceDetected");
    }

    @Override
    public void onHookDetected() {
        Log.d(TAG, "Hook detected");
        threatDetected("onHookDetected");
    }

    @Override
    public void onDeviceBindingDetected() {
        Log.d(TAG, "Device binding detected");
        threatDetected("onDeviceBindingDetected");
    }

    @Override
    public void onObfuscationIssuesDetected() {
        Log.d(TAG, "Obfuscation issues detected");
        threatDetected("onObfuscationIssuesDetected");
    }

    @Override
    public void onScreenshotDetected() {
        Log.d(TAG, "Screenshot detected");
        threatDetected("onScreenshotDetected");
    }

    @Override
    public void onScreenRecordingDetected() {
        threatDetected("onScreenRecordingDetected");
    }

    @Override
    public void onUnlockedDeviceDetected() {
        Log.d(TAG, "Unlocked device detected");
        threatDetected("onUnlockedDeviceDetected");
    }

    @Override
    public void onHardwareBackedKeystoreNotAvailableDetected() {
        Log.d(TAG, "Hardware backed keystore not available detected");
        threatDetected("onHardwareBackedKeystoreNotAvailableDetected");
    }

    @Override
    public void onDeveloperModeDetected() {
        Log.d(TAG, "Developer mode detected");
        threatDetected("onDeveloperModeDetected");
    }

    @Override
    public void onADBEnabledDetected() {
        Log.d(TAG, "ADB enabled detected");
        threatDetected("onADBEnabledDetected");
    }

    @Override
    public void onSystemVPNDetected() {
        Log.d(TAG, "System VPN detected");
        threatDetected("onSystemVPNDetected");
    }       
}