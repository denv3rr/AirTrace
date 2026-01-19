#include "ui/inputValidation.h"

#include <iostream>
#include <limits>
#include <sstream>

// Function to validate integer input within a range set as parameters 2 and 3
int getValidatedIntInput(const std::string &prompt, int min, int max)
{
    int input = min;
    if (!tryGetValidatedIntInput(prompt, min, max, input))
    {
        return min;
    }
    return input;
}

bool tryGetValidatedIntInput(const std::string &prompt, int min, int max, int &out)
{
    std::string line;
    while (true)
    {
        std::cout << prompt;
        if (!std::getline(std::cin, line))
        {
            return false;
        }

        std::stringstream ss(line);
        int input = 0;
        char trailing = '\0';
        if (!(ss >> input) || (ss >> trailing) || input < min || input > max)
        {
            std::cout << "Invalid input. Enter a number between " << min << " and " << max << ".\n";
            continue;
        }
        out = input;
        return true;
    }
}

void clearInputStream()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool inputStreamAvailable()
{
    return std::cin.good() && !std::cin.eof();
}
