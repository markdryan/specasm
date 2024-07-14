# Specasm

Specasm is a Z80 assembler designed to run on the 48k and 128k ZX Spectrums with an ESXDOS SD card solution, and the ZX Spectrum Next.  It includes an editor, an assembler and a linker.

## Editor/Assembler

The editor can be launched from the command line using the .specasm command.  Specasm is an integrated editor/assembler.

> [!TIP]
> The ZX Spectrum Next version of Specasm allows the name of a single .x file to be passed on the command line.  This file will be automatically loaded into the editor on startup, e.g., .specasm game.x.

The editor allows one single source, .x, file to be edited at any one time.  Each such file can contain a maximum of 512 lines.  Each line can be

- An empty line
- An assembly language instruction or directive followed by an optional comment
- A comment
- A label

The editor parses the contents of the current line when the "ENTER" key is pressed, when the user navigates away from the line using another key, e.g., the down cursor, or the user invokes command mode, via "SYMSHFT"+w.  If the contents of the line are deemed to be valid, the editor will format the line and re-display it.  If there's an error in the line, the editor will display an error message and will not allow the user to navigate away from the line or enter command mode until the error has been fixed.  To try this out, try typing

```
ld a
```

and pressing "ENTER".  You should see the error "Comma expected" displayed at the bottom of the screen in red.  To navigate away from the line you need to fix the error, e.g.,

```
ld a, 10
```

Now when "ENTER" is pressed the instruction is accepted and the editor moves to the next line.

The editor assigns special meaning to some of the keys and key combinations available on the Spectrum.  These are summarised in the table below

| Keys        | Command |
|-------------|---------|
| Cursor keys | Move up, down, left and right |
| ENTER       | Move to next line.  Insert new line if in insert mode and cursor is at column 0 |
| CAPS + 1    | Go to start of file |
| CAPS + 2    | Go to end of file |
| CAPS + 3    | Page Up          |
| CAPS + 4    | Page Down        |
| SYMSHIFT + i | Toggle insert mode |
| SYMSHIFT + q | Go to start of line |
| SYMSHITF + e | Go to end of line |
| SYMSHIFT + w | Enter command mode |

On entering command mode, a '>' prompt appears at the bottom of the screen.  From this prompt a number of different commands are available.  Some of the commands accept a parameter.  To execute a command type the command name followed by any parameters and then press ENTER.  Here's a list of the currently supported commands.

| Command | Description |
|---------|-------------|
| a       | Selects the entire file.  See selecting below |
| n       | Resets the editor, deleting any lines currently in memory |
| q       | Quits the editor and returns back to BASIC |
| d       | Deletes the currently selected block |
| b       | Counts the number of machine code bytes in the selected block of code or data |
| c       | Copies the selected block of code to the cursor's position |
| m       | Moves the selected block of code to the cursor's position |
| sel     | Enters selection mode.  See below |
| ver     | Displays the current version of Specasm |
| l *filename* | Loads *filename* into the editor.  *filename* should not be surrounded by quotes.  The .x extension is optional |
| s *filename* | Saves the contents of the editor to *filename*.  The .x extension is optional.  If the name of the current file being edited is known to Specasm, the file name can be ommitted, i.e., **> s** saves the current file. |
| g *line number* | Moves the cursor to the specified line number|
| f *string* | Searches for the first instance of *string* starting from the current line.  There's no wrap around. |

The Next version of Specasm uses one of the Next's 8Kb memory banks to implement a clipboard.  This provides a more traditional copy, cut and paste mechanism than the copy and move commands described above, and allows code to be copied from one file to another.  Clipboard support is provided via three new Next specific commands.

| Command | Description |
|---------|-------------|
| x       | Replaces the contents of the clipboard with the selected code and deletes the code from the current file |
| cc       | Replaces the contents of the clipboard with the selected code |
| v        | Pastes the contents of the clipboard into the selected code at the cursor position |

The Next and the Spectrum 128 versions of Specasm provide two additional commands that can be used to help with code analysis.  Both commands operate on a previously selected block of code.

| Command | Description |
|---------|-------------|
| t       | Displays the number of M cycles and T states the selected code will take to execute.  Specasm prints two numbers for each value, a minimum and a maximum.  The value is likely to be inaccurate for instructions whose running times depend on runtime state, e.g., LDIR.  |
| fl       | Displays the flags modified by a selected block of code. |

#### Selecting Mode

The *sel* command switches the editor into selection mode.  In selection mode the user can press the up and down keys to select a block of code.  They can also press the 'a' key to select the entire file.  Only whole lines can be selected.  To cancel the selection and return to editor mode, press SPACE.  To delete the selected lines and return to editor mode, press DELETE.  On the Next, the selected lines may be cut to the clipboard by typing 'x'.  To confirm the selection and return to editor mode, press ENTER.  Once the selection has been confirmed an '*' will appear in the status row at the bottom of the screen to the right of 'INS' or 'OVR'.  This indicates that some lines are currently selected.  These lines can be manipulated using the 'd', 'm', 'c','b', 'x' and 'cc' commands described above.  For example, to count the number of machine code bytes in the selected line, type SYMSHIFT+w followed by 'b' and ENTER.  The editor is capable of computing the exact size in bytes of the machine code associated with the instructions and data in the selected region as it has already assembled these instructions and knows exactly how much space they will consume.

