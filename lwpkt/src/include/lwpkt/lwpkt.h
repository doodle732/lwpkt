/**
 * \file            lwpkt.h
 * \brief           Lightweight packet protocol
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwPKT - Lightweight packet protocol library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v1.0.0
 */
#ifndef LWPKT_HDR_H
#define LWPKT_HDR_H

#include <string.h>
#include <stdint.h>
#include "lwrb/lwrb.h"
#include "lwpkt/lwpkt_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWPKT Lightweight packet protocol
 * \brief           Lightweight packet protocol
 * \{
 */

/**
 * \brief           Packet state enumeration
 */
typedef enum {
    LWPKT_STATE_START = 0x00,                   /*!< Packet waits for start byte */
#if LWPKT_CFG_USE_ADDR || __DOXYGEN__
    LWPKT_STATE_FROM,                           /*!< Packet waits for "packet from" byte */
    LWPKT_STATE_TO,                             /*!< Packet waits for "packet to" byte */
#endif /* LWPKT_CFG_USE_ADDR || __DOXYGEN__ */
#if LWPKT_CFG_USE_CMD || __DOXYGEN__
    LWPKT_STATE_CMD,                            /*!< Packet waits for "packet cmd" byte */
#endif /* LWPKT_CFG_USE_CMD || __DOXYGEN__ */
    LWPKT_STATE_LEN,                            /*!< Packet waits for (multiple) data length bytes */
    LWPKT_STATE_DATA,                           /*!< Packet waits for actual data bytes */
#if LWPKT_CFG_USE_CRC || __DOXYGEN__
    LWPKT_STATE_CRC,                            /*!< Packet waits for CRC data */
#endif /* LWPKT_CFG_USE_CRC || __DOXYGEN__ */
    LWPKT_STATE_STOP,                           /*!< Packet waits for stop byte */
} lwpkt_state_t;

/**
 * \brief           Packet result enumeration
 */
typedef enum {
    lwpktOK = 0x00,                             /*!< Function returns successfully */
    lwpktERR,                                   /*!< General error for function status */
    lwpktINPROG,                                /*!< Receive is in progress */
    lwpktVALID,                                 /*!< packet valid and ready to be read as CRC is valid and STOP received */
    lwpktERRCRC,                                /*!< CRC integrity error for the packet. Will not wait STOP byte if received */
    lwpktERRSTOP,                               /*!< Packet error with STOP byte, wrong character received for STOP */
    lwpktWAITDATA,                              /*!< Packet state is in start mode, waiting start byte to start receiving */
} lwpktr_t;

/**
 * \brief           CRC structure for packet
 */
typedef struct {
    uint8_t crc;
} lwpkt_crc_t;

/**
 * \brief           Packet structure
 */
typedef struct {
#if LWPKT_CFG_USE_ADDR || __DOXYGEN__
    uint8_t addr;                               /*!< Current device address */
#endif /* LWPKT_CFG_USE_ADDR || __DOXYGEN__ */
    uint8_t data[LWPKT_CFG_MAX_DATA_LEN];       /*!< Memory to write received data */
    LWRB_VOLATILE lwrb_t* tx_rb;                /*!< TX ringbuffer */
    LWRB_VOLATILE lwrb_t* rx_rb;                /*!< RX ringbuffer */
    uint32_t last_rx_time;                      /*!< Last RX time in units of milliseconds */

    struct {
        lwpkt_state_t state;                    /*!< Actual packet state machine */
#if LWPKT_CFG_USE_CRC || __DOXYGEN__
        lwpkt_crc_t crc;                        /*!< Packet CRC byte */
#endif /* LWPKT_CFG_USE_CRC || __DOXYGEN__ */
#if LWPKT_CFG_USE_ADDR || __DOXYGEN__
        uint8_t from;                           /*!< Device address packet is coming from */
        uint8_t to;                             /*!< Device address packet is intended for */
#endif /* LWPKT_CFG_USE_ADDR || __DOXYGEN__ */
#if LWPKT_CFG_USE_CMD || __DOXYGEN__
        uint8_t cmd;                            /*!< Command packet */
#endif /* LWPKT_CFG_USE_CMD || __DOXYGEN__ */
        size_t len;                             /*!< Number of bytes to receive */
        size_t index;                           /*!< General index variable for multi-byte parts of packet */
    } m;                                        /*!< Module that is periodically reset for next packet */
} lwpkt_t;

/**
 * \brief           List of event types
 */
typedef enum {
    LWPKT_EVT_PKT,                              /*!< Valid packet ready to read */
    LWPKT_EVT_TIMEOUT                           /*!< Timeout on packat, reset event */
} lwpkt_evt_type_t;

/**
 * \brief           LwPKT event function
 * \param[in]       pkt: LwPKT instance with valid packet
 * \param[in]       type: Event type
 */
typedef void(*lwpkt_evt_fn)(lwpkt_t* pkt, lwpkt_evt_type_t type);

lwpktr_t    lwpkt_init(lwpkt_t* pkt, LWRB_VOLATILE lwrb_t* tx_rb, LWRB_VOLATILE lwrb_t* rx_rb);
lwpktr_t    lwpkt_set_addr(lwpkt_t* pkt, uint8_t addr);
lwpktr_t    lwpkt_read(lwpkt_t* pkt);
lwpktr_t    lwpkt_write(lwpkt_t* pkt,
#if LWPKT_CFG_USE_ADDR || __DOXYGEN__
    uint8_t to,
#endif /* LWPKT_CFG_USE_ADDR || __DOXYGEN__ */
#if LWPKT_CFG_USE_CMD || __DOXYGEN__
    uint8_t cmd,
#endif /* LWPKT_CFG_USE_CMD || __DOXYGEN__ */
    const void* data, size_t len);
lwpktr_t    lwpkt_reset(lwpkt_t* pkt);
lwpktr_t    lwpkt_process(lwpkt_t* pkt, uint32_t time, lwpkt_evt_fn evt_fn);

#define lwpkt_get_from_addr(pkt)            (uint8_t)   (((pkt) != NULL) ? ((pkt)->m.from) : 0)
#define lwpkt_get_to_addr(pkt)              (uint8_t)   (((pkt) != NULL) ? ((pkt)->m.to) : 0)
#define lwpkt_get_data_len(pkt)             (size_t)    (((pkt) != NULL) ? ((pkt)->m.len) : 0)
#define lwpkt_get_data(pkt)                 (void *)    (((pkt) != NULL) ? ((pkt)->data) : NULL)
#define lwpkt_get_cmd(pkt)                  (uint8_t)   (((pkt) != NULL) ? ((pkt)->m.cmd) : 0)
#define lwpkt_is_for_me(pkt)                (((pkt) != NULL) ? ((pkt)->m.to == (pkt)->addr) : 0)
#define lwpkt_is_broadcast(pkt)             (((pkt) != NULL) ? ((pkt)->m.to == LWPKT_CFG_ADDR_BROADCAST) : 0)

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWPKT_HDR_H */
