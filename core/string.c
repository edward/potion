//
// string.c
// internals of utf-8 and byte strings
//
// (c) 2008 why the lucky stiff, the freelance professor
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "potion.h"
#include "internal.h"
#include "khash.h"
#include "table.h"

unsigned potion_add_str(Potion *P, PN self, PN s) {
  int ret;
  vPN(StrTable) t = (struct PNStrTable *)self;
  unsigned k = kh_put(str, t->kh, s, &ret);
  PN_TOUCH(self);
  return k;
}

PN potion_lookup_str(PN self, const char *str) {
  vPN(StrTable) t = (struct PNStrTable *)self;
  unsigned k = kh_get(str, t->kh, str);
  if (k != kh_end(t->kh)) return kh_key(t->kh, k);
  return PN_NIL;
}

PN potion_str(Potion *P, const char *str) {
  PN val = potion_lookup_str(P->strings, str);
  if (!val) {
    size_t len = strlen(str);
    vPN(String) s = PN_ALLOC_N(PN_TSTRING, struct PNString, len + 1);
    s->len = (unsigned int)len;
    PN_MEMCPY_N(s->chars, str, char, len);
    s->chars[len] = '\0';
    potion_add_str(P, P->strings, (PN)s);
    val = (PN)s;
  }
  return val;
}

PN potion_str2(Potion *P, char *str, size_t len) {
  PN s;
  char *tmp = (char *)malloc(len + 1);
  memcpy(tmp, str, len);
  tmp[len] = '\0';
  s = potion_str(P, tmp);
  free(tmp);
  return s;
}

static PN potion_str_length(Potion *P, PN closure, PN self) {
  return PN_NUM(potion_cp_strlen_utf8(PN_STR_PTR(self)));
}

static PN potion_str_eval(Potion *P, PN closure, PN self) {
  return potion_eval(P, PN_STR_PTR(self));
}

static PN potion_str_number(Potion *P, PN closure, PN self) {
  char *str = PN_STR_PTR(self);
  int i = 0, dec = 0, sign = 0, len = PN_STR_LEN(self);
  if (len < 1) return PN_ZERO;

  sign = (str[0] == '-' ? -1 : 1);
  if (str[0] == '-' || str[0] == '+') {
    dec++; str++; len--;
  }
  for (i = 0; i < len; i++)
    if (str[i] < '0' || str[i] > '9')
      break;
  if (i < 10 && i == len) {
    return PN_NUM(sign * PN_ATOI(str, i, 10));
  }

  return potion_decimal(P, PN_STR_LEN(self), dec + i, PN_STR_PTR(self));
}

static PN potion_str_string(Potion *P, PN closure, PN self) {
  return self;
}

static PN potion_str_print(Potion *P, PN closure, PN self) {
  printf("%s", PN_STR_PTR(self));
  return PN_NIL;
}

static size_t potion_utf8char_offset(const char *s, size_t index) {
  int i;
  for (i = 0; s[i]; i++)
    if ((s[i] & 0xC0) != 0x80)
      if (index-- == 0)
        return i;
  return i;
}

inline static PN potion_str_slice_index(PN index, size_t len, int nilvalue) {
  int i = PN_INT(index);
  int corrected;
  if (PN_IS_NIL(index)) {
    corrected = nilvalue;
  } else if (i < 0) {
    corrected = i + len;
    if (corrected < 0) {
      corrected = 0;
    }
  } else if (i > len) {
    corrected = len;
  } else {
    corrected = i;
  }
  return PN_NUM(corrected);
}

static PN potion_str_slice(Potion *P, PN closure, PN self, PN start, PN end) {
  char *str = PN_STR_PTR(self);
  size_t len = potion_cp_strlen_utf8(str);
  size_t startoffset = potion_utf8char_offset(str, PN_INT(potion_str_slice_index(start, len, 0)));
  size_t endoffset = potion_utf8char_offset(str, PN_INT(potion_str_slice_index(end, len, len)));
  return potion_str2(P, str + startoffset, endoffset - startoffset);
}

