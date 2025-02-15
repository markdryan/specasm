# Specasm

Specasm is a Z80 assembler designed to run on the 48k and 128k ZX Spectrums and the ZX Spectrum Next.  It requires an SD card solution running ESXDOS 0.87 or greater to function on the 48Kb and 128Kb ZX Spectrum.  For detailed information about how Specasm works, please see the [documentation](https://github.com/markdryan/specasm/blob/master/docs/specasm.md).  To get started, carry on reading.

## Getting Started

[Download](https://github.com/markdryan/specasm/releases) the latest release of Specasm appropriate for your Spectrum and unzip the contents of the file into the root directory of your SD card.

> [!TIP]
> There are two different Zip files available, specasm.zip for the 48kb and 128kb Spectrums, and specasmnext.zip for the ZX Spectrum Next.  The specasm.zip file contains separate binaries and install scripts for both the 48kb and 128kb versions.  You must download the appropriate version for your machine.

You should now have a folder in your root directory called SPECASM.  It should look something like this if you downloaded specasm.zip

![Installing](/docs/install.png)

To install Specasm you need to run a short BASIC program.  Run INST48.BAS when installing on a 48kb Spectrum, INST128.BAS when installing on a 128kb Spectrum and INSTALL.BAS when running on the Next.  Navigate to the appropriate install file and press **ENTER** to execute it.  On the 48kb or 128kb Spectrum this will use ESXDOS's **.launcher** command to set up some command line short cuts for the tap files in the SPECASM directory.  It will also copy some executables to the /bin folder.  On the ZX Spectrum Next it will copy the various executable programs that compose Specasm to the /dot folder.

> [!TIP]
> Note on previous releases of Specasm, there were separate zip files just for the 48kb and the 128kb Spectrums and users ran a BASIC script called INSTALL.BAS to install Specasm on these machines.  From release v11 onwards, the zip files for the 48kb and 128kb Spectrums have been combined and you must run INST48.BAS or INST128.BAS to install the correct version.  If you run the wrong install program by mistake just run REMOVE.BAS to uninstall Specasm and then execute the correct installer.

## Reinstalling Specasm

To upgrade to a new version of Specasm perform the following steps.

1. Execute the REMOVE.BAS BASIC program.  This can be done from the ESXDOS file browser or by loading it from the BASIC prompt.  This will remove the old version of Specasm stored under the /bin or the /dot folders, and will also de-register the old **.launcher** shortcuts.
2. Manually remove the old /Specasm directory on your SD card, e.g., rm -rf /Specasm
3. Download the latest release and unzip its contents to /Specasm on the SD card
4. Run the INSTALL.BAS BASIC program.  This will install the new version of Specasm.

## Assembling your First Program

Now, create a new directory somewhere on your SD card and cd into it and type .specasm, to launch Specasm's IDE and integrated assembler, e.g.,


```
.mkdir /asm
.cd /asm
.mkdir demo
.cd demo
.specasm
```

Enter the following program

![Hello Specasm](/docs/specasm.png)

Note when entering the program there's no need to worry about formatting.  Specasm assembles and formats each line immediately after you've typed it, i.e., pressed **ENTER** or navigated off the line.  When you've finished typing the program, hit **SYMSHIFT w** to take you to command mode.  Now when the command prompt appears, type

```
> s hello
```

to save your work.  We've now entered, assembled and saved our first program.  Before we can run it however, we need to link it (basically resolve all the jumps and label references).  There's not enough memory on the 48k spectrum to do the linking inside the IDE so we need to exit it and run another program.  This can be achieved by entering command mode once more by pressing **SYMSHIFT w**, following by q and **ENTER**, e.g.,

```
> q
```

You should be returned to the BASIC prompt.  To link the program type

```
.salink
```

You should see something like this appear on your screen

![Hello Specasm](/docs/salink.png)

Once the linker has finished a binary file will be created.  The name of the file will be reported by the linker.  In the example above it's 'hello'.  We need to create a BASIC loader before we can execute the program.  To do this type

```
CLEAR 32767
.samake
```

> [!TIP]
> Note the CLEAR statement is not needed on the ZX Spectrum Next.

This should create a file called hello.BAS.  To run your program on a 48Kb or 128Kb Spectrum type

```
LOAD * "hello.bas"
```

On the ZX Spectrum Next simply type

```
LOAD  "hello.bas"
```

You should see.

![Hello Specasm](/docs/hello.png)

## Building Specasm

### For the 48kb Spectrum

Specasm is built with [z88dk](https://github.com/z88dk/z88dk) and GNU Make.  To build Specasm for the 48k Spectrum clone the repoistory and type

```
cd build/48/specasm
make -j
```

and then wait.   All the tap and dotx files will be created in the build directory.

To create a zip file with all the files that need to be copied onto the spectrum, type

```
make release
```

from the same directory.  The specasm48.zip file can be found in the build/release folder.

### For the 128kb Spectrum

```
cd build/128/specasm
make -j
make release
```

This will create a zip file called specasm128.zip.

### For the Spectrum Next

```
cd build/next/specasm
make -j
make release
```

This will create a zip file called specasmnext.zip.

### On Linux or macOS

The samake, saexport, saimport and salink commands can be built and run on POSIX compatible systems.  Simply type make from the main Specasm directory.  The saimport is essentially the assembler without the editor, so can be used in conjunction with salink to assemble and build Spectrum programs directly on a modern machine, but where's the fun in that?

## Tests

To run the unit tests simply type

```
make
./unittests
```

from the project's top level folder.

To run the tests extra instructions supported by the Spectrum Next type

```
CFLAGS="-DSPECASM_TARGET_NEXT_OPCODES" make -j
./unittests
```

To run the linker tests perform the following steps

```
make
cd tests
./tests.sh
```

To run the linker tests with the Next Opcodes enabled, type

```
cd tests && SPECASM_TARGET_NEXT_OPCODES=1 ./tests.sh
```

A large proportion (but not all) of the unit tests can be run on the Spectrums themselves.  To build these tests for the 48kb Spectrum type

```
build/48/unit
make
make tests
```

This will create a folder called tests in the unit folder.  Inside this folder are 3 files that need to be copied to the same directory on your spectrum.  Run the unitzx.tap file to run the tests.

To build for the 128kb Spectrum, type

```
build/128/unit
make
make tests
```

To build for the ZX Spectrum Next, type

```
build/next/unit
make -j
make tests
```

This will create a folder called tests in the unit folder.  Inside this folder are 3 files that need to be copied to the same directory on your Next.  Enter the folder and then type ../unitnext from the command line to launch the tests.