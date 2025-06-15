// #define BUFFER_SIZE 2000
// #define RX_DES_LEN 1024
// #define TX_DES_LEN 256
// #define OWN (1<<31)
// #define EOR (1<<30)
// #define FS  (1<<29)
// #define LS  (1<<28)
// #define LGSEN (1<<27)
// #define IPCS (1<<18)
// #define UDPCS (1<<17)
// #define TCPCS (1<<16)

// #define USE_SEMA 0

// #define outportw(port, val) rtems_outw(port,val)
// #define inportw(port)  rtems_inw(port)


// static inline unsigned int inportl(unsigned short port)
// {
// 	unsigned int ret = 0;
// 	__asm__ __volatile__(	"inl	%%dx,	%0	\n\t"
// 				"mfence			\n\t"
// 				:"=a"(ret)
// 				:"d"(port)
// 				:"memory");
// 	return ret;
// }

// static inline void outportl(unsigned short port,unsigned int val)
// {
// 	__asm__ __volatile__(	"outl	%0,	%%dx	\n\t"
// 				"mfence			\n\t"
// 				:
// 				:"a"(val),"d"(port)
// 				:"memory");
// }

// typedef struct
// {
//     uint32_t command;  /* command/status uint32_t */
//     uint32_t vlan;     /* currently unused */
//     uint32_t low_buf;  /* low 32-bits of physical buffer address */
//     uint32_t high_buf; /* high 32-bits of physical buffer address */
// }Descriptor;


// typedef struct
// {
//     uint32_t     base_io;
//     int          IRQ;
//     BYTE         mac_address[6];
//     int          Cur_tx;
//     int          Cur_rx;
//     Descriptor*  rx_desc;
//     Descriptor*  tx_desc;
//     uint8_t*     rx_buffer;
//     uint8_t*     tx_buffer;
//     uint16_t     status;
//     #if USE_SEMA
//     sys_sem_t sem_rx;
//     #else
//     rtems_id    rx_msgbox;
//     #endif
// }rtl8169_nic;


// uint32_t rtl8168_recv_packet(rtl8169_nic* nic, unsigned char* buff);
// void rtl8168_send_packet(rtl8169_nic* nic, unsigned char* buff, int len, uint32_t flags);
// void rtl8168_send_NPQ(rtl8169_nic* nic);
// void rtl8168_reset(rtl8169_nic* nic);

// extern rtl8169_nic rtl[2];


// extern void* aligned_malloc(size_t required_bytes, size_t alignment);
// extern void aligned_free(void *p2) ;


// rtl8169_nic rtl[2];




// void realse_sem(void* ignored);

// void setup_rx_descriptors(rtl8169_nic* nic)
// {
//     /* rx_buffer_len is the size (in bytes) that is reserved for incoming packets */
//     nic->rx_desc = (Descriptor *)aligned_malloc(sizeof(Descriptor)*RX_DES_LEN, 256);
//     nic->rx_buffer = malloc(RX_DES_LEN * BUFFER_SIZE);
//     memset(nic->rx_desc, 0, sizeof(Descriptor)*RX_DES_LEN);
//     memset(nic->rx_buffer, 0, RX_DES_LEN * BUFFER_SIZE);
//     int i;
//     for(i = 0; i < RX_DES_LEN; i++) /* num_of_rx_descriptors can be up to 1024 */
//     {
//         if(i == (RX_DES_LEN - 1)) /* Last descriptor? if so, set the EOR bit  | (0x7FF0000)*/
//             nic->rx_desc[i].command = (OWN | EOR | (BUFFER_SIZE | 0x3FFF) );
//         else
//             nic->rx_desc[i].command = (OWN | (BUFFER_SIZE | 0x3FFF) );

//         /** packet_buffer_address is the *physical* address for the buffer */
//         nic->rx_desc[i].low_buf = (uint32_t)(nic->rx_buffer + i*BUFFER_SIZE);
//         nic->rx_desc[i].high_buf = 0;
//         /* If you are programming for a 64-bit OS, put the high memory location in the 'high_buf' descriptor area */
//     }
// }

// void setup_tx_descriptors(rtl8169_nic* nic)
// {
//     nic->tx_desc = (Descriptor *)aligned_malloc(sizeof(Descriptor)*TX_DES_LEN, 256);
//     nic->tx_buffer = malloc(TX_DES_LEN * BUFFER_SIZE);
//     memset(nic->tx_desc, 0, sizeof(Descriptor)*TX_DES_LEN);
//     memset(nic->tx_buffer, 0, TX_DES_LEN * BUFFER_SIZE);
//     int i;
//     for(i = 0; i < TX_DES_LEN; i++) /* num_of_tx_descriptors can be up to 1024 */
//     {
//         if(i == (TX_DES_LEN - 1)) /* Last descriptor? if so, set the EOR bit */
//             nic->tx_desc[i].command = (EOR | (BUFFER_SIZE | 0x3FFF));
//         else
//             nic->tx_desc[i].command = ((BUFFER_SIZE | 0x3FFF));

