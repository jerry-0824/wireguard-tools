/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2021 WireGuard LLC. All Rights Reserved.
 */

#ifndef _WIREGUARD_NT_H
#define _WIREGUARD_NT_H

#include <ntdef.h>
#include <ws2def.h>
#include <ws2ipdef.h>
#include <inaddr.h>
#include <in6addr.h>

#ifndef ALIGN_DOWN_BY
#define ALIGN_DOWN_BY(size, align) \
    ((ULONG_PTR)(size) & ~((ULONG_PTR)(align) - 1))
#endif

#ifndef ALIGN_UP_BY
#define ALIGN_UP_BY(size, align) \
    (ALIGN_DOWN_BY(((ULONG_PTR)(size) + align - 1), align))
#endif

#define WG_KEY_LEN 32

typedef struct _WG_IOCTL_ALLOWED_IP
{
    ADDRESS_FAMILY AddressFamily;
    UCHAR Cidr;
    union
    {
        UCHAR Address[16];
        IN_ADDR Address4;
        IN6_ADDR Address6;
    } __attribute__((aligned(8)));
} WG_IOCTL_ALLOWED_IP;

#define WG_IOCTL_PEER_HAS_PUBLIC_KEY (1 << 0)
#define WG_IOCTL_PEER_HAS_PRESHARED_KEY (1 << 1)
#define WG_IOCTL_PEER_HAS_PERSISTENT_KEEPALIVE (1 << 2)
#define WG_IOCTL_PEER_HAS_ENDPOINT (1 << 3)
#define WG_IOCTL_PEER_REPLACE_ALLOWED_IPS (1 << 4)
#define WG_IOCTL_PEER_REMOVE (1 << 5)
#define WG_IOCTL_PEER_UPDATE (1 << 6)

typedef struct _WG_IOCTL_PEER
{
    /* Bitwise combination of WG_IOCTL_PEER__... constants. */
    ULONG Flags;

    /* Peer protocol version: 0 = latest protocol, 1 = this protocol. */
    ULONG ProtocolVersion;

    /* Public key calculated by wg pubkey from a private key, and usually transmitted out of band to the author of the
     * configuration file. */
    UCHAR PublicKey[WG_KEY_LEN];

    /* Preshared key generated by wg genpsk. Optional, and may be omitted. This option adds an additional layer of
     * symmetric-key cryptography to be mixed into the already existing public-key cryptography, for post-quantum
     * resistance. */
    UCHAR PresharedKey[WG_KEY_LEN];

    /* Seconds interval, between 1 and 65535 inclusive, of how often to send an authenticated empty packet to the peer
     * for the purpose of keeping a stateful firewall or NAT mapping valid persistently. For example, if the interface
     * very rarely sends traffic, but it might at anytime receive traffic from a peer, and it is behind NAT, the
     * interface might benefit from having a persistent keepalive interval of 25 seconds. If set to 0, this option is
     * disabled. By default or when unspecified, this option is 0. Most users will not need this.
     */
    USHORT PersistentKeepalive;

    /* Endpoint IP and port number. This endpoint will be updated automatically to the most recent source IP address and
     * port of correctly authenticated packets from the peer. */
    union
    {
        SOCKADDR Sa;
        SOCKADDR_IN Sa4;
        SOCKADDR_IN6 Sa6;
    } Endpoint;

    /* Number of bytes transmitted. */
    ULONG64 TxBytes;

    /* Number of bytes received. */
    ULONG64 RxBytes;

    /* Time of the last handshake with peer. Contains a 64-bit value representing the number of 100-nanosecond intervals
     * since January 1, 1601 (UTC). */
    ULONG64 LastHandshake;

    /* Number of WG_IOCTL_ALLOWED_IP structs following. IOCTL imposes 4GB limit on us. */
    ULONG AllowedIPsCount;
} WG_IOCTL_PEER;

#define SIZEOF_WG_IOCTL_PEER ALIGN_UP_BY(sizeof(WG_IOCTL_PEER), __alignof(WG_IOCTL_ALLOWED_IP))

#define WG_IOCTL_INTERFACE_HAS_PUBLIC_KEY (1 << 0)
#define WG_IOCTL_INTERFACE_HAS_PRIVATE_KEY (1 << 1)
#define WG_IOCTL_INTERFACE_HAS_LISTEN_PORT (1 << 2)
#define WG_IOCTL_INTERFACE_REPLACE_PEERS (1 << 3)

typedef struct _WG_IOCTL_INTERFACE
{
    /* Bitwise combination of WG_IOCTL_INTERFACE_... constants. */
    ULONG Flags;

    /* Port for listening. Optional; if 0, chosen randomly. */
    USHORT ListenPort;

    /* Private key generated by wg genkey. */
    UCHAR PrivateKey[WG_KEY_LEN];

    /* Public key generated by wg pubkey from a private key. */
    UCHAR PublicKey[WG_KEY_LEN];

    /* Number of WG_IOCTL_PEER structs following. Must be less than MAX_PEERS_PER_DEVICE. */
    ULONG PeersCount;
} WG_IOCTL_INTERFACE;

#define SIZEOF_WG_IOCTL_INTERFACE ALIGN_UP_BY(sizeof(WG_IOCTL_INTERFACE), __alignof(WG_IOCTL_PEER))

/* Get adapter properties.
 *
 * The lpOutBuffer and nOutBufferSize parameters of DeviceIoControl() must describe an user allocated buffer
 * and its size in bytes. The buffer will be filled with a WG_IOCTL_INTERFACE struct followed by zero or more
 * WG_IOCTL_PEER structs. Should all data not fit into the buffer, ERROR_MORE_DATA is returned with the required
 * size of the buffer.
 */
#define WG_IOCTL_GET CTL_CODE(51821U, 0xc71U, METHOD_NEITHER, FILE_READ_DATA)

/* Set adapter properties.
 *
 * The lpInBuffer and nInBufferSize parameters of DeviceIoControl() must describe a WG_IOCTL_INTERFACE struct followed
 * by PeersCount times WG_IOCTL_PEER struct.
 */
#define WG_IOCTL_SET CTL_CODE(51821U, 0xc70U, METHOD_NEITHER, FILE_WRITE_DATA)

/* Bring adapter up after being configured.
 */
#define WG_IOCTL_UP CTL_CODE(51821U, 0x9f4U, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)

/* Bring adapter down for reconfiguration.
 */
#define WG_IOCTL_DOWN CTL_CODE(51821U, 0x9f5U, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)

/* Force close all open handles to allow for updating.
 */
#define WG_IOCTL_FORCE_CLOSE_HANDLES CTL_CODE(51821U, 0xa71U, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)

#endif
