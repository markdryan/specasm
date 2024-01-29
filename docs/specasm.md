# Specasm

Specasm is a Z80 assembler designed to run on the 48k and 128k ZX Spectrums with an ESXDOS SD card solution, and the ZX Spectrum Next.  It includes an editor, an assembler and a linker.

## Editor/Assembler

The editor can be launched from the command line using the .specasm command.  Specasm is an integrated editor/assembler.

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

#### Selecting Mode

The *sel* command switches the editor into selection mode.  In selection mode the user can press the up and down keys to select a block of code.  They can also press the 'a' key to select the entire file.  Only whole lines can be selected.  To cancel the selection and return to editor mode, press SPACE.  To delete the selected lines and return to editor mode, press DELETE.  To confirm the selection and return to editor mode, press ENTER.  Once the selection has been confirmed an '*' will appear in the status row at the bottom of the screen to the right of 'INS' or 'OVR'.  This indicates that some lines are currently selected.  These lines can be manipulated using the 'd', 'm', 'c' and 'b' commands described above.  For example, to count the number of machine code bytes in the selected line, type SYMSHIFT+w followed by 'b' and ENTER.  The editor is capable of computing the exact size in bytes of the machine code associated with the instructions and data in the selected region as it has already assembled these instructions and knows exactly how much space they will consume.

All of the four commands that manipulate selections, cancel the selection once they have finished executing.  So if you select a block of text, and then issue the 'b' command to count the number of bytes it consumes, you'll need to reselect the text once more to copy it.  In addition, the current selection is cancelled if you make any changes to the contents of the editor, e.g., edit an existing line or insert a new one.

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
> Label subtraction outside of expressions is no longer supported as of Specasm v7.  This syntax was rendered redundant by the introduction of expressions in Specasm v5 and deprecated in that release.  The linker does still support label substraction so any .x files assembled with an earlier version of Specasm can sill be linked and binary compatibility with older versions of Specasm is maintained.  Any instructions in .x files that use label subtraction will be rewritten to use the new expression syntax when loaded into Specasm's editor.  The remainder of this section exists purely for historical value.

Direct label subtraction is supported in certain instructions and directives without the use of an expression.

Label subtraction permits the size of a given block of code or data delimited by two labels to be encoded into the binary at link time.  Label subtraction can be used in place of an immediate value in four specific mnemonics.


| Mnemonic                 | Description                                            |
|--------------------------|--------------------------------------------------------|
| ld [a-l], end-start      | 8 bit immediate loads into a, b, c, d, e, h and l      |
| ld [bc/de/hl], end-start | 16 bit immediate loads to bc, de and hl                |
| db end-start             | 8 bit data directive                                   |
| dw end-start             | 16 bit data directive                                  |

An error will be generated at link time if the 8 bit versions of the mnemonics specify two labels
that are more than 255 bytes apart.

It's possible to specify both local and global labels when computing the difference between two labels in this manner, but it only really makes sense to subtract labels that occur in the same file.

Here's an example of label subtraction

```
ld bc, end-start
ret
.start
db 10, 10
"hello"
ds 12, ' '
.end
```

The ld instruction would be encoded as follows in the final binary.

```
ld bc, 19
```


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

A Specasm program is comprised of one or more .x files that occupy the same directory.  When you build a Specasm program with the .salink command it looks for all the .x files in the folder in which it is run, and links them all together, concatenating them all into one single file and resolving any addresses, e.g., jump targets.  One of the .x files in the current folder must contain a label called **Main**, .e.g, it must have the following statement somewhere within one of the files

```
.Main
```

The name of the resulting binary will be derived from the name of the .x file that contains the **Main** label.  So if a project places the Main label in a file called **game.x**, the name of the resulting binary created by salink will be **game**.

The salink command will place the code from the .x file that declares the Main label first in the newly created binary.  The order in which the rest of the code is written to the binary is arbitrary and the user has no control over this.  They can however, ask the linker to generate a map file to figure out where all the symbols ended up.  This is done by specifying the **map** directive on one line of one .x file, e.g.,

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

The linker doesn't currently create a loader program or a tap file.  This can however be done using the samake command.

### Libraries

Specasm supports two directives that allow the user to include .x files from other directories in the final executable.

| Directive    | Description                                                                                |
|--------------|--------------------------------------------------------------------------------------------|
| - <filename> | Here filename is either an absolute or a relative path (relative to the current .x file)   |
| + <filename> | Here filename is a path relative /specasm/.                                                |

These are both linker directives rather than assembler directives.  The target of these directives are not included directly into the current source file.  Instead they are added to the final binary when it is linked.  The '-' directive is intended to be used to create custom libraries or to split your program into multiple folders.  The '+' directive is intended to be used to include .x files from a future Specasm standard library.  The trailing '.x' extension in <filename> is optional.

Here are some examples of their use

```
; include /specasm/gr/rows.x
+gr/rows

; include ./lib/math.x
-lib/math.x
```

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
