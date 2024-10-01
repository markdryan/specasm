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

#include <stdlib.h>
#include <input.h>
#include <z80.h>

#include "doc.h"
#include "editor.h"
#include "line_common.h"
#include "peer.h"
#include "scratch.h"

#include <string.h>

#define SPECASM_DOC_ENCODING_X 16
#define SPECASM_DOC_M_CYCLES_X 27
#define SPECASM_DOC_T_STATES_X 30

struct specasm_ins_form_t_ {
	const char *form;
	const char *encoding;
	uint8_t m_cycles;
	uint8_t t_states;
};

typedef struct specasm_ins_form_t_ specasm_ins_form_t;

#define SPECASM_DOC_MAX_REG_ENCODING (SPECASM_BYTE_REG_IY+1)

struct specasm_ins_doc_t_ {
	char name[8];
	specasm_ins_form_t forms[6];
	uint8_t reg_encoding;
	uint8_t bits;
	uint8_t all_cc;
	const char *description;
	const char *flags;
};

typedef struct specasm_ins_doc_t_ specasm_ins_doc_t;

static const char* const reg_names[] = {
	"a",
	"b",
	"c",
	"d",
	"e",
	"h",
	"l",
	"bc",
	"de",
	"hl",
	"af",
	"sp",
	"ix",
	"iy"
};

const uint8_t reg_encodings[][SPECASM_DOC_MAX_REG_ENCODING] = {
	{ 8, 1, 2, 3, 4, 5, 6, 1, 17, 33, 0, 49 },
	{ 8, 1, 2, 3, 4, 5, 6 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 17, 33, 0, 49 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 17, 0, 0, 49, 33 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 17, 0, 0, 49, 0, 33},
	{ 57, 1, 9, 17, 25, 33, 49},
	{ 0, 0, 0, 0, 0, 0, 0, 1, 17, 33, 49 },
};

/*
 * Must be in the same order as the opcode_table in line_parse.c.
 */

const static char specasm_doc_add_16[] =
	"The second argument is added to the contents of the "
	"destination register. The carry flag is set according "
	"to the result of the addition. H represents carry from "
	"bit 11.";

const static char specasm_doc_inc[] =
	"The specified operand is incremented by 1.";

