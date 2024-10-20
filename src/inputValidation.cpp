#include "inputValidation.h"
#include <iostream>
#include <limits>

// Function to validate integer input within a range set as parameters 2 and 3
int getValidatedIntInput(const std::string &prompt, int min, int max)
{
    int input;

    while (true)
    {
        std::cout << prompt;
        std::cin >> input;

        // Check for invalid input or out of range
        if (std::cin.fail() || input < min || input > max)
        {
            std::cout << "\033[31mInvalid input. Please enter a number between " << min << " and " << max << ".\033[0m\n\n\n";
            clearInputStream(); // Clear the invalid input
        }
        else
        {
            return input; // Valid input
        }
    }
}

// Function to clear the input stream in case of invalid input
void clearInputStream()
{
    std::cin.clear();                                                   // Clear the error flag
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
}