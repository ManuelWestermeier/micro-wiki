#pragma once

#include <vector>
#include <string>
#include <cstdint>

#include "state/button.hpp"
#include "state/splash.hpp"
#include "state/menu.hpp"
#include "state/list_browser.hpp"
#include "state/article_browser.hpp"
#include "state/wifi_portal.hpp"
#include "state/shutdown.hpp"
#include "state/loop.hpp"

extern bool isMenuOpen;
extern std::vector<std::string> currentPath;
extern uint8_t menuState;
