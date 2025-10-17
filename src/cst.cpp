#include "cst.hpp"

#include <iostream>

#ifdef DEBUG
#define DEBUG_PRINT(s) std::cout << s << " in " << __func__ << " seeing " << t.content << " --> " << tk->peek().content << " on line " << tk->getLine() << "\n";
#else
#define DEBUG_PRINT(s)
#endif

bool isBrace(Token t) {
    return t.type == L_BRACE || t.type == R_BRACE;
}

bool Cst::is_relational_expression(Token t) {
    return any(t, LT, GT, LT_EQUAL, GT_EQUAL, BOOLEAN_EQUAL, BOOLEAN_NOT_EQUAL);
}

bool Cst::is_numerical_operator(Token t) {
    return any(t, PLUS, MINUS, ASTERISK, DIVIDE, MODULO, CARET);
}

bool Cst::is_boolean_operator(Token t) {
    return any(t, BOOLEAN_AND, BOOLEAN_OR);
}

bool Cst::is_boolean_literal(Token t) {
    return t.type == IDENTIFIER && any(t, "TRUE", "FALSE");
}

bool Cst::is_datatype_specifier(Token t) {
    return t.type == IDENTIFIER && any(t, "char", "bool", "int");
}

bool Cst::in_boolean_prefix() {
    Token x = t.type == L_PAREN ? tk->peek() : t;
    return is_boolean_literal(x) || x.type == BOOLEAN_NOT || is_boolean_operator(x);
}

bool Cst::not_reserved_word(Token t) {
    return !any(t, is_boolean_literal, is_datatype_specifier, "procedure",
                "function", "getchar", "printf", "sizeof", "return", "void",
                "for", "while", "if");
}

// L_BRACE> <COMPOUND_STATEMENT> <R_BRACE> | <L_BRACE> <R_BRACE>
bool Cst::parse_block() {
    if (t.type != L_BRACE) {
        return false;
    }
    advance_child();

    parse_compound();

    expect_child(R_BRACE);

    return true;
}

/*
return <EXPRESSION> <SEMICOLON>
return <SINGLE_QUOTED_STRING> <SEMICOLON>
return <DOUBLE_QUOTED_STRING> <SEMICOLON>
*/
bool Cst::parse_return() {
    if (t.type != IDENTIFIER || t.content != "return") {
        return false;
    }
    advance_child();

    if (!parse_expression()) {
        if (t.type != SINGLE_QUOTE && t.type != DOUBLE_QUOTE) {
            syntaxError("Functions can only return expressions or strings");
        }
        TokenType quoteType = t.type;
        advance_sibling();
        expect_sibling(STRING);
        expect_sibling(quoteType);
    }
    expect_sibling(SEMICOLON);
    return true;
}

bool Cst::parse_declaration() {
    if (!is_datatype_specifier(t) || tk->peek().type != IDENTIFIER) {
        return false;
    }

    advance_child();  // data type
    if (!parse_identifier_and_ident_arr_list()) {
        syntaxError("empty declarations not allowed");
    }
    expect_sibling(SEMICOLON);

    return true;
}

/*
<IDENTIFIER> <L_PAREN> <IDENTIFIER_AND_IDENTIFIER_ARRAY_PARAMETER_LIST> <R_PAREN>
<IDENTIFIER> <L_PAREN> <EXPRESSION> <R_PAREN>
*/
bool Cst::parse_call() {
    if (t.type != IDENTIFIER || tk->peek().type != L_PAREN) {
        return false;
    }

    advance_sibling();
    advance_sibling();

    if (!parse_identifier_and_ident_arr_param_list() && !parse_expression()) {
        syntaxError("Expected param list or expression in function call");
    }

    expect_sibling(R_PAREN);

    return true;
}

