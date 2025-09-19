/**
 * @file main.cpp
 * @author Hartley Blakey
 * @brief Program to remove the comments from a ChagaLite source file and print the
 * resulting code to the standard output. If errors were encountered when processing
 * they are printed instead of the output, and a non-zero exit code is returned.
 * 
 * Usage: ./build/comments file.c
 */

#include <iostream>
#include <fstream>
#include <filesystem>

#include "comments.hpp"

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cout << "Usage: ./build/comments path/to/my-file.c\n";
        return 1;
    }

    std::ifstream inputFile(argv[1]);

    if (!inputFile.is_open()) {
        std::cout << "ERROR: Failed to open file\n";
        return 2;
    }

    // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
    auto size = std::filesystem::file_size(argv[1]);
    std::string content(size, '\0');
    inputFile.read(&content[0], size);

    std::string error = removeComments(content);
    
    if (error.empty()) {
        std::cout << content;
        return 0;
    } else {
        std::cout << error;
        return 3;
    }
    
}