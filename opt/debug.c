/*  debug.c -- optional debugging utilities              */
/*  Copyright (c) 2009 Alex Shinn.  All rights reserved. */
/*  BSD-style license: http://synthcode.com/license.txt  */

#define SEXP_DISASM_MAX_DEPTH 8
#define SEXP_DISASM_PAD_WIDTH 4

static const char* reverse_opcode_names[] =
  {"NOOP", "RAISE", "RESUMECC", "CALLCC", "APPLY1", "TAIL-CALL", "CALL",
   "FCALL0", "FCALL1", "FCALL2", "FCALL3", "FCALL4", "FCALL5", "FCALL6",
   "JUMP-UNLESS", "JUMP", "PUSH", "DROP", "GLOBAL-REF", "GLOBAL-KNOWN-REF",
   "STACK-REF", "LOCAL-REF", "LOCAL-SET",
   "CLOSURE-REF", "VECTOR-REF", "VECTOR-SET", "VECTOR-LENGTH", "STRING-REF",
   "STRING-SET", "STRING-LENGTH", "MAKE-PROCEDURE", "MAKE-VECTOR", "AND",
   "NULL?", "FIXNUM?", "SYMBOL?", "CHAR?",
   "EOF?", "TYPEP", "MAKE", "SLOT-REF", "SLOT-SET", "CAR", "CDR",
   "SET-CAR", "SET-CDR", "CONS", "ADD", "SUB",
   "MUL", "DIV", "QUOTIENT", "REMAINDER", "NEGATIVE", "INVERSE",
   "LT", "LE", "EQN", "EQ", "EXACT->INEXACT", "INEXACT->EXACT",
   "CHAR->INTEGER", "INTEGER->CHAR", "CHAR-UPCASE", "CHAR-DOWNCASE",
   "WRITE-CHAR", "NEWLINE", "READ-CHAR", "PEEK-CHAR", "RET", "DONE",
  };

static sexp disasm (sexp ctx, sexp bc, sexp out, int depth) {
  sexp tmp;
  unsigned char *ip, opcode, i;

  if (sexp_procedurep(bc)) {
    bc = sexp_procedure_code(bc);
  } else if (sexp_opcodep(bc)) {
    sexp_printf(ctx, out, "%s is a primitive\n", sexp_opcode_name(bc));
    return SEXP_VOID;
  } else if (! sexp_bytecodep(bc)) {
    return sexp_type_exception(ctx, "not a procedure", bc);
  }
  if (! sexp_oportp(out))
    return SEXP_VOID;

  for (i=0; i<(depth*SEXP_DISASM_PAD_WIDTH); i++)
    sexp_write_char(ctx, ' ', out);
  sexp_write_string(ctx, "-------------- ", out);
  if (sexp_truep(sexp_bytecode_name(bc))) {
    sexp_write(ctx, sexp_bytecode_name(bc), out);
    sexp_write_char(ctx, ' ', out);
  }
  sexp_printf(ctx, out, "%p\n", bc);

  ip = sexp_bytecode_data(bc);

 loop:
  for (i=0; i<(depth*SEXP_DISASM_PAD_WIDTH); i++)
    sexp_write_char(ctx, ' ', out);
  opcode = *ip++;
  if (opcode*sizeof(char*) < sizeof(reverse_opcode_names)) {
    sexp_printf(ctx, out, "  %s ", reverse_opcode_names[opcode]);
  } else {
    sexp_printf(ctx, out, "  <unknown> %d ", opcode);
  }
  switch (opcode) {
  case OP_STACK_REF:
  case OP_LOCAL_REF:
  case OP_LOCAL_SET:
  case OP_CLOSURE_REF:
  case OP_JUMP:
  case OP_JUMP_UNLESS:
  case OP_TYPEP:
  case OP_FCALL0:
  case OP_FCALL1:
  case OP_FCALL2:
  case OP_FCALL3:
  case OP_FCALL4:
  case OP_FCALL5:
  case OP_FCALL6:
    sexp_printf(ctx, out, "%ld", (sexp_sint_t) ((sexp*)ip)[0]);
    ip += sizeof(sexp);
    break;
  case OP_SLOT_REF:
  case OP_SLOT_SET:
  case OP_MAKE:
    ip += sizeof(sexp)*2;
    break;
  case OP_GLOBAL_REF:
  case OP_GLOBAL_KNOWN_REF:
  case OP_TAIL_CALL:
  case OP_CALL:
  case OP_PUSH:
    tmp = ((sexp*)ip)[0];
    if (((opcode == OP_GLOBAL_REF) || (opcode == OP_GLOBAL_KNOWN_REF))
        && sexp_pairp(tmp))
      tmp = sexp_car(tmp);
    else if ((opcode == OP_PUSH) && (sexp_pairp(tmp) || sexp_idp(tmp)))
      sexp_write_char(ctx, '\'', out);
    sexp_write(ctx, tmp, out);
    ip += sizeof(sexp);
    break;
  }
  sexp_write_char(ctx, '\n', out);
  if ((opcode == OP_PUSH) && (depth < SEXP_DISASM_MAX_DEPTH)
      && (sexp_bytecodep(tmp) || sexp_procedurep(tmp)))
    disasm(ctx, tmp, out, depth+1);
  if (ip - sexp_bytecode_data(bc) < sexp_bytecode_length(bc))
    goto loop;
  return SEXP_VOID;
}

static sexp sexp_disasm (sexp ctx, sexp bc, sexp out) {
  return disasm(ctx, bc, out, 0);
}

#if USE_DEBUG_VM
static void sexp_print_stack (sexp ctx, sexp *stack, int top, int fp, sexp out) {
  int i;
  if (! sexp_oport(out)) out = sexp_current_error_port(ctx);
  for (i=0; i<top; i++) {
    sexp_printf(ctx, out, "%s %02d: ", ((i==fp) ? "*" : " "), i);
    sexp_write(ctx, stack[i], out);
    sexp_printf(ctx, out, "\n");
  }
}
#endif

