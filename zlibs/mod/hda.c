/*****************************************************************************\
|   === hda.c : 2025 ===                                                      |
|                                                                             |
|    Intel High Definition Audio (HDA) driver implementation       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/


#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <cpu/ports.h>
#include <cpu/timer.h>
#include <minilib.h>

#define logf kprintf

#define wait(ms) process_sleep(process_get_pid(), ms)

static inline void mmio_outb(uint32_t base, uint8_t value) {
    *(volatile uint8_t *) base = value;
}

static inline void mmio_outw(uint32_t base, uint16_t value) {
    *(volatile uint16_t *) base = value;
}

static inline void mmio_outd(uint32_t base, uint32_t value) {
    *(volatile uint32_t *) base = value;
}

static inline uint8_t mmio_inb(uint32_t base) {
    return *(volatile uint8_t *) base;
}

static inline uint16_t mmio_inw(uint32_t base) {
    return *(volatile uint16_t *) base;
}

static inline uint32_t mmio_ind(uint32_t base) {
    return *(volatile uint32_t *) base;
}

///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
} pci_addr_t;

static void pci_write_config(pci_addr_t *pci, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (pci->bus << 16) | (pci->slot << 11) | (pci->func << 8) | (offset & 0xFC);
    port_write32(0xCF8, address);
    port_write32(0xCFC, value);
}

static uint32_t pci_read_config(pci_addr_t *pci, uint32_t offset) {
    uint32_t lbus  = (uint32_t) pci->bus;
    uint32_t lslot = (uint32_t) pci->slot;
    uint32_t lfunc = (uint32_t) pci->func;

    // Write out the address
    port_write32(0xCF8, (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t) 0x80000000)));

    return port_read32(0xCFC);
}

static uint32_t pci_get_bar(pci_addr_t *pci, uint8_t barN) {
    return pci_read_config(pci, 0x10 + (barN * 4));
}

static pci_addr_t pci_find(uint16_t vendor, uint16_t device) {
    // scan all PCI devices and return the first one that matches
    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 16; slot++) {
            for (int func = 0; func < 8; func++) {
                pci_addr_t pci = {bus, slot, func};
                uint16_t vid = pci_read_config(&pci, 0) & 0xffff;
                if (vid == 0xFFFF)
                    continue;
                uint16_t did = (pci_read_config(&pci, 2) >> 16) & 0xffff;
                if (vid == vendor && did == device) {
                    return pci;
                }
            }
        }
    }
    return (pci_addr_t){0xFF, 0xFF, 0xFF}; // not found
}

///////////////////////////////////////////////////////////////////////////////////////////////

struct hda_info_t {
    uint32_t base;
    uint32_t input_stream_base;
    uint32_t output_stream_base;
    uint32_t communication_type;
    uint32_t codec_number;
    uint32_t is_initalized_useful_output;
    uint32_t selected_output_node;

    uint32_t *corb_mem;
    uint32_t corb_pointer;
    uint32_t corb_number_of_entries;
    uint32_t *rirb_mem;
    uint32_t rirb_pointer;
    uint32_t rirb_number_of_entries;

    uint32_t length_of_node_path;

    uint32_t afg_node_sample_capabilities;
    uint32_t afg_node_stream_format_capabilities;
    uint32_t afg_node_input_amp_capabilities;
    uint32_t afg_node_output_amp_capabilities;

    uint32_t audio_output_node_number;
    uint32_t audio_output_sample_capabilities;
    uint32_t audio_output_stream_format_capabilities;

    uint32_t output_amp_node_number;
    uint32_t output_amp_node_capabilities;

    uint32_t second_audio_output_node_number;
    uint32_t second_audio_output_sample_capabilities;
    uint32_t second_audio_output_stream_format_capabilities;
    uint32_t second_output_amp_node_number;
    uint32_t second_output_amp_node_capabilities;

    uint32_t pin_output_node_number;
    uint32_t pin_headphone_node_number;
};

#define HDA_UNINITALIZED 0
#define HDA_CORB_RIRB 1
#define HDA_PIO 2

#define HDA_WIDGET_AUDIO_OUTPUT 0x0
#define HDA_WIDGET_AUDIO_INPUT 0x1
#define HDA_WIDGET_AUDIO_MIXER 0x2
#define HDA_WIDGET_AUDIO_SELECTOR 0x3
#define HDA_WIDGET_PIN_COMPLEX 0x4
#define HDA_WIDGET_POWER_WIDGET 0x5
#define HDA_WIDGET_VOLUME_KNOB 0x6
#define HDA_WIDGET_BEEP_GENERATOR 0x7
#define HDA_WIDGET_VENDOR_DEFINED 0xF

#define HDA_PIN_LINE_OUT 0x0
#define HDA_PIN_SPEAKER 0x1
#define HDA_PIN_HEADPHONE_OUT 0x2
#define HDA_PIN_CD 0x3
#define HDA_PIN_SPDIF_OUT 0x4
#define HDA_PIN_DIGITAL_OTHER_OUT 0x5
#define HDA_PIN_MODEM_LINE_SIDE 0x6
#define HDA_PIN_MODEM_HANDSET_SIDE 0x7
#define HDA_PIN_LINE_IN 0x8
#define HDA_PIN_AUX 0x9
#define HDA_PIN_MIC_IN 0xA
#define HDA_PIN_TELEPHONY 0xB
#define HDA_PIN_SPDIF_IN 0xC
#define HDA_PIN_DIGITAL_OTHER_IN 0xD
#define HDA_PIN_RESERVED 0xE
#define HDA_PIN_OTHER 0xF

#define HDA_OUTPUT_NODE 0x1
#define HDA_INPUT_NODE 0x2

void hda_add_new_pci_device(void);

void hda_initalize_sound_card(void);
uint32_t hda_send_verb(uint32_t codec, uint32_t node, uint32_t verb, uint32_t command);

uint8_t hda_get_node_type(uint32_t codec, uint32_t node);
uint16_t hda_get_node_connection_entry(uint32_t codec, uint32_t node, uint32_t connection_entry_number);
void hda_set_node_gain(uint32_t codec, uint32_t node, uint32_t node_type, uint32_t capabilities, uint32_t gain);
void hda_enable_pin_output(uint32_t codec, uint32_t pin_node);
void hda_disable_pin_output(uint32_t codec, uint32_t pin_node);
uint8_t hda_is_headphone_connected(void);

void hda_initalize_codec(uint32_t codec_number);
void hda_initalize_audio_function_group(uint32_t afg_node_number);
void hda_initalize_output_pin(uint32_t pin_node_number);
void hda_initalize_audio_output(uint32_t audio_output_node_number);
void hda_initalize_audio_mixer(uint32_t audio_mixer_node_number);
void hda_initalize_audio_selector(uint32_t audio_selector_node_number);

void hda_set_volume(uint32_t volume);
void hda_check_headphone_connection_change(void);

uint8_t hda_is_supported_channel_size(uint8_t size);
uint8_t hda_is_supported_sample_rate(uint32_t sample_rate);
uint16_t hda_return_sound_data_format(uint32_t sample_rate, uint32_t channels, uint32_t bits_per_sample);
void hda_play_pcm_data(uint32_t sample_rate, void* pcm_data, uint32_t buffer_size, uint16_t lvi);
void hda_stop_sound(void);
uint32_t hda_get_actual_stream_position(void);

#define HDA_VERB_ERROR 0

///////////////////////////////////////////////////////////////////////////////////////////////

struct hda_info_t g_hda;

uint32_t ticks;

int __init(void) {
    logf("\n");

    mem_set(&g_hda, 0, sizeof(g_hda));

    // read other device informations
    pci_addr_t device = pci_find(0x8086, 0x2668);

    if (device.bus == 0xFF && device.slot == 0xFF && device.func == 0xFF) {
        device = pci_find(0x8086, 0x3198); // my laptop

        if (device.bus == 0xFF && device.slot == 0xFF && device.func == 0xFF) {
            logf("HDA: No HDA PCI device found\n");
            return 0;
        }
    }

    logf("HDA: Found HDA PCI device at %d:%d:%d\n",
        device.bus,
        device.slot,
        device.func);

    g_hda.base = pci_get_bar(&device, 0);

    logf("HDA: HDA PCI device MMIO base %x\n", g_hda.base);
    g_hda.base &= 0xFFFFFFF0; // mask memory type

    logf("HDA:                       -> %x\n", g_hda.base);

    scuba_call_map((void *)g_hda.base, (void *)g_hda.base, 0);

    // configure PCI
    // pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_MMIO);
    pci_write_config(&device, 0x04, pci_read_config(&device, 0x04) | 0x6);

    hda_initalize_sound_card();

    return 0;
}

void my_handler(uint32_t sample_rate, void* pcm_data, uint32_t buffer_size, uint16_t lvi) {
    kprintf_serial("my_handler 1\n");
    hda_play_pcm_data(sample_rate, pcm_data, buffer_size, lvi);
}

int my_handler2(uint32_t sample_rate) {
    kprintf_serial("my_handler 2\n");
    scuba_call_map((void *)g_hda.base, (void *)g_hda.base, 0);
    return hda_is_supported_sample_rate(sample_rate);
}

void my_handler3(uint32_t volume) {
    kprintf_serial("my_handler 3\n");
    hda_set_volume(volume);
}

uint32_t my_handler4(void) {
    return hda_get_actual_stream_position();
}

void my_handler5(void) {
    kprintf_serial("my_handler 5\n");
    hda_check_headphone_connection_change();
}

void my_handler6(void) {
    kprintf_serial("my_handler 6\n");
    hda_stop_sound();
}

void hda_initalize_sound_card(void) {
    uint32_t hda_base = g_hda.base;
    g_hda.communication_type = HDA_UNINITALIZED;
    g_hda.is_initalized_useful_output = 0;

    // reset card and set operational state
    mmio_outd(hda_base + 0x08, 0x0);
    ticks = TIMER_TICKS;
    while (TIMER_TICKS - ticks < 10) {
        asm("nop");
        if ((mmio_ind(hda_base + 0x08) & 0x1)==0x0) {
            break;
        }
    }

    mmio_outd(hda_base + 0x08, 0x1);
    ticks = TIMER_TICKS;
    while (TIMER_TICKS - ticks < 10) {
        asm("nop");
        if ((mmio_ind(hda_base + 0x08) & 0x1)==0x1) {
            break;
        }
    }

    if ((mmio_ind(hda_base + 0x08) & 0x1)==0x0) {
        logf("HDA ERROR: card can not be set to operational state\n");
        return;
    }

    // read capabilities
    logf("Version: %d.%d\n", mmio_inb(hda_base + 0x03), mmio_inb(hda_base + 0x02));
    g_hda.input_stream_base = (hda_base + 0x80);
    g_hda.output_stream_base = (hda_base + 0x80 +
                (0x20 * ((mmio_inw(hda_base + 0x00) >> 8) & 0xF))); // skip input streams ports

    // disable interrupts
    mmio_outd(hda_base + 0x20, 0);

    // turn off dma position transfer
    mmio_outd(hda_base + 0x70, 0);
    mmio_outd(hda_base + 0x74, 0);

    // disable synchronization
    mmio_outd(hda_base + 0x34, 0);
    mmio_outd(hda_base + 0x38, 0);

    // stop CORB and RIRB
    mmio_outb(hda_base + 0x4C, 0x0);
    mmio_outb(hda_base + 0x5C, 0x0);

    // TODO: the CORB/RIRB interface is not working and causes random issues

    /*
    // configure CORB
    g_hda.corb_mem = (uint32_t *) (aligned_calloc(256*4, 0x7F));
    mmio_outd(hda_base + 0x40, (uint32_t)g_hda.corb_mem); // CORB lower memory
    mmio_outd(hda_base + 0x44, 0); // CORB upper memory

    if ((mmio_inb(hda_base + 0x4E) & 0x40) == 0x40) {
        g_hda.corb_number_of_entries = 256;
        mmio_outb(hda_base + 0x4E, 0x2); // 256 entries
        logf("CORB: 256 entries\n");
    } else if ((mmio_inb(hda_base + 0x4E) & 0x20) == 0x20) {
        g_hda.corb_number_of_entries = 16;
        mmio_outb(hda_base + 0x4E, 0x1); // 16 entries
        logf("CORB: 16 entries\n");
    } else if ((mmio_inb(hda_base + 0x4E) & 0x10) == 0x10) {
        g_hda.corb_number_of_entries = 2;
        mmio_outb(hda_base + 0x4E, 0x0); // 2 entries
        logf("CORB: 2 entries\n");
    } else { // CORB/RIRB is not supported
        logf("CORB: no size allowed\n");
        goto hda_use_pio_interface;
    }

    mmio_outw(hda_base + 0x4A, 0x8000); // reset read pointer

    ticks = TIMER_TICKS;
    while (TIMER_TICKS - ticks < 5) {
        asm("nop");
        if ((mmio_inw(hda_base + 0x4A) & 0x8000) == 0x8000) { // wait until reset is complete
            break;
        }
    }

    if ((mmio_inw(hda_base + 0x4A) & 0x8000) == 0x0000) { // CORB read pointer was not reseted
        logf("HDA: CORB pointer can not be put to reset state\n");
        goto hda_use_pio_interface;
    }

    mmio_outw(hda_base + 0x4A, 0x0000); // go back to normal state

    ticks = TIMER_TICKS;
    while (TIMER_TICKS - ticks < 5) {
        asm("nop");
        if ((mmio_inw(hda_base + 0x4A) & 0x8000) == 0x0000) { // wait until is CORB read pointer in normal state
            break;
        }
    }

    if ((mmio_inw(hda_base + 0x4A) & 0x8000) == 0x8000) { // CORB read pointer is still in reset
        logf("HDA: CORB pointer can not be put from reset state\n");
        goto hda_use_pio_interface;
    }

    mmio_outw(hda_base + 0x48, 0); // set write pointer
    g_hda.corb_pointer = 1;

    // configure RIRB
    g_hda.rirb_mem = (uint32_t *) (aligned_calloc(256 * 8, 0x7F));
    mmio_outd(hda_base + 0x50, (uint32_t) g_hda.rirb_mem); // RIRB lower memory
    mmio_outd(hda_base + 0x54, 0); // RIRB upper memory

    if ((mmio_inb(hda_base + 0x5E) & 0x40) == 0x40) {
        g_hda.rirb_number_of_entries = 256;
        mmio_outb(hda_base + 0x5E, 0x2); //256 entries
        logf("RIRB: 256 entries\n");
    } else if ((mmio_inb(hda_base + 0x5E) & 0x20) == 0x20) {
        g_hda.rirb_number_of_entries = 16;
        mmio_outb(hda_base + 0x5E, 0x1); // 16 entries
        logf("RIRB: 16 entries\n");
    } else if ((mmio_inb(hda_base + 0x5E) & 0x10) == 0x10) {
        g_hda.rirb_number_of_entries = 2;
        mmio_outb(hda_base + 0x5E, 0x0); // 2 entries
        logf("RIRB: 2 entries\n");
    } else { // CORB/RIRB is not supported
        logf("RIRB: no size allowed\n");
        goto hda_use_pio_interface;
    }

    mmio_outw(hda_base + 0x58, 0x8000); // reset write pointer
    wait(10);
    mmio_outw(hda_base + 0x5A, 0); // disable interrupts
    g_hda.rirb_pointer = 1;

    // start CORB and RIRB
    mmio_outb(hda_base + 0x4C, 0x2);
    mmio_outb(hda_base + 0x5C, 0x2);

    // find codec and working communication interface
    // TODO: find more codecs

    for (uint32_t codec_number = 0, codec_id = 0; codec_number < 16; codec_number++) {
        g_hda.communication_type = HDA_CORB_RIRB;
        codec_id = hda_send_verb(codec_number, 0, 0xF00, 0);

        if (codec_id != HDA_VERB_ERROR) {
            logf("HDA: CORB/RIRB communication interface\n");
            hda_initalize_codec(codec_number);
            return; // initalization is complete
        }
    }
    */

    hda_use_pio_interface:
    // stop CORB and RIRB
    mmio_outb(hda_base + 0x4C, 0x0);
    mmio_outb(hda_base + 0x5C, 0x0);

    for (uint32_t codec_number = 0, codec_id = 0; codec_number < 16; codec_number++) {
        g_hda.communication_type = HDA_PIO;
        codec_id = hda_send_verb(codec_number, 0, 0xF00, 0);

        if (codec_id != HDA_VERB_ERROR) {
            logf("HDA: PIO communication interface\n");
            hda_initalize_codec(codec_number);
            return; // initalization is complete
        }
    }
}