All of the six commands that manipulate selections, cancel the selection once they have finished executing.  So if you select a block of text, and then issue the 'b' command to count the number of bytes it consumes, you'll need to reselect the text once more to copy it.  In addition, the current selection is cancelled if you make any changes to the contents of the editor, e.g., edit an existing line or insert a new one.

#### The Status Row

The bottom row of the editors display is the status row.  It has three fields.  The first field, indicates whether insert mode or overwrite mode is active.  It also, indicates via a '*' whether any lines are currently selected.  The middle field displays the number of lines used in the buffer and the maximum number of lines available.  The final field, displays the current column number and the current line number.  Note that the first line in the file is line 0, not line 1.

### Assembly Syntax

#### Mnemonics

The assembler support all the documented z80 instructions.  In addition, the ZX Spectrum Next version of Specasm supports all the Z80n instructions.  All mnemonics are entered in lower case.  Commas, ',', are used to separate multiple arguments.  The condition codes for jump and call instructions are separated from the main mnemonic by a space and from the target address by a comma.  For example,

```
add a, c
call nc, label
jr c, label
```

#### Index Registers

There are some peculiarities related to the use of the index registers when used to access data indirectly.

* An offset must always be specified, even if the offset is 0.
* The '+' sign separating the index register from the offset must be specified.

Here are some valid examples

```
ld (ix+0), a
ld (ix+-1), a
ld (iy+$7f), a
```

and here are some invalid examples

```
ld (ix), a
ld (ix-1), a
```

#### Numbers

Numbers can be specified in decimal, hex or as characters.  Decimal numbers require no prefix, hex numbers use the '$' prefix and characters are enclosed with single quotes.  For example,

```
org $8000
ld ix, -32768
ld a, 'c'
ld l, (ix+-100)
```

Note how the + must be specified when using a negative number with an index register.

In most places the user can chose what form of number to use.  For example, the assembler permits

```
ld ix, -1
```

and

```
ld ix, $ffff
```

Both statements will assemble to the same opcode.  However, there are certain places where this is not allowed, namely when dereferencing an absolute address or specifying an offset to one of the index registers.  For example, the follow two statements will not assemble.

```
ld hl, (-1)
sbc a, (ix+-129)
```


#### Labels

Label definitions must occur on a line by themselves.  They are defined by specifying a '.' followed by the label name.  The first character of a label name can be the underscore character '_' or an upper or lower case letter.  Subsequent characters can be underscores, upper or lower case letter or numbers.  The leading '.' is not considered to be part of the label name.

Labels that begin with upper case letters are considered to be global labels, i.e., they are visible to code in other .x files.  All other labels are local to the file in which they are defined.

Labels can be up to 31 characters in length but in practice they need to be shorter, otherwise it will not be possible to refer to them in instructions or directives, e.g.,

```
.this_is_a_very_very_long_label
  call this_is_a_very_very_long_label
```

The assembler will let us define the label 'this_is_a_very_very_long_label' but the following statement cannot be entered as it is longer than 32 characters.

Labels cannot be register names.  For example, 'a' and 'hl' are invalid label names.

Here are some valid label examples.

```
; Main label, this one is special
.Main
; A global label
.PrintString
; A local label
.l_1
```

#### Expressions and Constants ####

Constants can be defined anywhere within .x files.  The syntax is

```
.label equ expression
```

Where label is a normal label, either local or global, and expression is an expression that evaluates to a 16 bit integer.  For example

```
.topbit equ 1 << 7
```

The labels of equ statements share the same namespace as ordinary labels.  So, code like

```
.topbit equ 1 << 7
.topbit
```

is not allowed.

Expressions in Specasm can contain 16 bit numbers and labels. The labels can either be a label of a location within the code or that of an equ statement.  There is one restriction here.  An equ statement that defines a global label, i.e.,  one whose label begins with an upper case letter, cannot contain a reference to a local label.  However, equ statements that define local labels can reference global labels.  In addition,
recursive definitions are not permitted.  For example, the following sequences of equ definitions are invalid in Specasm.

```
.local equ 10
.Global equ local / 2
```

```
.local1 equ local2
.local2 equ local3
.local3 equ local1
```

The following operators are supported

| Operator   | Description |  Precedence |
|------------|-------------|------------ |
| -          | unary minus |  0          |
| ~          | complement  |  0          |
| ( )        | brackets    |  0          |
| *          | multiply    |  1          |
| /          | div         |  1          |
| %          | mod         |  1          |
| +          | plus        |  2          |
| -          | minus       |  2          |
| <<         | left shift  |  3          |
| >>         | right shift |  3          |
| &          | bitwise and |  4          |
| \|         | bitwise or  |  4          |
| ^          | bitwise eor |  4          |

