//
// internal.c
// memory allocation and innards
//
// (c) 2008 why the lucky stiff, the freelance professor
//
#include <stdio.h>
#include <stdlib.h>
#include "potion.h"
#include "internal.h"
#include "gc.h"

PN PN_allocate, PN_break, PN_call, PN_class, PN_compile, PN_continue, PN_def,
   PN_delegated, PN_else, PN_elsif, PN_if, PN_lookup, PN_loop, PN_print,
   PN_return, PN_self, PN_string, PN_while;

PN potion_allocate(Potion *P, PN cl, PN self, PN len) {
  struct PNData *obj = PN_ALLOC_N(PN_TUSER, struct PNData, PN_INT(len));
  obj->siz = len;
  return (PN)obj;
}

static void potion_init(Potion *P) {
  PN vtable, obj_vt;
  P->lobby = potion_type_new(P, PN_TLOBBY, 0);
  vtable = potion_type_new(P, PN_TVTABLE, P->lobby);
  obj_vt = potion_type_new(P, PN_TOBJECT, P->lobby);
  potion_type_new(P, PN_TNIL, obj_vt);
  potion_type_new(P, PN_TNUMBER, obj_vt);
  potion_type_new(P, PN_TBOOLEAN, obj_vt);
  potion_type_new(P, PN_TSTRING, obj_vt);
  potion_type_new(P, PN_TTABLE, obj_vt);
  potion_type_new(P, PN_TCLOSURE, obj_vt);
  potion_type_new(P, PN_TTUPLE, obj_vt);
  potion_type_new(P, PN_TSTATE, obj_vt);
  potion_type_new(P, PN_TSOURCE, obj_vt);
  potion_type_new(P, PN_TBYTES, obj_vt);
  potion_type_new(P, PN_TPROTO, obj_vt);
  potion_type_new(P, PN_TWEAK, obj_vt);
  potion_type_new(P, PN_TLICK, obj_vt);
  potion_str_hash_init(P);

  PN_allocate = potion_str(P, "allocate");
  PN_break = potion_str(P, "break");
  PN_call = potion_str(P, "call");
  PN_continue = potion_str(P, "continue");
  PN_def = potion_str(P, "def");
  PN_delegated = potion_str(P, "delegated");
  PN_class = potion_str(P, "class");
  PN_compile = potion_str(P, "compile");
  PN_else = potion_str(P, "else");
  PN_elsif = potion_str(P, "elsif");
  PN_if = potion_str(P, "if");
  PN_lookup = potion_str(P, "lookup");
  PN_loop = potion_str(P, "loop");
  PN_print = potion_str(P, "print");
  PN_return = potion_str(P, "return");
  PN_self = potion_str(P, "self");
  PN_string = potion_str(P, "string");
  PN_while = potion_str(P, "while");

  potion_def_method(P, 0, vtable, PN_lookup, PN_FUNC(potion_lookup, 0));
  potion_def_method(P, 0, vtable, PN_def, PN_FUNC(potion_def_method, 0));

  potion_send(vtable, PN_def, PN_allocate, PN_FUNC(potion_allocate, 0));
  potion_send(vtable, PN_def, PN_delegated, PN_FUNC(potion_delegated, 0));

  potion_vm_init(P);
  potion_lobby_init(P);
  potion_object_init(P);
  potion_primitive_init(P);
  potion_num_init(P);
  potion_str_init(P);
  potion_table_init(P);
  potion_source_init(P);
  potion_lick_init(P);
  potion_compiler_init(P);
  potion_file_init(P);

  GC_PROTECT(P);
}

Potion *potion_create(void *sp) {
  Potion *P = potion_gc_boot(sp);
  P->vt = PN_TSTATE;
  P->uniq = (PNUniq)potion_rand_int();
  PN_FLEX_NEW(P->vts, PN_TFLEX, PNFlex, TYPE_BATCH_SIZE);
  PN_FLEX_SIZE(P->vts) = PN_TYPE_ID(PN_TUSER) + 1;
  potion_init(P);
  return P;
}

void potion_destroy(Potion *P) {
  potion_gc_release(P);
}

PN potion_delegated(Potion *P, PN closure, PN self) {
  PNType t = PN_FLEX_SIZE(P->vts) + PN_TNIL;
  PN_FLEX_NEEDS(1, P->vts, PN_TFLEX, PNFlex, TYPE_BATCH_SIZE);
  return potion_type_new(P, t, self);
}

PN potion_call(Potion *P, PN cl, PN_SIZE argc, PN * volatile argv) {
  vPN(Closure) c = PN_CLOSURE(cl);
  switch (argc) {
    case 0:
    return c->method(P, cl, cl);
    case 1:
    return c->method(P, cl, argv[0]);
    case 2:
    return c->method(P, cl, argv[0], argv[1]);
    case 3:
    return c->method(P, cl, argv[0], argv[1], argv[2]);
    case 4:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3]);
    case 5:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4]);
    case 6:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5]);
    case 7:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6]);
    case 8:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7]);
    case 9:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8]);
    case 10:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8], argv[9]);
    case 11:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8], argv[9], argv[10]);
    case 12:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11]);
    case 13:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11],
        argv[12]);
    case 14:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11],
        argv[12], argv[13]);
    case 15:
    return c->method(P, cl, argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11],
        argv[12], argv[13], argv[14]);
  }
  return PN_NIL; // TODO: error "too many arguments"
}

PNType potion_kind_of(PN obj) {
  return potion_type(obj);
}

void potion_p(Potion *P, PN x) {
  potion_send(potion_send(x, PN_string), PN_print);
  printf("\n");
}

void potion_esp(void **esp) {
  PN x;
  *esp = (void *)&x;
}

void potion_dump_stack(Potion *P) {
  PN_SIZE n;
  PN *end, *start = P->mem->cstack;
  POTION_ESP(&end);
#if POTION_STACK_DIR > 0
  n = end - start;
#else
  n = start - end + 1;
  start = end;
  end = P->mem->cstack;
#endif

  while (n--) {
    printf("stack(%u) = %lx\n", n, *start);
    start++;
  }
}
