#ifndef MENU_H
#define MENU_H

#include "Object.h"

// Displays the main menu and handles user choices
void showMainMenu();

// Displays the test menu with options specific to test mode
void showTestMenu();

// Displays the test logs
void viewTestLogs();

// Runs the main scenario mode (declaration only for modular call in main menu)
void runScenarioMainMode(Object &follower, int speed, int iterations);

#endif // MENU_H