uint32_t hda_send_verb(uint32_t codec, uint32_t node, uint32_t verb, uint32_t command) {
    uint32_t value = ((codec << 28) | (node << 20) | (verb << 8) | (command));

    if (g_hda.communication_type == HDA_CORB_RIRB) { // CORB/RIRB interface
        // write verb
        g_hda.corb_mem[g_hda.corb_pointer] = value;

        // move write pointer
        mmio_outw(g_hda.base + 0x48, g_hda.corb_pointer);

        // wait for response
        ticks = TIMER_TICKS;
        while (TIMER_TICKS - ticks < 5) {
            asm("nop");
            if (mmio_inw(g_hda.base + 0x58) ==
                        g_hda.corb_pointer) {
                break;
            }
        }

        if (mmio_inw(g_hda.base + 0x58) != g_hda.corb_pointer) {
            logf("HDA ERROR: no response\n");
            g_hda.communication_type = HDA_UNINITALIZED;
            return HDA_VERB_ERROR;
        }

        // read response
        value = g_hda.rirb_mem[g_hda.rirb_pointer * 2];

        // move pointers
        g_hda.corb_pointer++;
        if (g_hda.corb_pointer == g_hda.corb_number_of_entries)
            g_hda.corb_pointer = 0;

        g_hda.rirb_pointer++;
        if (g_hda.rirb_pointer == g_hda.rirb_number_of_entries)
            g_hda.rirb_pointer = 0;

        // return response
        return value;
    }

    else if (g_hda.communication_type == HDA_PIO) { // PIO interface
        // clear Immediate Result Valid bit
        mmio_outw(g_hda.base + 0x68, 0x2);

        // write verb
        mmio_outd(g_hda.base + 0x60, value);

        // start verb transfer
        mmio_outw(g_hda.base + 0x68, 0x1);

        // pool for response
        ticks = TIMER_TICKS;
        while (TIMER_TICKS - ticks < 6) {
            asm("nop");

            // wait for Immediate Result Valid bit = set and Immediate Command Busy bit = clear
            if ((mmio_inw(g_hda.base + 0x68) & 0x3)==0x2) {
                // clear Immediate Result Valid bit
                mmio_outw(g_hda.base + 0x68, 0x2);

                // return response
                return mmio_ind(g_hda.base + 0x64);
            }
        }

        // there was no response after 6 ms
        logf("HDA ERROR: no response\n");
        g_hda.communication_type = HDA_UNINITALIZED;
        return HDA_VERB_ERROR;
    }

    return HDA_VERB_ERROR;
}

