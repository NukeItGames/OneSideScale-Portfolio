#pragma once

#include "Templates/SharedPointer.h"

class FSlateStyleSet;
class UToolMenu;

class FOneSideScaleToolbar
{
public:
    void Register();
    void Unregister();

private:
    void RegisterMenus();
    void BuildTransformMenu(UToolMenu* Menu);
    TSharedPtr<FSlateStyleSet> Style;
};
