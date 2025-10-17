#ifndef CST_HPP
#define CST_HPP

#include <cassert>
#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include "tokenize.hpp"
#include <optional>
#include <string_view>
#include <variant>
#include <iostream>

class SymbolTable {
    
    enum VariableType {
        Int,
        Char,
        String,
        Bool
    };

    struct Node;

    struct FunctionType {
        std::optional<VariableType> return_type;
        std::vector<Node*> params;
    };

    struct Node {
        std::string_view name;
        std::variant<FunctionType, VariableType> payload;
        uint32_t scope = 0;
        Node* next = nullptr;
    };

    Node* head = nullptr;
    Node* tail = nullptr;

    uint32_t next_scope = 1;

    Node* current_function = nullptr;

    uint32_t current_scope() { return current_function ? current_function->scope : 0; }

    void add_node(Node* n) {
        if (tail) {
            tail->next = n;
            tail = tail->next;
        } else {
            tail = n;
            head = n;
        }
    }

    Node* resolve(std::string_view name) {
        Node* n = head;
        uint32_t scope = current_scope();
        while (n) {
            if ((n->name == name && n->scope == 0) || n->scope == scope) {
                break;
            }
        }
        return n;
    }

    void enter_function(std::string_view name, std::optional<VariableType> return_type) {
        current_function = new Node{name, FunctionType{return_type, {}}, next_scope};
        next_scope++;
        add_node(current_function);
    }

    void add_param(std::string_view name, VariableType type) {
        Node* p = new Node{name, type, current_scope()};
        add_node(p);
        std::get<FunctionType>(current_function->payload).params.push_back(p);
    }

    void exit_function() {
        current_function = nullptr;
    }

    void add_var(std::string_view name, VariableType type) {
        Node* v = new Node{name, type, current_scope()};
        add_node(v);
    }

    std::string_view vartype_to_name(VariableType t) {
        switch (t) {
            case VariableType::Bool:
                return "bool";
            case VariableType::Int:
                return "int";
            case VariableType::Char:
                return "char";
            default:
                return "unknown";
        }
    }
public:
    void print() {
        Node* n = head;
        while (n) {
            std::cout << n->scope << " | " << n->name << " : ";
            if (VariableType* v = std::get_if<VariableType>(&n->payload)) {
                std::cout << vartype_to_name(*v);

            } else if (FunctionType* f = std::get_if<FunctionType>(&n->payload)) {
                std::cout << "function (";
                if (f->params.empty()) {
                    std::cout << "void";
                }
                for (size_t i = 0; i < f->params.size(); i++) {
                    if (i) {
                        std::cout << ", ";
                    }
                    std::cout << vartype_to_name(std::get<VariableType>(f->params[i]->payload));
                }
                std::cout << ") --> " << (f->return_type.has_value() ? vartype_to_name(*(f->return_type)) : "void");
            }
            std::cout << "\n";
            n = n->next;
        }
    }

    friend class Cst;
};

struct CstNode {
    CstNode* child;
    CstNode* sib;
    Token t;

    CstNode(Token tok) {
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
    using StNode = SymbolTable::Node;
    using Datatype = SymbolTable::VariableType;

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

    ~Cst() {
        destroy();
    }

    void print();

    std::string getError() { return error; }
    bool ok() { return error.empty(); }

    CstNode const* getRoot() const { return root; }

    static bool is_relational_expression(Token t);
    static bool is_numerical_operator(Token t);
    static bool is_boolean_operator(Token t);
    static bool is_boolean_literal(Token t);
    static bool is_datatype_specifier(Token t);
    static bool not_reserved_word(Token t);
    static Datatype to_datatype(Token t) {
        if (t.content == "int") {
            return Datatype::Int;
        } else if (t.content == "bool") {
            return Datatype::Bool;
        } else if (t.content == "char") {
            return Datatype::Char;
        } else {
            assert(false);
        }
    }

    SymbolTable table;

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
        advance_child();
    }

    void expect_sibling(TokenType type) {
        expect(type);
        advance_sibling();
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
    bool parse_identifier_and_ident_arr_param_list();
    bool parse_identifier_and_ident_arr_list();
    bool parse_parameter_decl();

    bool in_boolean_prefix();

    static bool any(const Token& t, TokenType aType) {
        return t.type == aType;
    }

    static bool any(const Token& t, const char* aContent) {
        return t.content == aContent;
    }

    static bool any(const Token& t, bool (*aTest)(Token)) {
        return aTest(t);
    }

    /**
     * @brief Helper to test if a token matches either a TokenType,
     * token content string, or predicate function
     */
    template <typename... Args>
    static bool any(const Token& t, Args... args) {
        return (any(t, args) || ...);
    }

    /**
     * @brief Emit a formatted syntax error. If this is called twice, only the
     * first will be recorded in the error string.
     *
     * Arguments are forwards to std::format() for the error message.
     *
     * Automatically prints the line number, and the line itself if DEBUG is defined
     */
    template <typename... Args>
    void syntaxError(std::format_string<Args...> fmt, Args&&... args) {
        if (!ok()) {
            return;
        }

#ifdef DEBUG
        // show extra context in debug
        error = std::format("Syntax error on line {}: {}\n{}",
                            tk->getLine(),
                            std::format(fmt, std::forward<Args>(args)...),
                            tk->getLineDebug());

#else
        error = std::format("Syntax error on line {}: {}",
                            tk->getLine(),
                            std::format(fmt, std::forward<Args>(args)...));

#endif
    }

    /**
     * @brief Expect the current token to match the pattern, which can be a
     * token type enum, a string to check against the token content, or a
     * predicate function to call on the current token.
     *
     * If the token does not match, emit a formatted syntax error
     *
     * @tparam T either TokenType, const char*, or a Token --> bool function
     * @tparam Args The type arguments to std::format
     * @param expected The patten to expect
     * @param fmt The format string for the syntax error
     * @param args The value arguments to std::format
     * @return true if the token matched
     * @return false if the token did not match
     */
    template <typename T, typename... Args>
    bool expect(T expected, std::format_string<Args...> fmt, Args&&... args) {
        if (!ok()) {
            return false;
        }
        if (!any(t, expected)) {
            syntaxError(fmt, std::forward<Args>(args)...);
            return false;
        }
        return true;
    }

    /**
     * @brief Expect the current token to match the given token type or content
     * Otherwise, emit a syntax error with the current line number.
     *
     * @tparam T either TokenType or const char*
     * @param expected the expected token type or content string
     * @return true if the current token matches
     * @return false otherwise
     */
    template <typename T>
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

    bool parse_first_accepted(std::initializer_list<bool (Cst::*)()> f_args) {
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
