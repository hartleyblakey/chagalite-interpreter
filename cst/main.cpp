#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

#include "comments.hpp"
#include "tokenize.hpp"
#include "cst.hpp"

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cout << "Usage: tokenize path/to/my-file.c\n";
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

    // strip comments
    std::string commentsError = removeComments(content);
    if (!commentsError.empty()) {
        std::cout << commentsError;
        return 3;
    }

    Tokenizer tokenizer(content);

    Cst cst(&tokenizer);

    if (!cst.ok()) {
        std::cout << cst.getError() << "\n";
    }

    cst.print();

    std::cout << "\n";

    return 0;
}