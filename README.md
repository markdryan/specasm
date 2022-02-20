# Specasm

Specasm is a Z80 assembler designed to run on the 48k and 128k ZX Spectrum.  It requires an SD card solution running ESXDOS 0.87 or greater to function.  For detailed information about how Specasm works, please see the [documentation](https://github.com/markdryan/specasm/blob/master/docs/specasm.md).  To get started, carry on reading.

## Getting Started

Download the latest release of Specasm, and unzip the contents of the file into the root directory of your SD card.  You should now have a folder in your root directory called SPECASM that contains some .tap and some BASIC files.  It should look something like this.

![Installing](/docs/install.png)

Now navigate to the INSTALL file, which is a BASIC program, and press **ENTER** to execute it.  This will use ESXDOS's **.launcher** command to set up some command line short cuts for the tap files in the SPECASM directory.

You may noticed from the screenshot that there are some other files in the SPECASM folder, namely two dotx commands called saimport and saexport.  These commands allow you to convert between Specasm's internal file format, .x files, and assembly language text files, .s files.  I couldn't work out how to install these dotx commands cleanly in BASIC so this needs to be done manually.

```
.mkdir /bin/extra
.cp SAIMPORT /bin
.cp SAIMPORT.X /bin/extra/SAIMPORT.X
.cp SAEXPORT /bin
.cp SAEXPORT.X /bin/extra/SAEXPORT.X
```

Note this step is optional and only necessary if you need to convert between .x and .s files on the spectrum itself.

## Assembling your First Program

Now, create a new directory somewhere on your SD card and cd into it and type .specasm, to launch Specasm's IDE and integrated assembler, e.g.,


```
.mkdir /asm/demo
.cd /asm/demo
.specasm
```

Enter the following program

![Hello Specasm](/docs/specasm.png)

Note when entering the program there's no need to worry about formatting.  Specasm assembles and formats each line immediately after you've typed it, i.e., pressed **ENTER** or navigated off the line.  When you've finished typing the program, hit **SYMSHIFT w** to take you to command mode.  Now when the command prompt appears, type

```
> s hello
```

to save your work.  We've now entered, assembled and saved our first program.  Before we can run it however, we need to link it (basically resolve all the jumps and label references).  There's not enough memory on the 48k spectrum to do the linking inside the IDE so we need to exit it and run another program.  The can be achieved by entering command mode once more by pressing **SYMSHIFT w**, following by q and **ENTER**, e.g.,

```
> q
```

You should be returned to the BASIC prompt.  To link the program type

```
.salink
```

You should see something like this appear on your screen

![Hello Specasm](/docs/salink.png)

Once the linker has finished a binary file will be created.  The name of the file will be reported by the linker.  In the example above its 'hello'.  To execute the program, enter the BASIC commands at the bottom of the image above and press **ENTER**.  You should see.

![Hello Specasm](/docs/hello.png)

## Building Specasm

Specasm is built with [z88dk](https://github.com/z88dk/z88dk) and GNU Make.  Install a recent version and then

```
cd build
make -j -f Makefile.spec
```

And then wait.   All the tap and dotx files will be created in the build directory.

The saexport, saimport and salink commands can be built and run on POSIX compatible systems.  Simply type make from the main Specasm directory.  The saimport is essentially the assembler without the editor, so can be used in conjunction with salink to assemble and build Spectrum programs directly on a modern machine, but where's the fun in that?





