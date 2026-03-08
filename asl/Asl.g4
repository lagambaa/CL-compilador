//////////////////////////////////////////////////////////////////////
//
//    Asl - Another simple language (grammar)
//
//    Copyright (C) 2020-2030  Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public License
//    as published by the Free Software Foundation; either version 3
//    of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: José Miguel Rivero (rivero@cs.upc.edu)
//             Computer Science Department
//             Universitat Politecnica de Catalunya
//             despatx Omega.110 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
//////////////////////////////////////////////////////////////////////

/*
> dubtes:
        - mod te prioritat o no sobremul/div?
        - els arrays han de ser N-dim?
        - modulo sobre enters nomes o sobre floats tb?
        - array [size=0] of .. es correcte? o ha de ser size > 0?

> problemes:
        - (SOLUCIONAT) arreglar la propagacio de errorTy (al principi asumies tipus)

> coses que puc afegir per practicar:
        - arrays n-dim
        - strings
        - else if / elif
        - structs
        - classes
*/



grammar Asl;

//////////////////////////////////////////////////
/// Parser Rules
//////////////////////////////////////////////////

// A program is a list of functions
program : function+ EOF
        ;

/* A function has:
        - a name
        - a list of parameters (can be empty)
        - a list of statements (can be empty?)
        - and at most 1 return type (basic_type) (can be empty -> void)
*/
function
        : FUNC ID OPAR parameters? CPAR (COLON basic_type)? declarations statements ENDFUNC
        ;

parameters
        : parameter (SEP parameter)*
        ;

parameter
        : ID COLON type
        ;

args    
        : expr (SEP expr)*
        ;

declarations
        : (variable_decl)*
        ;

variable_decl
        : VAR ID (SEP ID)* COLON type
        ;

basic_type
         : INT
         | BOOL
         | CHAR
         | FLOAT
         ;

type    : basic_type
        | ARRAY OCOR INTVAL CCOR OF basic_type
        ;

statements
        : (statement)*
        ;

// The different types of instructions
statement
          // return 
        : RET expr? SEMI                      # returnStmt
          // Assignment
        | left_expr ASSIGN expr SEMI          # assignStmt
          // if-then-else statement (else is optional)
        | IF expr THEN statements (ELSE statements)? ENDIF       # ifStmt
          // while statement
        | WHILE expr DO statements ENDWHILE   # whileStmt
          // A function/procedure call has a list of arguments in parenthesis (possibly empty)
        | ident OPAR args? CPAR SEMI          # procCall
          // Read a variable
        | READ left_expr SEMI                 # readStmt
          // Write an expression
        | WRITE expr SEMI                     # writeExpr
          // Write a string
        | WRITE STRING SEMI                   # writeString
        ;

// Grammar for left expressions (l-values in C++)
left_expr
        : ident OCOR expr CCOR                # leftArrayAccess
        | ident                               # leftIdent
        ;

// Grammar for expressions with boolean, relational and aritmetic operators
expr    : op=(PLUS|SUB|NOT) expr              # negate
        | OPAR expr CPAR                      # parenthesis
        | expr op=(MUL|DIV|MOD) expr          # arithmetic 
        | expr op=(PLUS|SUB) expr             # arithmetic
        | expr op=(EQUAL|LT|GT|LE|GE|NE) expr # relational
        | expr op=(AND|OR|XOR) expr           # logical
        | ident OCOR expr CCOR                # arrayAccess
        // procCall no fa servir el return value pero functionCall si
        | ident OPAR args? CPAR               # functionCall
        | valueExpr                           # value
        | ident                               # exprIdent
        ;

// values
valueExpr
        : INTVAL 
        | FLOATVAL 
        | BOOLVAL 
        | CHARVAL
        ; 

cmpOp
        : 
        ;

// Identifiers
ident   : ID
        ;

//////////////////////////////////////////////////
/// Lexer Rules
//////////////////////////////////////////////////

SEP       : ',' ;
SEMI      : ';' ;
COLON     : ':' ;
OPAR      : '(' ;
CPAR      : ')' ;
OCOR      : '[' ;
CCOR      : ']' ;
SUB       : '-' ;
DIV       : '/' ;
NOT       : 'not' | '!' ;
ASSIGN    : '=' ;
EQUAL     : '==' ;
LT        : '<' ;
GT        : '>' ;
LE        : '<=' ;
GE        : '>=' ;
NE        : '!=' ;
AND       : 'and' | '&&' ;
OR        : 'or' | '||' ;
XOR       : 'xor' | '^^' ;
PLUS      : '+' ;
MUL       : '*';
MOD       : '%';
VAR       : 'var';
INT       : 'int';
BOOL      : 'bool' ;
CHAR      : 'char' ;
FLOAT     : 'float' ;
IF        : 'if' ;
THEN      : 'then' ;
ELSE      : 'else' ;
ENDIF     : 'endif' ;
WHILE     : 'while' ;
DO        : 'do' ;
ENDWHILE  : 'endwhile' ;
FUNC      : 'func' ;
ENDFUNC   : 'endfunc' ;
RET       : 'return' ;
READ      : 'read' ;
WRITE     : 'write' ;
BOOLVAL   : 'true' | 'false' ;
ARRAY     : 'array' ;
OF        : 'of' ;
ID        : ('a'..'z'|'A'..'Z') ('a'..'z'|'A'..'Z'|'_'|'0'..'9')* ;
INTVAL    : DIGIT+ ;
FLOATVAL  : (DIGIT+ '.' DIGIT*) | (DIGIT* '.' DIGIT+) ;
CHARVAL   :  '\'' CHARACTER '\'';

// Strings (in quotes) with escape sequences
STRING    : '"' CHARACTER* '"' ;

fragment
ESC_SEQ   : '\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\') ;
fragment
DIGIT     : ('0'..'9') ;
fragment
CHARACTER : ESC_SEQ | ~('\''|'\\') ;

// Comments (inline C++-style and multiline C-style)
COMMENT   : ( '//' ~('\n'|'\r')* '\r'? '\n'
            | '/*' .*? '*/'
            ) -> skip
          ;

// White spaces
WS        : (' '|'\t'|'\r'|'\n')+ -> skip ;
// Alternative description
// WS        : [ \t\r\n]+ -> skip ;
