#pragma once

#include "Engine/DeveloperSettings.h"
#include "OneSideScaleSettings.generated.h"

/** Per-user editor preferences; no settings are read by a packaged game. */
UCLASS(Config=EditorPerProjectUserSettings, meta=(DisplayName="OneSideScale"))
class UOneSideScaleSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    virtual FName GetContainerName() const override { return TEXT("Editor"); }
    virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

    UPROPERTY(EditAnywhere, Config, Category="Gizmo", meta=(DisplayName="Handle Radius (Pixels)", ClampMin="4.0", ClampMax="20.0"))
    double HandleHalfSizePixels = 9.0;
};
