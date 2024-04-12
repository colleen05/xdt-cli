# XDT CLI - A command-line tool for creating and modifying XDT files.
**XDT CLI** provides functionality for manipulating XDT files (see: [libxdt on GitHub](https://github.com/colleen05/libxdt)) with easy-to-use syntax.

## :warning: :rotating_light: DISCONTINUED C++ PROJECT :rotating_light: :warning:
I'm no longer using C++ for personal projects, thus this project's development has been discontinued.
Check my GitHub or [**my website**](https://colleen05.me/) for projects coming in the future.

## Building & Using
### Linux
**IMPORTANT:** You **MUST** have a `CXX` environment variable defined, and configured to be the path to your C++ compiler of choice.

Execute the `build_linux.sh` file, which will produce a `xdt` executable in the `bin/linux/` directory. If all is successful, you should see this:
```
$ ./build_linux.sh
Building libxdt [linux]...
Creating directories...
Compiling object files...
Building static library...
Removing temporary directories...
Building xdt-cli [linux]...
Creating directories...
Compiling...
$ 
```

### Windows
I haven't made an official Windows build script, but the provided `build_linux.sh` and `build_libxdt_linux.sh` files should be easy to convert into a Windows build script with minimal effort, provided you have some experience with both .sh and .bat, or experience with compilers.

## Syntax and Using XDT CLI
Executing `xdt --help` will yield help on syntax and general use of XDT CLI:
```
$ xdt --help
Usage: xdt <file> <action> [args...] [options...]

Actions:
    list                            List all items in file.
    get     <names...>              Get value(s) in file.
    set     <setters...>            Set value(s) in file, by setters.
    mutate  <mutators...>           Cast type of value to new type, by mutators.
    remove  <names...>              Remove value(s) in file.
    dump    <directory> [items...]  Dump items to directory. (All items by default.)

Setter:     <type>:<name>=<value>
Mutator:    <name>:<type>

Options:
    --version or -v     Display version information.
    --help    or -h     Display this help information.
    --backup  or -b     Create backup file before modification.

Types:
  - byte            Byte.
  - bool            Boolean.
  - int16           16-bit integer (signed).
  - uint16          16-bit integer (unsigned).
  - int32           32-bit integer (signed).
  - uint32          32-bit integer (unsigned).
  - int64           64-bit integer (signed).
  - uint64          64-bit integer (unsigned).
  - float           Float.
  - double          Double.
  - time            Timestamp.
  - longtime        Long timestamp (64-bit).
  - string          ASCII string.
  - utf8string      UTF-8 string.
  - file            File data.
  - bin             Binary data.

XDT CLI v1.0.0 - December, 2022
by Colleen (@colleen05), and GitHub contributors.

This software is distributed under the zlib license.
For more information, see the 'LICENSE' file provided, or visit:
https://choosealicense.com/licenses/zlib/
```

### Example
Creating a file, `myTable.xdt`, with info about a person:
```
$ xdt myTable.xdt create string:name=John int32:year=1989 float:height=1.83
(ASCIIString) "name": "John"
(Int32) "year": 1989
(Float) "height": 1.83
```
Listing the contents:
```
$ xdt myTable.xdt list
File "myTable.xdt" (3 items):
(ASCIIString) "name": "John"
(Int32) "year": 1989
(Float) "height": 1.83
```

Changing the `year` item from a signed 32-bit integer, to an unsigned 16-bit integer:
```
$ xdt myTable.xdt mutate year:uint16
(Uint16) "year": 1989
```

Updating the `height` item from 1.83 to 1.82:
```
$ xdt myTable.xdt set float:height=1.82
(Float) "height": 1.82
```

Listing final file:
```
$ xdt myTable.xdt list
File "myTable.xdt" (3 items):
(ASCIIString) "name": "John"
(Uint16) "year": 1989
(Float) "height": 1.82
```