Expressions can be used in certain instructions that accept 8 or 16 bit immediate integers. When an expression or a constant, defined by an equ statement, is used by an instruction they must be preceded by an '=' sign.  For example,

```
.equ index 6
.start
ld a, =10*2
or =1 << index
res =index, a
.end
db =end-start
```

Constants defined by equ statements cannot be used directly as the target of a jump or call instruction, without specifying the '=' sign.  For example,

```
.opench equ 5563
.target
call =opench  ; Allowed
jp target     ; Allowed
ret
call opench   ; Not allowed
jp opench     ; Not allowed
call target   ; Allowed
call =target  ; Allowed
jp =target    ; Allowed
```

Note however that normal labels that mark a position within the code, can be used in expressions, either by themselves or in combination with other operators, e.g.,

```
.target
ret

.fn
call target
call =target
call =target + 1
```

The first two call statements in this example generate the same machine code instruction.

The following instructions support expressions.

| Instruction / Directive       |
|-------------------------------|
| adc a, =expression            |
| add a, =expression            |
| add hl, =expression (nx)      |
| add de, =expression (nx)      |
| add bc, =expression (nx)      |
| and =expression               |
| bit =expression, [a-l]        |
| call =expression              |
| cp =expression                |
| jp =expression                |
| in a, (=expression)           |
| ld a, =expression             |
| ld a, (=expression)           |
| ld (=expression), a           |
| ld b, =expression             |
| ld c, =expression             |
| ld d, =expression             |
| ld e, =expression             |
| ld h, =expression             |
| ld l, =expression             |
| ld bc, =expression            |
| ld de, =expression            |
| ld hl, =expression            |
| ld hl, (=expression)          |
| ld (hl), =expression          |
| ld (ix+n), = expression       |
| ld (iy+n), = expression       |
| ld (=expression), hl          |
| ld sp, =expression            |
| out (=expression), a          |
| or =expression                |
| rst =expression               |
| sbc a, =expression            |
| sub a, expression             |
| xor =expressio                |
| bit =expression, [a-l]        |
| bit =expression, (hl)         |
| ld ix, =expression            |
| ld iy, =expression            |
| ld bc, (=expression)          |
| ld (=expression), bc          |
| ld de, (=expression)          |
| ld (=expression), de          |
| ld ix, (=expression)          |
| ld (=expression), ix          |
| ld iy, (=expression)          |
| ld (=expression), iy          |
| ld (=expression), sp          |
| ld sp, (=expression)          |
| im =expression                |
| nextreg =expression, imm (nx) |
| nextreg =expression, a (nx)   |
| push =expression (nx)         |
| res =expression, [a-l]        |
| res =expression, (hl)         |
| set =expression, [a-l]        |
| set =expression, (hl)         |
| test =expression (nx)         |
| dw =expression                |
| db =expression                |

The (nx) suffix indicates that the instruction is a Spectrum Next specific instruction.  The nextreg instruction only allows an expression in the first parameter.

Expressions cannot be used as an offset in combination with an index register, e.g.,

```
add a, (ix + =10*10)
```

is not supported.

There's one additional restriction.  Only a single value can be specified when an expression is used with a data directive.  For example,

```
dw =10^1, =constant
```

is not permitted.  This needs to be written as

```
dw =10^1
dw =constant
```

One final note.  Expressions are not parsed or evaluated until link time.  As a result you will not be warned about a syntax error in your expressions until you link your program.  For example, Specasm's editor will allow you to enter and save the following code.

```
.shift equ 1 <<
ld a, =shift
```

No errors will be reported until link time.  This is unfortunate and contrary to the way Specasm handles instructions that do not use expressions.  Unfortunately, there's not enough memory left to include an expression checker in the editor/asssembler, and even if there was, it would not be possible to catch all the errors at assembly time, as it may not be possible to resolve the expressions until link time.  For example,

*main.x*
```
.Global equ 16
```

*other.x*
```
im =Global
```

Here, the expression used in the im statement is invalid as it resolves to a value that is larger than 2.  However, this can't be known until link time as the constant Global is defined in a different file.

#### Label Subtraction

> **Warning**
> Old versions of Specasm, pre v7 supported something called label subtraction, which allowed the result of subtracting one label from another to be stored in a register, e.g., ld hl, end-start.  This syntax was rendered redundant by the introduction of expressions in Specasm v5 and deprecated in that release. In Specasm v5 and above the statement should be written as ld hl, =end-start.  Support for label subtraction was dropped from the editor in Specasm v7 and from the linker in Specasm v9.  If you try to link an old .x file created with Specasm v5 or older that performs label subtraction, the linker in Specasm v9 and above will report an error that will look something like this;  "code.x too old. Load/save  in Specasm".  To resolve this error simply follow the instructions.  Load the code.x file in Specasm and then  save it.  Any instructions in code.x that use label subtraction will be rewritten to use the new expression syntax when loaded into Specasm's editor, and can be successfully linked.

#### Comments

Comments are introduced using the semi-colon.  Comments can appear on the same line as an instruction or directive (but not a label).  Comments that appear on the same line as an instruction are limited to 11 characters in length.  Comments that appear in a line by themselves can occupy the entire line, i.e., can be 31 characters in length.