/*
<IDENTIFIER> <L_PAREN> <IDENTIFIER_AND_IDENTIFIER_ARRAY_PARAMETER_LIST> <R_PAREN>
<IDENTIFIER> <L_PAREN> <EXPRESSION> <R_PAREN>
*/
bool Cst::parse_call_statement() {
    if (t.type != IDENTIFIER || tk->peek().type != L_PAREN) {
        return false;
    }
    advance_child();
    advance_sibling();
    parse_identifier_and_ident_arr_param_list() || parse_expression();
    expect_sibling(R_PAREN);
    expect_sibling(SEMICOLON);
    return true;
}

bool Cst::parse_sizeof() {
    if (t.type != IDENTIFIER || t.content != "sizeof") {
        return false;
    }
    advance_child();
    expect_sibling(L_PAREN);
    expect_sibling(IDENTIFIER);
    expect_sibling(R_PAREN);
    return true;
}

bool Cst::parse_getchar() {
    if (t.type != IDENTIFIER || t.content != "getchar") {
        return false;
    }
    advance_child();
    expect_sibling(L_PAREN);
    expect("void", "getchar must be called with argument 'void'");
    expect_sibling(IDENTIFIER);
    expect_sibling(R_PAREN);
    return true;
}

/*
printf <L_PAREN> <DOUBLE_QUOTED_STRING> <R_PAREN> <SEMICOLON>
printf <L_PAREN> <SINGLE_QUOTED_STRING> <R_PAREN> <SEMICOLON>
printf <L_PAREN> <DOUBLE_QUOTED_STRING> <COMMA> <IDENTIFIER_AND_IDENTIFIER_ARRAY_PARAMETER_LIST> <R_PAREN> <SEMICOLON>
printf <L_PAREN> <SINGLE_QUOTED_STRING> <COMMA> <IDENTIFIER_AND_IDENTIFIER_ARRAY_PARAMETER_LIST> <R_PAREN> <SEMICOLON>
*/
bool Cst::parse_printf() {
    if (t.type != IDENTIFIER || t.content != "printf") {
        return false;
    }

    advance_child();  // printf
    expect_sibling(L_PAREN);

    if ((t.type == SINGLE_QUOTE || t.type == DOUBLE_QUOTE) && tk->peek().type == STRING) {
        auto quoteType = t.type;
        advance_sibling();          // quote
        advance_sibling();          // string
        expect_sibling(quoteType);  // end quote
    } else {
        syntaxError("printf statement must have a format string");
    }

    if (t.type == COMMA) {
        advance_sibling();
        parse_identifier_and_ident_arr_param_list();
    }

    expect_sibling(R_PAREN);
    expect_sibling(SEMICOLON);

    return true;
}

bool Cst::parse_assignment() {
    if (t.type != IDENTIFIER || is_datatype_specifier(t)) {
        return false;
    }

    expect(not_reserved_word, "Cannot assign to reserved word {}", t.content);
    advance_child();

    if (t.type == L_BRACKET) {
        advance_sibling();

        if (!parse_numerical_expression()) {
            syntaxError("Invalid array index");
        }
        expect_sibling(R_BRACKET);
    }

    expect_sibling(ASSIGNMENT_OPERATOR);
    if ((t.type == SINGLE_QUOTE || t.type == DOUBLE_QUOTE) && tk->peek().type == STRING) {
        auto quoteType = t.type;
        advance_sibling();          // quote
        expect_sibling(STRING);     // escaped char
        expect_sibling(quoteType);  // end quote
    } else if (parse_expression()) {
    } else if (t.type == IDENTIFIER) {
        expect(not_reserved_word, "attempted to take value of reserved word {}", t.content);
        advance_sibling();
    } else if ((t.type == SINGLE_QUOTE || t.type == DOUBLE_QUOTE) && tk->peek().type == ESCAPED_CHARACTER) {
        auto quoteType = t.type;
        advance_sibling();          // quote
        advance_sibling();          // escaped char
        expect_sibling(quoteType);  // end quote
    } else {
        syntaxError("invalid assignment");
    }

    expect_sibling(SEMICOLON);

    return true;
}

