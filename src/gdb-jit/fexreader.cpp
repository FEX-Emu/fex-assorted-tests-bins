#include <stddef.h>
#include <stdio.h>

#include "gdb/jit-reader.h"

GDB_DECLARE_GPL_COMPATIBLE_READER;

struct blocks_t {
const char name[512];
GDB_CORE_ADDR start;
GDB_CORE_ADDR end;
};

struct info_t {
  char filename[512];
  long blocks_ofs;
  long lines_ofs;

  int nblocks;
  int nlines;
};

#define printf(...)
extern "C" {
static enum gdb_status read_debug_info(struct gdb_reader_funcs* self,
                                       struct gdb_symbol_callbacks* cbs,
                                       void* memory, long memory_sz) {
    info_t* info = (info_t*) memory;
    blocks_t* blocks = (blocks_t*)(info->blocks_ofs + (long)memory);
    gdb_line_mapping* lines = (gdb_line_mapping*)(info->lines_ofs + (long)memory);
	printf("info: %p\n", info);
	printf("info: s %p\n", info->filename);
	printf("info: s %s\n", info->filename);
	printf("info: l %d\n", info->nlines);
	printf("info: b %d\n", info->nblocks);
    struct gdb_object* object = cbs->object_open(cbs);
    struct gdb_symtab* symtab = cbs->symtab_open(cbs, object, info->filename);

    for (int i = 0; i < info->nblocks; i++) {
	printf("info: %d\n", i);
	printf("info: %lx\n", blocks[i].start);
	printf("info: %lx\n", blocks[i].end);
	printf("info: %s\n", blocks[i].name);
    	cbs->block_open(cbs, symtab, NULL, blocks[i].start, blocks[i].end, blocks[i].name);
    }


	printf("info: lines %d\n", info->nlines);
	printf("info: lines %p\n", lines);

    for (int i = 0; i < info->nlines; i++) {
	printf("info: line: %d\n", i);
	printf("info: line pc: %lx\n", lines[i].pc);
	printf("info: line file: %d\n", lines[i].line);
}
    cbs->line_mapping_add(cbs, symtab, info->nlines, lines);

    cbs->symtab_close(cbs, symtab);
    cbs->object_close(cbs, object);
    return GDB_SUCCESS;
}

enum gdb_status unwind_frame(struct gdb_reader_funcs* self,
                             struct gdb_unwind_callbacks* cbs) {
    return GDB_SUCCESS;
}

struct gdb_frame_id get_frame_id(struct gdb_reader_funcs* self,
                                 struct gdb_unwind_callbacks* cbs) {
    struct gdb_frame_id frame = { 0x1234000, 0 };
    return frame;
}

void destroy_reader(struct gdb_reader_funcs* self) { }

extern struct gdb_reader_funcs* gdb_init_reader(void) {
    static struct gdb_reader_funcs funcs = {
        GDB_READER_INTERFACE_VERSION,
	NULL,
	read_debug_info,
	unwind_frame,
	get_frame_id,
	destroy_reader
    };
    return &funcs;
}

}
