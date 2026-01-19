#ifndef INPUTVALIDATION_H
#define INPUTVALIDATION_H

#include <string>
#include <iostream>
#include <limits>

int getValidatedIntInput(const std::string &prompt, int min, int max);
bool tryGetValidatedIntInput(const std::string &prompt, int min, int max, int &out);
void clearInputStream();
bool inputStreamAvailable();

#endif // INPUTVALIDATION_H