/*
for     <L_PAREN> <INITIALIZATION_EXPRESSION> <SEMICOLON> <BOOLEAN_EXPRESSION> <SEMICOLON> <ITERATION_ASSIGNMENT> <R_PAREN> <STATEMENT>
for     <L_PAREN> <INITIALIZATION_EXPRESSION> <SEMICOLON> <BOOLEAN_EXPRESSION> <SEMICOLON> <ITERATION_ASSIGNMENT> <R_PAREN> <BLOCK_STATEMENT>
while   <L_PAREN> <BOOLEAN_EXPRESSION> <R_PAREN> <STATEMENT>
while   <L_PAREN> <BOOLEAN_EXPRESSION> <R_PAREN> <BLOCK_STATEMENT>
*/
bool Cst::parse_iteration() {
    if (t.type != IDENTIFIER) {
        return false;
    }

    if (t.content == "for") {
        advance_child();
        expect_sibling(L_PAREN);
        parse_initialization();
        expect_sibling(SEMICOLON);
        parse_boolean_expression();
        expect_sibling(SEMICOLON);
        parse_iteration_assignment();
        expect_sibling(R_PAREN);
        if (!parse_block() && !parse_statement()) {
            syntaxError("Expected expression or block after for loop");
        }
    } else if (t.content == "while") {
        advance_child();
        expect_sibling(L_PAREN);
        parse_boolean_expression();
        expect_sibling(R_PAREN);
        if (!parse_block() && !parse_statement()) {
            syntaxError("Expected expression or block after while loop");
        }
    } else {
        return false;
    }

    return true;
}

/*
if <L_PAREN> <BOOLEAN_EXPRESSION> <R_PAREN> <STATEMENT> |
if <L_PAREN> <BOOLEAN_EXPRESSION> <R_PAREN> <BLOCK_STATEMENT> |

if <L_PAREN> <BOOLEAN_EXPRESSION> <R_PAREN> <STATEMENT> else <STATEMENT> |
if <L_PAREN> <BOOLEAN_EXPRESSION> <R_PAREN> <STATEMENT> else <BLOCK_STATEMENT>

if <L_PAREN> <BOOLEAN_EXPRESSION> <R_PAREN> <BLOCK_STATEMENT> else <STATEMENT> |
if <L_PAREN> <BOOLEAN_EXPRESSION> <R_PAREN> <BLOCK_STATEMENT> else <BLOCK_STATEMENT> |

*/
bool Cst::parse_selection() {
    if (t.type != IDENTIFIER || t.content != "if") {
        return false;
    }

    advance_child();
    expect_sibling(L_PAREN);

    if (!parse_boolean_expression()) {
        syntaxError("Expected boolean expression in 'if' statement");
    }

    expect_sibling(R_PAREN);

    if (!parse_block() && !parse_statement()) {
        syntaxError("Expected statement in 'if' block");
    }

    if (t.type != IDENTIFIER || t.content != "else") {
        return true;
    }

    advance_child();  // else

    if (!parse_block() && !parse_statement()) {
        syntaxError("Expected statement in 'else' block");
    }

    return true;
}

/*
<IDENTIFIER> <ASSIGNMENT_OPERATOR> <EXPRESSION>
<IDENTIFIER> <ASSIGNMENT_OPERATOR> <SINGLE_QUOTED_STRING>
<IDENTIFIER> <ASSIGNMENT_OPERATOR> <DOUBLE_QUOTED_STRING>
*/
bool Cst::parse_iteration_assignment() {
    return parse_initialization();
}