const static specasm_ins_doc_t docs[] = {
	{
		"adc",
		{
			{"a,r", "88+r", 1, 4},
			{"a,n", "CE n",  2, 7},
			{"a,(hl)", "8E", 2, 7},
			{"a,(ix + d)", "DD 8E d", 5, 19 },
			{"a,(iy + d)", "FD 8E d", 5, 19 },
			{"hl,rr", "ED 4A+rr", 4, 15 },
		},
		1,
		0,
		0,
		"The second argument and the carry flag are added to the "
		"contents of the destination register, either a or hl.",
		"XX X X0X",
	},
	{
		"add",
		{
			{"a,r", "80+r", 1, 4},
			{"a,n", "C6 n",  2, 7},
			{"a,(hl)", "86", 2, 7},
			{"a,(ix + d)", "DD 86 d", 5, 19 },
			{"a,(iy + d)", "FD 86 d", 5, 19 },
		},
		2,
		0,
		0,
		"The second argument is added to the contents of the "
		"destination register.",
		"XX X X0X",
	},
	{
		"add",
		{
			{"hl,rr", "9+rr", 3, 11 },
		},
		3,
		0,
		0,
		specasm_doc_add_16,
		"   X  0X",
	},
	{
		"add",
		{
			{"ix,rr", "DD 9+rr", 4, 15 },
		},
		4,
		0,
		0,
		specasm_doc_add_16,
		"   X  0X",
	},
	{
		"add",
		{
			{"iy,rr", "FD 9+rr", 4, 15 },
		},
		5,
		0,
		0,
		specasm_doc_add_16,
		"   X  0X",
	},
	{
		"align",
		{
			{"n", "0", 1, 4 },
		},
		0,
		0,
		0,
		"The align directive takes one immediate argument that must be "
		"a power of 2, greater than or equal to 2 and less and or equal"
		" to 256.  It inserts null bytes into the binary until the "
		"requested alignment is achieved.  The number of t-states "
		"consumed by an align directive is the number of bytes inserted"
		" * 4",
		NULL,
	},
	{
		"and",
		{
			{"r", "A0+r", 1, 4},
			{"n", "E6 n",  2, 7},
			{"(hl)", "A6", 2, 7},
			{"(ix + d)", "DD A6 d", 5, 19 },
			{"(iy + d)", "FD A6 d", 5, 19 },
		},
		2,
		0,
		0,
		"The result of a bitwise AND of the accumulator and the "
		"argument is stored in the accumulator.",
		"XX 1 X00",
	},
	{
		"bit",
		{
			{"n,r", "CB 40+b+r", 2, 8},
			{"n,(HL)", "CB 46+b", 3, 12},
			{"n,(IX + d)", "DDCBd46+b", 5, 20},
			{"n,(IY + d)", "FDCBd46+b", 5, 20},
		},
		2,
		8,
		0,
		"Sets the zero flag to 1 if bit n of the 2nd operand "
		"is 0, or to 0 if bit N is 1.",
		"?X 1 ?0 ",
	},
	{
		"call",
		{
			{"nn", "CD n n", 5, 17},
			{"cc,nn", "C4+cc n n", 5, 17},
			{"cc,nn", "C4+cc n n", 3, 10},
		},
		0,
		0,
		8,
		"Pushes the PC on the stack and jumps to nn. The conditional "
		"version of the instruction takes fewer t-states to execute "
		"if the call is not taken."
		,
		NULL,
	},
	{
		"ccf",
		{
			{"", "3F", 1, 4},
		},
		0,
		0,
		0,
		"Inverts the carry flag, setting it to 1 if it were previously "
		"0 and 1 if it were previously 0.",
		"   ?  0X"
	},
	{
		"cp",
		{
			{"r", "B8+r", 1, 4},
			{"n", "FE n",  2, 7},
			{"(hl)", "BE", 2, 7},
			{"(ix + d)", "DD BE d", 5, 19 },
			{"(iy + d)", "FD BE d", 5, 19 },
		},
		2,
		0,
		0,
		"The operand is subtracted from the accumulator setting the "
		"flags accordingly. The result of the subtraction is "
		"discarded.",
		"XX X X1X",
	},
	{
		"cpd",
		{
			{"", "ED A9", 4, 16},
		},
		0,
		0,
		0,
		"The byte pointed to by the address in hl is subtracted from "
		"the accumulator and the flags are set accordingly. The result "
		"is discarded. bc and hl are decremented. The p flag is set "
		"if bc != 0 after the instruction has finished and is "
		"otherwise reset.",
		"XX X X1 ",
	},
	{
		"cpdr",
		{
			{"", "ED B9", 4, 16},
			{"", "ED B9", 5, 21},
		},
		0,
		0,
		0,
		"The byte pointed to by the address in hl is subtracted from "
		"the accumulator and the flags are set accordingly. The result "
		"is discarded. bc and hl are decremented. If bc > 0 and the "
		"result of the subtraction is != 0 the instructon repeats. "
		"The p flag is set if bc != 0 after the instruction has "
		"finished and is otherwise reset. The slower timings apply "
		"when the instruction repeats.",
		"XX X X1 ",
	},
	{
		"cpi",
		{
			{"", "ED A1", 4, 16},
		},
		0,
		0,
		0,
		"The byte pointed to by the address in hl is subtracted from "
		"the accumulator and the flags are set accordingly. The result "
		"is discarded. bc is decremented while hl is incremented. The "
		"p flag is set if bc != 0 after the instruction has finished "
		"and is otherwise reset.",
		"XX X X1 ",
	},
	{
		"cpir",
		{
			{"", "ED B1", 4, 16},
			{"", "ED B1", 5, 21},
		},
		0,
		0,
		0,
		"The byte pointed to by the address in hl is subtracted from "
		"the accumulator and the flags are set accordingly. The result "
		"is discarded. bc is decremented while hl is incremented. If "
		"bc > 0 and the result of the subtraction is != 0 the "
		"instructon repeats. The p flag is set if bc != 0 after the "
		"instruction has finished and is otherwise reset. The slower "
		"timings apply when the instruction repeats.",
		"XX X X1 ",
	},
	{
		"cpl",
		{
			{"", "2F", 1, 4},
		},
		0,
		0,
		0,
		"Invert the contents of the accumlator.",
		"   1  1 ",
	},
	{
		"daa",
		{
			{"", "27", 1, 4},
		},
		0,
		0,
		0,
		"Conditionally adjusts the accumulator for BCD addition "
		"and subtraction.",
		"XX X X X",
	},
	{
		"db",
		{
			{"n", "n", 0, 0},
			{"n,n", "n n", 0, 0},
			{"n,n,n", "n n n", 0, 0},
			{"n,n,n,n", "n n n n", 0, 0},
		},
		0,
		0,
		0,
		"Stores up to 4 bytes in the program binary.  All ns must be "
		"formatted in the same way. Only one byte can be specified if "
		"an expression is used.",
		NULL,
	},
	{
		"dec",
		{
			{"r", "5+r", 1, 4},
			{"(hl)", "35", 3, 11},
			{"(ix + d)", "DD 35 d", 6, 23 },
			{"(iy + d)", "FD 35 d", 6, 23 },
		},
		6,
		0,
		0,
		"The specified operand is decremented by 1.",
		"XX X X1 ",
	},
	{
		"dec",
		{
			{"rr", "B+rr", 1, 6},
			{"ix", "DD 2B", 2, 10 },
			{"iy", "FD 2B", 2, 10 },
		},
		3,
		0,
		0,
		"The specified operand is decremented by 1.",
		NULL,
	},
	{
		"di",
		{
			{"", "F3", 1, 4},
		},
		0,
		0,
		0,
		"Disables maskable interrupts.",
		NULL,
	},
	{
		"djnz",
		{
			{"n", "10 n", 2, 8},
			{"n", "10 n", 3, 13},
		},
		0,
		0,
		0,
		"The b register is decremented by 1.  If the result is > 0 "
		"the cpu jumps to PC + 2 + n, where n is a signed 8 byte.  The "
		"instruction executes more quickly when the jump it not taken.",
		NULL,
	},
	{
		"ds",
		{
			{"c n", "n c times", 0, 0},
		},
		0,
		0,
		0,
		"Stores c copies of the byte n in the binary.",
		NULL,
	},
	{
		"dw",
		{
			{"nn", "nn", 0, 0},
			{"nn,nn", "nn nn", 0, 0},
		},
		0,
		0,
		0,
		"Stores up to 2 words in the program binary.  All nns must be "
		"formatted in the same way. Only one word can be specified if "
		"an expression is used.",
		NULL,
	},
	{
		"ei",
		{
			{"", "FB", 1, 4},
		},
		0,
		0,
		0,
		"Enables maskable interrupts.",
		NULL,
	},
	{
		"ex",
		{
			{"af, af'", "08", 1, 4},
			{"de, hl", "EB", 1, 4},
			{"sp, (hl)", "E3", 5, 19},
			{"(sp), ix", "DD E3", 6, 23},
			{"(sp), iy", "FD E3", 6, 23},
		},
		0,
		0,
		0,
		"The contents of the two operands are exchanged. "
		"ex af, af' affects all the flags while the other forms of the "
		"instruction have no effect on the flags.",
		"XXXXXXXX",
	},
	{
		"exx",
		{
			{"", "D9", 1, 4},
		},
		0,
		0,
		0,
		"Exchange BC, DE and HL with BC', DE', HL'.",
		NULL,
	},
	{
		"halt",
		{
			{"", "76", 1, 4},
		},
		0,
		0,
		0,
		"CPU execution is suspended until the next interrupt or reset.",
		NULL,
	},
	{
		"im",
		{
			{"0", "ED 46", 2, 8},
			{"1", "ED 56", 2, 8},
			{"2", "ED 5E", 2, 8},
		},
		0,
		0,
		0,
		"Sets the interrupt mode.  With IM 2 the MSB of the vector "
		"address is taken from the I register.",
		NULL,
	},
	{
		"in",
		{
			{"r, (c)", "ED 40+r", 3, 12},
		},
		6,
		0,
		0,
		"Reads a byte from the device at the adddress stored in the bc "
		"register and stores it in r.",
		"XX X X0 ",
	},
	{
		"in",
		{
			{"a, (n)", "DB n", 3, 11},
		},
		0,
		0,
		0,
		"Reads a byte from the device at the adddress whose MSB is "
		"taken from the accumulator and whose LSB is n. The byte is "
		"stored in the accumulator.",
		NULL,
	},
	{
		"inc",
		{
			{"r", "4+r", 1, 4},
			{"(hl)", "34", 3, 11},
			{"(ix + d)", "DD 34 d", 6, 23 },
			{"(iy + d)", "FD 34 d", 6, 23 },
		},
		6,
		0,
		0,
		specasm_doc_inc,
		"XX X X0 ",
	},
	{
		"inc",
		{
			{"rr", "3+rr", 1, 6},
			{"ix", "DD 23", 2, 10 },
			{"iy", "FD 23", 2, 10 },
		},
		3,
		0,
		0,
		specasm_doc_inc,
		NULL,
	},
	{
		"ldd",
		{
			{"", "ED A8", 4, 16},
		},
		0,
		0,
		0,
		"The byte pointed to by hl is loaded into the address in de. "
		"hl, de and bc are decremented.",
		"    0 X0 ",
	},
	{
		"lddr",
		{
			{"", "ED B8", 4, 16},
			{"", "ED B8", 5, 21},
		},
		0,
		0,
		0,
		"The byte pointed to by hl is loaded into the address in de. "
		"hl, de and bc are decremented.  If bc!=0 the instruction "
		"repeats. The instruction consumes more t-states when it "
		"repeats.",
		"   0 00 ",
	},
	{
		"ldi",
		{
			{"", "ED A0", 4, 16},
		},
		0,
		0,
		0,
		"The byte pointed to by hl is loaded into the address in de. "
		"Both hl and de are incremented while bc is decremented.",
		"    0 X0 ",
	},
	{
		"ldir",
		{
			{"", "ED B0", 4, 16},
			{"", "ED B0", 5, 21},
		},
		0,
		0,
		0,
		"The byte pointed to by hl is loaded into the address in de. "
		"Both hl and de are incremented while bc is decremented.  If "
		"bc!=0 the instruction repeats. The instruction consumes more "
		"t-states when it repeats.",
		"   0 00 ",
	},
	{
		"jp",
		{
			{"nn", "C3 n n", 3, 10},
			{"(hl)", "E9", 1, 4},
			{"(ix)", "DD E9", 2, 8},
			{"(iy)", "FD E9", 2, 8},
			{"cc,nn", "C2+cc n n", 3, 12},
			{"cc,nn", "C2+cc n n", 2, 7},
		},
		0,
		0,
		8,
		"Jump to the last operand if the condition is met or no "
		"condition is supplied. Instruction consumes fewer t-states "
		"if condition is not met.",
		NULL,
	},
	{
		"map",
		{
			{"", "", 0, 0},
		},
		0,
		0,
		0,
		"Instructs the linker to generate a map file.",
		NULL,
	},
	{
		"neg",
		{
			{"", "ED 44", 2, 8},
		},
		0,
		0,
		0,
		"Let a = 0 - a.",
		"XX X X1X",
	},
	{
		"nop",
		{
			{"", "0", 1, 4},
		},
		0,
		0,
		0,
		"CPU does nothing for 1 m-cycle.",
		NULL,
	},
	{
		"or",
		{
			{"r", "B8+r", 1, 4},
			{"n", "F6 n",  2, 7},
			{"(hl)", "B6", 2, 7},
			{"(ix + d)", "DD B6 d", 5, 19 },
			{"(iy + d)", "FD B6 d", 5, 19 },
		},
		2,
		0,
		0,
		"The result of a bitwise OR of the accumulator and the "
		"argument is stored in the accumulator.",
		"XX 0 X00",
	},
	{
		"org",
		{
			{"nn","", 0, 0},
		},
		0,
		0,
		0,
		"Assembler directive that sets the org address of the program, "
		"i.e., the address the first byte in the .x or .t file that "
		"contains the Main label is assembled at.",
		NULL,
	},
	{
		"out",
		{
			{"(c), r", "ED 41+r", 3, 12},
		},
		6,
		0,
		0,
		"Writes the register r to the device at the adddress stored "
		"in the bc register.",
		NULL,
	},
	{
		"outdr",
		{
			{"","ED BB", 4, 16},
			{"","ED BB", 5, 21},
		},
		0,
		0,
		0,
		"b is decremented. The byte at the address in hl is written to "
		"the port given by the address in bc, where b has already been "
		"decremented. hl is decremented. The instruction repeats if "
		"b!=0. The instruction consumes more t-states when it repeats.",
		"?1 ? ?1 ",
	},
	{
		"outir",
		{
			{"","ED B3", 4, 16},
			{"","ED B3", 5, 21},
		},
		0,
		0,
		0,
		"b is decremented. The byte at the address in hl is written to "
		"the port given by the address in bc, where b has already been "
		"decremented. hl is incremented. The instruction repeats if "
		"b!=0. The instruction consumes more t-states when it repeats.",
		"?1 ? ?1 ",
	},
	{
		"out",
		{
			{"(n), a", "D3 n", 3, 11},
		},
		0,
		0,
		0,
		"Writes the accumulator to the device at the adddress whose MSB"
		" is taken from the accumulator and whose LSB is n.",
		NULL,
	},
	{
		"outd",
		{
			{"","ED AB", 4, 16},
		},
		0,
		0,
		0,
		"b is decremented. The byte at the address in hl is written to "
		"the port given by the address in bc, where b has already been "
		"decremented. hl is decremented.",
		"?X ? ?1 ",
	},
	{
		"outi",
		{
			{"","ED A3", 4, 16},
		},
		0,
		0,
		0,
		"b is decremented. The byte at the address in hl is written to "
		"the port given by the address in bc, where b has already been "
		"decremented. hl is incremented.",
		"?X ? ?1 ",
	},
	{
		"pop",
		{
			{"rr", "C1+rr", 3, 10},
			{"ix", "DD E1", 4, 14},
			{"iy", "FD E1", 4, 14},
		},
		7,
		0,
		0,
		"Pops 2 bytes off the stack into the operand.",
		NULL,
	},
	{
		"push",
		{
			{"rr", "C5+rr", 3, 11},
			{"ix", "DD E5", 4, 15},
			{"iy", "FD E5", 4, 15},
		},
		7,
		0,
		0,
		"Pushes the operand onto the stack.",
		NULL,
	},
	{
		"res",
		{
			{"n,r", "CB 80+b+r", 2, 8},
			{"n,(HL)", "CB 86+b", 4, 15},
			{"n,(IX + d)", "DDCBd86+b", 6, 23},
			{"n,(IY + d)", "FDCBd86+b", 6, 23},
		},
		2,
		8,
		0,
		"Resets bit n in the second operand.",
		NULL,
	},
	{
		"ret",
		{
			{"", "C9", 3, 10},
			{"cc", "C0 + cc", 1, 5},
			{"cc", "C0 + cc", 3, 10},
		},
		0,
		0,
		8,
		"If there is no condition or the condition is met value of "
		"the stack is popped into the PC, from which program execution "
		"continues. ret takes fewer cycles if the condition is not met"
		".",
		NULL,
	},
	{
		"reti",
		{
			{"", "ED 4D", 4, 14},
		},
		0,
		0,
		0,
		"Return from interrupt. The stack is popped into PC. An ei "
		"instruction must be executed prior to the reti to renable "
		"maskable interrupts.",
		NULL,
	},
	{
		"retn",
		{
			{"", "ED 45", 4, 14},
		},
		0,
		0,
		0,
		"Return from NMI. The stack is popped into PC and the "
		"maskable interrupts are re-enabled if they were enabled "
		"before the NMI.",
		NULL,
	},
	{
		"rl",
		{
			{"r", "CB 10+r", 2, 8},
			{"(hl)", "CB 16", 4, 15},
			{"(ix + d)", "DD CB d 16", 6, 23 },
			{"(iy + d)", "FD CB d 16", 6, 23 },
		},
		2,
		0,
		0,
		"The operand is shifted left by 1 bit. The carry flag "
		"is moved into bit 0 of the operand and the old bit 7 of "
		"the operand is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"rla",
		{
			{"", "17", 1, 4},
		},
		0,
		0,
		0,
		"The accumulator is rotated left 1 bit. The carry flag is "
		"moved to bit 0 of the accumulator and its old bit 7 is moved "
		"to the carry flag.",
		"   0  0X",
	},
	{
		"rlc",
		{
			{"r", "CB r", 2, 8},
			{"(hl)", "CB 06", 4, 15},
			{"(ix + d)", "DD CB d 06", 6, 23 },
			{"(iy + d)", "FD CB d 06", 6, 23 },
		},
		2,
		0,
		0,
		"The operand is rotated left 1 bit. The old bit 7 is moved to "
		"both the carry flag and bit 0.",
		"XX 0 X0X",
	},
	{
		"rlca",
		{
			{"", "07", 1, 4},
		},
		0,
		0,
		0,
		"The accumulator is rotated left 1 bit. The old bit 7 is moved "
		"to both the carry flag and bit 0.",
		"   0  0X",
	},
	{
		"rld",
		{
			{"", "ED 6F", 5, 18},
		},
		0,
		0,
		0,
		"Let tmp = (hl) >> 4             "
		"Let (hl) = ((hl) << 4)|(a & $f) "
		"Let a = (a & $f0)|tmp",
		"XX 0 X0 ",
	},
	{
		"rr",
		{
			{"r", "CB 18+r", 2, 8},
			{"(hl)", "CB 1E", 4, 15},
			{"(ix + d)", "DD CB d 1E", 6, 23 },
			{"(iy + d)", "FD CB d 1E", 6, 23 },
		},
		2,
		0,
		0,
		"The operand is shifted right by 1 bit. The carry flag "
		"is moved into bit 7 of the operand and the old bit 0 of "
		"the operand is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"rra",
		{
			{"", "1F", 1, 4},
		},
		0,
		0,
		0,
		"The accumulator is shifted right 1 bit. The carry flag is "
		"moved into bit 7 of the accumulator and the old contents of "
		"the accumulator's bit 0 are moved into the carry flag."
		"   0  0X",
	},
	{
		"rrc",
		{
			{"r", "CB 8+r", 2, 8},
			{"(hl)", "CB 0E", 4, 15},
			{"(ix + d)", "DD CB d 0E", 6, 23 },
			{"(iy + d)", "FD CB d 0E", 6, 23 },
		},
		2,
		0,
		0,
		"The second argument and the carry flag are subtracted from "
		"the contents of the destination register, either a or hl.",
		"XX 0 X0X",
	},
	{
		"rrca",
		{
			{"", "0F", 1, 4},
		},
		0,
		0,
		0,
		"The accumulator is rotated right by 1 bit. The old bit 0 is "
		"moved into both the carry flag and bit 7 of the accumulator.",
		"   0  0X",
	},
	{
		"rrd",
		{
			{"", "ED 67", 5, 18},
		},
		0,
		0,
		0,
		"Let tmp = a << 4             "
		"Let a = (a & $f0)|((hl) & $f) "
		"Let (hl) = tmp | ((hl) >> 4)",
		"XX 0 X0 ",
	},
	{
		"rst",
		{
			{"n", "C3+n", 3, 11},
		},
		0,
		0,
		0,
		"The PC is pushed to the stack and the CPU jumps to the "
		"address n.  Valid values of n are 0, 8, 16, 24, 32, 40, 48,"
		" and 56.",
		NULL,
	},
	{
		"sbc",
		{
			{"a,r", "98+r", 1, 4},
			{"a,n", "DE n",  2, 7},
			{"a,(hl)", "9E", 2, 7},
			{"a,(ix + d)", "DD 9E d", 5, 19 },
			{"a,(iy + d)", "FD 9E d", 5, 19 },
			{"hl,rr", "ED 42+rr", 4, 15 },
		},
		1,
		0,
		0,
		"The second argument and the carry flag are subtracted from "
		"the contents of the destination register, either a or hl.",
		"XX X X1X",
	},
	{
		"scf",
		{
			{"", "37", 1, 4},
		},
		0,
		0,
		0,
		"Sets the carry flag.",
		"   0  01"
	},
	{
		"set",
		{
			{"n,r", "CB C0+b+r", 2, 8},
			{"n,(HL)", "CB C6+b", 4, 15},
			{"n,(IX + d)", "DDCBdC6+b", 6, 23},
			{"n,(IY + d)", "FDCBdC6+b", 6, 23},
		},
		2,
		8,
		0,
		"Sets bit n in the second operand.",
		NULL,
	},
	{
		"sla",
		{
			{"r", "CB 20+r", 2, 8},
			{"(hl)", "CB 26", 4, 15},
			{"(ix + d)", "DD CB d 26", 6, 23 },
			{"(iy + d)", "FD CB d 26", 6, 23 },
		},
		2,
		0,
		0,
		"The operand is shifted left by 1 bit. Bit 0 "
		"is set to 0. Bit 7 is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"sra",
		{
			{"r", "CB 28+r", 2, 8},
			{"(hl)", "CB 2E", 4, 15},
			{"(ix + d)", "DD CB d 2E", 6, 23 },
			{"(iy + d)", "FD CB d 2E", 6, 23 },
		},
		2,
		0,
		0,
		"The operand is arithmetically shifted right by 1 bit. Bit 7 "
		"is set to 0. Bit 0 is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"srl",
		{
			{"r", "CB 38+r", 2, 8},
			{"(hl)", "CB 3E", 4, 15},
			{"(ix + d)", "DD CB d 3E", 6, 23 },
			{"(iy + d)", "FD CB d 3E", 6, 23 },
		},
		2,
		0,
		0,
		"The operand is logically shifted right by 1 bit. Bit 7 is "
		"unchanged. Bit 0 is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"sub",
		{
			{"r", "A0+r", 1, 4},
			{"n", "D6 n",  2, 7},
			{"(hl)", "96", 2, 7},
			{"(ix + d)", "DD 96 d", 5, 19 },
			{"(iy + d)", "FD 96 d", 5, 19 },
		},
		2,
		0,
		0,
		"The operand is subracted from the accumlator.",
		"XX X X1X",
	},
	{
		"xor",
		{
			{"r", "A8+r", 1, 4},
			{"n", "EE n",  2, 7},
			{"(hl)", "AE", 2, 7},
			{"(ix + d)", "DD AE d", 5, 19 },
			{"(iy + d)", "FD AE d", 5, 19 },
		},
		2,
		0,
		0,
		"The result of a bitwise XOR of the accumulator and the "
		"argument is stored in the accumulator.",
		"XX 0 X00",
	},
	{
		"zx81",
		{
			{"", "", 0, 0 },
		},
		0,
		0,
		0,
		"Linker directive that causes string and character literals to "
		"be transliterated from ASCII to ZX81 encoding.  It also sets "
		"org address to 16514.",
		NULL,
	},

};

