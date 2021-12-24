#!/bin/bash
T=0                             # global test number

((T++))
tnames[T]="get globals string1"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
test-input/globals
./patchsym test-input/globals string1 string
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> test-input/globals
string1: Hello
string2: Goodbye cruel world
string3: All your bass
int1: aabbccdd
int2: ffeeddcc
a_doub: 1.234567
> ./patchsym test-input/globals string1 string
GET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
Found Symbol 'string1'
- 53 symbol index
- 0x4040 value
- 8 size
- 23 section index
- 32 offset in .data of value for symbol
string value: 'Hello'
ENDOUT

((T++))
tnames[T]="get globals string2"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
test-input/globals
./patchsym test-input/globals string2 string
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> test-input/globals
string1: Hello
string2: Goodbye cruel world
string3: All your bass
int1: aabbccdd
int2: ffeeddcc
a_doub: 1.234567
> ./patchsym test-input/globals string2 string
GET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
Found Symbol 'string2'
- 66 symbol index
- 0x4060 value
- 64 size
- 23 section index
- 64 offset in .data of value for symbol
string value: 'Goodbye cruel world'
ENDOUT


((T++))
tnames[T]="get globals nonexist not found"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
./patchsym test-input/globals nonexist string
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> ./patchsym test-input/globals nonexist string
GET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
ERROR: Symbol 'nonexist' not found
ENDOUT


((T++))
tnames[T]="set globals2 string1"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
cp test-input/globals test-data/globals2
test-data/globals2
./patchsym test-data/globals2 string1 string "ADIOS!"
test-data/globals2
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> cp test-input/globals test-data/globals2
> test-data/globals2
string1: Hello
string2: Goodbye cruel world
string3: All your bass
int1: aabbccdd
int2: ffeeddcc
a_doub: 1.234567
> ./patchsym test-data/globals2 string1 string 'ADIOS!'
SET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
Found Symbol 'string1'
- 53 symbol index
- 0x4040 value
- 8 size
- 23 section index
- 32 offset in .data of value for symbol
string value: 'Hello'
New val is: 'ADIOS!'
> test-data/globals2
string1: ADIOS!
string2: Goodbye cruel world
string3: All your bass
int1: aabbccdd
int2: ffeeddcc
a_doub: 1.234567
ENDOUT

((T++))
tnames[T]="set globals2 string1-3"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
cp test-input/globals test-data/globals2
test-data/globals2
./patchsym test-data/globals2 string1 string "Adios!"
./patchsym test-data/globals2 string2 string "Say hello to a new day."
./patchsym test-data/globals2 string3 string "Move ZIG!"
test-data/globals2
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> cp test-input/globals test-data/globals2
> test-data/globals2
string1: Hello
string2: Goodbye cruel world
string3: All your bass
int1: aabbccdd
int2: ffeeddcc
a_doub: 1.234567
> ./patchsym test-data/globals2 string1 string 'Adios!'
SET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
Found Symbol 'string1'
- 53 symbol index
- 0x4040 value
- 8 size
- 23 section index
- 32 offset in .data of value for symbol
string value: 'Hello'
New val is: 'Adios!'
> ./patchsym test-data/globals2 string2 string 'Say hello to a new day.'
SET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
Found Symbol 'string2'
- 66 symbol index
- 0x4060 value
- 64 size
- 23 section index
- 64 offset in .data of value for symbol
string value: 'Goodbye cruel world'
New val is: 'Say hello to a new day.'
> ./patchsym test-data/globals2 string3 string 'Move ZIG!'
SET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
Found Symbol 'string3'
- 52 symbol index
- 0x40a0 value
- 16 size
- 23 section index
- 128 offset in .data of value for symbol
string value: 'All your bass'
New val is: 'Move ZIG!'
> test-data/globals2
string1: Adios!
string2: Say hello to a new day.
string3: Move ZIG!
int1: aabbccdd
int2: ffeeddcc
a_doub: 1.234567
ENDOUT