/*
<IDENTIFIER> <ASSIGNMENT_OPERATOR> <EXPRESSION>
<IDENTIFIER> <ASSIGNMENT_OPERATOR> <SINGLE_QUOTED_STRING>
<IDENTIFIER> <ASSIGNMENT_OPERATOR> <DOUBLE_QUOTED_STRING>
*/
bool Cst::parse_initialization() {
    if (t.type != IDENTIFIER || tk->peek().type != ASSIGNMENT_OPERATOR) {
        return false;
    }

    advance_sibling();
    advance_sibling();

    if (!parse_expression()) {
        if ((t.type == SINGLE_QUOTE || t.type == DOUBLE_QUOTE) && tk->peek().type == STRING) {
            TokenType quoteType = t.type;
            advance_sibling();          // quote
            advance_sibling();          // string
            expect_sibling(quoteType);  // end quote
        } else {
            syntaxError("iteration assignment value must be string or expression");
        }
    }

    return true;
}

bool Cst::parse_expression() {
    // we need to strip the parenthesis since (x + y) and (x + y < 4) dead end
    //     if you try to parse the first as boolean or the second as numeric
    bool parenthesised = false;
    if (t.type == L_PAREN) {
        parenthesised = true;
        advance_sibling();  // '('
    }

    // potential shared prefix
    if (parse_numerical_expression()) {
        if (parenthesised && t.type == R_PAREN) {
            advance_sibling();
            parenthesised = false;
        }
        if (parse_relational_expression()) {
            if (!parse_numerical_expression()) {
                syntaxError("Expected numerical expression on rhs of relational operator");
            }
            if (parenthesised && t.type == R_PAREN) {
                advance_sibling();
                parenthesised = false;
            }
            // <num expr> <rel op> <num expr>
            if (is_boolean_operator(t)) {
                if (!parse_boolean_expression()) {
                    syntaxError("Expected boolean expression after boolean operator");
                }
                // <num expr> <rel op> <num expr> <bool op> <bool expr>
            }
        }
        if (parenthesised) {
            expect_sibling(R_PAREN);
        }
    } else {
        if (!parenthesised) {
            return parse_boolean_expression();
        }
        if (!parse_boolean_expression()) {
            syntaxError("Expected expression in parenthesis");
        }
        expect_sibling(R_PAREN);

        if (is_boolean_operator(t)) {
            advance_sibling();
            if (!parse_boolean_expression()) {
                syntaxError("Expected boolean expression after boolean operator");
            }
        }
    }

    return true;
}

/*
                        <NUMERICAL_OPERAND> |
<L_PAREN> 				<NUMERICAL_OPERAND> 	<R_PAREN> |
                        <NUMERICAL_OPERAND> 	<NUMERICAL_OPERATOR> 	                <NUMERICAL_EXPRESSION> |
<L_PAREN> 				<NUMERICAL_OPERAND> 	<NUMERICAL_OPERATOR> 	                <NUMERICAL_EXPRESSION> <R_PAREN> |
                        <NUMERICAL_OPERAND> 	<NUMERICAL_OPERATOR> 	<L_PAREN> 		<NUMERICAL_EXPRESSION> <R_PAREN> <NUMERICAL_OPERATOR> <NUMERICAL_EXPRESSION> |
<L_PAREN> 				<NUMERICAL_OPERAND> 	<NUMERICAL_OPERATOR> 	                <NUMERICAL_EXPRESSION> <R_PAREN> |
                        <NUMERICAL_OPERAND> 	<NUMERICAL_OPERATOR> 	<L_PAREN> 		<NUMERICAL_EXPRESSION> <R_PAREN> |
<L_PAREN> 				<NUMERICAL_OPERAND> 	<NUMERICAL_OPERATOR> 	                <NUMERICAL_EXPRESSION> <R_PAREN> <NUMERICAL_OPERATOR> <NUMERICAL_EXPRESSION> |
                        <NUMERICAL_OPERATOR> 	<NUMERICAL_EXPRESSION>
*/
bool Cst::parse_numerical_expression() {
    if (in_boolean_prefix()) {
        return false;
    }

    if (is_numerical_operator(t)) {
        advance_sibling();
        if (!parse_numerical_expression()) {
            syntaxError("Expected numerical expression after numerical operator");
        }
    } else {
        bool expecting_parenthesis = false;
        if (t.type == L_PAREN) {
            expecting_parenthesis = true;
            advance_sibling();
        }

        if (!parse_numerical_operand()) {
            if (expecting_parenthesis) {
                syntaxError("Expected numerical operand in numerical expression");
            } else {
                return false;
            }
        }

        if (t.type == R_PAREN) {
            if (!expecting_parenthesis) {
                return true;
            }
            expecting_parenthesis = false;
            advance_sibling();

        } else {
            if (!is_numerical_operator(t)) {
                if (expecting_parenthesis) {
                    syntaxError("failed to find closing parenthesis in numerical expression");
                }
                // just a single operand
                return true;
            }

            advance_sibling();

            if (t.type == L_PAREN) {
                if (expecting_parenthesis) {
                    syntaxError("Unexpected L_PAREN");
                }
                expecting_parenthesis = true;
                advance_sibling();
            }

            if (!parse_numerical_expression()) {
                syntaxError("Expected numerical expression");
            }

            if (expecting_parenthesis) {
                expect_sibling(R_PAREN);
                expecting_parenthesis = false;
            }

            if (is_numerical_operator(t)) {
                advance_sibling();

                if (!parse_numerical_expression()) {
                    syntaxError("Expected numerical expression");
                }
            }
        }

        if (expecting_parenthesis) {
            expect_sibling(R_PAREN);
            expecting_parenthesis = false;
        }
    }

    return true;
}