const uint8_t max_docs = sizeof(docs) / sizeof(specasm_ins_doc_t);

static uint8_t prv_pretty_print(uint8_t y, const char * text)
{
	uint8_t i;

	while (*text) {
		memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
		scratch[SPECASM_LINE_MAX_LEN] = 0;
		for (i = 0; *text && i < SPECASM_LINE_MAX_LEN; i++) {
			scratch[i] = *text++;
		}

		if (i == SPECASM_LINE_MAX_LEN) {
			for (i = i - 1; scratch[i] != ' '; i--) {
				text--;
				scratch[i] = ' ';
				if (i == 0)
					break;
			}
		}

		specasm_util_print(scratch, 0, y++, PAPER_BLACK | INK_WHITE);
	}

	return y;
}

static void prv_justify_num(uint8_t num)
{
	char *s = scratch;

	if (num < 10) {
		s[0] = ' ';
		s++;
	}
	itoa(num, s, 10);
}

static uint8_t prv_print_register_encoding(const specasm_ins_doc_t *doc,
					   uint8_t y)
{
	uint8_t i;
	uint8_t reg_encoding;
	const char* reg_name;
	char *s2;
	char *rr_start = NULL;
	const uint8_t* enc;
	char *s = scratch;

	if (!doc->reg_encoding)
		return y;

	enc = (const uint8_t*) &reg_encodings[doc->reg_encoding-1];

	for (i = 0; i < SPECASM_DOC_MAX_REG_ENCODING; i++) {
		reg_encoding = enc[i];
		if (reg_encoding) {
			if ((i > 6) && !rr_start)
				rr_start = s;
			s += 3;
		}
	}

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	if (!rr_start) {
		scratch[15] = 'r';
	} else if (rr_start == scratch) {
		scratch[14] = 'r';
		scratch[15] = 'r';
	} else {
		scratch[(rr_start - scratch) / 2] = 'r';
		s2 = SPECASM_LINE_MAX_LEN + scratch;
		s = &rr_start[((s2 - rr_start) / 2)];
		s -= 3;
		*s = 'r';
		s[1] = 'r';
	}
	specasm_util_print(scratch, 0, y, PAPER_WHITE | INK_BLACK);
	y++;

	s = scratch;
	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	for (i = 0; i < SPECASM_DOC_MAX_REG_ENCODING; i++) {
		reg_encoding = enc[i];
		if (reg_encoding) {
			s2 = s;
			reg_name = reg_names[i];
			if (!reg_name[1])
				s2++;
			memcpy(s2, reg_name, strlen(reg_name));
			s +=3;
		}
	}
	specasm_util_print(scratch, 0, y, PAPER_BLUE | INK_WHITE);

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	s = scratch;
	for (i = 0; i < SPECASM_DOC_MAX_REG_ENCODING; i++) {
		reg_encoding = enc[i];
		if (reg_encoding) {
			reg_encoding--;
			s2 = s;
			if (reg_encoding < 16)
				s2++;
			itoa(reg_encoding, s2, 16);
			s2[strlen(s2)] = ' ';
			s +=3;
		}
	}

	specasm_util_print(scratch, 0, y + 1, PAPER_BLACK | INK_WHITE | 64);

	return y + 3;
}

