grammar kaleidoscope;

/* ------------------------------------------------------------------ */

program
    : statement_list SEMI? EOF
    ;

statement_list
    : stmt stmt_continue
    ;

stmt_continue
    : (SEMI stmt)*
    ;

stmt
    : function_decl
    | expression
    ;

// Правила парсера с приоритетами
expression
    : term expr_continue
    ;

expr_continue
    : (infix_op term)*
    ;

infix_op : INFIX_OP;

term : prefix_expression;

// Префиксные (средний приоритет)
prefix_expression
    : PREFIX_OP postfix_expression
    | postfix_expression
    ;

// Постфиксные (высший приоритет)
postfix_expression
    : primary_expression POSTFIX_OP?
    ;

primary_expression
    : identifier
    | number
    | LPAREN expression RPAREN
    ;

identifier : IDENTIFIER;
number     : NUMBER;

function_decl
    : FUNCTION IDENTIFIER LPAREN RPAREN (COLON IDENTIFIER)?
    ;

/* ------------------------------------------------------------------ */

// Комментарии должны быть ВЫШЕ операторов!
// SINGLE_LINE_COMMENT : '#' ~[\r\n]* -> skip;
WS                  : [ \t\r\n]+ -> skip;

// Ключевые слова
FUNCTION   : 'function';
DISCARD    : 'discard';

// Символы
LPAREN     : '(';
RPAREN     : ')';
COLON      : ':';
EQUALS     : '=';
SEMI       : ';';

// Разные типы операторов - ИСКЛЮЧАЕМ '#' из операторов!
OP         : [-+]+;  // убрали '#' из списка
PREFIX_OP  : OP;
POSTFIX_OP : OP;
INFIX_OP   : OP;

NUMBER     : [0-9]+;
IDENTIFIER : [_a-zA-Z][a-zA-Z_0-9]*;
