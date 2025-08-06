#include "FreeRASPPlugin.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FFreeRASPPluginModule"

DEFINE_LOG_CATEGORY(LogFreeRASPPlugin);

void FFreeRASPPluginModule::StartupModule()
{
    UE_LOG(LogFreeRASPPlugin, Warning, TEXT("FreeRASPPlugin module has been loaded"));
}

void FFreeRASPPluginModule::ShutdownModule()
{
    UE_LOG(LogFreeRASPPlugin, Warning, TEXT("FreeRASPPlugin module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFreeRASPPluginModule, FreeRASPPlugin)