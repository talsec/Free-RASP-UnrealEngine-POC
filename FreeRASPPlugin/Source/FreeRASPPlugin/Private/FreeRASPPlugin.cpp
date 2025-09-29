#include "FreeRASPPlugin.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FFreeRASPPluginModule"

DEFINE_LOG_CATEGORY(LogFreeRASPPlugin);

void FFreeRASPPluginModule::StartupModule()
{

}

void FFreeRASPPluginModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFreeRASPPluginModule, FreeRASPPlugin)