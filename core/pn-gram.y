//
// pn-gram.y
// the parser for Potion
//
// (c) 2008 why the lucky stiff, the freelance professor
//
%include {
#define POTION_GRAMMAR_C
#include <assert.h>
#include "potion.h"
#include "internal.h"
#include "pn-ast.h"
#include "pn-gram.h"
#define TOK_NEED(t1, t2) \
  if (x == PN_TOK_##t1) { \
    yymajor = PN_TOK_##t2; \
    P->yerror = PN_TOK_MISSING | PN_TOK_##t2; \
    break; \
  }
}
%name LemonPotion
%extra_argument { Potion *P }
%token_type { PN }
%type arg { PNArg }
%type closure { PNArg }
%token_prefix PN_TOK_
%token_destructor { if (PN_IS_PTR($$)) { P->xast++; } }

%syntax_error {
  P->yerror = yymajor;
  if (!yymajor) {
    int i = yypParser->yyidx - 1, x = 0;
    while (i > 0) {
      x = yypParser->yystack[i].major;
      TOK_NEED(BEGIN_LICK, END_LICK);
      TOK_NEED(BEGIN_TABLE, END_TABLE);
      TOK_NEED(BEGIN_BLOCK, END_BLOCK);
      i--;
    }
  }
  P->yerrname = (char *)yyTokenName[yymajor];
}

%left OR AND.
%right ASSIGN.
%left CMP EQ NEQ.
%left GT GTE LT LTE. 
%left PIPE CARET.
%left AMP.
%left BITL BITR.
%left PLUS MINUS.
%left TIMES DIV REM.
%right POW.
%nonassoc PPLUS MMINUS.
%right NOT. // WAVY.

potion(A) ::= all(B). { A = P->source = PN_AST(CODE, B); }

all(A) ::= statements(B). { A = B; }
all(A) ::= statements(B) SEP. { A = B; }

statements(A) ::= statements(B) SEP statement(C). { A = PN_PUSH(B, C); }
statements(A) ::= statement(B). { A = PN_TUP(B); }

statement(A) ::= expr(B). { A = PN_AST(EXPR, B); }
statement(A) ::= name(B) PPLUS. { A = PN_OP(AST_INC, B, PN_NUM(1)); }
statement(A) ::= name(B) MMINUS. { A = PN_OP(AST_INC, B, PN_NUM(-1)); }
statement(A) ::= PPLUS name(B). { A = PN_OP(AST_INC, B, PN_NUM(1) ^ 1); }
statement(A) ::= MMINUS name(B). { A = PN_OP(AST_INC, B, PN_NUM(-1) ^ 1); }
statement(A) ::= statement(B) ASSIGN statement(C). { A = PN_AST2(ASSIGN, B, C); }
statement(A) ::= MINUS statement(B). { A = PN_OP(AST_MINUS, PN_AST(VALUE, PN_ZERO), B); }
statement(A) ::= NOT statement(B). { A = PN_AST(NOT, B); }
statement(A) ::= statement(B) OR statement(D). { A = PN_OP(AST_OR, B, D); }
statement(A) ::= statement(B) OR ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_OR, B, D)); }
statement(A) ::= statement(B) AND statement(D). { A = PN_OP(AST_AND, B, D); }
statement(A) ::= statement(B) AND ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_AND, B, D)); }
statement(A) ::= statement(B) CMP statement(D). { A = PN_OP(AST_CMP, B, D); }
statement(A) ::= statement(B) EQ statement(D). { A = PN_OP(AST_EQ, B, D); }
statement(A) ::= statement(B) NEQ statement(D). { A = PN_OP(AST_NEQ, B, D); }
statement(A) ::= statement(B) GT statement(D). { A = PN_OP(AST_GT, B, D); }
statement(A) ::= statement(B) GTE statement(D). { A = PN_OP(AST_GTE, B, D); }
statement(A) ::= statement(B) LT statement(D). { A = PN_OP(AST_LT, B, D); }
statement(A) ::= statement(B) LTE statement(D). { A = PN_OP(AST_LTE, B, D); }
statement(A) ::= statement(B) PIPE statement(D). { A = PN_OP(AST_PIPE, B, D); }
statement(A) ::= statement(B) PIPE ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_PIPE, B, D)); }
statement(A) ::= statement(B) CARET statement(D). { A = PN_OP(AST_CARET, B, D); }
statement(A) ::= statement(B) CARET ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_CARET, B, D)); }
statement(A) ::= statement(B) AMP statement(D). { A = PN_OP(AST_AMP, B, D); }
statement(A) ::= statement(B) AMP ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_AMP, B, D)); }
statement(A) ::= statement(B) BITL statement(D). { A = PN_OP(AST_BITL, B, D); }
statement(A) ::= statement(B) BITL ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_BITL, B, D)); }
statement(A) ::= statement(B) BITR statement(D). { A = PN_OP(AST_BITR, B, D); }
statement(A) ::= statement(B) BITR ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_BITR, B, D)); }
statement(A) ::= statement(B) PLUS statement(D). { A = PN_OP(AST_PLUS, B, D); }
statement(A) ::= statement(B) PLUS ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_PLUS, B, D)); }
statement(A) ::= statement(B) MINUS statement(D). { A = PN_OP(AST_MINUS, B, D); }
statement(A) ::= statement(B) MINUS ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_MINUS, B, D)); }
statement(A) ::= statement(B) TIMES statement(D). { A = PN_OP(AST_TIMES, B, D); }
statement(A) ::= statement(B) TIMES ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_TIMES, B, D)); }
statement(A) ::= statement(B) DIV statement(D). { A = PN_OP(AST_DIV, B, D); }
statement(A) ::= statement(B) DIV ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_DIV, B, D)); }
statement(A) ::= statement(B) REM statement(D). { A = PN_OP(AST_REM, B, D); }
statement(A) ::= statement(B) REM ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_REM, B, D)); }
statement(A) ::= statement(B) POW statement(D). { A = PN_OP(AST_POW, B, D); }
statement(A) ::= statement(B) POW ASSIGN statement(D). { A = PN_AST2(ASSIGN, B, PN_OP(AST_POW, B, D)); }

