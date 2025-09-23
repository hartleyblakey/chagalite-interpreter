#ifndef CST_HPP
#define CST_HPP

#include "tokenize.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <format>


enum CstNodeType {
    CST_TOKEN,
    DOUBLE_QUOTED_STRING,
    SINGLE_QUOTED_STRING,
    IDENTIFIER_LIST,
    IDENTIFIER_ARRAY_LIST,
    IDENTIFIER_AND_IDENTIFIER_ARRAY_LIST,
    IDENTIFIER_AND_IDENTIFIER_ARRAY_PARAMETER_LIST,
    IDENTIFIER_AND_IDENTIFIER_ARRAY_PARAMETER_LIST_DECLARATION,
    DATATYPE_SPECIFIER,
    NUMERICAL_OPERAND,
    NUMERICAL_OPERATOR,
    BOOLEAN_OPERATOR,
    RELATIONAL_EXPRESSION,
    NUMERICAL_EXPRESSION,
    BOOLEAN_EXPRESSION,
    INITIALIZATION_EXPRESSION,
    EXPRESSION,
    ITERATION_ASSIGNMENT,
    SELECTION_STATEMENT,
    ITERATION_STATEMENT,
    ASSIGNMENT_STATEMENT,
    PRINTF_STATEMENT,
    GETCHAR_FUNCTION,
    SIZEOF_FUNCTION,
    USER_DEFINED_FUNCTION,
    USER_DEFINED_PROCEDURE_CALL_STATEMENT,
    DECLARATION_STATEMENT,
    RETURN_STATEMENT,
    STATEMENT,
    COMPOUND_STATEMENT,
    BLOCK_STATEMENT,
    PARAMETER_LIST,
    FUNCTION_DECLARATION,
    PROCEDURE_DECLARATION,
    MAIN_PROCEDURE,
    PROGRAM_TAIL,
    PROGRAM,
};

struct CstNode {
    CstNode* child;
    CstNode* sib;
    CstNodeType type;
    Token t;

    CstNode (CstNodeType new_type) {
        t = Token(UNKNOWN);
        child = nullptr;
        sib = nullptr;
        type = new_type;
    }
    CstNode (CstNodeType new_type, Token tok) {
        t = tok;
        child = nullptr;
        sib = nullptr;
        type = new_type;
    }

    void add_child(CstNodeType t) {
        CstNode* node = new CstNode(t);
        add_child(node);
    }

    void add_child(CstNode* c) {
        if (!child) {
            child = c;
            return;
        }

        CstNode* last = child;
        while (last->sib) {
            last = last->sib;
        }
        last->sib = c;
    }
};

class Cst {
public:
    Cst() = delete;
    Cst(Tokenizer* tk) {
        this->tk = tk;
        root = nullptr;
        current = 0;
        error = {};
        t = Token(UNKNOWN);
        build();
    }

    void print();

    std::string getError() { return error; }
    bool ok() { return error.empty(); }

private:


    Token t;
    Tokenizer* tk;
    CstNode* root;
    size_t current;

    std::string error;

    void build();
    void destroy();

    void advance() {
        t = tk->next();
    }

    CstNode* parse_program();
    CstNode* parse_main();
    CstNode* parse_program_tail();
    CstNode* parse_procedure();
    CstNode* parse_function();
    CstNode* parse_parameters();
    CstNode* parse_block();
    CstNode* parse_compound();
    CstNode* parse_statement();
    CstNode* parse_return();
    CstNode* parse_declaration();
    CstNode* parse_call();
    CstNode* parse_call_statement();
    CstNode* parse_sizeof();
    CstNode* parse_getchar();
    CstNode* parse_printf();
    CstNode* parse_assignment();
    CstNode* parse_iteration();
    CstNode* parse_selection();
    CstNode* parse_iteration_assignment();
    CstNode* parse_expression();
    CstNode* parse_initialization();
    CstNode* parse_boolean_expression();
    CstNode* parse_numerical_expression();
    CstNode* parse_relational_expression();
    CstNode* parse_numerical_operand();
    CstNode* parse_identifier_and_ident_arr_param_list_decl();
    CstNode* parse_identifier_and_ident_arr_param_list();
    CstNode* parse_identifier_and_ident_arr_list();
    CstNode* parse_identifier_arr_list();
    CstNode* parse_identifier_list();
    CstNode* parse_single_quoted_string();
    CstNode* parse_double_quoted_string();
    CstNode* parse_string();

    bool parse_parameter_decl(CstNode* parent);
    
    static bool isRelationalExpression(Token t);
    static bool isNumericalOperator(Token t);
    static bool isBooleanOperator(Token t);
    static bool isBooleanLiteral(Token t);
    static bool isDatatypeSpecifier(Token t);
    static bool notReservedWord(Token t);

    static bool any(const Token& t, TokenType aType) {
        return t.type == aType;
    }

    static bool any(const Token& t, const char* aContent) {
        return t.content == aContent;
    }

    static bool any(const Token& t, bool(*aTest)(Token)) {
        return aTest(t);
    }

    template<typename... Args>
    static bool any(const Token& t, Args... args) {
        return (any(t, args) || ...);
    }

    template<typename... Args>
    void syntaxError(std::format_string<Args...> fmt, Args&&... args) {
        error = std::format("Syntax error on line {}: {}",
            tk->getLine(),
            std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename T, typename... Args>
    bool expect(T expected, std::format_string<Args...> fmt, Args&&... args) {
        if (!ok()) {
            return false;
        }
        if (!any(t, expected)) {
            syntaxError(fmt,  std::forward<Args>(args)...);
            return false;
        }
        return true;
    }

    template<typename T>
    bool expect(T expected) {
        if (!ok()) {
            return false;
        }
        if (!any(t, expected)) {
            syntaxError("Expected {}, got \"{}\" ({})", expected, t.content, t.type);
            return false;
        }
        return true;
    }

    CstNode* parse_first_accepted(std::initializer_list<CstNode*(Cst::*)()> f_args) {
        CstNode* ret = nullptr;
        for (auto f : f_args) {
            ret = (this->*f)();
            if (ret) {
                break;
            }
        }
        return ret;
    }


};


#endif /* CST_HPP */