Here are some example comments.

```
; Load 10 into the accumulator.

ld a, 10
or 8                  ; or with 8
```

#### Data directives

The assembler provides three directives that can be used to store numeric constant data directly in machine code programs.  These are

| Directive | Description                                                               |
|-----------|---------------------------------------------------------------------------|
| db        | Used to encode up to 1 to 4 bytes.  Multiple bytes are comma separated.   |
| dw        | Used to encode 1 or 2 16 bit words.  Multiple words are comma separated. Can also be used to encode the address of a label resolved at link time|
| ds        | Encodes 1 or more copies of a given byte |


Numbers can be specified as hex digits, signed and unsigned numbers or as characters.  There's one restriction here though.  If you specify multiple numbers on the same line the numbers must be of the same format, e.g., all hex digits or all characters.  Unsigned and signed numbers can be mixed as long as all the numbers can be represented by a signed number.  Here are some examples that are allowed

```
db 10
dw $1000
db 10, -10, 11, -11
db 'a', 'b', 'c'
dw 'c'
db 255, 255
```

And here are some examples that aren't

```
; Both numbers cannot be represented by a signed byte
db -1, 255
; Mixing hex and signed decimals
db -1, -2, -3, $20
; Mixing characters and decimals
db 'A', 1, 2
```
In addition to encoding numbers the **dw** directive can be used to encode the address of a label.  When used in this format, the **dw** directive can only contain a single argument.

For example,

```
dw data
```

will encode the address of the label data directly into the program.  The address is encoded at link time when salink has figured out the final address of the label.  To encode the difference between two labels the expression syntax must be used.

```
dw =end-start
.start
dw 10, 10
db 1
.end
```

will store the value 5 in a 16 bit word in the final program.

The **db** directive cannot be used to encode the address of a label as the address is unlikely to fit into a single byte.  It can however, be used to encode the difference between two labels using the expression syntax, providing the difference does fit into a byte.  If the labels are too far apart an error will be generated at link time.  When used in this form, no other numbers can follow the label subtraction on the same line.  For example,

```
db =end-start
.start
dw 10, 10
db 1
.end
```

Will store the byte 5 in memory followed by 5 bytes of data.

The **ds** directive can be used to store multiple copies of the same byte value directly into the program code.  **ds** requires two parameters separated by a comma.  The first is 16 bit integer that specifies the number of copies of the first parameter to be encoded.  The second value is the byte value to store.  This can be specified as a signed or unsigned decimal number, a hexadecimal number or a character.   It is mandatory.  For example,

```
ds 12, 'A'
```

encodes 12 copies of the ASCII value of 'A' directly in the program.  The size of this mnemonic will be reported as 12 in the editor.

#### Strings

Strings of characters can be encoded directly into the object code using one of four string directives.  Each of these directives is a denoted by a single characters; ', ", @, #.  The string directives can be used in two ways.  The string can be enclosed completely between matching directives, e.g.,

```
"hello"
```

In this case the 5 bytes are written into the program code at link time.  Alternatively, it's possible to leave the string unterminated, e.g.,

```
"hello
```

In this case the entire contents of the line following the " character are written to the linked binary, 31 bytes in total.  It's possible to use the single quote character instead of the double quote character.  This is necessary if you need to encode the double quote character into the program code, e.g.,

```
'"Hello"'
```

The @ and # directives output the length of the string, as a byte, before outputting the actual string.  So,

```
@Hello@
```

Actually outputs 6 bytes; 5, 'H', 'e', 'l', 'l', 'o'.  Again two different directives are provided so that both the # and @ characters can be encoded.

#### Align

Specasm supports one more directive, **align**, that can be used to align the code or data that follows the **align** directive to a given power of 2.  The **align** directive takes one argument that must be a power of 2, greater than or equal to 2 and less and or equal to 256.  It inserts null bytes into the binary until the requested alignment is achieved.  Consider the following, simple program

```
.Main
org $800a
align 16
db 1
```

Once assembled and linked our binary would be 7 bytes in size.  The first 6 bytes would be 0s and the final byte would contain a 1.

#### NBRK

On the ZX Spectrum Next a pseudo instruction called NBRK is supported.  This generates the byte code sequence for NEXTREG 2, 8 which launches the Next's built in debugger.

## Limitations

### Strings

Each .x file is limited to a total of 32 long strings and 128 short strings.  Long strings are strings which are greater than 11 characters.  Strings in this context refer to strings used in the string data directives, strings used for label names and strings used for comments.  If you exceed the limit of short or long strings in a file you'll get an error.  This means your limited to a maximum of 160 labels per file, and only if your file doesn't contain any comments or string data directives.  Using the exact same string multiple times only counts as one string.  For example, the following code consumes only 1 of the available 128 short strings, even though it uses 3 strings.

```
.hello
call hello ;hello
```

