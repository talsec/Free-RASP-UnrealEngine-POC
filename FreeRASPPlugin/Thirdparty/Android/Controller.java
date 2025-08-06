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
        threatDetected("root");
    }

    @Override
    public void onTamperDetected() {
        threatDetected("tamper");
    }

    @Override
    public void onMalwareDetected(List<SuspiciousAppInfo> list) {
        // not implemented yet
    }

    @Override
    public void onDebuggerDetected() {
        threatDetected("debugger");
    }

    @Override
    public void onEmulatorDetected() {      
        threatDetected("emulator");
    }

    @Override
    public void onUntrustedInstallationSourceDetected() {
        threatDetected("untrustedInstallationSource");
    }

    @Override
    public void onHookDetected() {
        threatDetected("hook");
    }

    @Override
    public void onDeviceBindingDetected() {
        threatDetected("deviceBinding");
    }

    @Override
    public void onObfuscationIssuesDetected() {
        threatDetected("obfuscationIssues");
    }

    @Override
    public void onScreenshotDetected() {
        threatDetected("screenshot");
    }

    @Override
    public void onScreenRecordingDetected() {
        threatDetected("screenRecording");
    }

    @Override
    public void onUnlockedDeviceDetected() {
        threatDetected("unlockedDevice");
    }

    @Override
    public void onHardwareBackedKeystoreNotAvailableDetected() {
        threatDetected("hardwareBackedKeystoreNotAvailable");
    }

    @Override
    public void onDeveloperModeDetected() {
        threatDetected("developerMode");
    }

    @Override
    public void onADBEnabledDetected() {
        threatDetected("adbEnabled");
    }

    @Override
    public void onSystemVPNDetected() {
        threatDetected("systemVPN");
    }       
}