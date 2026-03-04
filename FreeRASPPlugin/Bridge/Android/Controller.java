package com.talsec.free.rasp;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.telephony.TelephonyManager;
import android.widget.Toast;
import android.os.Handler;
import android.os.Looper;

import com.aheaditec.talsec_security.security.api.SuspiciousAppInfo;
import com.aheaditec.talsec_security.security.api.Talsec;
import com.aheaditec.talsec_security.security.api.TalsecConfig;
import com.aheaditec.talsec_security.security.api.TalsecMode;
import com.aheaditec.talsec_security.security.api.ThreatListener;

import java.util.List;

public class Controller implements ThreatListener.ThreatDetected, ThreatListener.DeviceState {
    
    private static final String TAG = Controller.class.getSimpleName();

    // Native method declaration - implemented in C++
    // this is used for threat detection
    private static native void threatDetected(String message);

    // Native method declaration - implemented in C++
    // this is used for notifying the native code that the RASP execution has finished
    private static native void raspExecutionFinished();
    
    public static class AppRaspExecutionState extends ThreatListener.RaspExecutionState {
        @Override
        public void onAllChecksFinished() {
            raspExecutionFinished();
        }
    }
    
    private boolean talSecInitialized;
    
    private AppRaspExecutionState appRaspExecutionState;

    public Controller() {
        talSecInitialized = false;
        appRaspExecutionState = new AppRaspExecutionState();
    }

    public void initializeTalsec(Context context, String packageName,
                                   String [] signingCertificateBase64Hash,
                                   String [] supportedAlternativeStores,
                                   String watcherEmailAddress, boolean isProd) {
        if(!talSecInitialized) {
            TalsecConfig config = new TalsecConfig.Builder(packageName,
                    signingCertificateBase64Hash)
                    .supportedAlternativeStores(supportedAlternativeStores)
                    .watcherMail(watcherEmailAddress)
                    .prod(isProd)
                    .build();
            ThreatListener threatListener = new ThreatListener(this, this, appRaspExecutionState);
            threatListener.registerListener(context);
            Talsec.start(context, config, TalsecMode.BACKGROUND);
            talSecInitialized = true;
        }
    }

    @Override
    public void onRootDetected() {
        threatDetected("onPrivilegedAccess");
    }

    @Override
    public void onTamperDetected() {
        threatDetected("onAppIntegrity");
    }

    @Override
    public void onMalwareDetected(List<SuspiciousAppInfo> list) {
        // not implemented yet
    }

    @Override
    public void onDebuggerDetected() {
        threatDetected("onDebug");
    }

    @Override
    public void onEmulatorDetected() {      
        threatDetected("onSimulator");
    }

    @Override
    public void onUntrustedInstallationSourceDetected() {
        threatDetected("onUnofficialStore");
    }

    @Override
    public void onHookDetected() {
        threatDetected("onHooks");
    }

    @Override
    public void onDeviceBindingDetected() {
        threatDetected("onDeviceBinding");
    }

    @Override
    public void onObfuscationIssuesDetected() {
        threatDetected("onObfuscationIssues");
    }

    @Override
    public void onScreenshotDetected() {
        threatDetected("onScreenshot");
    }

    @Override
    public void onScreenRecordingDetected() {
        threatDetected("onScreenRecording");
    }

    @Override
    public void onUnlockedDeviceDetected() {
        threatDetected("onPasscode");
    }

    @Override
    public void onHardwareBackedKeystoreNotAvailableDetected() {
        threatDetected("onSecureHardwareNotAvailable");
    }

    @Override
    public void onDeveloperModeDetected() {
        threatDetected("onDevMode");
    }

    @Override
    public void onADBEnabledDetected() {
        threatDetected("onADBEnabled");
    }

    @Override
    public void onSystemVPNDetected() {
        threatDetected("onSystemVPN");
    }
    
    @Override
    public void onMultiInstanceDetected() {
        threatDetected("onMultiInstance");
    }
    
    @Override
    public void onUnsecureWifiDetected() {
        threatDetected("onUnsecureWifi");
    }
    
    @Override
    public void onTimeSpoofingDetected() {
        threatDetected("onTimeSpoofing");
    }
    
    @Override
    public void onLocationSpoofingDetected() {
        threatDetected("onLocationSpoofing");
    }
}