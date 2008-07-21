#ifndef OPSOUP_H
#define OPSOUP_H 1

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "nasmlib.h"
#include "disasm.h"
#include "sync.h"

/* segment types */
typedef enum {
    seg_NONE = 0,
    seg_CODE,
    seg_DATA,
    seg_BSS,
    seg_IMPORT,
    seg_RELOC
} segment_type_t;

/* segment data */
typedef struct segment_st {
    char            *name;
    segment_type_t   type;
    uint8_t         *start;
    uint8_t         *end;
    uint32_t         size;
    void            *info;
} segment_t;

typedef struct _image_st {
    int              fd;
    uint8_t         *core;
    uint32_t         size;
    segment_t       *segment;
} image_t;

typedef struct _reloc_st {
    uint8_t         *mem;
    uint8_t         *target;
} reloc_t;

typedef struct _opsoup_st {
    image_t          image;

    reloc_t         *reloc;
    int              nreloc;

    int              verbose;
} opsoup_t;

extern opsoup_t *o;


int         image_load(void);
segment_t   *image_seg_find(uint8_t *mem);



/* labels */
 
/* label types */
typedef enum {
    label_NONE          = 0x0,

    label_VTABLE        = 0x1,      /* vtable mask */

    label_RELOC         = 0x10,     /* relocation target */

    label_DATA          = 0x20,     /* data item */
    label_DATA_VTABLE   = 0x21,     /* jump/call vector table */

    label_BSS           = 0x40,     /* uninitialised data item */
    label_BSS_VTABLE    = 0x41,     /* uninitialised jump/call vector table */

    label_CODE          = 0x80,     /* code item */
    label_CODE_JUMP     = 0x82,     /* jump point ("jmp" instruction target) */
    label_CODE_CALL     = 0x84,     /* call point ("call" instruction target) */

    label_IMPORT        = 0x100     /* vector to imported function */
} label_type_t;

/* extra magic for import labels */
typedef struct import_st {
    char            *dllname;       /* name of the dll this symbol is in */
    char            *symbol;        /* symbol name */
    int             hint;           /* symbol hint for if the symbol name isn't present */
} import_t;

/* label data */
typedef struct label_st {
    uint8_t         *target;         /* where this label is located */
    label_type_t    type;           /* its type */
    segment_t       *seg;           /* segment the label is in, for convenience */

    int             num;            /* label number (generated by label_number()) */

    import_t        import;         /* import fu */
} label_t;

extern label_t  *label;
extern int      nlabel;

label_t         *label_find(uint8_t *target);
label_t         *label_insert(uint8_t *target, label_type_t type, segment_t *s);
void            label_remove(uint8_t *target);
void            label_ref_check(void);
void            label_reloc_upgrade(void);
int             label_print_count(char *str);
void            label_number(void);


/* label references */

/* max possible targets per offset */
#define MAX_REF_TARGET (4)

/* reference data */
typedef struct ref_st {
    uint8_t       *mem;                    /* location of the reference (eg instruction that uses the label */
    uint8_t       *target[MAX_REF_TARGET]; /* location of target labels (one instruction can use more than one label) */    
    int             ntarget;                /* number of targets */
} ref_t;

extern ref_t    *ref;
extern int      nref;

ref_t           *ref_insert(uint8_t *source, uint8_t *target);


/* imports */
void    import_process(void);
void    import_output(FILE *f);
void    import_stub(FILE *f);


/* disassembly */
void    dis_pass1(void);
int     dis_pass2(int n);
void    dis_pass3(FILE *f);


/* data "disassembly" */
void    data_output(FILE *f);
void    data_bss_output(FILE *f);


/* elf stuff */
int elf_make_segment_table(image_t *image);
int elf_relocate(opsoup_t *o);

#endif
