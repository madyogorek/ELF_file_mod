// Template to complete the patchsym program which can retrive global
// symbol values or change them. Sections that start with a CAPITAL in
// their comments require additions and modifications to complete the
// program.
//Madelyn Ogorek ogore014 5454524 CSCI 2021

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>

int DEBUG = 0;                  // controls whether to print debug messages

#define GET_MODE 1              // only get the value of a symbol
#define SET_MODE 2              // change the value of a symbol

int main(int argc, char **argv){
  // PROVIDED: command line handling of debug option
  if( argc > 1 && strcmp(argv[1], "-d")==0 ){
    DEBUG = 1;                  // check 1st arg for -d debug
    argv++;                     // shift args forward if found
    argc--;
  }

  if(argc < 4){
    printf("usage: %s [-d] <file> <symbol> <type> [newval]\n",argv[0]);
    return 0;
  }

  int mode = GET_MODE;          // default to GET_MODE
  char *new_val = NULL;
  if(argc == 5){                // if an additional arg is provided run in SET_MODE
    printf("SET mode\n");
    mode = SET_MODE;
    new_val = argv[4];
  }
  else{
    printf("GET mode\n");
  }
  char *objfile_name = argv[1];
  char *symbol_name = argv[2];
  char *symbol_kind = argv[3];

  // PROVIDED: open file to get file descriptor
  int fd = open(objfile_name, O_RDWR);

  // DETERMINE size of file and create read/write memory map of the file
  //determining size of file, setting it equal to size
  struct stat stat_buffer;
  fstat(fd, &stat_buffer);
  int size = stat_buffer.st_size;

  //creating mem map and assigning a pointer to the beginning of the file
  void *elf_mm = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  // CREATE A POINTER to the intial bytes of the file which are an ELF64_Ehdr struct
  Elf64_Ehdr *ehdr = NULL;
  ehdr = (Elf64_Ehdr *) elf_mm;

  // CHECK e_ident field's bytes 0 to for for the sequence {0x7f,'E','L','F'}.
  // Exit the program with code 1 if the bytes do not match
  if(ehdr->e_ident[0] != 0x7f || ehdr->e_ident[1] != 'E' ||
  ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F')
  {
    printf("ERROR: Magic bytes wrong, this is not an ELF file\n");
    munmap(elf_mm, size);
    close(fd);
    return 1;
  }


  // PROVIDED: check for a 64-bit file
  if(ehdr->e_ident[EI_CLASS] != ELFCLASS64){
    printf("ERROR: Not a 64-bit file ELF file\n");
    // UNMAP, CLOSE FD
    munmap(elf_mm, size);
    close(fd);
    return 1;
  }

  // PROVIDED: check for x86-64 architecture
  if(ehdr->e_machine != EM_X86_64){
    printf("ERROR: Not an x86-64 file\n");
    // UNMAP, CLOSE FD
    munmap(elf_mm, size);
    close(fd);
    return 1;
  }

  // DETERMINE THE OFFSET of the Section Header Array (e_shoff), the
  // number of sections (e_shnum), and the index of the Section Header
  // String table (e_shstrndx). These fields are from the ELF File
  // Header.
  Elf64_Off section_header_offset = ehdr->e_shoff;
  Elf64_Half num_sections = ehdr->e_shnum;
  Elf64_Half sec_hdr_index = ehdr->e_shstrndx;

  // SET UP a pointer to the array of section headers. Use the section
  // header string table index to find its byte position in the file
  // and set up a pointer to it.
  Elf64_Shdr *sec_hdrs = (elf_mm + section_header_offset);
  //offseting the beginning of the section header section by the index the
  //section header string table starts at
  Elf64_Shdr *sec_hdr_str_table = (Elf64_Shdr *) sec_hdrs + sec_hdr_index ;


  // potentially introduce new variables.

  //these ints will let me whether or not these 3 were found in the
  //section header array, they will remain 0 if they were not found
  int symtabint = 0;
  int dataint = 0;
  int strtabint = 0;

  //tells me the offset of the symbol table
  Elf64_Off symtab_offset;

  //the size of the symbol table
  Elf64_Xword symtab_bytes;
  //the size of each entry in the symbol table, using these 2 vars we
  //calculate the number of entries in the string table
  Elf64_Xword entsize;

  //offset for the data section
  Elf64_Off data_offset = 0;
  //offset for the string table (different from the section header's string table)
  Elf64_Off strtab_offset;

  //letting me know where in the array data was found
  int data_index;
  //will be filled with the preferred virtual loading address for .data
  Elf64_Addr dat_addy = 0;
  //letting me know where in the array the symbol table was found
  int symtab_index;

  // SEARCH the Section Header Array for sections with names .symtab
  // (symbol table) .strtab (string table), and .data (initialized
  // data sections).  Note their positions in the file (sh_offset
  // field).  Also note the size in bytes (sh_size) and and the size
  // of each entry (sh_entsize) for .symtab so its number of entries
  // can be computed. Finally, note the .data section's index (i value
  // in loop) and its load address (sh_addr).

  for(int i=0; i< num_sections; i++){
    //finding the sh_name offset for this entry, adding it to the
    //beginning of the file + the location of the beginning of the section
    //header's string table so that we know where in this table to look for
    //the correct name
    char *sec_name = elf_mm + sec_hdr_str_table->sh_offset + sec_hdrs[i].sh_name;

    //each of these if statements should be entered once, if not error
    if(strcmp(sec_name, ".symtab\0") == 0)
    {
      symtab_offset = sec_hdrs[i].sh_offset;
      symtab_bytes = sec_hdrs[i].sh_size;
      entsize = sec_hdrs[i].sh_entsize;
      symtabint = 1;
      symtab_index = i;
    }
    else if(strcmp(sec_name, ".strtab\0") == 0)
    {
      strtab_offset = sec_hdrs[i].sh_offset;
      strtabint = 1;
    }
    else if(strcmp(sec_name, ".data\0") == 0)
    {
      data_offset = sec_hdrs[i].sh_offset;
      data_index = i;
      dat_addy = sec_hdrs[i].sh_addr;
      dataint = 1;
    }
  }


  // ERROR check to ensure everything was found based on things that
  // could not be found.
  if(symtabint == 0){
    printf("ERROR: Couldn't find symbol table\n");
    // UNMAP, CLOSE FD
    munmap(elf_mm, size);
    close(fd);
    return 1;
  }

  if(strtabint == 0){
    printf("ERROR: Couldn't find string table\n");
    // UNMAP, CLOSE FD
    munmap(elf_mm, size);
    close(fd);
    return 1;
  }

  if(dataint == 0){
    printf("ERROR: Couldn't find data section\n");
    // UNMAP, CLOSE FD
    munmap(elf_mm, size);
    close(fd);
    return 1;
  }

  // PRINT info on the .data section where symbol values are stored

  printf(".data section\n");
  printf("- %hd section index\n", data_index);
  printf("- %lu bytes offset from start of file\n", data_offset);
  printf("- 0x%lx preferred virtual address for .data\n",dat_addy);


  // PRINT byte information about where the symbol table was found and
  // its sizes. The number of entries in the symbol table can be
  // determined by dividing its total size in bytes by the size of
  // each entry.

  printf(".symtab section\n");
  printf("- %hd section index\n", symtab_index);
  printf("- %lu bytes offset from start of file\n", symtab_offset);
  printf("- %lu bytes total size\n", symtab_bytes);
  printf("- %lu bytes per entry\n", entsize);
  printf("- %lu entries\n", symtab_bytes / entsize);

  // SET UP pointers to the Symbol Table and associated String Table
  // using offsets found earlier. Then SEARCH the symbol table for for
  // the specified symbol.


  //using the symbol table's offset that was found in the first for loop,
  //we can find that table and assign a pointer to it
  Elf64_Sym *symtable = elf_mm + symtab_offset;
  //we do the same for the string table
  char *strtable = elf_mm + strtab_offset;

    //iterating over number of entries
  for(int i=0; i< (symtab_bytes / entsize); i++){
    //.st_name will tell us (as an index) where to look in the String
    //table for this symbol's name
    char *name = strtable + symtable[i].st_name;

    //should enter once, if not the symbol isn't in the file (error)
    if(strcmp(name, symbol_name) == 0){
      // If symbol at position i matches the 'symbol_name' variable
      // defined at the start of main(), it is the symbol to work on.
      // PRINT data about the found symbol.
      printf("Found Symbol '%s'\n",symbol_name);
      printf("- %d symbol index\n",i);
      printf("- 0x%lx value\n",symtable[i].st_value);
      printf("- %lu size\n",symtable[i].st_size);
      printf("- %hu section index\n",symtable[i].st_shndx);

      // CHECK that the symbol table field st_shndx matches the index
      // of the .data section; otherwise the symbol is not a global
      // variable and we should bail out now.

      if(symtable[i].st_shndx != data_index){
        printf("ERROR: '%s' in section %hd, not in .data section %hd\n",symbol_name,symtable[i].st_shndx,data_index);
        // UNMAP, CLOSE FD
        return 1;
      }

      // CALCULATE the offset of the value into the .data section. The
      // 'value' field of the symbol is its preferred virtual
      // address. The .data section also has a preferred load virtual
      // address. The difference between these two is the offset into
      // the .data section of the mmap()'d file.

      //finding the value of this specific symbol by subtracting .data's
      //loading address from where this symbol starts
      Elf64_Addr offset = symtable[i].st_value - dat_addy;

      printf("- %ld offset in .data of value for symbol\n",offset);

      // Symbol found, location in .data found, handle each kind (type
      // in C) of symbol value separately as there are different
      // conventions to change a string, an int, and so on.

      // string is the only required kind to handle
      if( strcmp(symbol_kind,"string")==0 ){
        // PRINT the current string value of the symbol in the .data section
        char *offset_str = offset + elf_mm + data_offset;
        printf("string value: '%s'\n",offset_str);

        // Check if in SET_MODE in which case change the current value to a new one
        if(mode == SET_MODE){

          // CHECK that the length of the new value of the string in
          // variable 'new_val' is short enough to fit in the size of
          // the symbol.
          if(strlen(new_val) > symtable[i].st_size ){
            // New string value is too long, print an error
            printf("ERROR: Cannot change symbol '%s': existing size too small\n",symbol_name);
            printf("Cur Size: %lu '%s'\n", symtable[i].st_size, offset_str);
            printf("New Size: %lu '%s'\n", strlen(new_val) + 1, new_val);
            // UNMAP, CLOSE FD
            return 1;
          }
          else{
            // COPY new string into symbols space in .data as it is big enough
            strcpy(offset_str,new_val);
            // PRINT the new string value for the symbol
            printf("New val is: '%s'\n", new_val);
          }
        }
      }

      // OPTIONAL: fill in else/ifs here for other types on might want
      // to support such as int and double

      else{
        printf("ERROR: Unsupported data kind '%s'\n",symbol_kind);
        // UNMAP, CLOSE FD
        munmap(elf_mm, size);
        close(fd);
        return 1;
      }

      // succesful completion of getting or setting the symbol
      // UNMAP, CLOSE FD
      munmap(elf_mm, size);
      close(fd);
      return 0;
    }
  }

  // Iterated through whole symbol tabel and did not find symbol, error out.
  printf("ERROR: Symbol '%s' not found\n",symbol_name);
  // UNMAP, CLOSE FD
  munmap(elf_mm, size);
  close(fd);
  return 1;
}