There's a known issue with the way in which Specasm treats strings.  Strings are never garbage collected.  This means that if you create a new label called say '.lbel', press ENTER, and then subsequently update the label's name to say '.label', the original label '.lbel' is not garbage collected.  It will continue to occupy a slot in the string table of the .x file you're editing.  If this file is small or doesn't change much this is unlikely to ever cause any issues.  If however, you're working on a large file with lots of strings and which, over time, receives lots of edits, Specasm may eventually report an error, e.g., "Too many long strings", even though your .x file uses less than the maximum 32 number of long strings.  I've decided not to fix this for now as doing so would use up some valuable bytes, that I'd prefer to use for something else, and would also slow down the editor.  If you do encounter this problem it can be solved, by using .saexport to export your .x file to a text file, a .s file, and then .saimport to re-assemble it, e.g.,

```
CLEAR 32767
.saexport big.x
.saimport big.s
```

> [!TIP]
> Note the CLEAR statement is not needed on the ZX Spectrum Next as .saexport and .saimport are implemented as dotn files.

A future 128kb version of Specasm will incorporate the import and export commands directly in the editor and will also contain a 'gc' command to allow the garbage collection to be performed in one simple step.

### Source files

Specasm edits .x files, which are not text files. They're annotated object files, in which all the instructions are pre-assembled. This approach has some advantages and some disadvantages. The advantages are fast load and save times as there's no parsing required, and fast build times as the build, performed by the .salink command, consists of only one stage, the linker. The disadvantages are that it's more cumbersome to submit source files to source control as the source files are not text files. Separate tools need to be run, saimport and saexport that convert between .x files to .s files, before source code and be moved to and from source control. The other disadvantage is that there's a limited amount of memory per line (5 bytes) and it's not possible to encode all the formatting and expression information into just five bytes.  For this reason Specasm places a restrictions on how some instructions and expressions can be written, e.g, expressions cannot be used as offsets to an index register.

To convert a .s file, a text file containing specasm code, into a .x file you need to use the .saimport dotx command.  It accepts one or more command line parameters, which must be the paths of .s files.  It will convert each of these .s files to .x files.  For example, to convert the file hello.s into hello.x copy hello.s to the spectrum and type

```
CLEAR 32767
.saimport *.s
```

> [!TIP]
> Note the CLEAR statement is not needed on the ZX Spectrum Next.

The reverse process can be performed using the .saexport command.  Note the use of the CLEAR statement.  This needs to be executed on the 48Kb or 128Kb Spectrum before the .saimport or .saexport commands as these are dotx commands and part of their code is loaded and executed from BASIC's memory.  To ensure there's no interference between BASIC and the dotx commands, the CLEAR command must be issued before their use.  If you're executing a sequence of .saimport/.saexport commands, You only need to issue the CLEAR command once.  The CLEAR statement is not needed at all on the ZX Spectrum Next which implements saexport and saimport using dotn commands.

## Program structure

A Specasm program is comprised of one or more .x files.  When you build a Specasm program with the .salink command it looks for all the .x files in the folder in which it is run, and links them all together, concatenating them all into one single file and resolving any addresses, e.g., jump targets.

> [!TIP]
> Specasm programs can actually span multiple directories.  See the +, - and ! directives below.


One of the .x files must contain a label called **Main**, .e.g, it must have the following statement somewhere within one of the files

```
.Main
```

The name of the resulting binary will be derived from the name of the .x file that contains the **Main** label.  So if a project places the Main label in a file called **game.x**, the name of the resulting binary created by salink will be **game**.

The salink command will place the code from the .x file that declares the Main label first in the newly created binary.  In Specasm versions v7 and eearlier, the order in which the rest of the code is written to the binary is arbitrary and the user has no control over this.  In Specasm v8 and above, the order in which the code from the remaining .x files is written to the target binary is defined by the alphabetical order (ascending) of their names.  Suppose our program consisted of 3 files, main.x, 02_code.x, 01_data.x, the data/code in main.x would be written first, followed by the contents of 01_data.x, and finally, by the contents of 02_code.x.

You can ask the linker to generate a map file to figure out where all the symbols ended up.  This is done by specifying the **map** directive on one line of one .x file, e.g.,

```
map
```

This will generate a map file in the current directory.  The name of the map file is derived from the name of the .x file that contains the **Main** label and is helpfully output by the linker.  Here's an example map file.

```
Globals
-------
$8000 - SINSCRL.X:Main

SINSCRL.X
-------
$8000 - Main
$8003 - l1
$801E - l2
$8054 - stack
$8065 - data
$806A - roll
$806F - l3
$80B0 - l4
$80B6 - down
$80B9 - l5
```

By default, the entry point of a Specasm program is $8000 hex, or 32768 decimal.  This can be modified by using the **org** directive.  The **org** directive can only appear in one .x file in a directory.  That file does not need to be the file that contains the **Main** label but, by convention, the **org** statement follows or precedes the **Main** label.  The parameter to the **org** statement can be specified in decimal or hex, e.g.,

```
org 23760
```

will cause the linked program to be assembled at 23760.

The linker only creates pure binary files.  It isn't capable of creating a loader program or a tap file.  This is the task of the samake program, introduced in Specasm v7.  See below for more details.