/*
<BOOLEAN_TRUE> | <BOOLEAN_FALSE> | <IDENTIFIER> |
<IDENTIFIER> <BOOLEAN_OPERATOR> <BOOLEAN_EXPRESSION> |
<BOOLEAN_NOT> <BOOLEAN_TRUE> |
<BOOLEAN_NOT> <BOOLEAN_FALSE> |
<BOOLEAN_NOT> <IDENTIFIER> |
<USER_DEFINED_FUNCTION> |
<USER_DEFINED_FUNCTION> <BOOLEAN_OPERATOR> <BOOLEAN_EXPRESSION> |
<BOOLEAN_NOT> <USER_DEFINED_FUNCTION> |
<L_PAREN> <USER_DEFINED_FUNCTION> <R_PAREN> |
<L_PAREN> <IDENTIFIER> <BOOLEAN_OPERATOR> <BOOLEAN_EXPRESSION> <R_PAREN> |
<NUMERICAL_EXPRESSION> <RELATIONAL_EXPRESSION> <NUMERICAL_EXPRESSION> |
<L_PAREN> <NUMERICAL_OPERAND> <RELATIONAL_EXPRESSION> <NUMERICAL_OPERAND> <R_PAREN> |
<L_PAREN> <NUMERICAL_OPERAND> <RELATIONAL_EXPRESSION> <NUMERICAL_OPERAND> <R_PAREN> <BOOLEAN_OPERATOR> <BOOLEAN_EXPRESSION> |
<L_PAREN> <BOOLEAN_NOT> <IDENTIFIER> <R_PAREN> |
<L_PAREN> <BOOLEAN_NOT> <IDENTIFIER> <BOOLEAN_OPERATOR> <BOOLEAN_EXPRESSION> <R_PAREN> |
<L_PAREN> <BOOLEAN_NOT> <USER_DEFINED_FUNCTION> <R_PAREN> |
<L_PAREN> <BOOLEAN_NOT> <USER_DEFINED_FUNCTION> <R_PAREN> <BOOLEAN_OPERATOR> <BOOLEAN_EXPRESSION> |
<L_PAREN> <NUMERICAL_OPERAND> <RELATIONAL_EXPRESSION> <NUMERICAL_EXPRESSION> <R_PAREN>
*/
bool Cst::parse_boolean_expression() {
    bool parenthesized = false;

    if (t.type == INTEGER && !is_relational_expression(tk->peek())) {
        return false;
    }

    if (t.type == L_PAREN && tk->peek().type == IDENTIFIER) {
        parenthesized = true;
        advance_sibling();

        if (is_boolean_operator(tk->peek())) {
            advance_sibling();
            parse_boolean_expression();
            expect_sibling(R_PAREN);
            return true;
        } else if (parse_call()) {
            expect_sibling(R_PAREN);
            return true;
        }
    }

    if (parenthesized && t.type == IDENTIFIER && is_relational_expression(tk->peek())) {
        // parse as
        // <L_PAREN> <NUMERICAL_OPERAND> <RELATIONAL_EXPRESSION> <NUMERICAL_OPERAND> <R_PAREN>
        // <L_PAREN> <NUMERICAL_OPERAND> <RELATIONAL_EXPRESSION> <NUMERICAL_OPERAND> <R_PAREN> <BOOLEAN_OPERATOR> <BOOLEAN_EXPRESSION>

        parse_numerical_operand();
        parse_relational_expression();
        parse_numerical_operand();

        expect_sibling(R_PAREN);

        if (is_boolean_operator(t)) {
            advance_sibling();
            parse_boolean_expression();
        }

        return true;
    }

    if (parse_numerical_expression()) {
        if (parenthesized && t.type == R_PAREN) {
            advance_sibling();
            parenthesized = false;
        }
        expect(is_relational_expression, "Expected relational operator after numerical expression, found {}", t.content);
        advance_sibling();
        if (!parse_numerical_expression()) {
            syntaxError("Expected numerical expression after relational operator");
        }
        if (parenthesized) {
            expect_sibling(R_PAREN);
        }
        return true;
    }

    if (t.type == L_PAREN) {
        parenthesized = true;
        advance_sibling();
    }

    if (t.type == BOOLEAN_NOT) {
        advance_sibling();
    }

    if (is_boolean_literal(t)) {
        advance_sibling();
        if (parenthesized) {
            expect_sibling(R_PAREN);
        }

        return true;
    }

    if (parse_call()) {
        if (parenthesized) {
            expect_sibling(R_PAREN);
        }

        if (is_boolean_operator(t)) {
            advance_sibling();
            if (!parse_boolean_expression()) {
                syntaxError("Expected boolean expression");
            }
        }

        return true;
    }

    if (t.type == IDENTIFIER) {
        advance_sibling();

        if (is_boolean_operator(t)) {
            advance_sibling();

            if (!parse_boolean_expression()) {
                syntaxError("Expected boolean expression");
            }
        }

        if (parenthesized) {
            expect_sibling(R_PAREN);
        }

        return true;
    }

    if (parenthesized && parse_numerical_operand()) {
        expect(is_relational_expression, "Expected relational operator after numerical operand, found {}", t.content);
        advance_sibling();

        if (parse_numerical_operand()) {
            expect_sibling(R_PAREN);

            // <BOOLEAN_OPERATOR> <BOOLEAN_EXPRESSION>
            if (is_boolean_operator(t)) {
                advance_sibling();

                if (!parse_boolean_expression()) {
                    syntaxError("expected boolean expression after boolean operator");
                }
            }

        } else if (parse_numerical_expression()) {
            expect_sibling(R_PAREN);
        } else {
            syntaxError("Expected numerical value after numerical operand + relational expression");
        }

        return true;
    }

    return false;
}

