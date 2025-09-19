
#include "tokenize.hpp"
#include <iostream>
#include <ostream>
#include <vector>
#include <sstream>

/**
 * @brief Check if the tokenizer has encountered an error
 */
bool Tokenizer::ok() {
    return error.empty();
}

std::string Tokenizer::getError() {
    return error;
}

std::string syntaxError(size_t line, std::string reason) {
    return std::string("Syntax error on line ") + std::to_string(line) + ": " + reason;
}

bool singleCharEscape(char c) {
    return std::string("abfnrtv\\?\'\"").find(c) != std::string::npos;
}

bool hexDigit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

Token Tokenizer::next() {

    Token t(UNKNOWN);
    
    if (pending.type != UNKNOWN) {
        t = pending;
        pending = Token(UNKNOWN);
        return t;
    }

    while (t.type == UNKNOWN) {
        if (i >= file.size()) {
            t = Token(END);
            break;
        }
        
        char c = file[i];

        switch (state) {
            case START_STATE:
                tokenStart = i;
                if (isspace(c)) {
                    if (c == '\n') {
                        line++;
                    }
                } else if (isalpha(c) || c == '_') {
                    state = IN_IDENT;
                } else if (isdigit(c)) {
                    state = IN_INTEGER;
                } else if (c == '\"') {
                    t = Token(DOUBLE_QUOTE, file.substr(i, 1));
                    state = IN_D_STRING;
                    stringCharState = STR_BASE;
                } else if (c == '\'') {
                    t = Token(SINGLE_QUOTE, file.substr(i, 1));
                    state = IN_S_STRING;
                    stringCharState = STR_BASE;
                } else if (c == '+') {
                    state = ONE_PLUS;
                } else if (c == '-') {
                    state = ONE_MINUS;
                } else if (c == '(') {
                    t = Token(L_PAREN, file.substr(i, 1));
                } else if (c == ')') {
                    t = Token(R_PAREN, file.substr(i, 1));
                } else if (c == '[') {
                    t = Token(L_BRACKET, file.substr(i, 1));
                } else if (c == ']') {
                    t = Token(R_BRACKET, file.substr(i, 1));
                } else if (c == '{') {
                    t = Token(L_BRACE, file.substr(i, 1));
                } else if (c == '}') {
                    t = Token(R_BRACE, file.substr(i, 1));
                } else if (c == ';') {
                    t = Token(SEMICOLON, file.substr(i, 1));
                } else if (c == ',') {
                    t = Token(COMMA, file.substr(i, 1));
                } else if (c == '*') {
                    t = Token(ASTERISK, file.substr(i, 1));
                } else if (c == '/') {
                    t = Token(DIVIDE, file.substr(i, 1));
                } else if (c == '%') {
                    t = Token(MODULO, file.substr(i, 1));
                } else if (c == '=') {
                    state = ONE_EQUAL;
                } else if (c == '>') {
                    state = ONE_GT;
                } else if (c == '<') {
                    state = ONE_LT;
                } else if (c == '!') {
                    state = ONE_EXCLAMATION;
                } else if (c == '&') {
                    state = ONE_AMPERSAND;
                } else if (c == '|') {
                    state = ONE_PIPE;
                } else {
                    std::stringstream ss;
                    ss << "unknown character: " << c << " (" << (unsigned)c << ")";
                    error = syntaxError(line, ss.str());
                    t = Token(END);
                }
                break;

            case IN_D_STRING:
                if (c == '\"') {
                    pending = Token(DOUBLE_QUOTE, file.substr(i, 1));
                    if (stringCharState == ESC_CHAR) {
                        t = Token(ESCAPED_CHARACTER, file.substr(tokenStart + 1, i - tokenStart - 1));
                    } else {
                        t = Token(STRING, file.substr(tokenStart + 1, i - tokenStart - 1));
                    }
                    state = START_STATE;
                } else if (c == '\\') {
                    state = D_STR_ESC;
                    if (stringCharState == STR_BASE) {
                        // only single strings have non-hex escaped characters for now
                        stringCharState = REG_CHAR;
                    } else {
                        stringCharState = FULL_STR;
                    }
                } else {
                    if (stringCharState == STR_BASE) {
                        stringCharState = REG_CHAR;
                    } else {
                        stringCharState = FULL_STR;
                    }
                }
                break;

            case IN_S_STRING:
                if (c == '\'') {
                    pending = Token(SINGLE_QUOTE, file.substr(i, 1));
                    if (stringCharState == ESC_CHAR) {
                        t = Token(ESCAPED_CHARACTER, file.substr(tokenStart + 1, i - tokenStart - 1));
                    } else {
                        t = Token(STRING, file.substr(tokenStart + 1, i - tokenStart - 1));
                    }
                    state = START_STATE;
                } else if (c == '\\') {
                    state = S_STR_ESC;
                    if (stringCharState == STR_BASE) {
                        stringCharState = ESC_CHAR;
                    } else {
                        stringCharState = FULL_STR;
                    }
                } else {
                    if (stringCharState == STR_BASE) {
                        stringCharState = REG_CHAR;
                    } else {
                        stringCharState = FULL_STR;
                    }
                }
                break;

            case IN_IDENT:
                if (!isalnum(c) && c != '_') {
                    t = Token(IDENTIFIER, file.substr(tokenStart, i - tokenStart));
                    state = START_STATE;
                    continue; // reprocess the current char from the start
                }
                break;

            case IN_INTEGER:
                if (isalpha(c) || c == '_') {
                    error = syntaxError(line, "invalid integer");
                    t = Token(END);
                } else if (!isdigit(c)) {
                    t = Token(INTEGER, file.substr(tokenStart, i - tokenStart));
                    state = START_STATE;
                    continue; // reprocess the current char from the start
                }
                break;

            case ONE_PLUS:
                if (isdigit(c)) {
                    state = IN_INTEGER;
                } else {
                    t = Token(PLUS, file.substr(i - 1, 1));
                    state = START_STATE;
                    continue; // reprocess the current char from the start
                }
                break;

            case ONE_MINUS:
                if (isdigit(c)) {
                    state = IN_INTEGER;
                } else {
                    t = Token(MINUS, file.substr(i - 1, 1));
                    state = START_STATE;
                    continue; // reprocess the current char from the start
                }
                break;

            case ONE_EQUAL:
                if (c == '=') {
                    t = Token(BOOLEAN_EQUAL, file.substr(i-1, 2));
                    state = START_STATE;
                } else {
                    t = Token(ASSIGNMENT_OPERATOR, file.substr(i-1, 1));
                    state = START_STATE;
                    continue; // reprocess the current char from the start
                }
                break;

            case ONE_AMPERSAND:
                if (c == '&') {
                    t = Token(BOOLEAN_AND, file.substr(tokenStart, 2));
                    state = START_STATE;
                } else {
                    error = syntaxError(line, "expected '&&', found '&'");
                    t = Token(END);
                }
                break;

            case ONE_PIPE:
                if (c == '|') {
                    t = Token(BOOLEAN_OR, file.substr(tokenStart, 2));
                    state = START_STATE;
                } else {
                    error = syntaxError(line, "expected '||', found '|'");
                    t = Token(END);
                }
                break;

            case ONE_GT:
                if (c == '=') {
                    t = Token(GT_EQUAL, file.substr(tokenStart, 2));
                    state = START_STATE;
                } else {
                    t = Token(GT, file.substr(tokenStart, 1));
                    state = START_STATE;
                    continue; // reprocess the current char from the start
                }
                break;

            case ONE_LT:
                if (c == '=') {
                    t = Token(LT_EQUAL, file.substr(tokenStart, 2));
                    state = START_STATE;
                } else {
                    t = Token(LT, file.substr(tokenStart, 1));
                    state = START_STATE;
                    continue; // reprocess the current char from the start
                }
                break;

            case ONE_EXCLAMATION:
                if (c == '=') {
                    t = Token(BOOLEAN_NOT_EQUAL, file.substr(tokenStart, 2));
                    state = START_STATE;
                } else {
                    t = Token(BOOLEAN_NOT, file.substr(tokenStart, 1));
                    state = START_STATE;
                    continue; // reprocess the current char from the start
                }
                break;
            
            case D_STR_ESC:
                if (c == 'x') {

                    // hex digit
                    state = D_STR_HEX;
                    
                    if (stringCharState == REG_CHAR) {
                        // only hex digits are escaped characters for now
                        stringCharState = ESC_CHAR;
                    }
                } else if (singleCharEscape(c)) {
                    state = IN_D_STRING;
                } else {
                    error = syntaxError(line, "invalid escape");
                    t = Token(END);
                }
                break;

            case S_STR_ESC:
                if (c == 'x') {
                    // hex digit
                    state = S_STR_HEX;
                } else if (singleCharEscape(c)) {
                    state = IN_S_STRING;
                } else {
                    error = syntaxError(line, "invalid escape");
                    t = Token(END);
                }
                break;

            case D_STR_HEX:
                if (!hexDigit(c)) {
                    state = IN_D_STRING;
                    continue; // reprocess the current char as a string
                }
                break;

            case S_STR_HEX:
                if (!hexDigit(c)) {
                    state = IN_S_STRING;
                    continue; // reprocess the current char as a string
                }
                break;
        }

        i++;
    }
    return t;
}

