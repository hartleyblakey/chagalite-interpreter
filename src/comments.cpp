/**
 * @file comments.cpp
 * @author Hartley Blakey
 * @brief Implementation of a function to remove the comments from a ChagaLite source
 * file using a procedural DFA
 */

#include "comments.hpp"
#include <iostream>
#include <ostream>

enum CommentParseState {
    /// @brief Not inside a comment or string 
    Initial,

    /// @brief Potentially the first character of either comment type 
    OneSlash,

    /// @brief Inside a line comment (// xyz) 
    LineMiddle,

    /// @brief Inside a block comment (/* xyz */)
    BlockMiddle,

    /// @brief Asterisk inside of a block comment
    Asterisk,

    /// @brief Asterisk outside of a block comment
    EarlyAsterisk,

    /// @brief Inside a double quoted string
    DoubleString,

    /// @brief Inside a single quoted string
    SingleString,

    /// @brief An escaped character in a double quoted string
    DoubleEscape,

    /// @brief An escaped character in a single quoted string
    SingleEscape,
};

/**
 * @brief Function to replace the given range of non-whitespace
 * characters with spaces.
 * 
 * @param file  The string to modify
 * @param first The first character to check
 * @param last  The last character to check
 */
void clearNonWs(std::string& file, size_t first, size_t last) {
    for (size_t i = first; i <= last; i++) {
        if (!iswspace(file[i])) {
            file[i] = ' ';\
        }
    }
}



std::string removeComments(std::string& file) {
    CommentParseState state = CommentParseState::Initial;

    size_t i = 0;
    size_t line = 1;

    size_t commentStart = i;
    size_t commentStartLine = line;

    // Only used to report unterminated string errors.
    // Should be handled by the tokenizer instead.
    // size_t stringStartLine = line;

    while (i < file.size()) {
        char c = file[i];

        switch (state) {
            case Initial:
                if (c == '/') {
                    state = OneSlash;
                    commentStart = i;
                    commentStartLine = line;
                } else if (c == '\"') {
                    state = DoubleString;
                    // stringStartLine = line;
                } else if (c == '\'') {
                    state = SingleString;
                    // stringStartLine = line;
                } else if (c == '*') {
                    state = EarlyAsterisk;
                }
                // implicit self loop on anything not covered
                break;
            case EarlyAsterisk:
                if (c == '/') {
                    // block comment terminator outside of a block comment
                    return std::string(
                        "ERROR: Program contains C-style, unterminated comment on line "
                    ) + std::to_string(line) + "\n";
                }
                
                if (c != '*') {
                    state = Initial;
                }
                // implicit self loop on *
                break;
            case DoubleString:
                if (c == '\"') {
                    state = Initial;
                } else if (c == '\\') {
                    state = DoubleEscape;
                }
                // implicit self loop
                break;
            case SingleString:
                if (c == '\'') {
                    state = Initial;
                } else if (c == '\\') {
                    state = SingleEscape;
                }
                // implicit self loop
                break;
            case DoubleEscape:
                /* Escape sequences can be multiple bytes. 
                However, none of them can contain a quote character after the first byte,
                so I think this should be fine for the scope of this assignment. */

                // Ignore the current character. 
                // Valid escape or not, it can't terminate the string
                state = DoubleString;
                break;
            case SingleEscape:
                // Ignore the current character. 
                // Valid escape or not, it can't terminate the string
                state = SingleString;
                break;
            case OneSlash:
                if (c == '/') {
                    state = LineMiddle;
                } else if (c == '*') {
                    state = BlockMiddle;
                } else if (c == '\'') {
                    state = SingleString;
                    // stringStartLine = line;
                } else if (c == '\"') {
                    state = DoubleString;
                    // stringStartLine = line;
                } else {
                    state = Initial;
                }
                break;
            case LineMiddle:
                if (c == '\n') {
                    state = Initial;
                    clearNonWs(file, commentStart, i);
                }
                // implicit self loop on "not \n"
                break;
            case BlockMiddle:
                if (c == '*') {
                    state = Asterisk;
                }
                // implicit self loop on "not *"
                break;
            case Asterisk:
                if (c == '/') {
                    state = Initial;
                    clearNonWs(file, commentStart, i);
                } else if (c != '*') {
                    // false alarm, not a block comment terminator
                    state = BlockMiddle;
                }
                // implicit state = Asterisk if c == '*'
                break;
        }
        i++;
        if (c == '\n') {
            line++;
        }
    }

    // EOF should also be a valid line comment terminator
    if (state == LineMiddle) {
        clearNonWs(file, commentStart, i - 1);
    }

    // If the program contains an unterminated block comment or string, report an error
    if (state == BlockMiddle) {
        return std::string(
            "ERROR: Program contains C-style, unterminated comment on line "
        ) + std::to_string(commentStartLine) + "\n";
    }
    
    if (state == DoubleString || state == SingleString) {
        // Apparently not our problem, should fail in tokenizer instead
        //
        // return std::string("ERROR: Program contains an unterminated string on line ") 
        //     + std::to_string(stringStartLine) + "\n";
    }

    // Empty error string indicates success
    return {};
}