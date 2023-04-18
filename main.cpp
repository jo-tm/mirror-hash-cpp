#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include "mirror256.h"

std::string randomAlphanumericString(int N) {
    std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result(N, ' ');
    for (int i = 0; i < N; ++i) {
        result[i] = characters[rand() % characters.size()];
    }
    return result;
}

int main(int argc, char *argv[]) {
    srand(777);
    if (argc > 3) {
        std::cerr << "Usage: " << argv[0] << " [-e | -f <filename>]" << std::endl;
        return 1;
    }

    std::string input;
    if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "-e") {
            // Example input
            input = "This is the canary.";
            // Otherwise, run the example loop
            int c = 0;
            time_t t = time(nullptr);
            Mirror256 h;
            for (int i = 0; i < 1024; ++i) {
                std::string randStr = randomAlphanumericString(32);
                std::string msg = "This is the canary #" + std::to_string(i) + ". asdfasdfasdfasdfasdfqwerqwerqwerdfnnjkdfnjldljknsvv";
                h.update(msg);
                c += 1;
                if (time(nullptr) > t + 1) {
                    std::cout << c << " hashes per second!" << std::endl;
                    c = 0;
                    t = time(nullptr);
                    exit(0);
                }
                std::cout << "Example message = " << msg << std::endl << std::endl;
                std::cout << "Example digest = " << h.hexdigest() << std::endl << std::endl;
                //std::cout << "Example message = " << randStr << std::endl;
                //std::cout << "Example digest = " << Mirror256(randStr).hexdigest() << std::endl;
            }
            return 0;
        } else if (arg == "-f") {
            // Read input from file
            std::ifstream file(argv[2]);
            if (!file.is_open()) {
                std::cerr << "Error: could not open file " << argv[2] << std::endl;
                return 1;
            }
            std::getline(file, input, '\0');
            file.close();
        } else {
            std::cerr << "Error: invalid argument " << arg << std::endl;
            return 1;
        }
    } else {
        // Read input from stdin
        std::getline(std::cin, input, '\0');
    }

    Mirror256 h(input);
    std::string digest = h.hexdigest();
    std::cout << digest << std::endl;

    return 0;
}

