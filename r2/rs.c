#include <stdio.h>
#include <string.h>

#include <r_core.h>
#include <r_bin.h>
#include <r_util.h>

#include "elf64.h"

/**
    This is a test of using radare2 to resize a file.

    Using the normal API, resizing a section and then writing the
    file does not work, so we have to muck around the underlying
    elf interface to make it happen.

    Even calling rabin2 -O r/.payload/xxxx doesn't even modify the
    binary...

    Using this inside of the malware's runtime, we can have the 
    malware resize itself. 
*/ 
int main(int argc, char *argv[])
{   
   if (argc != 4) {
     printf("Usage: %s <ELF file> <section> <size>\n", argv[0]);
   }
   /** This code is mostly copied from the rabin.c source file. */ 
   struct r_bin_t *bin;
   RCore core;
   RCoreFile *cf = NULL;
   int fd = -1;
   int rawstr = 0;
   int xtr_idx = 0; // load all files if extraction is necessary
   r_core_init(&core);

   bin = core.bin;
    
   cf = r_core_file_open (&core, argv[1], R_IO_READ, 0);
   fd = cf ? r_core_file_cur_fd (&core) : -1;

   if (!cf || fd == -1) {
       eprintf("r_core: Cannot open file\n");
       r_core_fini(&core);
       return 1;
   }
 
   if (!r_bin_load(bin, argv[1], 0, 0, xtr_idx, fd, rawstr)) {
       fprintf(stderr, "Cannot open '%s'.\n", argv[1]);
       exit(1);
   }

   ut64 sz = r_num_math(NULL, argv[3]);
   printf("Setting size to: %d\n", sz);

   ut64 delta = r_bin_wr_scn_resize(bin, argv[2], r_num_math(NULL, argv[3])); 

   if (!delta) {
       fprintf(stderr, "Cannot resize section %s", argv[2]);
       exit(1);
   }
   printf("File has changed %d bytes\n", delta);

   RBinFile *file = r_bin_cur(bin);
   struct Elf_(r_bin_elf_obj_t) *bin_obj = file->o->bin_obj;

   printf("New length %d\n", bin_obj->b->length);

   struct r_bin_elf_symbol_t *sym =
       Elf_(r_bin_elf_get_symbols)(bin_obj, R_BIN_ELF_SYMBOLS);

   /**
     Offset gives the symbol's address in the program. Therefore, we can
     easily change its offset in the struct we have been given,
     but I need to look at the elf_get_symbols code to see how
     I can actually change it in the elf file.
   */
   while (sym->last != 1) {
       if (!strcmp(sym->name, "_binary_runtime_bc_enc_size")) {
         printf("name: %s\n", sym->name);
         printf("offset: %x\n", sym->offset);
         printf("size: %x\n", sym->size);
         printf("ordinal: %x\n", sym->ordinal);
       }
       sym++;
   }

   r_file_dump("myboot", bin_obj->b->buf, bin_obj->b->length);

   return 1;
}
