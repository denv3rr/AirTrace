#ifndef MENU_H
#define MENU_H

#include "core/Object.h"

// Displays the main menu and handles user choices
bool showMainMenu();

// Displays the test menu with options specific to test mode
bool showTestMenu();

// Displays the test logs
void viewTestLogs();

// Runs the main scenario mode (declaration only for modular call in main menu)
bool runScenarioMainMode(Object &follower, int gpsTimeoutSeconds, int heatTimeoutSeconds);

#endif // MENU_H

