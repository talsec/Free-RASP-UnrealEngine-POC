#include "FreeRASPPlugin.h"

#define LOCTEXT_NAMESPACE "FFreeRASPPluginModule"

void FFreeRASPPluginModule::StartupModule()
{
    // This code will execute after your module is loaded into memory
}

void FFreeRASPPluginModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFreeRASPPluginModule, FreeRASPPlugin)