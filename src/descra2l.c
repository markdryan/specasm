/*
 * Copyright contributors to Specasm
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "descra2l.h"

const char specasm_doc_adc[] =
    "The second argument and the carry flag are added to the "
    "contents of the destination register.";

const char specasm_doc_add[] =
    "The second argument is added to the contents of the "
    "destination register.";

const char specasm_doc_add_16[] =
    "The second argument is added to the contents of the "
    "destination register. The carry flag is set according "
    "to the result of the addition. h represents carry from "
    "bit 11.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_add_rra[] =
    "A is added to the destination register. Carry flag undefined.";
const char specasm_doc_add_16imm[] =
    "The 16 bit immediate in the second argument is added to the "
    "destination register.";
#endif

const char specasm_doc_align[] =
    "The align directive takes one immediate argument that must be "
    "a power of 2, >= 2 and <= 256.  It inserts null bytes into "
    "the binary until the requested alignment is achieved. The "
    "number of t-states consumed by an align directive is the "
    "number of bytes inserted * 4.";

const char specasm_doc_and[] = "The result of a bitwise AND of a and the "
			       "argument is stored in a.";

const char specasm_doc_bit[] =
    "Sets the zero flag to 1 if bit n of the 2nd operand "
    "is 0, or to 0 if bit N is 1.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_brlc[] =
    "Rotates de left by the value in the lower nibble of b.";
const char specasm_doc_bsla[] =
    "Shifts de left by the value in the bottom 5 bits of b.";
const char specasm_doc_bsra[] =
    "Performs an arithmetic right shift of de by the value in the "
    "bottom 5 bits of b.";
const char specasm_doc_bsrf[] =
    "Shifts de right by the value in the bottom 5 bits of b. "
    "The vacated bits are set to 1.";
const char specasm_doc_bsrl[] =
    "Performs a logical right shift of de by the value in the "
    "bottom 5 bits of b.";
#endif

const char specasm_doc_call[] =
    "Pushes the PC on the stack and jumps to nn. The conditional "
    "version of the instruction takes fewer t-states to execute "
    "if the call is not taken.";

const char specasm_doc_ccf[] = "Inverts the carry flag.";

const char specasm_doc_cp[] =
    "The operand is subtracted from a setting the "
    "flags accordingly. The result of the subtraction is "
    "discarded.";

const char specasm_doc_cpd[] =
    "(hl) is subtracted from a and the flags are set "
    "accordingly. The result is discarded. bc and hl are "
    "decremented. The p flag is set if bc != 0 after the "
    "instruction has finished and is otherwise reset.";

const char specasm_doc_cpdr[] =
    "(hl) is subtracted from a and the flags are set accordingly. "
    "The result is discarded. bc and hl are decremented. If bc>0 "
    "and the result of the subtraction is != 0 cpdr "
    "repeats. The p flag is set if bc!=0 after cpdr has "
    "finished and is otherwise reset. The slower timings apply "
    "when cpdr repeats.";

const char specasm_doc_cpi[] =
    "(hl) is subtracted from a and the flags are set accordingly. "
    "The result is discarded. bc is decremented while hl is "
    "incremented. The p flag is set if bc != 0 after the cpi has "
    "finished and is otherwise reset.";

const char specasm_doc_cpir[] =
    "(hl) is subtracted from a and the flags are set accordingly. "
    "The result is discarded. bc is decremented while hl is "
    "incremented. If bc>0 and the result of the subtraction is != 0"
    " cpir repeats. The p flag is set if bc!=0 after cpir "
    "has finished and is otherwise reset. The slower "
    "timings apply when the cpir repeats.";

const char specasm_doc_cpl[] = "Invert a.";

const char specasm_doc_daa[] =
    "Conditionally adjusts a for BCD addition and subtraction.";

const char specasm_doc_db[] =
    "Stores up to 4 bytes in the program binary. All ns must be "
    "formatted in the same way. Only one byte can be specified if "
    "an expression is used.";

const char specasm_doc_dec[] = "The specified operand is decremented by 1.";

const char specasm_doc_di[] = "Disables maskable interrupts.";

const char specasm_doc_djnz[] =
    "b is decremented by 1. If the result is>0 the cpu jumps to "
    "PC+2+n, where n is a signed 8 byte. djnz executes "
    "more quickly when the jump it not taken.";

const char specasm_doc_ds[] = "Stores c copies of the byte n in the binary.";

const char specasm_doc_dw[] =
    "Stores up to 2 words in the program binary.  All nns must be "
    "formatted in the same way. Only one word can be specified if "
    "an expression is used.";

const char specasm_doc_ei[] = "Enables maskable interrupts.";

const char specasm_doc_ex[] =
    "The contents of the two operands are exchanged. "
    "ex af, af' affects all the flags while the other forms of the "
    "instruction have no effect on the flags.";

const char specasm_doc_exx[] = "Exchange bc, de and hl with bc', de', hl'.";

const char specasm_doc_halt[] =
    "CPU execution is suspended until the next interrupt or reset.";

const char specasm_doc_im[] =
    "Sets the interrupt mode.  With im 2 the MSB of the vector "
    "address is taken from the i register.";

const char specasm_doc_in[] =
    "Reads a byte from the device identified by bc and stores it in r.";

const char specasm_doc_in2[] =
    "Reads a byte from the device adddress whose MSB is "
    "taken from a and whose LSB is n. The byte is "
    "stored in a.";

const char specasm_doc_inc[] = "The specified operand is incremented by 1.";

const char specasm_doc_ind[] =
    "A byte is read from the device identified by bc and stored "
    "in (hl). b and hl are decremented.";

const char specasm_doc_indr[] =
    "A byte is read from the device identified by bc and stored "
    "(hl). b and hl are decremented. If bc!=0 indr "
    "repeats. indr consumes more t-states when it "
    "repeats.";

const char specasm_doc_ini[] =
    "A byte is read from the device identified by bc "
    "and stored in (hl). b is decremented and h is incremented.";

const char specasm_doc_inir[] =
    "A byte is read from the device identified by bc "
    "and stored in (hl). b is decremented and h is incremented. "
    "If bc!=0 inir repeats. inir consumes more t-states when it "
    "repeats.";

const char specasm_doc_jp[] =
    "Jump to the last operand if the condition is met or no "
    "condition is supplied. Instruction consumes fewer t-states "
    "if condition is not met.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_jp_c[] =
    "Jumps to address formed from the address of the next instruction "
    "and data read from an io port. Bit 14 and 15 are taken "
    "from the PC of the next instruction.  Bits 0-5 are 0.  Bits "
    "6-13 come from an implicit in (c).";
#endif

const char specasm_doc_jr[] =
    "Jump to PC+2+n if the condition is met or no condition is "
    "supplied. Range of jump is -126 + 129 bytes. Instruction "
    "consumes fewer t-states if condition is not met.";

const char specasm_doc_ld1[] =
    "Loads register pair from an immediate value or from an "
    "absolute address.";

const char specasm_doc_ld2[] =
    "Loads SP with the value of another 16 bit register pair.";

const char specasm_doc_ld3[] =
    "Loads byte register an immediate value or another register.";

const char specasm_doc_ld4[] =
    "Stores a byte register to the memory location provided "
    "by the first operand.";

const char specasm_doc_ld5[] =
    "Loads a 16 bit value from a register into an absolute "
    "address.";

const char specasm_doc_ld6[] =
    "Stores an 8 bit immediate to the memory location provided "
    "by the first operand.";

const char specasm_doc_ld7[] =
    "Indirectly loads a byte into register using the pointer given "
    "in the second operator.";

const char specasm_doc_ld8[] =
    "Instructions to load and store the interrupt vector and "
    "memory refresh registers to and from a.";

const char specasm_doc_ldd[] =
    "Load (hl) into (de). hl, de and bc are decremented.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_lddrx[] =
    "Load (hl) into (de) if (hl) != a. hl and bc are decremented while"
    "de in incremented. If bc!=0 lddrx repeats. lddrx consumes more "
    "t-states when it repeats.";
const char specasm_doc_lddx[] =
    "Load (hl) into (de) if (hl) != a. de is incremented. bc and "
    "hl are decremented.";
#endif

const char specasm_doc_lddr[] =
    "Load (hl) into (de). hl, de and bc are decremented. If "
    "bc!=0 lddr repeats. lddr consumes more t-states when it "
    "repeats.";

const char specasm_doc_ldi[] =
    "Load (hl) into (de). Both hl and de are incremented while bc "
    "is decremented.";

const char specasm_doc_ldir[] =
    "Load (hl) into (de). Both hl and de are incremented while bc "
    "is decremented. If bc!=0 ldir repeats. ldir "
    "consumes more t-states when it repeats.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_ldirx[] =
    "Load (hl) into (de) if (hl) != a. hl and de are incremented, while "
    "bc is decremented. If bc != 0 ldirx repeats. ldirx consumes more "
    "t-states when it repeats.";

const char specasm_doc_ldix[] =
    "Load (hl) into (de) if (hl) != a. Both hl and de "
    "are incremented while bc is decremented.";

const char specasm_doc_ldpirx[] =
    "Load the byte whose address is formed from the top 13 bits of hl "
    "and bottom 3 bits of de into (de) if the byte != a. de is incremented, "
    "while bc is decremented. If bc != 0 ldpirx repeats. ldpirx consumes more "
    "t-states when it repeats.";

const char specasm_doc_ldws[] =
    "Load (hl) into (de) and increment l and d. The v flag is set "
    "if d was $7f before increment.";

#endif