Specasm projects are not limited to the .x files in the main directory.  The '-' and '+' directives can be used to include .x files in other directories, allowing a Specasm project to span multiple directories.  See below for more details.


### Multi-directory Projects and Libraries

Specasm supports two directives that allow the user to include .x files from other directories in the final executable.

| Directive    | Description                                                                                |
|--------------|--------------------------------------------------------------------------------------------|
| - <filename> | Here filename is either an absolute or a relative path (relative to the current .x file)   |
| + <filename> | Here filename is a path relative /specasm/.                                                |

These are both linker directives rather than assembler directives.  The target of these directives are not included directly into the current source file.  Instead they are added to the final binary when it is linked.  The '-' directive is intended to be used to create custom libraries or to split your program into multiple folders.  The '+' directive is intended to be used to include .x files from a future Specasm standard library.  The trailing '.x' extension in <filename> is optional.

If the path passed to a '-' or a '+' directive is itself a directory, Specasm (v8 and above) will add all the .x files it can find in that directory, and only that directory, to the final binary.

Here are some examples of '-' and '+'

```
; include /specasm/gr/rows.x
+gr/rows

; include ./lib/math.x
-lib/math.x

; include all .x files in mylib
; Specasm v8 and above.
-mylib
```

The '-' and '+' directives will not add .x files located in any sub-directory of an included directory.  For example,

```
-mylib
```

will add mylib/peqnp.x but it will not add mylib/tests/binpack.x.  If mylib/tests/binpack.x is needed in the project it will need to be included separately, either with a

```
-mylib/tests/binpack.x
```

or

```
-mylib/tests
```

if all the .x files in mylib/tests are to be part of the project.

### Binary files

Specasm supports one directive that allows a file to be inserted directly into the final binary at the position at which the directive appears.  This is useful when including binary data directly into your final executable rather than shipping it as a separate file and reading it in at runtime,  or encoding it using db or dw statements, which isn't really practical when there's a lot of data.

| Directive    | Description                                                                                |
|--------------|--------------------------------------------------------------------------------------------|
| ! <filename> | Filename is either an absolute or a relative path (relative to the current .x file)        |

For example,

```
  ; load pointer to sprite data into hl

  ld hl, sprites

  ; Load size of sprite file into bc

  ld bc, =sprite_end-sprites

  ; do something with the sprites

  ret
.sprites
! spritefile
.sprite_end
```

The data in spritefile will be inserted between the .sprites and .sprite_end label in the final binary by the linker.  The register bc will contain the size of that file.

## Creating Loaders with SAMAKE

Salink generates a raw binary file.  Before the binary can be executed, memory needs to be reserved, the binary needs to be loaded into memory at the address at which it was assembled and, finally, it needs to be inovked.  Assuming that the binary was assembled to the default address, i.e., 32768, and is called "bin", this can all be achieved in BASIC as follows.

```
CLEAR 32767: LOAD * "bin" CODE 32768: RANDOMIZE USR 32768
```

> [!TIP]
> Note LOAD * is simply LOAD on the ZX Spectrum Next.

Typing this is a bit cumbersome, and also error prone.  For this reason Specasm includes a tool called samake that can automatically create loaders.

Currently, samake is capable of creating two types of loaders, BASIC loaders with a +3DOS header than can be loaded directly from the SD card, and tap loaders which bundle a BASIC loader and the binary file together in a tap file.

Executing

```
CLEAR 32767
.samake
```

> [!TIP]
> Note the CLEAR statement is not needed on the ZX Spectrum Next as .samake is implemented as a dotn file.

will automatically create a BASIC loader.  The name of the loader will be derived from the .x file that contains the .Main label.  The address at which the loader loads the code is defined by the org statement in the x files, or 32768 if no org statement is provided.

Samake takes two optional arguments.  The first argument specifies the type of loader to be created, and can currently be set to either "bas" (the default) or "tap".  The second argument specifies the directory containing the Specasm project.  If the second argument is not specified, samake assumes the current directory.


## Versions, Binary and Source code Compatibility

There are two separate versions of Specasm, one for the 48kb and 128kb ZX Spectrums and one for the ZX Spectrum Next.  The two versions are incompatible with each other.  The 48kb version will not work on the Next and the Next version will not work on an original Spectrum.  The two versions are also binary incompatible with each other.  .x files created on the ZX Spectrum Next cannot be loaded by Specasm on the 48kb and vice-versa.  Both versions of Specasm are source code compatible however, provided that the sources do not contain any of the Z80N instructions.  The same .s file can be assembled by .saimport on both the Next and the 48kb Spectrum.

## ZX81 Support

Version v9 and above of Specasm add support for generating binaries for the ZX81.  This support comes in two forms; a new linker directive and updates to samake.

A new linker directive, **zx81**, has been added.  This directive can be placed anywhere in your program.  It is an instruction to the linker to transliterate any strings and characters it encounters from ASCII to the character encoding used by the ZX81.  This transliteration is performed on all string and character literals.  For example, consider the following code.