//         /** packet_buffer_address is the *physical* address for the buffer */
//         nic->tx_desc[i].low_buf = (uint32_t)(nic->tx_buffer + i*BUFFER_SIZE);
//         nic->tx_desc[i].high_buf = 0;
//         /* If you are programming for a 64-bit OS, put the high memory location in the 'high_buf' descriptor area */
//     }
// }

// void rtl8168_reset(rtl8169_nic* nic)
// {
//     unsigned int i;
//     uint32_t ioaddr = nic->base_io;
//     rtems_status_code sc;
//     #if USE_SEMA
//     if(sys_sem_new(&nic->sem_rx, 0) == ERR_OK)
//         printk("sem created ok\n");
//     #else
//     //create msgbox

//     sc = rtems_message_queue_create(
//         rtems_build_name('R', 'M', 'S', 'G'),
//         512,
//         sizeof(void*),
//         RTEMS_DEFAULT_ATTRIBUTES,
//         &nic->rx_msgbox);
//     #endif

//     sc = rtems_interrupt_handler_install(
//       nic->IRQ,
//       "lwip rx",
// 	  RTEMS_INTERRUPT_SHARED,
// 	  realse_sem,
//       (void*)nic);

// 	if(sc != RTEMS_SUCCESSFUL)
// 	{
// 		LOGMSG("lwip install isr falied,%s\n", rtems_status_text(sc));
// 		return ;
// 	}

//     outportb(ioaddr + 0x37, 0x10); /* Send the Reset bit to the Command register */
//     while(inportb(ioaddr + 0x37) & 0x10){} /* Wait for the chip to finish resetting */

//     for (i = 0; i < 6; i++)
//         nic->mac_address[i] = inportb(ioaddr + i);

//     setup_tx_descriptors(nic);
//     setup_rx_descriptors(nic);
//     outportb(ioaddr + 0x50, 0xC0); /* Unlock config registers */

//     //init rx
//     outportl(ioaddr + 0x44, 0xE71F); /* RxConfig = RXFTH: unlimited, MXDMA: unlimited, AAP: set (promisc. mode set) 0x0000E70F */
//     outportw(ioaddr + 0xDA, 0x1FFF); /* Max rx packet size */
//     outportl(ioaddr + 0xE4, (unsigned long)&nic->rx_desc[0]); /* Tell the NIC where the first Rx descriptor is. NOTE: If writing a 64-bit address, split it into two 32-bit writes.*/
//     outportl(ioaddr + 0xE8, 0);

//     //init tx
//     outportb(ioaddr + 0x37, 0x04); /* Enable Tx in the Command register, required before setting TxConfig */
//     outportb(ioaddr + 0xEC, 0x3F); //no early transmit
//     outportl(ioaddr + 0x40, 0x03000700); /* TxConfig = IFG: normal, MXDMA: unlimited */
//     outportl(ioaddr + 0x20, (unsigned long)&nic->tx_desc[0]); /* Tell the NIC where the first Tx descriptor is. NOTE: If writing a 64-bit address, split it into two 32-bit writes.*/
//     outportl(ioaddr + 0x24, 0);
//     outportl(ioaddr + 0x28, 0);
//     outportl(ioaddr + 0x2C, 0);

//     outportb(ioaddr + 0x37, 0x0C); /* Enable Rx/Tx in the Command register */
//     outportb(ioaddr + 0x50, 0x00); /* Lock config registers */

//     /* enable rx interrupts */
//     outportw(ioaddr + 0x3C, 0xFFFF);

//     uint32_t hwrev = inportl(ioaddr + 0x40);
//     uint8_t rev = ((hwrev >> 25) & 0x3E) | ((hwrev >> 23) & 0x1);
//     printk("HWRev=0x%x, 0x%x\n", rev, hwrev);
// }

// void realse_sem(void* ignored)
// {
//     rtl8169_nic* nic = (rtl8169_nic*)ignored;
//     struct pbuf *p;
//     nic->status = inportw(nic->base_io + 0x3E);

//     if((nic->status & 1) == 1)
//     {
//         #if USE_SEMA
//         //outportw(nic->base_io + 0x3C, 0);
//         sys_sem_signal(&nic->sem_rx);
//         #else
//         while(p = low_level_input(nic))
//         {
//             rtems_status_code sc = rtems_message_queue_send(
//                 nic->rx_msgbox, ( void * )&p, sizeof(p) );
//             if(sc != RTEMS_SUCCESSFUL)
//             {
//                 printk("send rx msg failed:%s\n", rtems_status_text(sc));
//             }
//         }
//         #endif
//     }