uint8_t hda_get_node_type(uint32_t codec, uint32_t node) {
    return ((hda_send_verb(codec, node, 0xF00, 0x09) >> 20) & 0xF);
}

uint16_t hda_get_node_connection_entry(uint32_t codec, uint32_t node, uint32_t connection_entry_number) {
    // read connection capabilities
    uint32_t connection_list_capabilities = hda_send_verb(codec, node, 0xF00, 0x0E);

    // test if this connection even exist
    if (connection_entry_number >= (connection_list_capabilities & 0x7F)) {
        return 0x0000;
    }

    // return number of connected node
    if ((connection_list_capabilities & 0x80) == 0x00) // short form
        return ((hda_send_verb(codec, node, 0xF02, ((connection_entry_number / 4) * 4))
                >> ((connection_entry_number % 4) * 8)) & 0xFF);

    else // long form
        return ((hda_send_verb(codec, node, 0xF02, ((connection_entry_number / 2) * 2))
                >> ((connection_entry_number % 2) * 16)) & 0xFFFF);
}

void hda_set_node_gain(uint32_t codec, uint32_t node, uint32_t node_type, uint32_t capabilities, uint32_t gain) {
    // this will apply to left and right
    uint32_t payload = 0x3000;

    // set type of node
    if ((node_type & HDA_OUTPUT_NODE) == HDA_OUTPUT_NODE) {
        payload |= 0x8000;
    }

    if ((node_type & HDA_INPUT_NODE) == HDA_INPUT_NODE) {
        payload |= 0x4000;
    }

    // set number of gain
    if (gain == 0 && (capabilities & 0x80000000) == 0x80000000) {
        payload |= 0x80; // mute
    } else {
        payload |= (((capabilities >> 8) & 0x7F) * gain / 100); // recalculate range 0-100 to range of node steps
    }

    // change gain
    hda_send_verb(codec, node, 0x300, payload);
}

