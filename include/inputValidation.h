#ifndef INPUTVALIDATION_H
#define INPUTVALIDATION_H

#include <string>
#include <iostream>
#include <limits>

int getValidatedIntInput(const std::string &prompt, int min, int max);
void clearInputStream();

#endif // INPUTVALIDATION_H