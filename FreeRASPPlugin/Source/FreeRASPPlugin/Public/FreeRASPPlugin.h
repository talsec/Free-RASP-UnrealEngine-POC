#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFreeRASPPlugin, Log, All);

class FFreeRASPPluginModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};