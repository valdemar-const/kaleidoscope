grammar kaleidoscope;

chunk
    : stmt_list EOF
    ;

stmt_list
    : stmt stmt_continue ';'?
    ;

stmt_continue
    : (';' stmt)*
    ;

stmt
    : fn_decl
    | expr
    |
    ;

/* ------------------------------------------------------------------ */

fn_decl
    : FN ID '(' param_list ')' ':' ID fn_decl_continue
    ;

param_list
    : param param_list_continue
    |
    ;

param_list_continue
    : (',' param)*
    ;

param
    : ID type_expr
    ;

type_expr
    : ':' ID
    |
    ;

fn_decl_continue
    : fn_block?
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
    : term expr_continue
    ;

expr_continue
    : (INFIX_OP term)*
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
    : ID '(' arg_list ')'
    ;

arg_list
    : expr arg_list_continue
    |
    ;

arg_list_continue
    : (',' expr)*
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