//     outportw(nic->base_io + 0x3E, nic->status);

//     if(nic->status & 0x70)
//     {
//         printk("status=0x%x\n", nic->status);
//     }

// }



// uint32_t rtl8168_recv_packet(rtl8169_nic* nic, unsigned char* buff)
// {
//     int frames = 0;
//     int total_len = 0;
//     uint32_t status;

//     do
//     {
//         status = (nic->rx_desc[nic->Cur_rx].command & (OWN | FS | LS));
//         if((status & OWN) == OWN)
//         {
//             //printk("no packets\n");
//             break;
//         }

//         #if DEBUG_RTL
//             printk("RX cmd=0x%x\n", status);
//         #endif
//         if(status == (FS | LS))
//         {
//             int len = nic->rx_desc[nic->Cur_rx].command & 0x3FFF;
//             memcpy(buff, nic->rx_desc[nic->Cur_rx].low_buf, len);
//             nic->rx_desc[nic->Cur_rx].command |= OWN;
//             buff += len;
//             total_len += len;
//             nic->Cur_rx++;
//             nic->Cur_rx %= RX_DES_LEN;
//             frames++;
//             break;
//         }
//         else if(status == FS)
//         {
//             int len = nic->rx_desc[nic->Cur_rx].command & 0x3FFF;
//             memcpy(buff, nic->rx_desc[nic->Cur_rx].low_buf, len);
//             nic->rx_desc[nic->Cur_rx].command |= OWN;
//             buff += len;
//             total_len += len;
//             nic->Cur_rx++;
//             nic->Cur_rx %= RX_DES_LEN;
//             frames++;
//             //printk("FS:%d\n", len);
//          }
//         else if(status == LS)
//         {
//             int len = nic->rx_desc[nic->Cur_rx].command & 0x3FFF;
//             memcpy(buff, nic->rx_desc[nic->Cur_rx].low_buf, len);
//             nic->rx_desc[nic->Cur_rx].command |= OWN;
//             total_len += len;
//             nic->Cur_rx++;
//             nic->Cur_rx %= RX_DES_LEN;
//             frames++;
//             //printk("LS:%d\n", len);
//             break;
//         }
//         else if(status == 0)
//         {
//             int len = nic->rx_desc[nic->Cur_rx].command & 0x3FFF;
//             memcpy(buff, nic->rx_desc[nic->Cur_rx].low_buf, len);
//             nic->rx_desc[nic->Cur_rx].command |= OWN;
//             buff += len;
//             total_len += len;
//             nic->Cur_rx++;
//             nic->Cur_rx %= RX_DES_LEN;
//             frames++;
//             //printk("MID:%d\n", len);
//         }
//         else
//         {
//             nic->rx_desc[nic->Cur_rx].command |= OWN;
//             nic->Cur_rx++;
//             nic->Cur_rx %= RX_DES_LEN;
//             printk("RX unavailable:0x%x\n", status);
//         }


//     }while(1);

//     #if 0
//     if(frames>1)
//         printk("recv frames=%d,len=%d\n", frames++, total_len);
//     #endif

//     #if DEBUG_RTL
//     printk("RX finised:%d\n", nic->Cur_rx);
//     #endif
//     return total_len;

// }
//84:2a:fd:89:5a:c1
// void rtl8168_send_packet(rtl8169_nic* nic, unsigned char* buff, int len, uint32_t flags)
// {

//     if(nic->tx_desc[nic->Cur_tx].command & OWN)
//     {
//         #if 1
//         printk("TX unavaiable:%d\n", nic->Cur_tx);
//         #endif
//     }

//     memcpy(nic->tx_desc[nic->Cur_tx].low_buf, buff, len);
//     nic->tx_desc[nic->Cur_tx].command = (OWN | flags | IPCS | UDPCS | TCPCS | len );
//     if(nic->Cur_tx == TX_DES_LEN - 1)
//         nic->tx_desc[nic->Cur_tx].command |= EOR;
//     nic->Cur_tx++;
//     nic->Cur_tx %= TX_DES_LEN;
//     //outportb(nic->base_io + 0x38, 0x40);
//     #if DEBUG_RTL
//     printk("TX finised:%d\n", nic->Cur_tx);
//     #endif
// }

// void rtl8168_send_NPQ(rtl8169_nic* nic)
// {

//     //while(inportb(nic->base_io + 0x38) & 0x40){}

//     outportb(nic->base_io + 0x38, 0x40);

//     //while(inportb(nic->base_io + 0x38) & 0x40){}
// }