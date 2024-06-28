#include <iostream>
#include <regex>
int main() {
    std::string line;
    std::regex pattern("^alias [a-zA-Z0-9_]+='[^']*'$"); // Regex pattern for matching alias commands

    std::cout << "Enter alias commands to check or 'q' to quit:" << std::endl;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);

        // Quit if the user enters 'q'
        if (line == "q") {
            break;
        }

        // Check if the line matches the regex pattern
        bool match = std::regex_match(line, pattern);

        // Print feedback
        if (match) {
            std::cout << "Valid alias command." << std::endl;
        } else {
            std::cout << "Invalid alias command." << std::endl;
        }
    }

    return 0;
}
