#include <drivers/network_utils.h>
#include <drivers/ethernet.h>
#include <drivers/pci.h>
#include <cpu/ports.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>
#include <ktype.h>
#include <cpu/ports.h>

void ethernet_handle_packet(ethernet_frame_t * packet, int len) {
    void * data = (void*) packet + sizeof(ethernet_frame_t);
    int data_len = len - sizeof(ethernet_frame_t);
    UNUSED(data);
    UNUSED(data_len);
    // ARP packet
    if(ntohs(packet->type) == ETHERNET_TYPE_ARP) {
        kprintf_serial("(ARP Packet)\n");
        //arp_handle_packet(data, data_len);
    }
    // IP packets(could be TCP, UDP or others)
    if(ntohs(packet->type) == ETHERNET_TYPE_IP) {
        kprintf_serial("(IP Packet)\n");
        //ip_handle_packet(data, data_len);
    }
}

int ethernet_init() {
    return 0;
}