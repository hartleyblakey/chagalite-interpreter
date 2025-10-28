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

// #define DEBUG

class SymbolTable {
    
    // make optional<Datatype> 4 bytes
    enum Datatype: uint16_t {
        Int,
        Char,
        Bool
    };

    // no values needed now, can be easily extended
    struct VariableNode {
        Datatype type;
        uint32_t length;
    };

    struct Node;

    struct FunctionNode {
        std::optional<Datatype> return_type;
        std::vector<Node*> params;
    };

    struct Node {
        std::string_view name;
        std::variant<FunctionNode, VariableNode> payload;
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
            if (n->name == name && (n->scope == 0 || n->scope == scope)) {
                break;
            }
            n = n->next;
        }
        return n;
    }

    // add a new function with empty parameters to the symbol table
    void enter_function(std::string_view name, std::optional<Datatype> return_type) {
        current_function = new Node{name, FunctionNode{return_type, {}}, next_scope};
        next_scope++;
        add_node(current_function);
    }

    // add a new variable and link it to the current function
    void add_param(std::string_view name, Datatype type) {
        Node* p = new Node{name, VariableNode{type, 0}, current_scope()};
        add_node(p);
        if (current_function) {
            std::get<FunctionNode>(current_function->payload).params.push_back(p);
        }
    }

    // make the last entry in the list an array
    void make_array(int length) {
        std::get<VariableNode>(tail->payload).length = length;
    }

    void exit_function() {
        current_function = nullptr;
    }

    // add a new variable to the symbol table
    void add_var(std::string_view name, Datatype type) {
        Node* v = new Node{name, VariableNode{type, 0}, current_scope()};
        add_node(v);
    }

    std::string_view vartype_to_name(Datatype t) {
        switch (t) {
            case Datatype::Bool:
                return "bool";
            case Datatype::Int:
                return "int";
            case Datatype::Char:
                return "char";
        }
    }
public:
    void print_varnode(VariableNode* v) {
        std::cout << vartype_to_name(v->type);
        if (v->length) {
            std::cout << '[' << v->length << ']';
        }
    }
    // condensed format for debugging
    void print_mine() {
        Node* n = head;
        while (n) {
            std::cout << n->scope << " | " << n->name << " : ";
            if (VariableNode* v = std::get_if<VariableNode>(&n->payload)) {
                print_varnode(v);
            } else if (FunctionNode* f = std::get_if<FunctionNode>(&n->payload)) {
                std::cout << "function (";
                if (f->params.empty()) {
                    std::cout << "void";
                }
                for (size_t i = 0; i < f->params.size(); i++) {
                    if (i) {
                        std::cout << ", ";
                    }
                    print_varnode(&std::get<VariableNode>(f->params[i]->payload));
                }
                std::cout << ") --> " << (f->return_type.has_value() ? vartype_to_name(*(f->return_type)) : "void");
            }
            std::cout << "\n";
            n = n->next;
        }
    }
/*
      IDENTIFIER_NAME: random_long_parameter_list
      IDENTIFIER_TYPE: function
             DATATYPE: bool
    DATATYPE_IS_ARRAY: no
  DATATYPE_ARRAY_SIZE: 0
                SCOPE: 2
*/


    void print() {
        Node* n = head;
        while (n) {
            std::cout << std::format("{:>21}: {}\n", "IDENTIFIER_NAME", n->name);
            if (VariableNode* v = std::get_if<VariableNode>(&n->payload)) {
                std::cout << std::format("{:>21}: {}\n", "IDENTIFIER_TYPE", "datatype");
                std::cout << std::format("{:>21}: {}\n", "DATATYPE", vartype_to_name(v->type));
                if (v->length) {
                    std::cout << std::format("{:>21}: {}\n", "DATATYPE_IS_ARRAY", "yes");
                } else {
                    std::cout << std::format("{:>21}: {}\n", "DATATYPE_IS_ARRAY", "no");
                }
                std::cout << std::format("{:>21}: {}\n", "DATATYPE_ARRAY_SIZE", v->length);
                std::cout << std::format("{:>21}: {}\n", "SCOPE", n->scope);
            } else if (FunctionNode* f = std::get_if<FunctionNode>(&n->payload)) {
                if (f->return_type.has_value()) {
                    std::cout << std::format("{:>21}: {}\n", "IDENTIFIER_TYPE", "function");
                    std::cout << std::format("{:>21}: {}\n", "DATATYPE", vartype_to_name(*f->return_type));
                } else {
                    std::cout << std::format("{:>21}: {}\n", "IDENTIFIER_TYPE", "procedure");
                    std::cout << std::format("{:>21}: {}\n", "DATATYPE", "NOT APPLICABLE");
                }
                std::cout << std::format("{:>21}: {}\n", "DATATYPE_IS_ARRAY", "no");
                std::cout << std::format("{:>21}: {}\n", "DATATYPE_ARRAY_SIZE", 0);
                std::cout << std::format("{:>21}: {}\n", "SCOPE", n->scope);

                // skip printing its parameters until the end
                for (size_t i = 0; i < f->params.size(); i++) {
                    n = n->next;
                }
            }
            
            std::cout << "\n";
            if (n) {
                n = n->next;
            }
        }
        
        // print functions and their parameters
        n = head;
        while (n) {
            if (FunctionNode* f = std::get_if<FunctionNode>(&n->payload)) {
                if (f->params.empty()) {
                    if (n) {
                        n = n->next;
                    }
                    continue;
                }
                std::cout << "\n";
                std::cout << std::format("{:>21}: {}\n", "PARAMETER LIST FOR", n->name);
                
                for (Node* p : f->params) {
                    std::cout << std::format("{:>21}: {}\n", "IDENTIFIER_NAME", p->name);
                    VariableNode* v = &std::get<VariableNode>(p->payload);
                    std::cout << std::format("{:>21}: {}\n", "DATATYPE", vartype_to_name(v->type));
                    if (v->length) {
                        std::cout << std::format("{:>21}: {}\n", "DATATYPE_IS_ARRAY", "yes");
                    } else {
                        std::cout << std::format("{:>21}: {}\n", "DATATYPE_IS_ARRAY", "no");
                    }
                    std::cout << std::format("{:>21}: {}\n", "DATATYPE_ARRAY_SIZE", v->length);
                    std::cout << std::format("{:>21}: {}\n", "SCOPE", p->scope);
                    std::cout << "\n";
                }
            }
            
            
            if (n) {
                n = n->next;
            }
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
    using Datatype = SymbolTable::Datatype;

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
    bool parse_identifier_and_ident_arr_list(Datatype type);
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
        table.print_mine();

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

    void check_shadow(std::string_view ident, std::string_view type) {
        if (!ok()) {
            return;
        }
        if (StNode* n = table.resolve(t.content)) {
            std::string_view locality;
            if (std::holds_alternative<SymbolTable::FunctionNode>(n->payload)) {
                locality = "";
            } else {
                if (n->scope == 0) {
                    locality = " globally";
                } else {
                    locality = " locally";
                }
            }
            error = std::format("Error on line {}: {} \"{}\" is already defined{}", tk->getLine(), type, ident, locality);
#ifdef DEBUG
            table.print_mine();
#endif
        }
    }
};

#endif /* CST_HPP */
