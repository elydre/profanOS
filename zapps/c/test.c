#include <modules/eth.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    uint32_t id = modeth_start();
    if (id == 0) {
        printf("Failed to initialize Ethernet module\n");
        return 1;
    }

    eth_info_t info;
    modeth_get_info(id, &info);

    uint8_t packet[60] = {0};

    packet[0] = 0xFF; // Destination MAC (broadcast)
    packet[1] = 0xFF;
    packet[2] = 0xFF;
    packet[3] = 0xFF;
    packet[4] = 0xFF;
    packet[5] = 0xFF;

    // Source MAC
    packet[6] = info.mac[0];
    packet[7] = info.mac[1];
    packet[8] = info.mac[2];
    packet[9] = info.mac[3];
    packet[10] = info.mac[4];
    packet[11] = info.mac[5];

    // EtherType (IPv4)
    packet[12] = 0x08;
    packet[13] = 0x00;

    // Payload (46 bytes minimum for Ethernet)
    memcpy(&packet[14], "Hello, Ethernet!", 17);

    for (int i = 0; i < 1000; i++) {
        if (modeth_send(packet, 60) < 0) {
            printf("Failed to send packet\n");
            modeth_end(id);
            return 1;
        }
    }

    printf("Packets sent successfully\n");

    modeth_end(id);

    return 0;
}
