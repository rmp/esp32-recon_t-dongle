// ============================================================================
//  AboutScreen.h  -  Firmware identity, one-button legend, and the authorized-
//  use reminder. Built on ScrollScreen (static content).
// ============================================================================
#pragma once

#include "ui/ScrollScreen.h"

class AboutScreen : public ScrollScreen {
public:
    AboutScreen() : ScrollScreen("About") {}
    void onEnter() override;
};
