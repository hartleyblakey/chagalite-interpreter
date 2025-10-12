#ifndef CST_HPP
#define CST_HPP

#include "tokenize.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <format>
#include <cassert>

#ifdef DEBUG
#define DEBUG_PRINT(s) std::cout << s << " in " << __func__ << " seeing " << t.content << " --> " << tk->peek().content << " on line " << tk->getLine() << "\n"; 
#else
#define DEBUG_PRINT(s)
#endif


struct CstNode {
    CstNode* child;
    CstNode* sib;
    Token t;

    CstNode (Token tok) {
        t = tok;
        child = nullptr;
        sib = nullptr;
    }

    void add_child(Token t) {
        CstNode* node = new CstNode(t);
        add_child(node);
    }

    bool add_child(CstNode* c) {
        if (!c) {
            return false;
        }

        if (!child) {
            child = c;
            return true;
        }

        CstNode* last = child;
        while (last->sib) {
            last = last->sib;
        }
        last->sib = c;
        return true;
    }
};

class Cst {
public:
    Cst() = delete;
    Cst(Tokenizer* tk) {
        this->tk = tk;
        root = new CstNode(UNKNOWN);
        current = root;
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
    CstNode* current;

    std::string error;

    void build();
    void destroy();

    void advance_child() {
        current->child = new CstNode(t);
        current = current->child;
        t = tk->next();
    }

    void advance_sibling() {
        current->sib = new CstNode(t);
        current = current->sib;
        t = tk->next();
    }

    void expect_child(TokenType type) {
        expect(type);
        current->child = new CstNode(t);
        current = current->child;
        t = tk->next();
    }

    void expect_sibling(TokenType type) {
        expect(type);
        current->sib = new CstNode(t);
        current = current->sib;
        t = tk->next();
    }


    bool parse_program();
    bool parse_main();
    bool parse_program_tail();
    bool parse_procedure();
    bool parse_function();
    bool parse_parameters();
    bool parse_block();
    bool parse_compound();
    bool parse_statement();
    bool parse_return();
    bool parse_declaration();
    bool parse_call();
    bool parse_call_statement();
    bool parse_sizeof();
    bool parse_getchar();
    bool parse_printf();
    bool parse_assignment();
    bool parse_iteration();
    bool parse_selection();
    bool parse_iteration_assignment();
    bool parse_expression();
    bool parse_initialization();
    bool parse_boolean_expression();
    bool parse_numerical_expression();
    bool parse_relational_expression();
    bool parse_numerical_operand();
    bool parse_identifier_and_ident_arr_param_list_decl();
    bool parse_identifier_and_ident_arr_param_list();
    bool parse_identifier_and_ident_arr_list();
    bool parse_identifier_arr_list();
    bool parse_identifier_list();
    bool parse_single_quoted_string();
    bool parse_double_quoted_string();
    bool parse_string();
    bool parse_parameter_decl();
    
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
        if (!ok()) {
            return;
        }

#ifdef DEBUG
        // assert(false);
        error = std::format("Syntax error on line {}: {}\n{}",
            tk->getLine(),
            std::format(fmt, std::forward<Args>(args)...),
            tk->getCurrentLine());
        
#else
        error = std::format("Syntax error on line {}: {}",
            tk->getLine(),
            std::format(fmt, std::forward<Args>(args)...));

#endif
        
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

    bool parse_first_accepted(std::initializer_list<bool(Cst::*)()> f_args) {
        bool ret = false;
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