```
zx81
db 'A'
```

This will generate a binary of one byte containing the decimal value 38, which corresponds to an 'A' in the ZX81 character encoding..  If the zx81 directive is removed, and we relink, the binary will contain a single byte, 65, corresponding to an ASCII 'A'.  Lower case letters, not supported by the ZX81, are converted to upper case before transliteration.  Other ASCII codes without ZX81 equivalents are replaced by '?' characters.

The **zx81** directive will also change the default ORG address from 32768 to 16514.  This default can of course be overridden using the **org** directive.

The samake tool has been updated to generate .p files.  For example, to generate a .p file simply type

```
CLEAR 32767
.samake p
```

> [!TIP]
> Note the CLEAR statement is not needed on the ZX Spectrum Next as .samake is implemented as a dotn file.

The 'p' argument can be omitted if one of the .x files in the current directory contains a z81 directive.  Samake requires ZX81 binaries to have an org address of 16514.

On the Spectrum Next, the generate .p file can be run from the browser which will execute it in the built-in ZX81 emulator.

## Unit Tests

Specasm v9 adds basic support for unit testing.  Test content is placed in a .t file.  .t files are binary files like .x files that can be edited by Specasm but are intended to contain test code only.  .t files can be saexported and saimported to and from .ts source files.  .t files can be included in the same directory as the .x files that constitute the main program.  When salink is run it builds a main binary out of all the .x files in the project.  In Specasm v9, if the project contains any .t files, it will also generate a second binary that contains the contents of all the .t and all the .x files in the project.  The second binary has the same name as the main binary with a '.tst' extension appended.

Tests can be added to .t files simply by adding a function identified with global label that begins with the word 'Test'.  These Test functions are intended to exercise some part of the main programm, returning with bc = 0 if the test passed or any value but 0 if the test failed.   Samake has been updated to generate BASIC test harnesses that will load the test binary, execute all the Test functions in sequence and printing an indication as to whether each test passed or failed.  The test harness interprets the failure codes returned in the bc register pair in two ways.

1. If the value in bc is less than the ORG address of the program, 32768 by default on the Spectrum, the value is interpreted as an integer error code and output as such.
2. If the value in bc is greater than or equal to the ORG address, it is interpreted as a null terminated string containing an error message.  The test harness will output the string on a new line, before executing the next test.

As an example, consider a directory with two files, main.x and test.x.  main.x contains

```
; Carry flag set on failure
.Main
.Magic
  or a
  ret nz
  scf
  ret
```

and test.t contains

```
.TestMagicNonZero
  ld a, 1
  call Magic
  jr c, fail
  ld bc, 0
  ret

.TestMagicZero
  xor a
  call Magic
  jr c, fail
  ld bc, 0
  ret

.fail
  ld bc, 1
  ret
```

Two binaries will be created when salink is run in this directory, main which will contain only the Magic function, and main.tst which will contain the Magic function in addition to the two Test functions.

A test harness can then be created and executed by typing

```
samake tst
LOAD * "unit.bas"
```

> [!TIP]
> Note LOAD * is simply LOAD on the ZX Spectrum Next.

You should see something like

```
MagicNonZero: OK
MagicZero: FAIL (1)
```

printed on the screen.

The BASIC test harnesses locate the Test functions by means of a jump table added to the end of the test binary.  The harnesses should not need to be regenerated unless the ORG address or the name of the program changes.  Existing harnesses should be able to pick up any new Test functions added to the project by inspecting the jump table, which will have been updated by the linker.

### .t Files and the Main Label

Normally, when you try to link a binary with salink, the linker will complain if none of the .x files that comprise the project contain a Main label.  In Specasm v9 the linker will not generate an error if the project happens to contain a .t file with a Main label.  In this case the main binary will not be created, but the test binary will be.  This is useful when splitting your program up accross multiple directories.  An error will still be generated if none of the .t files in the project contain a Main label.

### Including .t Files

.t files are ignored when including directories with the - or + directives.  They can however be individually included by name.  For example,

```
- module
```

will add all the .x files in module to the project but will not add any .t files it may contain.

```
- module/test.t
```

on the other hand will add module/test.t to the project.

.t files can themselves include .x files from other directories.  As .t files are not processed during the creation of the main binary, those .x files will not form part of the main binary.  They will however be included in the test binary.

### Map Files and Unit Tests

Map directives can be included in .t files just as they can in .x files.  A map file will be generated for the test binary if the map directive is included in any of the .x or .t files that comprise the project.  The map file generated for the test binary will have a .tmt extension and the file will contain the addresses for all the symbols in the test binary including the addresses of the test functions themselves.  If the map directive is included in a .t file and there are no map directives in any of the .x files that make up the project, a map file will only be generated for the test binary.  In this case no map file will be generated for the main binary, assuming that the main binary be generated.

### Regcheck

