#include <driver/rtc.h>
#include <filesystem.h>
#include <function.h>
#include <cpu/isr.h>
#include <system.h>
#include <iolib.h>
#include <time.h>
#include <task.h>
#include <mem.h>

void kernel_main() {
    clear_screen();
    fskprint("$6booting profanOS...\n");

    isr_install();
    irq_install();
    fskprint("ISR initialized\n");
    
    tasking_init();
    fskprint("Tasking initialized\n");

    rtc_init();
    time_gen_boot();
    init_rand();
    fskprint("RTC initialized\n");

    filesystem_init();
    fskprint("FileSys initialized\n");
        
    init_watfunc();
    fskprint("WatFunc initialized\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    fskprint("$C~~ version $4%s $C~~\n\n", VERSION);

    // launch of the shell.bin
    char **argv = malloc(sizeof(char *));
    argv[0] = "/bin/shell.bin";
    sys_run_ifexist("/bin/shell.bin", 1, argv);
    free(argv);
    start_kshell();
    
    sys_fatal("Nothing to run!");
}