bool Cst::parse_relational_expression() {
    if (is_relational_expression(t)) {
        advance_sibling();
        return true;
    }
    return false;
}

/*
<IDENTIFIER> |
<INTEGER> |
<IDENTIFIER> <L_BRACKET> <NUMERICAL EXPRESSION> <R_BRACKET> |
<GETCHAR_FUNCTION> |
<USER_DEFINED_FUNCTION> |
<SINGLE_QUOTE> <CHARACTER> <SINGLE_QUOTE> |
<SINGLE_QUOTE> <ESCAPED_CHARACTER> <SINGLE_QUOTE> |
<DOUBLE_QUOTE> <CHARACTER> <DOUBLE_QUOTE> |
<DOUBLE_QUOTE> <ESCAPED_CHARACTER> <DOUBLE_QUOTE> |
<SIZEOF_FUNCTION>
*/
bool Cst::parse_numerical_operand() {
    bool basic = parse_first_accepted({&Cst::parse_getchar,
                                       &Cst::parse_sizeof,
                                       &Cst::parse_call});

    if (basic) {
        return true;
    }

    if (is_boolean_literal(t)) {
        return false;
    }

    if (any(t, INTEGER, IDENTIFIER)) {
        advance_sibling();
        if (t.type == L_BRACKET) {
            advance_sibling();
            if (!parse_numerical_expression()) {
                syntaxError("invalid array index");
            }
            expect_sibling(R_BRACKET);
        }
        return true;
    } else if (any(t, SINGLE_QUOTE, DOUBLE_QUOTE)) {
        TokenType quote_type = t.type;

        advance_sibling();

        if (!any(t, ESCAPED_CHARACTER, CHARACTER)) {
            if (t.type == END) {
                syntaxError("unterminated string quote.");
            } else {
                syntaxError("expected character or escaped character in quotes");
            }
        }

        advance_sibling();

        expect_sibling(quote_type);
        return true;
    }

    return false;
}