Specasm v9 ships with some library code in the /specasm/lib/tst folder designed to support the creation of unit tests.  This code only provides one utility in Specasm v9, the ability to determine whether a function or piece of code only modifies the registers it claims to modify.  One of the trickest aspects of Z80 programming, in the opinion of Specasm's author, is debugging issues caused by functions not correctly delcaring the registers they modify.  A function may be preceeded by a comment saying this function modifies the hl register, whereas in fact it modifies, hl and b.  Code that takes the functions comments in good faith, is unlikely to work if it relies on the preservation of the b register.  These sorts of problems can be tricky to debug, particularly if there's a deep call chain.  Specasm v9 adds some support code designed to be integrated into the unit tests of functions.  The code enables the test writer to declare a list of registers that their function is allowed to modify and then to check, after the function has been called, that no other registers have been changed.  This is all a bit more clunky than it sounds.  The original plans for this feature included some sort of decorator that could be attached to global symbols, but there wasn't enough memory for that sort of finery, so it was abandoned in favour of a cumbersome series of function calls.  As an example, let us consider a simple function that increments a without changing the flags register.

```
.IncA
  push af
  pop bc
  ld a, c
  inc b
  ld c, a
  push bc
  pop af
  ret
```

Our unit test for this function might confirm that a is indeed incremented and that only the a register has changed.

```
+lib/regcheck
.Main
.TestIncA
  ld hl, regmask
  call TstParseRegs
  xor a
  call TstSaveRegs
  call IncA
  push af
  cp 1
  jr nz, fail
  pop af
  call TstCheckRegs
  ret
.fail
  pop af
  ld bc, 1
  ret

.regmask
"a"
db 0
```

Some things to note about the above code.  First we use the '+' directive to add /specasm/lib/regcheck.x to our test build.  Next we define a zero terminated string containing a list of the registers we expect to be modified during our test and we parse that string saving the results to some memory in regcheck.x using the TstParseRegs function.  Then, before we call our test function we call TstSaveRegs which stores a copy of the afdebchlixiya'f'b'c'd'e'h'l' registers in some more memory in regcheck.x.  Now we call the function we want to test after which we check to see whether it did what it was expected to do.  Note we make sure that our verification check here restores all registers.  Finally, we call TstCheckRegs, which compares the current state of the registers to those saved in regcheck.x by our earlier call to TstSaveRegs.  It then checks to see whether any of the altered registers are recorded in the register mask generated by TstParseRegs and stored in regcheck.x.  If no registers are altered or all altered registers are present in the register mask, TstCheckRegs returns with bc = 0.  Otherwise, bc points to a zero terminated string containing the names of the registers that were unexpectedly changed.  The BASIC test harness will detect the error, by virtue of bc being non zero, and will print out the string in red.  When we run our test harness we'd then expect to see

```
IncA: FAIL
regs: cb
```

As IncA modifies b and c which are not listed in the regmask string.  Updating regmask to contain "abc" should make the error go away.  Note that if IncA did not preserve the flags register we'd also need to include 'f' in regmask.

## Project Structure Revisited

Large projects should be divided across multiple directories.  Each well defined piece of functionality should be separated out into a module and placed in its own folder.  This folder should contain the .x files that implement the functionality and the .t files that provide the unit tests.  The Main label should be present in one of the .t module files and not in the one of the .x files.  This guarantees that when salink is run in a module's directory, the linker will only generate a test binary.  There's no point in generating a module binary that wil probably not be used.  The top level folder will the contain the main routine in a .x file and a Main label.  It will use the - directive to include all modules.  When salink is run in the top level folder, only one binary, the program executable will be created.  No test binaries will be created as .t files are not added to a project by directory inclusion.  An example directory hierarchy for an interpreter might look like this.

```
-- Inter
    +-- inter.x  (contains Main label)
    +-- inter    (program binary)
    +-- lexer
          +-- lexer.x
          +-- test.t (contains Main label)
	  +-- test.tst (test binary)
	  +-- unit.bas (test harness)
    +-- parser
          +-- parser.x
	  +-- tree.x
          +-- test.t (contains Main label)
	  +-- test.tst (test binary)
	  +-- unit.bas (test harness)
    +-- codegen
          +-- codgen.x
          +-- test.t (contains Main label)
	  +-- test.tst (test binary)
	  +-- unit.bas (test harness)
    +-- vm
          +-- vm.x
          +-- test.t (contains Main label)
	  +-- test.tst (test binary)
	  +-- unit.bas (test harness)
```

and the start of the inter.x file would look something like this

```
-lexer
-parser
-codegen
-vm
.Main
...
```

Now suppose we wanted to update our interpreter so that it ran on both the ZX Spectrum and the ZX81.  We might end up with something like this.

```
-- Inter
    +-- zx81
          +-- inter81.x  (contains Main label and zx81 statement)
	  +-- porting.x  (zx81 specific code))
          +-- inter81    (program binary)
    +-- spectrum
          +-- inter82.x  (contains Main label)
	  +-- porting.x  (spectrum specific code)
          +-- inter82    (program binary)
    +-- lexer
    +-- parser
    +-- codegen
    +-- vm
```

The inter81.x file might look something like this

```
-../lexer
-../parser
-../codegen
-../vm
zx81
.Main
...
```