void hda_enable_pin_output(uint32_t codec, uint32_t pin_node) {
    hda_send_verb(codec, pin_node, 0x707, (hda_send_verb(codec, pin_node, 0xF07, 0x00) | 0x40));
}

void hda_disable_pin_output(uint32_t codec, uint32_t pin_node) {
    hda_send_verb(codec, pin_node, 0x707, (hda_send_verb(codec, pin_node, 0xF07, 0x00) & ~0x40));
}

uint8_t hda_is_headphone_connected(void) {
    return g_hda.pin_headphone_node_number != 0 && (hda_send_verb(g_hda.codec_number,
                g_hda.pin_headphone_node_number, 0xF09, 0x00) & 0x80000000) == 0x80000000;
}

void hda_initalize_codec(uint32_t codec_number) {
    // test if this codec exist
    uint32_t codec_id = hda_send_verb(codec_number, 0, 0xF00, 0);
    if (codec_id == 0x00000000) {
        return;
    }

    g_hda.codec_number = codec_number;

    //log basic codec info
    logf("Codec %d\n", codec_number);
    logf("Vendor: %x\n", (codec_id >> 16) /*(pci_get_vendor_name(codec_id>>16))*/); //PROFAN EDIT
    logf("Number: %x\n", (codec_id & 0xFFFF));

    //find Audio Function Groups
    uint32_t subordinate_node_count_reponse = hda_send_verb(codec_number, 0, 0xF00, 0x04);

    logf("First Group node: %d Number of Groups: %d\n", (subordinate_node_count_reponse >> 16) & 0xFF,
                subordinate_node_count_reponse & 0xFF);

    for (uint32_t node = ((subordinate_node_count_reponse >> 16) & 0xFF),
                last_node = (node + (subordinate_node_count_reponse & 0xFF)); node < last_node; node++) {
        if ((hda_send_verb(codec_number, node, 0xF00, 0x05) & 0x7F) == 0x01) { // this is Audio Function Group
            hda_initalize_audio_function_group(node); // initalize Audio Function Group
            return;
        }
    }

    logf("HDA ERROR: No AFG founded\n");
}

