#include "cst.hpp"
#include <iostream>



bool isBrace(Token t) {
    return t.type == L_BRACE || t.type == R_BRACE;
}

bool Cst::isRelationalExpression(Token t) {
    return any(t, LT, GT, LT_EQUAL, GT_EQUAL, BOOLEAN_EQUAL, BOOLEAN_NOT_EQUAL);
}

bool Cst::isNumericalOperator(Token t) {
    return any(t, PLUS, MINUS, ASTERISK, DIVIDE, MODULO, CARET);
}

bool Cst::isBooleanOperator(Token t) {
    return any(t, BOOLEAN_AND, BOOLEAN_OR);
}

bool Cst::isBooleanLiteral(Token t) {
    return t.type == IDENTIFIER && any(t, "TRUE", "FALSE");
}

bool Cst::isDatatypeSpecifier(Token t) {
    return t.type == IDENTIFIER && any(t, "char", "bool", "int");
}

bool Cst::notReservedWord(Token t) {
    return !any(t, isBooleanLiteral, isDatatypeSpecifier, "procedure",
         "function", "getchar", "printf", "sizeof", "return", "void", 
         "for", "while", "if");
}

CstNode* Cst::parse_block() {
    return nullptr;
}




CstNode* Cst::parse_return() {
    return nullptr;
}

CstNode* Cst::parse_declaration() {
    if (!isDatatypeSpecifier(t)) {
        return nullptr;
    }

    CstNode* decl = new CstNode(DECLARATION_STATEMENT);

    while (isDatatypeSpecifier(t) && t.type != END) {
        decl->add_child(new CstNode(CST_TOKEN, t));
        advance();

        expect(IDENTIFIER);
        expect(notReservedWord, "reserved word \"{}\" cannot be used as a variable name", t.content);
        decl->add_child(new CstNode(CST_TOKEN, t));
        advance();

        if (t.type != COMMA) {
            break;
        }
        decl->add_child(new CstNode(CST_TOKEN, t));
        advance();
    }

    expect(SEMICOLON);
    decl->add_child(new CstNode(CST_TOKEN, t));
    advance();
    
    return nullptr;
}

CstNode* Cst::parse_call() {
    return nullptr;
}

CstNode* Cst::parse_call_statement() {
    return nullptr;
}

CstNode* Cst::parse_sizeof() {
    return nullptr;
}

CstNode* Cst::parse_getchar() {
    return nullptr;
}

CstNode* Cst::parse_printf() {
    return nullptr;
}

CstNode* Cst::parse_assignment() {
    if (t.type != IDENTIFIER) {
        return nullptr;
    }

    CstNode* assignment = new CstNode(ASSIGNMENT_STATEMENT);

    expect(notReservedWord, "Cannot assign to reserved word {}", t.content);
    assignment->add_child(new CstNode(CST_TOKEN, t));
    advance();

    if (t.type == L_BRACKET) {
        assignment->add_child(new CstNode(CST_TOKEN, t));
        advance();

        CstNode* expr = parse_numerical_expression();
        if (!expr) {
            syntaxError("Invalid array index");
        } else {
            assignment->add_child(expr);
        }

        expect(R_BRACKET);
        assignment->add_child(new CstNode(CST_TOKEN, t));
        advance();
    }

    expect(ASSIGNMENT_OPERATOR);
    assignment->add_child(new CstNode(CST_TOKEN, t));
    advance();

    if (t.type == IDENTIFIER) {
        expect(notReservedWord, "attempted to take value of reserved word {}", t.content);
        assignment->add_child(new CstNode(CST_TOKEN, t));
        advance();
    } else if ((t.type == SINGLE_QUOTE || t.type == DOUBLE_QUOTE) && tk->peek().type == ESCAPED_CHARACTER) {
        auto quoteType = t.type;
        assignment->add_child(new CstNode(CST_TOKEN, t));
        advance();

        assignment->add_child(new CstNode(CST_TOKEN, t));
        advance();

        expect(quoteType);
        assignment->add_child(new CstNode(CST_TOKEN, t));
        advance();
    } else if (t.type == SINGLE_QUOTE || t.type == DOUBLE_QUOTE) {
        auto quoteType = t.type;
        assignment->add_child(new CstNode(CST_TOKEN, t));
        advance();

        expect(STRING);
        assignment->add_child(new CstNode(CST_TOKEN, t));
        advance();

        expect(quoteType);
        assignment->add_child(new CstNode(CST_TOKEN, t));
        advance();
    } else {
        // must be an expression
        CstNode* expr = parse_expression();
        if (!expr) {
            syntaxError("invalid assignment, expecting expression");
        } else {
            assignment->add_child(expr);
        }
    }

    expect(SEMICOLON);
    assignment->add_child(new CstNode(CST_TOKEN, t));
    advance();
    return assignment;
}

