grammar kaleidoscope;

chunk
    : stmt_list EOF
    ;

stmt_list
    : stmt (';' stmt)* ';'?
    ;

stmt
    : fn_decl
    | expr
    |
    ;

/* ------------------------------------------------------------------ */

fn_decl
    : FN ID '(' param_list? ')' ':' ID fn_block?
    ;

param_list
    : param (',' param)*
    ;

param
    : ID type_expr?
    ;

type_expr
    : ':' ID
    ;

fn_block
    : fn_body
    ;

fn_body
    : BLOCK_START (fn_stmt_list) BLOCK_END
    ;

fn_stmt_list
    : fn_stmt (';' fn_stmt)* ';'?
    ;

fn_stmt
    : ret_stmt
    | stmt
    |
    ;

ret_stmt
    : RET expr
    ;

/* ------------------------------------------------------------------ */

expr
    : term (INFIX_OP term)*
    ;

term
    : expr_prefix
    ;

expr_prefix
    : (OP? expr_postfix)
    ;

expr_postfix
    : expr_primary OP?
    ;

expr_primary
    : NUMBER
    | expr_call
    | ID
    | '(' expr ')'
    ;

expr_call
    : ID '(' arg_list? ')'
    ;

arg_list
    : expr (',' expr)*
    ;

/* ================================================================== */

FN          : 'function';
BLOCK_START : 'do';
BLOCK_END   : 'end';
RET         : 'return';

ID       : [_a-zA-Z][_0-9a-zA-Z]*;
INFIX_OP : [ \t\n\r]OP[ \t\n\r];
OP       : [-*/%&^$@!~`'+]+;
NUMBER   : [0-9];

COMMENT : '#' ~[\n\r]* -> skip;
WS      : [ \t\n\r] -> skip;

//NEWLINE: '\r'? '\n';
//IDENT_HALF: NEWLINE '  ';
//IDENT: IDENT_HALF '  ';