((T++))
tnames[T]="set list_main2 PROMPT"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
cp test-input/list_main test-data/list_main2
echo 'exit' | test-data/list_main2 -echo
./patchsym test-data/list_main2 PROMPT string "[LIST]# "
./patchsym test-data/list_main2 PROMPT string
echo "exit" | test-data/list_main2 -echo
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> cp test-input/list_main test-data/list_main2
> echo exit
> test-data/list_main2 -echo
Linked List Demo
Commands:
  print:          shows the current contents of the list
  clear:          eliminates all elements from the list
  exit:           exit the program
  insert thing:   inserts the given string into the list
  get index:      get the item at the given index
  contains thing: determine if the given thing is in the list
                  (NOT YET IMPLEMENTED)
list> exit
> ./patchsym test-data/list_main2 PROMPT string '[LIST]# '
SET mode
.data section
- 23 section index
- 12384 bytes offset from start of file
- 0x4060 preferred virtual address for .data
.symtab section
- 26 section index
- 12472 bytes offset from start of file
- 1992 bytes total size
- 24 bytes per entry
- 83 entries
Found Symbol 'PROMPT'
- 76 symbol index
- 0x4080 value
- 32 size
- 23 section index
- 32 offset in .data of value for symbol
string value: 'list> '
New val is: '[LIST]# '
> ./patchsym test-data/list_main2 PROMPT string
GET mode
.data section
- 23 section index
- 12384 bytes offset from start of file
- 0x4060 preferred virtual address for .data
.symtab section
- 26 section index
- 12472 bytes offset from start of file
- 1992 bytes total size
- 24 bytes per entry
- 83 entries
Found Symbol 'PROMPT'
- 76 symbol index
- 0x4080 value
- 32 size
- 23 section index
- 32 offset in .data of value for symbol
string value: '[LIST]# '
> echo exit
> test-data/list_main2 -echo
Linked List Demo
Commands:
  print:          shows the current contents of the list
  clear:          eliminates all elements from the list
  exit:           exit the program
  insert thing:   inserts the given string into the list
  get index:      get the item at the given index
  contains thing: determine if the given thing is in the list
                  (NOT YET IMPLEMENTED)
[LIST]# exit
ENDOUT

((T++))
tnames[T]="set quote_main2 correct"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
cp test-input/quote_main test-data/quote_main2
test-data/quote_main2 2
./patchsym test-data/quote_main2 correct string "Java prevents you from shooting yourself in the foot by cutting off all your fingers."
test-data/quote_main2 2
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> cp test-input/quote_main test-data/quote_main2
> test-data/quote_main2 2
Complete this sentence by C++ creator Bjarne Stroustrup:
C makes it easy to shoot yourself in the foot; ...

enter a number from 0 to 15 on command line
2: Java prevents you from shooting yourself in the foot by cutting off all your fingers.

Have a nice tall glass of ... NOPE.
> ./patchsym test-data/quote_main2 correct string 'Java prevents you from shooting yourself in the foot by cutting off all your fingers.'
SET mode
.data section
- 23 section index
- 12352 bytes offset from start of file
- 0x4040 preferred virtual address for .data
.symtab section
- 26 section index
- 16528 bytes offset from start of file
- 1824 bytes total size
- 24 bytes per entry
- 76 entries
Found Symbol 'correct'
- 75 symbol index
- 0x4060 value
- 128 size
- 23 section index
- 32 offset in .data of value for symbol
string value: 'C++ makes it harder, but when you do, it blows away your whole leg.'
New val is: 'Java prevents you from shooting yourself in the foot by cutting off all your fingers.'
> test-data/quote_main2 2
Complete this sentence by C++ creator Bjarne Stroustrup:
C makes it easy to shoot yourself in the foot; ...

enter a number from 0 to 15 on command line
2: Java prevents you from shooting yourself in the foot by cutting off all your fingers.

Correct!
ENDOUT