CstNode* Cst::parse_iteration() {
    return nullptr;
}

CstNode* Cst::parse_selection() {
    return nullptr;
}

CstNode* Cst::parse_iteration_assignment() {
    return nullptr;
}

CstNode* Cst::parse_expression() {
    return nullptr;
}

CstNode* Cst::parse_initialization() {
    return nullptr;
}

CstNode* Cst::parse_boolean_expression() {
    return nullptr;
}

CstNode* Cst::parse_numerical_expression() {
    return nullptr;
}

CstNode* Cst::parse_relational_expression() {
    return nullptr;
}

CstNode* Cst::parse_numerical_operand() {
    return nullptr;
}

CstNode* Cst::parse_identifier_and_ident_arr_param_list_decl() {
    return nullptr;
}

CstNode* Cst::parse_identifier_and_ident_arr_param_list() {
    return nullptr;
}

CstNode* Cst::parse_identifier_and_ident_arr_list() {
    return nullptr;
}

CstNode* Cst::parse_identifier_arr_list() {
    return nullptr;
}

CstNode* Cst::parse_identifier_list() {
    return nullptr;
}

CstNode* Cst::parse_single_quoted_string() {
    return nullptr;
}

CstNode* Cst::parse_double_quoted_string() {
    return nullptr;
}

CstNode* Cst::parse_string() {
    return nullptr;
}



CstNode* Cst::parse_statement() {
    return parse_first_accepted({
        &Cst::parse_declaration, 
        &Cst::parse_assignment, 
        &Cst::parse_iteration, 
        &Cst::parse_return, 
        &Cst::parse_call_statement, 
        &Cst::parse_printf, 
        &Cst::parse_selection
    });
}

CstNode* Cst::parse_compound() {
    CstNode* compound = new CstNode(COMPOUND_STATEMENT);
    while (CstNode* next = parse_statement()) {
        compound->add_child(next);
    }
    return compound;
}


bool Cst::parse_parameter_decl(CstNode* params) {
    if (!isDatatypeSpecifier(t)) {
        return false;
    }

    params->add_child(new CstNode(CST_TOKEN, t));
    advance();

    expect(IDENTIFIER, "Expected an identifier following a datatype in a parameter list");
    expect(notReservedWord, "reserved word \"{}\" cannot be used for the name of a variable", t.content);
    params->add_child(new CstNode(CST_TOKEN, t));
    advance();
    
    if (t.type == L_BRACKET) {   
        params->add_child(new CstNode(CST_TOKEN, t));
        advance();

        expect(INTEGER, "Expected integer inside array braces, got {}", t.content);
        params->add_child(new CstNode(CST_TOKEN, t));
        advance();

        expect(R_BRACKET, "Expected closing array brace, got {}", t.content);
        params->add_child(new CstNode(CST_TOKEN, t));
        advance();
    }

    return true;
}


CstNode* Cst::parse_parameters() {
    
    
    if (!isDatatypeSpecifier(t)) {
        return nullptr;
    }

    CstNode* params = new CstNode(PARAMETER_LIST);

    while (t.type != END) {
        parse_parameter_decl(params);

        if (t.type == R_PAREN) {
            break;
        }

        expect(COMMA, "Expected comma separated items in parameter list");
        params->add_child(new CstNode(CST_TOKEN, t));
        advance();
    }

    return params;
}

CstNode* Cst::parse_procedure() {
    return nullptr;
}