void hda_initalize_audio_function_group(uint32_t afg_node_number) {
    // reset AFG
    hda_send_verb(g_hda.codec_number, afg_node_number, 0x7FF, 0x00);

    // enable power for AFG
    hda_send_verb(g_hda.codec_number, afg_node_number, 0x705, 0x00);

    // disable unsolicited responses
    hda_send_verb(g_hda.codec_number, afg_node_number, 0x708, 0x00);

    // read available info
    g_hda.afg_node_sample_capabilities = hda_send_verb(g_hda.codec_number, afg_node_number, 0xF00, 0x0A);
    g_hda.afg_node_stream_format_capabilities = hda_send_verb(g_hda.codec_number, afg_node_number, 0xF00, 0x0B);
    g_hda.afg_node_input_amp_capabilities = hda_send_verb(g_hda.codec_number, afg_node_number, 0xF00, 0x0D);
    g_hda.afg_node_output_amp_capabilities = hda_send_verb(g_hda.codec_number, afg_node_number, 0xF00, 0x12);

    // log AFG info
    logf("Audio Function Group node %d\n", afg_node_number);
    logf("AFG sample capabilities:          %x\n", g_hda.afg_node_sample_capabilities);
    logf("AFG stream format capabilities:   %x\n", g_hda.afg_node_stream_format_capabilities);
    logf("AFG input amp capabilities:       %x\n", g_hda.afg_node_input_amp_capabilities);
    logf("AFG output amp capabilities:      %x\n", g_hda.afg_node_output_amp_capabilities);

    // log all AFG nodes and find useful PINs
    logf("\nLIST OF ALL NODES IN AFG:\n");
    uint32_t subordinate_node_count_reponse =
            hda_send_verb(g_hda.codec_number, afg_node_number, 0xF00, 0x04);

    uint32_t pin_alternative_output_node_number = 0, pin_speaker_default_node_number = 0;
    uint32_t pin_speaker_node_number = 0, pin_headphone_node_number = 0;

    g_hda.pin_output_node_number = 0;
    g_hda.pin_headphone_node_number = 0;

    for (uint32_t node = ((subordinate_node_count_reponse >> 16) & 0xFF), last_node =
                (node + (subordinate_node_count_reponse & 0xFF)), type_of_node = 0; node < last_node; node++) {
        // log number of node
        logf("%d: ", node);

        // get type of node
        type_of_node = hda_get_node_type(g_hda.codec_number, node);

        // process node
        if (type_of_node == HDA_WIDGET_AUDIO_OUTPUT) {
            logf("Audio Output");

            // disable every audio output by connecting it to stream 0
            hda_send_verb(g_hda.codec_number, node, 0x706, 0x00);

        } else if (type_of_node == HDA_WIDGET_AUDIO_INPUT) {
            logf("Audio Input");
        } else if (type_of_node == HDA_WIDGET_AUDIO_MIXER) {
            logf("Audio Mixer");
        } else if (type_of_node == HDA_WIDGET_AUDIO_SELECTOR) {
            logf("Audio Selector");
        } else if (type_of_node == HDA_WIDGET_PIN_COMPLEX) {
            logf("Pin Complex ");

            // read type of PIN
            type_of_node = (hda_send_verb(g_hda.codec_number, node, 0xF1C, 0x00) >> 20) & 0xF;
            if (type_of_node == HDA_PIN_LINE_OUT) {
                logf("Line Out");

                // save this node, this variable contain number of last alternative output
                pin_alternative_output_node_number = node;
            } else if (type_of_node == HDA_PIN_SPEAKER) {
                logf("Speaker ");

                // first speaker node is default speaker
                if (pin_speaker_default_node_number == 0) {
                    pin_speaker_default_node_number = node;
                }

                // find if there is device connected to this PIN
                if ((hda_send_verb(g_hda.codec_number, node, 0xF00, 0x09) & 0x4) == 0x4) {
                    // find if it is jack or fixed device
                    if ((hda_send_verb(g_hda.codec_number, node, 0xF1C, 0x00)>>30) != 0x1) {
                        // find if is device output capable
                        if ((hda_send_verb(g_hda.codec_number, node, 0xF00, 0x0C) & 0x10) == 0x10) {
                            // there is connected device
                            logf("connected output device");

                            // we will use first pin with connected device, so save node number only for first PIN
                            if (pin_speaker_node_number == 0) {
                                pin_speaker_node_number = node;
                            }
                        } else {
                            logf("not output capable");
                        }
                    } else {
                        logf("not jack or fixed device");
                    }
                } else {
                    logf("no output device");
                }
            } else if (type_of_node == HDA_PIN_HEADPHONE_OUT) {
                logf("Headphone Out");

                // save node number
                // TODO: handle if there are multiple HP nodes
                pin_headphone_node_number = node;
            } else if (type_of_node == HDA_PIN_CD) {
                logf("CD");
                pin_alternative_output_node_number = node;
            } else if (type_of_node == HDA_PIN_SPDIF_OUT) {
                logf("SPDIF Out");
                pin_alternative_output_node_number = node;
            } else if (type_of_node == HDA_PIN_DIGITAL_OTHER_OUT) {
                logf("Digital Other Out");
                pin_alternative_output_node_number = node;
            } else if (type_of_node == HDA_PIN_MODEM_LINE_SIDE) {
                logf("Modem Line Side");
                pin_alternative_output_node_number = node;
            } else if (type_of_node==HDA_PIN_MODEM_HANDSET_SIDE) {
                logf("Modem Handset Side");
                pin_alternative_output_node_number = node;
            } else if (type_of_node == HDA_PIN_LINE_IN) {
                logf("Line In");
            } else if (type_of_node == HDA_PIN_AUX) {
                logf("AUX");
            } else if (type_of_node == HDA_PIN_MIC_IN) {
                logf("Mic In");
            } else if (type_of_node == HDA_PIN_TELEPHONY) {
                logf("Telephony");
            } else if (type_of_node == HDA_PIN_SPDIF_IN) {
                logf("SPDIF In");
            } else if (type_of_node == HDA_PIN_DIGITAL_OTHER_IN) {
                logf("Digital Other In");
            } else if (type_of_node == HDA_PIN_RESERVED) {
                logf("Reserved");
            } else if (type_of_node == HDA_PIN_OTHER) {
                logf("Other");
            }
        } else if (type_of_node==HDA_WIDGET_POWER_WIDGET) {
            logf("Power Widget");
        } else if (type_of_node==HDA_WIDGET_VOLUME_KNOB) {
            logf("Volume Knob");
        } else if (type_of_node==HDA_WIDGET_BEEP_GENERATOR) {
            logf("Beep Generator");
        } else if (type_of_node==HDA_WIDGET_VENDOR_DEFINED) {
            logf("Vendor defined");
        } else {
            logf("Reserved type");
        }

        // log all connected nodes
        logf(" ");

        uint8_t connection_entry_number = 0;
        uint16_t connection_entry_node = hda_get_node_connection_entry(g_hda.codec_number, node, 0);

        logf("( connections: ");
        while (connection_entry_node!=0x0000) {
            logf("%d ", connection_entry_node);
            connection_entry_number++;
            connection_entry_node = hda_get_node_connection_entry(g_hda.codec_number, node, connection_entry_number);
        }
        logf(")\n");
    }

    // reset variables of second path
    g_hda.second_audio_output_node_number = 0;
    g_hda.second_audio_output_sample_capabilities = 0;
    g_hda.second_audio_output_stream_format_capabilities = 0;
    g_hda.second_output_amp_node_number = 0;
    g_hda.second_output_amp_node_capabilities = 0;

    // initalize output PINs
    logf("\n");

    if (pin_speaker_default_node_number != 0) {
        // initalize speaker
        g_hda.is_initalized_useful_output = 1;

        if (pin_speaker_node_number != 0) {
            logf("Speaker output\n");
            // initalize speaker with connected output device
            hda_initalize_output_pin(pin_speaker_node_number);
            g_hda.pin_output_node_number = pin_speaker_node_number;
        } else {
            logf("Default speaker output\n");
            hda_initalize_output_pin(pin_speaker_default_node_number);
            g_hda.pin_output_node_number = pin_speaker_default_node_number;
        }

        // save speaker path
        g_hda.second_audio_output_node_number = g_hda.audio_output_node_number;
        g_hda.second_audio_output_sample_capabilities = g_hda.audio_output_sample_capabilities;
        g_hda.second_audio_output_stream_format_capabilities = g_hda.audio_output_stream_format_capabilities;
        g_hda.second_output_amp_node_number = g_hda.output_amp_node_number;
        g_hda.second_output_amp_node_capabilities = g_hda.output_amp_node_capabilities;

        // if codec has also headphone output, initalize it
        if (pin_headphone_node_number != 0) {
            logf("\nHeadphone output\n");
            hda_initalize_output_pin(pin_headphone_node_number); // initalize headphone output
            g_hda.pin_headphone_node_number = pin_headphone_node_number; // save headphone node number

            // if first path and second path share nodes, left only info for first path
            if (g_hda.audio_output_node_number==g_hda.second_audio_output_node_number) {
                g_hda.second_audio_output_node_number = 0;
            }

            if (g_hda.output_amp_node_number==g_hda.second_output_amp_node_number) {
                g_hda.second_output_amp_node_number = 0;
            }

            // find headphone connection status
            if (hda_is_headphone_connected()) {
                hda_disable_pin_output(g_hda.codec_number, g_hda.pin_output_node_number);
                g_hda.selected_output_node = g_hda.pin_headphone_node_number;
            } else {
                g_hda.selected_output_node = g_hda.pin_output_node_number;
            }

            // add task for checking headphone connection
            // create_task(hda_check_headphone_connection_change, TASK_TYPE_USER_INPUT, 50); PROFAN EDIT
        }
    } else if (pin_headphone_node_number!=0) { // codec do not have speaker, but only headphone output
        logf("Headphone output\n");
        g_hda.is_initalized_useful_output = 1;
        hda_initalize_output_pin(pin_headphone_node_number);        // initalize headphone output
        g_hda.pin_output_node_number = pin_headphone_node_number;   // save headphone node number
    } else if (pin_alternative_output_node_number != 0) {           // codec have only alternative output
        logf("Alternative output\n");
        g_hda.is_initalized_useful_output = 0;
        hda_initalize_output_pin(pin_alternative_output_node_number); // initalize alternative output
        g_hda.pin_output_node_number = pin_alternative_output_node_number;
    } else {
        logf("Codec do not have any output PINs\n");
    }
}

