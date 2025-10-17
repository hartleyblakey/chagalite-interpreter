#ifndef TOKENIZE_HPP
#define TOKENIZE_HPP

#include <string>
#include <string_view>
#include <deque>
#include <format>

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
    CARET,

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

    CHARACTER,
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
    // (string and quote). If this is not empty, next() returns pending.front().
    // deque needed for peek(), which is used in the CST parser. Otherwise single
    // pending token would work. push_front() used in peek implementation prevents
    // using std::queue instead.
    std::deque<Token> pending;

public:
    Tokenizer() = delete;
    Tokenizer(std::string_view file) {
        this->file = file;
        state = START_STATE;
        tokenStart = 0;
        i = 0;
        line = 1;
        pending = {};
    }

    /**
     * @brief Get the contents of the current line
     * 
     * @return std::string
     */
    std::string getLineDebug();
    

    /**
     * @brief Function to return the next token
     * 
     * @return The next token, with type END if there are no more tokens to read
     */
    Token next();

    /**
     * @brief Function to peek at the next value of next().
     * 
     * @return The token that will be retrieved by a subsequent call to next()
     */
    Token peek();

    /**
     * @brief Check if the tokenizer has encountered an error while parsing
     * 
     * @return true if no error has occurred and it is safe to call next()
     * @return false if there is an error and next() should not be called again
     */
    bool ok();

    /**
     * @brief Get the error explaining the false ok() return
     * 
     * @return the error string
     */
    std::string getError();

    size_t getLine() { return line; }

};

template <>
struct std::formatter<TokenType> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const TokenType& obj, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", tokenTypeName(obj));
    }
};



#endif /* TOKENIZE_HPP */