static void prv_print_flags(uint8_t y, const char *flags)
{
	if (!flags)
		return;

	specasm_util_print("sz-h-pnc", 12, y, PAPER_BLUE | INK_WHITE);
	y++;
	specasm_util_print("Flags", 5, y, PAPER_BLACK | INK_WHITE);
	specasm_util_print(flags, 12, y, PAPER_BLACK | INK_WHITE | 64);
}

static uint8_t prv_print_bits_encoding(const specasm_ins_doc_t *doc,
				       uint8_t y)
{
	uint8_t i;
	uint8_t x;
	uint8_t num;
	char *s;

	if (!doc->bits)
		return y;

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[15] = 'b';
	specasm_util_print(scratch, 0, y, PAPER_WHITE | INK_BLACK);
	y++;

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	s = scratch;
	for (i = 0; i < 8; i++) {
		s[2] = i + '0';
		s += 4;
	}
	specasm_util_print(scratch, 0, y, PAPER_BLUE | INK_WHITE);
	y++;

	for (i = 0; i < 8; i++) {
		num = doc->bits * i;
		itoa(num, scratch, 16);
		x = (i*4)+1;
		if (num < 16)
			x++;
		specasm_util_print(scratch, x, y, PAPER_BLACK | INK_WHITE);
	}

	return y + 2;
}