void hda_initalize_output_pin(uint32_t pin_node_number) {
    logf("Initalizing PIN %d\n", pin_node_number);

    // reset variables of first path
    g_hda.audio_output_node_number = 0;
    g_hda.audio_output_sample_capabilities = 0;
    g_hda.audio_output_stream_format_capabilities = 0;
    g_hda.output_amp_node_number = 0;
    g_hda.output_amp_node_capabilities = 0;

    // turn on power for PIN
    hda_send_verb(g_hda.codec_number, pin_node_number, 0x705, 0x00);

    // disable unsolicited responses
    hda_send_verb(g_hda.codec_number, pin_node_number, 0x708, 0x00);

    // disable any processing
    hda_send_verb(g_hda.codec_number, pin_node_number, 0x703, 0x00);

    // enable PIN
    hda_send_verb(g_hda.codec_number, pin_node_number, 0x707,
                (hda_send_verb(g_hda.codec_number, pin_node_number, 0xF07, 0x00) | 0x80 | 0x40));

    // enable EAPD + L-R swap
    hda_send_verb(g_hda.codec_number, pin_node_number, 0x70C, 0x6);

    // set maximal volume for PIN
    uint32_t pin_output_amp_capabilities = hda_send_verb(g_hda.codec_number, pin_node_number, 0xF00, 0x12);
    hda_set_node_gain(g_hda.codec_number, pin_node_number, HDA_OUTPUT_NODE, pin_output_amp_capabilities, 100);

    if (pin_output_amp_capabilities != 0) {
        // we will control volume by PIN node
        g_hda.output_amp_node_number = pin_node_number;
        g_hda.output_amp_node_capabilities = pin_output_amp_capabilities;
    }

    // start enabling path of nodes
    g_hda.length_of_node_path = 0;
    hda_send_verb(g_hda.codec_number, pin_node_number, 0x701, 0x00); // select first node

    uint32_t first_connected_node_number =
            hda_get_node_connection_entry(g_hda.codec_number, pin_node_number, 0);
    uint32_t type_of_first_connected_node =
            hda_get_node_type(g_hda.codec_number, first_connected_node_number);

    if (type_of_first_connected_node == HDA_WIDGET_AUDIO_OUTPUT) {
        hda_initalize_audio_output(first_connected_node_number);
    } else if (type_of_first_connected_node == HDA_WIDGET_AUDIO_MIXER) {
        hda_initalize_audio_mixer(first_connected_node_number);
    } else if (type_of_first_connected_node == HDA_WIDGET_AUDIO_SELECTOR) {
        hda_initalize_audio_selector(first_connected_node_number);
    } else {
        logf("HDA ERROR: PIN have connection %d\n", first_connected_node_number);
    }
}

void hda_initalize_audio_output(uint32_t audio_output_node_number) {
    logf("Initalizing Audio Output %d\n", audio_output_node_number);
    g_hda.audio_output_node_number = audio_output_node_number;

    // turn on power for Audio Output
    hda_send_verb(g_hda.codec_number, audio_output_node_number, 0x705, 0x00);

    // disable unsolicited responses
    hda_send_verb(g_hda.codec_number, audio_output_node_number, 0x708, 0x00);

    // disable any processing
    hda_send_verb(g_hda.codec_number, audio_output_node_number, 0x703, 0x00);

    // connect Audio Output to stream 1 channel 0
    hda_send_verb(g_hda.codec_number, audio_output_node_number, 0x706, 0x10);

    // set maximal volume for Audio Output
    uint32_t audio_output_amp_capabilities = hda_send_verb(g_hda.codec_number,
                audio_output_node_number, 0xF00, 0x12);
    hda_set_node_gain(g_hda.codec_number,
                audio_output_node_number, HDA_OUTPUT_NODE, audio_output_amp_capabilities, 100);

    if (audio_output_amp_capabilities != 0) {
        // we will control volume by Audio Output node
        g_hda.output_amp_node_number = audio_output_node_number;
        g_hda.output_amp_node_capabilities = audio_output_amp_capabilities;
    }

    // read info, if something is not present, take it from AFG node
    uint32_t audio_output_sample_capabilities = hda_send_verb(g_hda.codec_number,
            audio_output_node_number, 0xF00, 0x0A);

    if (audio_output_sample_capabilities == 0) {
        g_hda.audio_output_sample_capabilities = g_hda.afg_node_sample_capabilities;
    } else {
        g_hda.audio_output_sample_capabilities = audio_output_sample_capabilities;
    }

    uint32_t audio_output_stream_format_capabilities = hda_send_verb(g_hda.codec_number,
            audio_output_node_number, 0xF00, 0x0B);

    if (audio_output_stream_format_capabilities == 0) {
        g_hda.audio_output_stream_format_capabilities = g_hda.afg_node_stream_format_capabilities;
    } else {
        g_hda.audio_output_stream_format_capabilities = audio_output_stream_format_capabilities;
    }

    // if nodes in path do not have output amp capabilities,
    // volume will be controlled by Audio Output node with capabilities taken from AFG node
    if (g_hda.output_amp_node_number == 0) {
        g_hda.output_amp_node_number = audio_output_node_number;
        g_hda.output_amp_node_capabilities = g_hda.afg_node_output_amp_capabilities;
    }

    // because we are at end of node path, log all gathered info
    logf("Sample Capabilites: %x\n", g_hda.audio_output_sample_capabilities);
    logf("Stream Format Capabilites: %x\n", g_hda.audio_output_stream_format_capabilities);
    logf("Volume node: %d\n", g_hda.output_amp_node_number);
    logf("Volume capabilities: %x\n", g_hda.output_amp_node_capabilities);
}