expr(A) ::= expr(B) call(C). { A = PN_PUSH(B, C); }
expr(A) ::= closure(B). { A = PN_TUP(PN_AST2(PROTO, B.v, B.b)); }
expr(A) ::= table(B). { A = PN_TUP(B); }
expr(A) ::= value(B). { A = PN_TUP(B); }
expr(A) ::= value(B) arg(C). { PN_S(B, 1) = C.v; PN_S(B, 2) = C.b; A = PN_TUP(B); }
expr(A) ::= call(B). { A = PN_TUP(B); }

call(A) ::= name(B). { A = B; }
call(A) ::= name(B) arg(C). { PN_CLOSE(B); PN_S(B, 1) = C.v; PN_S(B, 2) = C.b; A = B; }

arg(A) ::= table(B) table(C) block(D). { PN_CLOSE(PN_S(C, 0));
  PN_PUSH(PN_S(B, 0), PN_AST(EXPR, PN_TUP(PN_AST2(PROTO, C, D))));
  A.v = B; A.b = PN_NIL; }
arg(A) ::= value(B) table(C) block(D). { PN_CLOSE(PN_S(C, 0));
  A.v = PN_AST(TABLE, PN_PUSH(PN_TUP(PN_AST(EXPR, PN_TUP(B))), PN_AST(EXPR, PN_TUP(PN_AST2(PROTO, C, D)))));
  A.b = PN_NIL; }
arg(A) ::= value(B) block(C). { A.v = B; A.b = C; }
arg(A) ::= table(B). { A.v = B; A.b = PN_NIL; }
arg(A) ::= value(B). { A.v = B; A.b = PN_NIL; }
arg(A) ::= closure(B). { A = B; }

closure(A) ::= table(B) block(C). { PN_CLOSE(PN_S(B, 0)); A.v = B; A.b = C; }
closure(A) ::= block(B). { A.v = PN_NIL; A.b = B; }

name(A) ::= MESSAGE(B). { A = PN_AST(MESSAGE, B); }
name(A) ::= QUERY(B). { A = PN_AST(QUERY, B); }
name(A) ::= PATH(B). { A = PN_AST(PATH, B); }
name(A) ::= PATHQ(B). { A = PN_AST(PATHQ, B); }

value(A) ::= NIL(B). { A = PN_AST(VALUE, B); }
value(A) ::= TRUE(B). { A = PN_AST(VALUE, B); }
value(A) ::= FALSE(B). { A = PN_AST(VALUE, B); }
value(A) ::= INT(B). { A = PN_AST(VALUE, B); }
value(A) ::= DECIMAL(B). { A = PN_AST(VALUE, B); }
value(A) ::= STRING(B). { A = PN_AST(VALUE, B); }
value(A) ::= STRING2(B). { A = PN_AST(VALUE, B); }
value(A) ::= lick(B). { A = B; }

block(A) ::= BEGIN_BLOCK all(B) END_BLOCK. { A = PN_AST(BLOCK, B); }
block(A) ::= BEGIN_BLOCK END_BLOCK. { A = PN_AST(BLOCK, PN_NIL); }

table(A) ::= BEGIN_TABLE all(B) END_TABLE. { A = PN_AST(TABLE, B); }
table(A) ::= BEGIN_TABLE END_TABLE. { A = PN_AST(TABLE, PN_NIL); }

lick(A) ::= BEGIN_LICK items(B) END_LICK. { A = PN_AST(TABLE, B); }
lick(A) ::= BEGIN_LICK END_LICK. { A = PN_AST(TABLE, PN_NIL); }

items(A) ::= items(B) SEP item(C). { A = PN_PUSH(B, C); }
items(A) ::= item(B). { A = PN_TUP(B); }

item(A) ::= MESSAGE(B). { A = PN_AST(LICK, B); }
item(A) ::= MESSAGE(B) value(C). { A = PN_AST2(LICK, B, C); }
item(A) ::= MESSAGE(B) table(C). { A = PN_AST3(LICK, B, PN_NIL, C); }
item(A) ::= MESSAGE(B) value(C) table(D). { A = PN_AST3(LICK, B, C, D); }
item(A) ::= MESSAGE(B) table(D) value(C). { A = PN_AST3(LICK, B, C, D); }
