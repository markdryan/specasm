# Specasm

Specasm is a Z80 assembler designed to run on the 48k and 128k ZX Spectrums with an ESXDOS SD card solution.  It includes an editor, an assembler and a linker.

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
| s *filename* | Saves the contents of the editor to *filename*.  The .x extension is optional |
| g *line number* | Moves the cursor to the specified line number|
| f *string* | Searches for the first instance of *string* starting from the current line.  There's no wrap around. |

#### Selecting Mode

The *sel* command switches the editor into selection mode.  In selection mode the user can press the up and down keys to select a block of code.  They can also press the 'a' key to select the entire file.  Only whole lines can be selected.  To cancel the selection and return to editor mode, press SPACE.  To delete the selected lines and return to editor mode, press DELETE.  To confirm the selection and return to editor mode, press ENTER.  Once the selection has been confirmed an '*' will appear in the status row at the bottom of the screen to the right of 'INS' or 'OVR'.  This indicates that some lines are currently selected.  These lines can be manipulated using the 'd', 'm', 'c' and 'b' commands described above.  For example, to count the number of machine code bytes in the selected line, type SYMSHIFT+w followed by 'b' and ENTER.  The editor is capable of computing the exact size in bytes of the machine code associated with the instructions and data in the selected region as it has already assembled these instructions and knows exactly how much space they will consume.

All of the four commands that manipulate selections, cancel the selection once they have finished executing.  So if you select a block of text, and then issue the 'b' command to count the number of bytes it consumes, you'll need to reselect the text once more to copy it.  In addition, the current selection is cancelled if you make any changes to the contents of the editor, e.g., edit an existing line or insert a new one.

#### The Status Row

The bottom row of the editors display is the status row.  It has three fields.  The first field, indicates whether insert mode or overwrite mode is active.  It also, indicates via a '*' whether any lines are currently selected.  The middle field displays the number of lines used in the buffer and the maximum number of lines available.  The final field, displays the current column number and the current line number.  Note that the first line in the file is line 0, not line 1.

### Assembly Syntax

#### Mnemonics

The assembler support all the documented z80 instructions.  All mnemonics are entered in lower case.  Commas, ',', are used to separate multiple arguments.  The condition codes for jump and call instructions are separated from the main mnemonic by a space and from the target address by a comma.  For example,

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

Specasm does not support expressions in the general sense.  There is one exception however.  In certain circumstances it supports label subtraction.  This permits the size of a given block of code or data delimited by two labels to be encoded into the binary at link time.  Label subtraction can be used in place of an immediate value in four specific mnemonics.


| Mnemonic                 | Description                                            |
|--------------------------|--------------------------------------------------------|
| ld [a-l], end-start      | 8 bit immediate loads into a, b, c, d, e, h and l      |
| ld [bc/de/hl], end-start | 16 bit immediate loads to bc, de and hl                |
| equb end-start           | 8 bit data directive                                   |
| equw end-start           | 16 bit data directive                                  |

An error will be generated at link time if the 8 bit versions of the mnemonics specify two labels
that are more than 255 bytes apart.

It's possible to specify both local and global labels when computing the difference between two labels in this manner, but it only really makes sense to subtract labels that occur in the same file.

Here's an example of label subtraction

```
ld bc, end-start
ret
.start
equb 10, 10
"hello"
repb ' ', 12
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

The assembler provides two directives that can be used to store numeric constant data directly in machine code programs.  These are

| Directive | Description                                                               |
|-----------|---------------------------------------------------------------------------|
| equb      | Used to encode up to 1 to 4 bytes.  Multiple bytes are comma separated.   |
| equw      | Used to encode 1 or 2 16 bit words.  Multiple words are comma separated. Can also be used to encode the address of a label resolved at link time|
| repb      | Encodes 1 or more copies of a given byte |


Numbers can be specified as hex digits, signed and unsigned numbers or as characters.  There's one restriction here though.  If you specify multiple numbers on the same line the numbers must be of the same format, e.g., all hex digits or all characters.  Unsigned and singed numbers can be mixed as long as all the numbers can be represented by a signed number.  Here are some examples that are allowed

```
equb 10
equw $1000
equb 10, -10, 11, -11
equb 'a', 'b', 'c'
equw 'c'
equb 255, 255
```

And here are some examples that aren't

```
; Both numbers cannot be represented by a signed byte
equb -1, 255
; Mixing hex and signed decimals
equb -1, -2, -3, $20
; Mixing characters and decimals
equb 'A', 1, 2
```
In addition to encoding numbers the **equw** directive can be used to encode the address of a label and also the difference between the addresses of two labels.  When used in this format, the **equw** directive can only contain a single argument.

For example,

```
equw data
```

will encode the address of the label data directly into the program.  The address is encoded at link time when salink has figured out the final address of the label.

```
equw end-start
.start
equw 10, 10
equb 1
.end
```

will store the value 5 in a 16 bit word in the final program.

The **equb** directive cannot be used to encode the address of a label as the address is unlikely to fit into a single byte.  It can however, be used to encode the difference between two labels providing the difference does fit into a byte.  If the labels are too far apart an error will be generated at link time.  When used in this form, no other numbers can follow the label subtraction on the same line.  For example,

```
equb end-start
.start
equw 10, 10
equb 1
.end
```

Will store the byte 5 in memory followed by 5 bytes of data.

The **repb** directive can be used to store multiple copies of them same byte value directly into the program code.  **repb** requires two parameters separated by a comma.  The first is the byte value to store.  This can be specified as a signed or unsigned decimal number, a hexidecimal number of a character.  The second value is 16 bit integer that specifies the number of copies of the first parameter to be encoded.  For example,

```
repb 'A', 12
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

A future 128kb version of Specasm will incorporate the import and export commands directly in the editor and will also contain a 'gc' command to allow the garbage collection to be performed in one simple step.

### Source files

Specasm edits .x files, which are not text files. They're annotated object files, in which all the instructions are pre-assembled. This approach has some advantages and some disadvantages. The advantages are fast load and save times as there's no parsing required, and fast build times as the build, performed by the .salink command, consists of only one stage, the linker. The disadvantages are that it's more cumbersome to submit source files to source control as the source files are not text files. Separate tools need to be run, saimport and saexport that convert between .x files to .s files, before source code and be moved to and from source control. The other disadvantage is that there's a limited amount of memory per line (4 bytes). This means that it's not easy to support a complex expression syntax when specifying the arguments of instructions. In practice, this isn't much of an issue though as there isn't really enough memory to support such expressions anyway.

To convert a .s file, a text file containing specasm code, into a .x file you need to use the .saimport dotx command.  It accepts one or more command line parameters, which must be the paths of .s files.  It will convert each of these .s files to .x files.  For example, to convert the file hello.s into hello.x copy hello.s to the spectrum and type

```
CLEAR 32767
.saimport *.s
```

The reverse process can be performed using the .saexport dotx command.  Note the use of the CLEAR statement.  This needs to be executed before the .saimport or .saexport commands as these are dotx commands and part of their code is loaded and executed from BASIC's memory.  To ensure there's no interference between BASIC and the dotx commands, the CLEAR command must be issued before their use.  If you're executing a sequence of .saimport/.saexport commands, You only need to issue the CLEAR command once.

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

The linker doesn't currently create a loader program or a tap file so this needs to be done manually.  It can be scripted using BASIC and the relevant ESXDOS commands.