void hda_initalize_audio_mixer(uint32_t audio_mixer_node_number) {
 if (g_hda.length_of_node_path>=10) {
    logf("HDA ERROR: too long path\n");
    return;
 }
    logf("Initalizing Audio Mixer %d\n", audio_mixer_node_number);

    // turn on power for Audio Mixer
    hda_send_verb(g_hda.codec_number, audio_mixer_node_number, 0x705, 0x00);

    // disable unsolicited responses
    hda_send_verb(g_hda.codec_number, audio_mixer_node_number, 0x708, 0x00);

    // set maximal volume for Audio Mixer
    uint32_t audio_mixer_amp_capabilities = hda_send_verb(g_hda.codec_number,
                audio_mixer_node_number, 0xF00, 0x12);

    hda_set_node_gain(g_hda.codec_number,
                audio_mixer_node_number, HDA_OUTPUT_NODE, audio_mixer_amp_capabilities, 100);

    if (audio_mixer_amp_capabilities != 0) {
        // we will control volume by Audio Mixer node
        g_hda.output_amp_node_number = audio_mixer_node_number;
        g_hda.output_amp_node_capabilities = audio_mixer_amp_capabilities;
    }

    // continue in path
    g_hda.length_of_node_path++;
    uint32_t first_connected_node_number = hda_get_node_connection_entry(g_hda.codec_number,
                audio_mixer_node_number, 0);

    uint32_t type_of_first_connected_node = hda_get_node_type(g_hda.codec_number,
                first_connected_node_number);

    if (type_of_first_connected_node == HDA_WIDGET_AUDIO_OUTPUT) {
        hda_initalize_audio_output(first_connected_node_number);
    } else if (type_of_first_connected_node == HDA_WIDGET_AUDIO_MIXER) {
        hda_initalize_audio_mixer(first_connected_node_number);
    } else if (type_of_first_connected_node == HDA_WIDGET_AUDIO_SELECTOR) {
        hda_initalize_audio_selector(first_connected_node_number);
    } else {
        logf("HDA ERROR: Mixer have connection %d\n", first_connected_node_number);
    }
}

void hda_initalize_audio_selector(uint32_t audio_selector_node_number) {
    if (g_hda.length_of_node_path >= 10) {
        logf("HDA ERROR: too long path\n");
        return;
    }

    logf("Initalizing Audio Selector %d\n", audio_selector_node_number);

    // turn on power for Audio Selector
    hda_send_verb(g_hda.codec_number, audio_selector_node_number, 0x705, 0x00);

    // disable unsolicited responses
    hda_send_verb(g_hda.codec_number, audio_selector_node_number, 0x708, 0x00);

    // disable any processing
    hda_send_verb(g_hda.codec_number, audio_selector_node_number, 0x703, 0x00);

    // set maximal volume for Audio Selector
    uint32_t audio_selector_amp_capabilities = hda_send_verb(g_hda.codec_number,
                audio_selector_node_number, 0xF00, 0x12);

    hda_set_node_gain(g_hda.codec_number,
                audio_selector_node_number, HDA_OUTPUT_NODE, audio_selector_amp_capabilities, 100);

    if (audio_selector_amp_capabilities != 0) {
        // we will control volume by Audio Selector node
        g_hda.output_amp_node_number = audio_selector_node_number;
        g_hda.output_amp_node_capabilities = audio_selector_amp_capabilities;
    }

    // continue in path
    g_hda.length_of_node_path++;
    hda_send_verb(g_hda.codec_number, audio_selector_node_number, 0x701, 0x00); // select first node

    uint32_t first_connected_node_number = hda_get_node_connection_entry(g_hda.codec_number,
                audio_selector_node_number, 0);

    uint32_t type_of_first_connected_node = hda_get_node_type(g_hda.codec_number,
                first_connected_node_number);

    if (type_of_first_connected_node == HDA_WIDGET_AUDIO_OUTPUT) {
        hda_initalize_audio_output(first_connected_node_number);
    } else if (type_of_first_connected_node == HDA_WIDGET_AUDIO_MIXER) {
        hda_initalize_audio_mixer(first_connected_node_number);
    } else if (type_of_first_connected_node == HDA_WIDGET_AUDIO_SELECTOR) {
        hda_initalize_audio_selector(first_connected_node_number);
    } else {
        logf("HDA ERROR: Selector have connection %d\n", first_connected_node_number);
    }
}

void hda_set_volume(uint32_t volume) {
    hda_set_node_gain(g_hda.codec_number, g_hda.output_amp_node_number,
                HDA_OUTPUT_NODE, g_hda.output_amp_node_capabilities, volume);

    if (g_hda.second_output_amp_node_number == 0)
        return;

    hda_set_node_gain(g_hda.codec_number, g_hda.second_output_amp_node_number,
                HDA_OUTPUT_NODE, g_hda.second_output_amp_node_capabilities, volume);
}

