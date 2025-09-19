/**
 * @file main.cpp
 * @author Hartley Blakey
 * @brief Program to tokenize a ChagaLite source file, printing the tokens or
 *        any errors if the lexer encountered an error
 * 
 * Usage: ./tokenize file.c
 */

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

#include "comments.hpp"
#include "tokenize.hpp"

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

    std::vector<Token> tokens;

    // read all tokens before printing any, in case we encounter an error
    while (tokenizer.ok()) {
        tokens.push_back(tokenizer.next());
        if (tokens.back().type == END) {
            tokens.pop_back();
            break;
        }
    }
 
    if (tokenizer.ok()) {
        std::cout << "\nToken list:\n\n";
        for (Token t : tokens) {
            std::cout << "Token type: " << tokenTypeName(t.type) << "\n";
            std::cout << "Token:      " << t.content << "\n";
            std::cout << "\n";
        }
        std::cout << "\n";
    } else {
        std::cout << tokenizer.getError() << "\n";
    }

    return 0;
}