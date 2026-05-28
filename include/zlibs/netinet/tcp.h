/*****************************************************************************\
|   === tcp.h : 2026 ===                                                      |
|                                                                             |
|    Implementation of the netinet/tcp.h header file from libC     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _NETINET_TCP_H
#define _NETINET_TCP_H

// User-settable options (used with setsockopt)

#define TCP_NODELAY               1  // Don't delay send to coalesce packets
#define TCP_MAXSEG                2  // Set maximum segment size
#define TCP_CORK                  3  // Control sending of partial frames
#define TCP_KEEPIDLE              4  // Start keeplives after this period
#define TCP_KEEPINTVL             5  // Interval between keepalives
#define TCP_KEEPCNT               6  // Number of keepalives before death
#define TCP_SYNCNT                7  // Number of SYN retransmits
#define TCP_LINGER2               8  // Life time of orphaned FIN-WAIT-2 state
#define TCP_DEFER_ACCEPT          9  // Wake up listener only when data arrive
#define TCP_WINDOW_CLAMP          10 // Bound advertised window
#define TCP_INFO                  11 // Information about this connection
#define TCP_QUICKACK              12 // Bock/reenable quick ACKs
#define TCP_CONGESTION            13 // Congestion control algorithm
#define TCP_MD5SIG                14 // TCP MD5 Signature (RFC2385)
#define TCP_COOKIE_TRANSACTIONS   15 // TCP Cookie Transactions
#define TCP_THIN_LINEAR_TIMEOUTS  16 // Use linear timeouts for thin streams
#define TCP_THIN_DUPACK           17 // Fast retrans. after 1 dupack
#define TCP_USER_TIMEOUT          18 // How long for loss retry before timeout
#define TCP_REPAIR                19 // TCP sock is under repair right now
#define TCP_REPAIR_QUEUE          20 // Set TCP queue to repair
#define TCP_QUEUE_SEQ             21 // Set sequence number of repaired queue
#define TCP_REPAIR_OPTIONS        22 // Repair TCP connection options
#define TCP_FASTOPEN              23 // Enable FastOpen on listeners
#define TCP_TIMESTAMP             24 // TCP time stamp
#define TCP_NOTSENT_LOWAT         25 // Limit number of unsent bytes in write queue
#define TCP_CC_INFO               26 // Get Congestion Control (optional) info
#define TCP_SAVE_SYN              27 // Record SYN headers for new connections
#define TCP_SAVED_SYN             28 // Get SYN headers recorded for connection
#define TCP_REPAIR_WINDOW         29 // Get/set window parameters
#define TCP_FASTOPEN_CONNECT      30 // Attempt FastOpen with connect
#define TCP_ULP                   31 // Attach a ULP to a TCP connection
#define TCP_MD5SIG_EXT            32 // TCP MD5 Signature with extensions
#define TCP_FASTOPEN_KEY          33 // Set the key for Fast Open (cookie)
#define TCP_FASTOPEN_NO_COOKIE    34 // Enable TFO without a TFO cookie
#define TCP_ZEROCOPY_RECEIVE      35
#define TCP_INQ                   36 // Notify bytes available to read as a cmsg on read
#define TCP_CM_INQ                TCP_INQ
#define TCP_TX_DELAY              37 // Delay outgoing packets by XX usec

#define TCP_REPAIR_ON             1
#define TCP_REPAIR_OFF            0
#define TCP_REPAIR_OFF_NO_WP     -1

#endif
