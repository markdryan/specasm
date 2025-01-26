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

#include "descrm2z.h"

const char specasm_doc_map[] = "Instructs the linker to generate a map file.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_mirror[] = "Reverses the order of the bits in a.";
const char specasm_doc_mul[] = "Let de = d * e.";
#endif

const char specasm_doc_neg[] = "Let a = 0 - a.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_nextreg[] =
    "Stores the 2nd operand in the Next register identified by the first "
    "operand.";
#endif

const char specasm_doc_nop[] = "CPU does nothing for 1 m-cycle.";
const char specasm_doc_or[] =
    "The result of a bitwise OR of a and the argument is stored "
    "in a.";
const char specasm_doc_org[] =
    "Assembler directive that sets the org address of the program, "
    "i.e., the address the first byte in the .x or .t file that "
    "contains the Main label is assembled at.";
const char specasm_doc_out1[] =
    "Writes the register r to the device identified by bc.";
const char specasm_doc_out2[] =
    "Writes a to the device at the adddress whose MSB "
    "is taken from the a and whose LSB is n.";
const char specasm_doc_outd[] =
    "b is decremented. (hl) is written to the device identified "
    "by  bc, where b has already been decremented. hl is "
    "decremented.";
const char specasm_doc_outdr[] =
    "b is decremented. (hl) is written to the device identified "
    "by bc, where b has already been decremented. hl is "
    "decremented. outdr repeats if b!=0. It consumes more t-states "
    "when it repeats.";
const char specasm_doc_outi[] =
    "b is decremented. (hl) is written to the device identified by "
    "bc, where b has already been decremented. hl is incremented.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_outinb[] =
    "(hl) is written to the device identified by bc. hl is incremented.";
#endif

const char specasm_doc_outir[] =
    "b is decremented. (hl) is written to the device identified by "
    "bc, where b has already been decremented. hl is incremented. "
    "outir repeats if b!=0. It consumes more t-states "
    "when it repeats.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_pixelad[] =
    "Stores in hl the address of the byte in the Spectrum's display file "
    "that contains the pixel addressed by the x and y coordinates in "
    "the e and d registers, respectively.";
const char specasm_doc_pixeldn[] =
    "On entry hl should point to the start of a 8 bit pixel block in the "
    "Spectrum's display file. On exit hl points to the same pixel block "
    "one line down.";
#endif

const char specasm_doc_pop[] = "Pops 2 bytes off the stack into the operand.";
const char specasm_doc_push[] = "Pushes the operand onto the stack.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_push_imm[] =
    "Pushes a 16 bit immediate onto the stack.  The immediate is big endian "
    "encoded.";
#endif

const char specasm_doc_res[] = "Resets bit n in the second operand.";
const char specasm_doc_ret[] =
    "If there is no condition or the condition is met, a word "
    "is popped off the stack into the PC, from which program "
    "execution continues. ret takes fewer cycles if the condition "
    "is not met.";
const char specasm_doc_reti[] =
    "Return from interrupt. A word is popped off the stack into PC."
    " An ei must be executed prior to the reti to renable "
    "maskable interrupts.";
const char specasm_doc_retn[] =
    "Return from NMI. A word is popped off the stack into PC and "
    "the maskable interrupts are re-enabled if they were enabled "
    "before the NMI.";
const char specasm_doc_rl[] =
    "The operand is shifted left by 1 bit. The carry flag "
    "is moved into bit 0 of the operand and the old bit 7 of "
    "the operand is moved into the carry flag.";
const char specasm_doc_rla[] =
    "a is rotated left 1 bit. The carry flag is moved to bit 0 of "
    "a and its old bit 7 is moved to the carry flag.";
const char specasm_doc_rlc[] =
    "The operand is rotated left 1 bit. The old bit 7 is moved to "
    "both the carry flag and bit 0 of the operand.";
const char specasm_doc_rlca[] =
    "a is rotated left 1 bit. The old bit 7 is moved "
    "to both the carry flag and bit 0 of a.";
const char specasm_doc_rld[] = "Let tmp = (hl) >> 4             "
			       "Let (hl) = ((hl) << 4)|(a & $f) "
			       "Let a = (a & $f0)|tmp";
const char specasm_doc_rr[] =
    "The operand is shifted right by 1 bit. The carry flag "
    "is moved into bit 7 of the operand and the old bit 0 of "
    "the operand is moved into the carry flag.";
const char specasm_doc_rra[] =
    "a is shifted right 1 bit. The carry flag is "
    "moved into bit 7 of a and the old contents of a's "
    "bit 0 are moved into the carry flag.";
const char specasm_doc_rrc[] =
    "The operand is rotated right by 1 bit. Its old bit 0 is "
    "moved into both the carry flag and bit 7 of the operand.";
const char specasm_doc_rrca[] =
    "a is rotated right by 1 bit. a's old bit 0 is "
    "moved into both the carry flag and bit 7 of a.";
const char specasm_doc_rrd[] = "Let tmp = a << 4             "
			       "Let a = (a & $f0)|((hl) & $f) "
			       "Let (hl) = tmp | ((hl) >> 4)";
const char specasm_doc_rst[] =
    "The PC is pushed to the stack and the CPU jumps to the "
    "address n.  Valid values of n are 0, 8, 16, 24, 32, 40, 48,"
    " and 56.";
const char specasm_doc_sbc[] =
    "The second argument and the carry flag are subtracted from "
    "the contents of the destination register, either a or hl.";
const char specasm_doc_scf[] = "Sets the carry flag.";
const char specasm_doc_set[] = "Sets bit n in the second operand.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_setae[] =
    "Let a = $80 >> (e & 7).         "
    "Used to set a bit corresponding to a pixel in the Spectrum's display "
    "file, where the x coordinate of the pixel is in e.";
#endif

const char specasm_doc_sla[] =
    "The operand is shifted left by 1 bit. Bit 0 "
    "is set to 0. Bit 7 is moved into the carry flag.";
const char specasm_doc_sra[] =
    "The operand is arithmetically shifted right by 1 bit. Bit 7 "
    "remains unchanged. Bit 0 is moved into the carry flag.";
const char specasm_doc_srl[] =
    "The operand is logically shifted right by 1 bit. Bit 7 is "
    "set to 0. Bit 0 is moved into the carry flag.";
const char specasm_doc_sub[] = "The operand is subracted from a.";

#ifdef SPECASM_TARGET_NEXT_OPCODES
const char specasm_doc_swapnib[] = "Swaps the nibbles in a";
const char specasm_doc_test[] =
    "ANDs the immediate operand with a, sets the flags and discards "
    "the result.";
#endif

const char specasm_doc_xor[] =
    "The result of a bitwise XOR of a and the argument is stored "
    "in a.";
const char specasm_doc_zx81[] =
    "Linker directive that causes string and character literals to "
    "be transliterated from ASCII to ZX81 encoding.  It also sets "
    "org address to 16514.";