/*
<IDENTIFIER>
<IDENTIFIER> <L_BRACKET> <IDENTIFIER> <R_BRACKET>
<IDENTIFIER> <COMMA> <IDENTIFIER_AND_IDENTIFIER_ARRAY_PARAMETER_LIST>
<IDENTIFIER> <L_BRACKET> <IDENTIFIER> <R_BRACKET> <COMMA> <IDENTIFIER_AND_IDENTIFIER_ARRAY_PARAMETER_LIST>
<IDENTIFIER> <L_BRACKET> <NUMERICAL_EXPRESSION> <R_BRACKET>
<IDENTIFIER> <L_BRACKET> <NUMERICAL_EXPRESSION> <R_BRACKET> <COMMA> <IDENTIFIER_AND_IDENTIFIER_ARRAY_PARAMETER_LIST>
*/
bool Cst::parse_identifier_and_ident_arr_param_list() {
    if (t.type != IDENTIFIER) {
        return false;
    }
    advance_sibling();

    if (t.type == L_BRACKET) {
        advance_sibling();  // l bracket
        if (t.type == IDENTIFIER) {
            advance_sibling();  // ident
        } else if (!parse_numerical_expression()) {
            syntaxError("Invalid array index in parameter list");
        }
        expect_sibling(R_BRACKET);
    }

    if (t.type != COMMA) {
        return true;
    }

    advance_sibling();  // comma

    parse_identifier_and_ident_arr_param_list();

    return true;
}

bool Cst::parse_identifier_and_ident_arr_list() {
    if (t.type != IDENTIFIER) {
        return false;
    }
    expect(not_reserved_word, "reserved word \"{}\" cannot be used for the name of a variable.", t.content);
    advance_sibling();  // ident
    if (t.type == L_BRACKET) {
        advance_sibling();
        if (!t.content.empty() && t.content[0] == '-') {
            syntaxError("array declaration size must be a positive integer.");
        }
        expect_sibling(INTEGER);
        expect_sibling(R_BRACKET);
    }

    if (t.type == COMMA) {
        advance_sibling();
        parse_identifier_and_ident_arr_list();
    }

    return true;
}

bool Cst::parse_statement() {
    return parse_first_accepted({
        &Cst::parse_declaration,
        &Cst::parse_printf,
        &Cst::parse_selection,
        &Cst::parse_iteration,
        &Cst::parse_return,
        &Cst::parse_call_statement,
        &Cst::parse_assignment,
    });
}

bool Cst::parse_compound() {
    // std::cout << "compound start on line " << tk->getLine() << " with " << t.content << "\n";
    while (parse_statement());
    // std::cout << "compound end on line " << tk->getLine() << " with " << t.content << "\n";
    return true;
}

