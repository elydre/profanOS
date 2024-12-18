#include <cpu/ports.h>
#include <ktype.h>
#include <minilib.h>
#include <drivers/pci.h>
#include <cpu/isr.h>
#include <kernel/snowflake.h>
#include <cpu/timer.h>

static int this_eth_ids[][2] = {
	{0x10ec, 0x8161},
	{0x10ec, 0x8168},
	{0x10ec, 0x8169},
	{0x1259, 0xc107},
	{0x1737, 0x1032},
	{0x16ec, 0x0116},
	{0, 0}
};

int eth_8168_init() {
	return 0;
}