void hda_check_headphone_connection_change(void) {
    int headphone_connected = hda_is_headphone_connected();

    // headphone was connected
    if (headphone_connected && g_hda.selected_output_node == g_hda.pin_output_node_number) {
        hda_disable_pin_output(g_hda.codec_number, g_hda.pin_output_node_number);
        g_hda.selected_output_node = g_hda.pin_headphone_node_number;
        // logf("\nHDA: Headphone connected, switched output to HP\n");
    }

    // headphone was disconnected
    else if (!headphone_connected && g_hda.selected_output_node == g_hda.pin_headphone_node_number) {
        hda_enable_pin_output(g_hda.codec_number, g_hda.pin_output_node_number);
        g_hda.selected_output_node = g_hda.pin_output_node_number;
        // logf("\nHDA: Headphone disconnected, switched output to speaker\n");
    }
}

uint8_t hda_is_supported_channel_size(uint8_t size) {
    uint8_t channel_sizes[5] = {8, 16, 20, 24, 32};
    uint32_t mask=0x00010000;

    // get bit of requested size in capabilities
    for (int i=0; i < 5; i++) {
        if (channel_sizes[i]==size)
            break;
        mask <<= 1;
    }

    return (g_hda.audio_output_sample_capabilities & mask) == mask; // return 1 if supported
}

uint8_t hda_is_supported_sample_rate(uint32_t sample_rate) {
    uint32_t sample_rates[11] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000};
    uint16_t mask=0x0000001;

    // get bit of requested sample rate in capabilities
    for (int i=0; i<11; i++) {
        if (sample_rates[i]==sample_rate) {
            break;
        }
        mask <<= 1;
    }

    return (g_hda.audio_output_sample_capabilities & mask) == mask; // return 1 if supported
}

uint16_t hda_return_sound_data_format(uint32_t sample_rate, uint32_t channels, uint32_t bits_per_sample) {
    uint16_t data_format = (channels - 1);

    // bits per sample
    if (bits_per_sample == 16) {
        data_format |= ((0b001) << 4);
    } else if (bits_per_sample == 20) {
        data_format |= ((0b010) << 4);
    } else if (bits_per_sample ==24) {
        data_format |= ((0b011) << 4);
    } else if (bits_per_sample == 32) {
        data_format |= ((0b100) << 4);
    }

    // sample rate
    switch (sample_rate) {
        case 8000:
            return (data_format | ((0b0000101)<<8));
        case 11025:
            return (data_format | ((0b1000011)<<8));
        case 16000:
            return (data_format | ((0b0000010)<<8));
        case 22050:
            return (data_format | ((0b1000001)<<8));
        case 32000:
            return (data_format | ((0b0001010)<<8));
        case 44100:
            return (data_format | ((0b1000000)<<8));
        case 48000:
            return (data_format | ((0b0000000)<<8));
        case 88200:
            return (data_format | ((0b1001000)<<8));
        case 96000:
            return (data_format | ((0b0001000)<<8));
        case 176400:
            return (data_format | ((0b1011000)<<8));
        case 192000:
            return (data_format | ((0b0011000)<<8));
        default:
            return data_format; // should not happen
    }

    return data_format; // should not happen
}

void hda_play_pcm_data(uint32_t sample_rate, void* pcm_data, uint32_t buffer_size, uint16_t lvi) {
    if ((g_hda.audio_output_stream_format_capabilities & 0x1)==0x0) {
        return; // this Audio Output do not support PCM sound data
    }

    // stop stream
    mmio_outb(g_hda.output_stream_base + 0x00, 0x00);
    ticks = TIMER_TICKS;
    while (TIMER_TICKS - ticks<2) {
        asm("nop");
        if ((mmio_inb(g_hda.output_stream_base + 0x00) & 0x2) == 0x0) {
            break;
        }
    }

    if ((mmio_inb(g_hda.output_stream_base + 0x00) & 0x2) == 0x2) {
        logf("\nHDA: can not stop stream");
        return;
    }

    // reset stream registers
    mmio_outb(g_hda.output_stream_base + 0x00, 0x01);

    ticks = TIMER_TICKS;
    while (TIMER_TICKS - ticks<10) {
        asm("nop");
        if ((mmio_inb(g_hda.output_stream_base + 0x00) & 0x1)==0x1) {
            break;
        }
    }

    if ((mmio_inb(g_hda.output_stream_base + 0x00) & 0x1)==0x0) {
        // logf("\nHDA: can not start resetting stream");
    }

    wait(5);
    mmio_outb(g_hda.output_stream_base + 0x00, 0x00);

    ticks = TIMER_TICKS;
    while (TIMER_TICKS - ticks<10) {
        asm("nop");
        if ((mmio_inb(g_hda.output_stream_base + 0x00) & 0x1)==0x0) {
            break;
        }
    }

    if ((mmio_inb(g_hda.output_stream_base + 0x00) & 0x1)==0x1) {
        logf("\nHDA: can not stop resetting stream");
        return;
    }

    wait(5);

    // clear error bits
    mmio_outb(g_hda.output_stream_base + 0x03, 0x1C);

    asm("wbinvd"); // flush processor cache to RAM to be sure sound card will read correct data

    // set buffer registers
    mmio_outd(g_hda.output_stream_base + 0x18, (uint32_t) pcm_data);
    mmio_outd(g_hda.output_stream_base + 0x08, buffer_size);
    mmio_outw(g_hda.output_stream_base + 0x0C, lvi);

    // set stream data format
    mmio_outw(g_hda.output_stream_base + 0x12, hda_return_sound_data_format(sample_rate, 2, 16));

    // set Audio Output node data format
    hda_send_verb(g_hda.codec_number, g_hda.audio_output_node_number, 0x200,
                hda_return_sound_data_format(sample_rate, 2, 16));

    if (g_hda.second_audio_output_node_number != 0)
        hda_send_verb(g_hda.codec_number, g_hda.second_audio_output_node_number, 0x200,
                    hda_return_sound_data_format(sample_rate, 2, 16));

    wait(10);

    // start streaming to stream 1
    mmio_outb(g_hda.output_stream_base + 0x02, 0x14);
    mmio_outb(g_hda.output_stream_base + 0x00, 0x02);
}


void hda_stop_sound() {
    mmio_outb(g_hda.output_stream_base + 0x00, 0x00);
}

uint32_t hda_get_actual_stream_position() {
    return mmio_ind(g_hda.output_stream_base + 0x04);
}