bool Cst::parse_parameter_decl() {
    if (!is_datatype_specifier(t)) {
        return false;
    }

    advance_sibling();

    expect(not_reserved_word, "reserved word \"{}\" cannot be used for the name of a variable.", t.content);
    expect_sibling(IDENTIFIER);

    if (t.type == L_BRACKET) {
        advance_sibling();
        expect_sibling(INTEGER);
        expect_sibling(R_BRACKET);
    }

    return true;
}

bool Cst::parse_parameters() {
    if (!is_datatype_specifier(t)) {
        return false;
    }

    while (t.type != END) {
        if (!parse_parameter_decl()) {
            syntaxError("Invalid parameter list");
        }

        if (t.type == R_PAREN) {
            break;
        }

        expect_sibling(COMMA);
    }

    return true;
}

bool Cst::parse_procedure() {
    if (t.content != "procedure") {
        return false;
    }

    advance_child();  // procedure

    expect(not_reserved_word, "reserved word \"{}\" cannot be the name of a procedure", t.content);
    expect_sibling(IDENTIFIER);  // name

    expect_sibling(L_PAREN);

    if (t.content == "void") {
        advance_sibling();
    } else {
        if (!parse_parameters()) {
            syntaxError("Invalid procedure parameter list");
        }
    }

    expect_sibling(R_PAREN);

    expect_child(L_BRACE);

    parse_compound();

    expect_child(R_BRACE);

    return true;
}

bool Cst::parse_function() {
    if (t.content != "function") {
        return false;
    }

    advance_child();  // function

    expect(is_datatype_specifier, "Expected datatype specifier after function declaration");
    advance_sibling();  // return type

    expect(not_reserved_word, "reserved word \"{}\" cannot be used for the name of a function.", t.content);
    expect_sibling(IDENTIFIER);  // name

    expect_sibling(L_PAREN);

    if (t.content == "void") {
        advance_sibling();
    } else {
        if (!parse_parameters()) {
            syntaxError("Empty parameter list not allowed");
        }
    }

    expect_sibling(R_PAREN);

    expect_child(L_BRACE);

    if (!parse_compound()) {
        syntaxError("Empty function body not allowed: must have a valid return");
    }

    expect_child(R_BRACE);

    return true;
}

bool Cst::parse_program_tail() {
    bool decl = parse_first_accepted({&Cst::parse_function,
                                      &Cst::parse_procedure,
                                      &Cst::parse_declaration});

    if (decl) {
        parse_program_tail();
    }

    return true;
}

bool Cst::parse_main() {
    if (t.content != "procedure" || tk->peek().content != "main") {
        return false;
    }
    advance_child();    // procedure
    advance_sibling();  // main

    expect_sibling(L_PAREN);
    expect("void", "Main procedure must have parameter type 'void', has {}", t.content);
    advance_sibling();

    expect_sibling(R_PAREN);

    if (!parse_block()) {
        syntaxError("Expected valid block statement following main declaration");
    }

    return true;
}

bool Cst::parse_program() {
    if (t.type != IDENTIFIER) {
        syntaxError("Programs should start with an identifier");
    }

    while (t.type != END) {
        if (parse_main()) {
            parse_program_tail();
            break;
        }

        bool decl = parse_first_accepted({&Cst::parse_function,
                                          &Cst::parse_procedure,
                                          &Cst::parse_declaration});

        if (!decl) {
            break;
        }
    }
    expect(END, "There should be no content after the end of the program");

    return true;
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
    t = tk->next();
    parse_program();
}

void printCstNode(CstNode* n) {
    if (!n) {
        std::cout << "NULL NODE POINTER!!";
        return;
    }
    while (n) {
        if (!n->sib && !n->child) {
            std::cout << n->t.content << "\n";
            break;
        }
        while (n->sib) {
            std::cout << n->t.content << "   ";
            n = n->sib;
        }
        std::cout << n->t.content << "\n";

        if (n->child) {
            n = n->child;
        }
    }
}

void Cst::print() {
    printCstNode(root->child);
}