CstNode* Cst::parse_function() {
    
    if (t.content != "function") {
        return nullptr;
    }
    CstNode* function = new CstNode(FUNCTION_DECLARATION);

    function->add_child(new CstNode(CST_TOKEN, t));
    advance();

    expect(isDatatypeSpecifier, "Expected datatype specifier after function declaration");
    function->add_child(new CstNode(CST_TOKEN, t));
    advance();

    expect(IDENTIFIER, "Expected identifier after datatype specifier");
    expect(notReservedWord, "reserved word \"{}\" cannot be the name of a function", t.content);
    function->add_child(new CstNode(CST_TOKEN, t));
    advance();

    expect(L_PAREN, "Expected L_PAREN");
    function->add_child(new CstNode(CST_TOKEN, t));
    advance();

    if (t.content == "void") {
        function->add_child(new CstNode(CST_TOKEN, t));
        advance();
    } else {
        CstNode* params = parse_parameters();
        if (!params) {
            syntaxError("Empty parameter list not allowed");
        } else {
            function->add_child(params);
        }
    }

    expect(R_PAREN);
    function->add_child(new CstNode(CST_TOKEN, t));
    advance();


    expect(L_BRACE);
    function->add_child(new CstNode(CST_TOKEN, t));
    advance();

    CstNode* body = parse_compound();
    if (!body) {
        syntaxError("Empty function body not allowed: must have a valid return");
    }
    function->add_child(body);


    expect(R_BRACE);
    function->add_child(new CstNode(CST_TOKEN, t));
    advance();

    return function;
}

CstNode* Cst::parse_program_tail() {
    CstNode* next = nullptr;
    next = parse_function();
    if (!next) {
        next = parse_declaration();
    }
    if (!next) {
        next = parse_procedure();
    }

    if (next) {
        std::cout << "Another top level decl to check\n";
        next->sib = parse_program_tail();
    }

    return next;
}

CstNode* Cst::parse_main() {
    if (t.content != "procedure" || tk->peek().content != "main") {
        return nullptr;
    }

    CstNode* mainProc = new CstNode(MAIN_PROCEDURE);

    
    mainProc->add_child(new CstNode(CST_TOKEN, t));
    advance(); // main
    mainProc->add_child(new CstNode(CST_TOKEN, t));
    advance();

    expect(L_PAREN, "Expected L_PAREN after 'procedure main', found {}", tokenTypeName(t.type));
    mainProc->add_child(new CstNode(CST_TOKEN, t));
    advance();

    expect("void", "Main procedure must have parameter type 'void', has {}", t.content);
    mainProc->add_child(new CstNode(CST_TOKEN, t));
    advance();

    expect(R_PAREN, "Expected R_PAREN after main parameter list, found {}", tokenTypeName(t.type));
    mainProc->add_child(new CstNode(CST_TOKEN, t));
    advance();

    auto block = parse_block();
    if (!block) {
        syntaxError("Expected valid block statement following main declaration");
    }
    mainProc->add_child(block);

    return mainProc;
}

CstNode* Cst::parse_program() {

    if (expect(IDENTIFIER, "Expected valid program start, got \"{}\"", t.content)) {
        CstNode* program = new CstNode(PROGRAM);
        CstNode* next = nullptr;

        while (t.type != END) {
            next = parse_main();
            if (next) {
                program->add_child(next);
                program->add_child(parse_program_tail());
                break;
            }

            next = parse_function();
            if (!next) {
                next = parse_procedure();
            }
            if (!next) {
                next = parse_declaration();
            }
            if (next) {
                program->add_child(next);
            } else {
                break;
            }
        }
        expect(END, "There should be no content after the end of the program");
        return program;
    }
    
    return nullptr;
}


void deleteTree(CstNode* node) {
    if (!node) {
        return;
    }
    deleteTree(node->child);
    deleteTree(node->sib);
    delete node;
}

void Cst::destroy() {
    deleteTree(root);
}


void Cst::build() {
    advance();
    root = parse_program();
}

void printNode(CstNode* n) {
    if (!n) {
        std::cout << "NULL NODE POINTER!!";
        return;
    }

    if (n->t.type != UNKNOWN) {
        std::cout << n->t.content;
    } else {
        std::cout << "[CST NODE]";
    }
    
    if (n->sib) {
        std::cout << "   ";
        printNode(n->sib);
    }
    if (n->child) {
        std::cout << "\n";
        printNode(n->child);
    }
}

void printCstNode(CstNode* n, int depth) {
    if (!n) {
        std::cout << "NULL NODE POINTER!!";
        return;
    }

    for (int i = 0; i < depth; i++) {
        std::cout << " |-";
    }

    if (n->t.type != UNKNOWN) {
        std::cout << n->t.content;
    } else {
        std::cout << "[CST NODE]";
    }
    
    if (n->sib) {
        std::cout << "\n";
        printCstNode(n->sib, depth);
    }
    if (n->child) {
        std::cout << "\n";
        printCstNode(n->child, depth + 1);
    }
}

void Cst::print() {
    printCstNode(root, 0);
    std::cout << "\n";
}