((T++))
tnames[T]="set greet_funcs.o greeting"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
cp test-input/greet_funcs.o test-data/greet_funcs2.o
gcc -o test-data/greet_main2 test-input/greet_main.o test-data/greet_funcs2.o
test-data/greet_main2
./patchsym test-data/greet_funcs2.o greeting string "Whazzzzup!?"
gcc -o test-data/greet_main2 test-input/greet_main.o test-data/greet_funcs2.o
test-data/greet_main2
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> cp test-input/greet_funcs.o test-data/greet_funcs2.o
> gcc -o test-data/greet_main2 test-input/greet_main.o test-data/greet_funcs2.o
> test-data/greet_main2
Hello fine folks!
Hello fine folks!
Hello fine folks!
Hello fine folks!
Hello fine folks!
> ./patchsym test-data/greet_funcs2.o greeting string 'Whazzzzup!?'
SET mode
.data section
- 3 section index
- 96 bytes offset from start of file
- 0x0 preferred virtual address for .data
.symtab section
- 9 section index
- 240 bytes offset from start of file
- 312 bytes total size
- 24 bytes per entry
- 13 entries
Found Symbol 'greeting'
- 8 symbol index
- 0x0 value
- 64 size
- 3 section index
- 0 offset in .data of value for symbol
string value: 'Hello fine folks!'
New val is: 'Whazzzzup!?'
> gcc -o test-data/greet_main2 test-input/greet_main.o test-data/greet_funcs2.o
> test-data/greet_main2
Whazzzzup!?
Whazzzzup!?
Whazzzzup!?
Whazzzzup!?
Whazzzzup!?
ENDOUT

((T++))
tnames[T]="Basic Error Conditions"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
./patchsym test-input/globals.c string1 string
./patchsym test-input/naked_globals string1 string
./patchsym test-input/globals nada string
./patchsym test-input/globals main string
./patchsym test-input/globals string2 unknown
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> ./patchsym test-input/globals.c string1 string
GET mode
ERROR: Magic bytes wrong, this is not an ELF file
> ./patchsym test-input/naked_globals string1 string
GET mode
ERROR: Couldn't find symbol table
> ./patchsym test-input/globals nada string
GET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
ERROR: Symbol 'nada' not found
> ./patchsym test-input/globals main string
GET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
Found Symbol 'main'
- 65 symbol index
- 0x1139 value
- 173 size
- 13 section index
ERROR: 'main' in section 13, not in .data section 23
> ./patchsym test-input/globals string2 unknown
GET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
Found Symbol 'string2'
- 66 symbol index
- 0x4060 value
- 64 size
- 23 section index
- 64 offset in .data of value for symbol
ERROR: Unsupported data kind 'unknown'
ENDOUT

((T++))
tnames[T]="Error: globals2 string1 too long"
#
read  -r -d '' cmd[$T] <<"ENDCMD"
cp test-input/globals test-data/globals2
test-data/globals2
./patchsym test-data/globals2 string1 string "Adios, muchachos. Lo siento."
test-data/globals2
ENDCMD
#
read  -r -d '' output[$T] <<"ENDOUT"
> cp test-input/globals test-data/globals2
> test-data/globals2
string1: Hello
string2: Goodbye cruel world
string3: All your bass
int1: aabbccdd
int2: ffeeddcc
a_doub: 1.234567
> ./patchsym test-data/globals2 string1 string 'Adios, muchachos. Lo siento.'
SET mode
.data section
- 23 section index
- 12320 bytes offset from start of file
- 0x4020 preferred virtual address for .data
.symtab section
- 26 section index
- 12504 bytes offset from start of file
- 1680 bytes total size
- 24 bytes per entry
- 70 entries
Found Symbol 'string1'
- 53 symbol index
- 0x4040 value
- 8 size
- 23 section index
- 32 offset in .data of value for symbol
string value: 'Hello'
ERROR: Cannot change symbol 'string1': existing size too small
Cur Size: 8 'Hello'
New Size: 29 'Adios, muchachos. Lo siento.'
> test-data/globals2
string1: Hello
string2: Goodbye cruel world
string3: All your bass
int1: aabbccdd
int2: ffeeddcc
a_doub: 1.234567
ENDOUT