/**
 * @brief Reflecting on C++
 */
const char* tokenTypeName(TokenType t) {
    switch (t) {
        case UNKNOWN: return "UNKNOWN";
        case END: return "END";
        case IDENTIFIER: return "IDENTIFIER";
        case SEMICOLON: return "SEMICOLON";
        case COMMA: return "COMMA";
        case L_PAREN: return "L_PAREN";
        case R_PAREN: return "R_PAREN";
        case L_BRACE: return "L_BRACE";
        case R_BRACE: return "R_BRACE";
        case L_BRACKET: return "L_BRACKET";
        case R_BRACKET: return "R_BRACKET";
        case INTEGER: return "INTEGER";
        case ASSIGNMENT_OPERATOR: return "ASSIGNMENT_OPERATOR";
        case MODULO: return "MODULO";
        case ASTERISK: return "ASTERISK";
        case DIVIDE: return "DIVIDE";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case STRING: return "STRING";
        case DOUBLE_QUOTE: return "DOUBLE_QUOTE";
        case SINGLE_QUOTE: return "SINGLE_QUOTE";
        case BOOLEAN_EQUAL: return "BOOLEAN_EQUAL";
        case BOOLEAN_NOT_EQUAL: return "BOOLEAN_NOT_EQUAL";
        case GT: return "GT";
        case GT_EQUAL: return "GT_EQUAL";
        case LT: return "LT";
        case LT_EQUAL: return "LT_EQUAL";
        case BOOLEAN_AND: return "BOOLEAN_AND";
        case BOOLEAN_OR: return "BOOLEAN_OR";
        case BOOLEAN_NOT: return "BOOLEAN_NOT";
        case CHAR: return "CHAR";
        case ESCAPED_CHARACTER: return "ESCAPED_CHARACTER";
        default: return "UNREACHABLE";
    }
}