static uint8_t prv_print_cc_encoding(const specasm_ins_doc_t *doc,
				     uint8_t y)
{
	uint8_t i;
	uint8_t x;
	uint8_t num;
	char *s;

	if (!doc->all_cc)
		return y;

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[14] = 'c';
	scratch[15] = 'c';
	specasm_util_print(scratch, 0, y, PAPER_WHITE | INK_BLACK);
	y++;

	specasm_util_print(" nz   z  nc   c  po  pe   p   m ", 0,
			   y, PAPER_BLUE | INK_WHITE);
	y++;

	for (i = 0; i < 8; i++) {
		num = doc->all_cc * i;
		itoa(num, scratch, 16);
		x = (i*4)+1;
		if (num < 16)
			x++;
		specasm_util_print(scratch, x, y, PAPER_BLACK | INK_WHITE);
	}

	return y + 2;
}

static void prv_draw_help(uint8_t ins_id)
{
	uint8_t x;
	uint8_t i;
	uint8_t y;
	uint8_t col;
	const specasm_ins_doc_t* doc = &docs[ins_id];
	const specasm_ins_form_t *form;
	const char* ins_name = doc->name;
	uint8_t ins_name_len = strlen(ins_name);

	specasm_cls(PAPER_BLACK | INK_WHITE);
	x = (SPECASM_LINE_MAX_LEN / 2) - (strlen(ins_name) >> 1);

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[SPECASM_LINE_MAX_LEN] = 0;
	memcpy(&scratch[x], ins_name, ins_name_len);

	specasm_util_print(scratch, 0, 0, PAPER_BLUE | INK_WHITE | 64);

	y = 2;
	memcpy(scratch, "Opcode          Encoding    M  T",
	       SPECASM_LINE_MAX_LEN);
	specasm_util_print(scratch, 0, y, PAPER_BLUE | INK_WHITE);
	y++;
	col = PAPER_BLACK | INK_WHITE;
	for (i = 0; i < 6; i++) {
		col ^= 1 << 6;
		form = &doc->forms[i];
		if (!form->form)
			break;
		x = 0;
		specasm_util_print(ins_name, x, y, col);
		x += ins_name_len + 1;
		specasm_util_print(form->form, x, y, col);
		specasm_util_print(form->encoding, SPECASM_DOC_ENCODING_X, y,
				   col);
		prv_justify_num(form->m_cycles);
		specasm_util_print(scratch, SPECASM_DOC_M_CYCLES_X, y, col);
		prv_justify_num(form->t_states);
		specasm_util_print(scratch, SPECASM_DOC_T_STATES_X, y, col);
		y++;
	}

	y = prv_print_register_encoding(doc, y + 1);
	y = prv_print_bits_encoding(doc, y);
	y = prv_print_cc_encoding(doc, y);
	y = prv_pretty_print(y, doc->description);
	prv_print_flags(y + 1, doc->flags);
}

static uint8_t prv_find_mnemomic(const char *ins_name)
{
	uint8_t m;
	uint8_t l;
	uint8_t r;
	int res;

	if (strcmp(ins_name, docs[0].name) < 0)
		return 0;

	l = 0;
	r =max_docs - 1;

	while (l <= r) {
		m = (l + r) >> 1;
		res = strcmp(docs[m].name, ins_name);
		if (res < 0) {
			l = m + 1;
		} else if (res > 0) {
			if (m == 0)
				break;
			r = m - 1;
		} else {
			return m;
		}
	}

	/*
	 * Opcode not found.  Let's return something close.
	 */

	return r;
}

void specasm_help_banked(const char *ins_name)
{
	uint8_t id;
	uint8_t k;

	id = prv_find_mnemomic(ins_name);
	do {
		prv_draw_help(id);
		do {
			specasm_sleep_ms(25);
		} while(!(k = in_inkey()));

		if (k == SPECASM_KEY_LEFT) {
			if (id > 0)
				id--;
		} else if (k == SPECASM_KEY_RIGHT) {
			if (id < max_docs - 1)
				id++;
		} else {
			break;
		}
	} while(1);

	specasm_cls(PAPER_BLACK | INK_WHITE);
}
