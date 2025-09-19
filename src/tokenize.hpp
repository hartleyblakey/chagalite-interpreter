#ifndef TOKENIZE_HPP
#define TOKENIZE_HPP

#include <string>
#include <string_view>
#include <deque>

enum TokenType {
    UNKNOWN,
    END,
    
    IDENTIFIER,
    SEMICOLON,
    COMMA,

    L_PAREN,
    R_PAREN,
    L_BRACE,
    R_BRACE,
    L_BRACKET,
    R_BRACKET,

    INTEGER,

    ASSIGNMENT_OPERATOR,
    MODULO,
    ASTERISK,
    DIVIDE,
    PLUS,
    MINUS,

    STRING,
    DOUBLE_QUOTE,
    SINGLE_QUOTE,

    BOOLEAN_EQUAL,
    BOOLEAN_NOT_EQUAL,
    GT,
    GT_EQUAL,
    LT,
    LT_EQUAL,

    BOOLEAN_AND,
    BOOLEAN_OR,
    BOOLEAN_NOT,

    CHAR,
    ESCAPED_CHARACTER,
};

enum State {
    START_STATE,
    IN_IDENT,

    IN_D_STRING,
    IN_S_STRING,
    D_STR_ESC,
    S_STR_ESC,

    IN_INTEGER,

    ONE_PLUS,
    ONE_MINUS,

    ONE_EQUAL,

    ONE_GT, // >
    ONE_LT, // <
    ONE_PIPE, // |
    ONE_AMPERSAND, // &
    ONE_EXCLAMATION, // !

    S_STR_HEX,
    D_STR_HEX,
};

const char* tokenTypeName(TokenType t);

class Token {
public:
    TokenType type;
    std::string_view content;

    Token() : type(UNKNOWN), content("") {}
    Token(TokenType t) {type = t;}
    Token(TokenType t, std::string_view c) {type = t; content = c;}

    friend class Tokenizer;
};

// Without this, I would need 4x more explicit states to represent strings
enum StringCharState {
    /// @brief An empty string
    STR_BASE,
    /// @brief An escaped character literal
    ESC_CHAR,
    /// @brief A regular character literal
    REG_CHAR,
    /// @brief A full string with multiple characters
    FULL_STR
};

class Tokenizer {
    std::string_view file;
    size_t i;
    size_t line;
    size_t tokenStart;

    State state;

    /// @brief only comes into effect when state==IN_S_STRING or state==IN_D_STRING
    StringCharState stringCharState;

    std::string error;

    // When the tokenizer reaches the end of a string, it emits two tokens at once
    // (string and quote). If type != UNKNOWN, return this instead of scanning
    Token pending;

public:
    Tokenizer() = delete;
    Tokenizer(std::string_view file) {
        this->file = file;
        state = START_STATE;
        tokenStart = 0;
        i = 0;
        line = 1;
        pending = Token(UNKNOWN);
    }

    std::string getError();
    Token next();
    bool ok();
};


#endif /* TOKENIZE_HPP */
