Versioning
==========
The version for this plugin is kept inside `FreeRASPPlugin.uplugin` file. Make sure to update this before making a release. 

Build (iOS)
===========
Building the iOS plugin involves the following steps
1. Compile swift code (FreeRASPSwiftHelper.swift)
2. Create an archive of the compiled object in #1
3. Place the generated header file inside Plugins/FreeRASPPlugin/source/FreeRASPPlugin/public/

Compile using the command below

    xcrun -sdk iphoneos swiftc -target arm64-apple-ios15.0 \
    -parse-as-library \
    -emit-object \
    -emit-objc-header -emit-objc-header-path FreeRASPPlugin-Swift.h \
    -module-name FreeRASPPlugin \
    -sdk $(xcrun --show-sdk-path --sdk iphoneos) \
    -framework UIKit \
    -F <location_of_unreal_project>/Plugins/FreeRASPPlugin/Thirdparty/iOS/TalsecRuntime.xcframework/ios-arm64 \
    -o FreeRASPSwiftHelper.o \
    FreeRASPSwiftHelper.swift


Create archive using $ar

    ar rcs libFreeRASPSwiftHelper.a FreeRASPSwiftHelper.o

Copy libFreeRASPSwiftHelper.a to Plugins/FreeRASPPlugin/Thirdparty/iOS/ folder (delete the object file afterwards)

Copy FreeRASPPlugin-Swift.h to Plugins/FreeRASPPlugin/source/FreeRASPPlugin/public/