static PN potion_str_at(Potion *P, PN closure, PN self, PN index) {
  char *str = PN_STR_PTR(self);
  size_t len = potion_cp_strlen_utf8(str);
  size_t offset = potion_utf8char_offset(str, PN_INT(potion_str_slice_index(index, len, 0)));
  return potion_str2(P, str + offset, 1);
}

PN potion_byte_str(Potion *P, const char *str) {
  size_t len = strlen(str);
  vPN(Bytes) s = (struct PNBytes *)potion_bytes(P, len + 1);
  PN_MEMCPY_N(s->chars, str, char, len);
  s->len = len;
  s->chars[len] = '\0';
  return (PN)s;
}

PN potion_bytes(Potion *P, size_t len) {
  vPN(Bytes) s = PN_ALLOC_N(PN_TBYTES, struct PNBytes, len + 1);
  s->len = (PN_SIZE)len;
  return (PN)s;
}

PN_SIZE pn_printf(Potion *P, PN bytes, const char *format, ...) {
  PN_SIZE len;
  va_list args;
  vPN(Bytes) s = (struct PNBytes *)potion_fwd(bytes);

  va_start(args, format);
  len = (PN_SIZE)vsnprintf(NULL, 0, format, args);
  va_end(args);
  PN_REALLOC(s, PN_TBYTES, struct PNBytes, s->len + len + 1);

  va_start(args, format);
  vsnprintf(s->chars + s->len, len + 1, format, args);
  va_end(args);

  s->len += len;
  return len;
}

void potion_bytes_obj_string(Potion *P, PN bytes, PN obj) {
  potion_bytes_append(P, 0, bytes, potion_send(obj, PN_string));
}

PN potion_bytes_append(Potion *P, PN closure, PN self, PN str) {
  vPN(Bytes) s = (struct PNBytes *)potion_fwd(self);
  PN fstr = potion_fwd(str);
  PN_SIZE len = PN_STR_LEN(fstr);
  PN_REALLOC(s, PN_TBYTES, struct PNBytes, s->len + len + 1);
  PN_MEMCPY_N(s->chars + s->len, PN_STR_PTR(fstr), char, len);
  s->len += len;
  s->chars[s->len] = '\0';
  return self;
}

static PN potion_bytes_length(Potion *P, PN closure, PN self) {
  PN str = potion_fwd(self);
  return PN_NUM(PN_STR_LEN(str));
}

static PN potion_bytes_string(Potion *P, PN closure, PN self) {
  return self;
}

static PN potion_bytes_print(Potion *P, PN closure, PN self) {
  PN str = potion_fwd(self);
  printf("%s", PN_STR_PTR(str));
  return PN_NIL;
}

void potion_str_hash_init(Potion *P) {
  vPN(StrTable) t = PN_CALLOC_N(PN_TSTRINGS, struct PNStrTable, sizeof(kh_str_t));
  P->strings = (PN)t;
}

void potion_str_init(Potion *P) {
  PN str_vt = PN_VTABLE(PN_TSTRING);
  PN byt_vt = PN_VTABLE(PN_TBYTES);
  potion_type_call_is(str_vt, PN_FUNC(potion_str_at, 0));
  potion_method(str_vt, "eval", potion_str_eval, 0);
  potion_method(str_vt, "length", potion_str_length, 0);
  potion_method(str_vt, "number", potion_str_number, 0);
  potion_method(str_vt, "print", potion_str_print, 0);
  potion_method(str_vt, "string", potion_str_string, 0);
  potion_method(str_vt, "slice", potion_str_slice, "start=N,end=N");
  potion_method(byt_vt, "append", potion_bytes_append, 0);
  potion_method(byt_vt, "length", potion_bytes_length, 0);
  potion_method(byt_vt, "print", potion_bytes_print, 0);
  potion_method(byt_vt, "string", potion_bytes_string, 0);
}
