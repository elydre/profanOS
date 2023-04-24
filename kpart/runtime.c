#include <kernel/filesystem.h>
#include <kernel/snowflake.h>
#include <driver/serial.h>
#include <kernel/process.h>
#include <minilib.h>
#include <system.h>
#include <type.h>

/*******************************************
 * binary_mem is pointer to the memory of *
 * the program with the following layout. *
 * |  <--Lstack--|--BINARY--|-Rstack->  | *
*******************************************/

typedef struct {
    int argc;
    char **argv;
    char *path;
} comm_struct_t;

void tasked_program() {
    int pid = process_get_pid();
    int ppid = process_get_ppid(pid);

    comm_struct_t *comm = process_get_bin_mem(pid);
    int argc = comm->argc;
    char **argv = comm->argv;
    char *path = comm->path;
    free(comm);

    // setup private memory

    // we need to allign to 4KB
    uint32_t physical = mem_alloc(RUN_BIN_VIRT, 0x100000, 4);
    mem_set((uint8_t *) physical, 0, RUN_BIN_VIRT);

    for (uint32_t i = 0; i < 0x100000; i += 0x1000) {
        scuba_map(process_get_directory(pid), 0xC0000000 + i, (uint32_t) physical + i);
    }

    // load binary
    fs_read_file(path, (char *) 0xC0000000);

    // call main
    int ret;
    asm volatile("call *%1" : "=a"(ret) : "r"(0xC0000000), "b"(argc), "c"(argv));

    int not_free_mem = mem_get_info(7, pid);

    if (not_free_mem) {
        sys_warning("Memory leak detected");

        kprintf("[auto free] %d alloc will be auto freed (total: %d bytes, pid: %d)\n",
                not_free_mem,
                mem_get_info(8, pid),
                pid
        );

        mem_free_all(pid);
    }

    free((void *) physical);

    process_wakeup(ppid);
    process_exit();
}

int run_binary(char path[], int argc, char **argv) {
    UNUSED(path);
    UNUSED(argc);
    UNUSED(argv);
    // TODO: check if file is executable

    serial_debug("RUNTIME", path);
    int pid = process_create(tasked_program, 2, path);

    comm_struct_t *comm = (comm_struct_t *) mem_alloc(sizeof(comm_struct_t), 0, 6);
    comm->argc = argc;
    comm->argv = argv;
    comm->path = path;

    process_set_bin_mem(pid, comm);
    process_handover(pid);

    return 0;
}

int run_ifexist(char path[], int argc, char **argv) {
    if (fs_does_path_exists(path) && fs_get_sector_type(fs_path_to_id(path)) == 2)
        return run_binary(path, argc, argv);
    sys_error("Program not found");
    return -1;
}
