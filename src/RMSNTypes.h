/*
   The MIT License (MIT)

   Copyright (C) 2014 John Donovan
   Copyright (C) 2017 Hong-She Liang <starofrainnight@gmail.com>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
 */

#ifndef __INCLUDED_FDCE12F8526A11E7AA6EA088B4D1658C
#define __INCLUDED_FDCE12F8526A11E7AA6EA088B4D1658C

#include <Arduino.h>

#define RMSN_PROTOCOL_ID 0x01

#define RMSN_FLAG_DUP                 0x80
#define RMSN_FLAG_QOS_0               0x00
#define RMSN_FLAG_QOS_1               0x20
#define RMSN_FLAG_QOS_2               0x40
#define RMSN_FLAG_QOS_M1              0x60
#define RMSN_FLAG_RETAIN              0x10
#define RMSN_FLAG_WILL                0x08
#define RMSN_FLAG_CLEAN               0x04
#define RMSN_FLAG_TOPIC_NAME          0x00
#define RMSN_FLAG_TOPIC_PREDEFINED_ID 0x01
#define RMSN_FLAG_TOPIC_SHORT_NAME    0x02

#define RMSN_QOS_MASK   (RMSN_FLAG_QOS_0 | RMSN_FLAG_QOS_1 | RMSN_FLAG_QOS_2 | \
                         RMSN_FLAG_QOS_M1)
#define RMSN_TOPIC_MASK (RMSN_FLAG_TOPIC_NAME | RMSN_FLAG_TOPIC_PREDEFINED_ID | \
                         RMSN_FLAG_TOPIC_SHORT_NAME)

// Recommended values for timers and counters. All timers are in seconds.
#define RMSN_T_ADV       960
#define RMSN_N_ADV       3
#define RMSN_T_SEARCH_GW 5
#define RMSN_T_GW_INFO   5
#define RMSN_T_WAIT      360
#define RMSN_T_RETRY     15
#define RMSN_N_RETRY     5

#define RMSN_INVALID_TOPIC_ID 0xFFFF

#define RMSN_STRUCT_PACKED __attribute__ ((__packed__))

enum RMSNReturnCode
{
  RMSNRC_ACCEPTED,
  RMSNRC_REJECTED_CONGESTION,
  RMSNRC_REJECTED_INVALID_TOPIC_ID,
  RMSNRC_REJECTED_NOT_SUPPORTED
};

enum RMSNMsgType
{
  RMSNMT_ADVERTISE,
  RMSNMT_SEARCHGW,
  RMSNMT_GWINFO,
  RMSNMT_CONNECT = 0x04,
  RMSNMT_CONNACK,
  RMSNMT_WILLTOPICREQ,
  RMSNMT_WILLTOPIC,
  RMSNMT_WILLMSGREQ,
  RMSNMT_WILLMSG,
  RMSNMT_REGISTER,
  RMSNMT_REGACK,
  RMSNMT_PUBLISH,
  RMSNMT_PUBACK,
  RMSNMT_PUBCOMP,
  RMSNMT_PUBREC,
  RMSNMT_PUBREL,
  RMSNMT_SUBSCRIBE = 0x12,
  RMSNMT_SUBACK,
  RMSNMT_UNSUBSCRIBE,
  RMSNMT_UNSUBACK,
  RMSNMT_PINGREQ,
  RMSNMT_PINGRESP,
  RMSNMT_DISCONNECT,
  RMSNMT_WILLTOPICUPD = 0x1a,
  RMSNMT_WILLTOPICRESP,
  RMSNMT_WILLMSGUPD,
  RMSNMT_WILLMSGRESP,

  /// Reserved value in mqtt, but we use as invalid value.
  RMSNMT_INVALID = 0xff,
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgHeader struct
 */
struct RMSNMsgHeader
{
  uint8_t length;
  uint8_t type; ///< RMSNMsgType
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgAdvertise struct
 */
struct RMSNMsgAdvertise : public RMSNMsgHeader
{
  uint8_t  gwId;
  uint16_t duration;
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgSearchGw struct
 */
struct RMSNMsgSearchGw : public RMSNMsgHeader
{
  uint8_t radius;
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgGwInfo struct
 */
struct RMSNMsgGwInfo : public RMSNMsgHeader
{
  uint8_t gwId;
  char    gwAdd[0];
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgConnect struct
 */
struct RMSNMsgConnect :  public RMSNMsgHeader
{
  uint8_t  flags;
  uint8_t  protocolId;
  uint16_t duration;
  char     clientId[0];
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgConnAck struct
 */
struct RMSNMsgConnAck :  public RMSNMsgHeader
{
  uint8_t returnCode; ///< RMSNReturnCode
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgWillTopic struct
 */
struct RMSNMsgWillTopic :  public RMSNMsgHeader
{
  uint8_t flags;
  char    willTopic[0];
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgWillMsg struct
 */
struct RMSNMsgWillMsg :  public RMSNMsgHeader
{
  char willmsg[0];
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgRegister struct
 */
struct RMSNMsgRegister :  public RMSNMsgHeader
{
  uint16_t topicId;
  uint16_t messageId;
  char     topicName[0];
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgRegAck struct
 */
struct RMSNMsgRegAck :  public RMSNMsgHeader
{
  uint16_t topicId;
  uint16_t messageId;
  uint8_t  returnCode; ///< RMSNReturnCode
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgPublish struct
 */
struct RMSNMsgPublish :  public RMSNMsgHeader
{
  uint8_t  flags;
  uint16_t topicId;
  uint16_t messageId;
  char     data[0];
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgPubAck struct
 */
struct RMSNMsgPubAck :  public RMSNMsgHeader
{
  uint16_t topicId;
  uint16_t messageId;
  uint8_t  returnCode; ///< RMSNReturnCode
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgPubQos2 struct
 */
struct RMSNMsgPubQos2 :  public RMSNMsgHeader
{
  uint16_t messageId;
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgSubscribe struct
 */
struct RMSNMsgSubscribe :  public RMSNMsgHeader
{
  uint8_t  flags;
  uint16_t messageId;

  union
  {
    char     topicName[0];
    uint16_t topicId;
  };
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgSubAck struct
 */
struct RMSNMsgSubAck :  public RMSNMsgHeader
{
  uint8_t  flags;
  uint16_t topicId;
  uint16_t messageId;
  uint8_t  returnCode; ///< RMSNReturnCode
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgUnsubscribe struct
 */
struct RMSNMsgUnsubscribe :  public RMSNMsgHeader
{
  uint8_t  flags;
  uint16_t messageId;

  union
  {
    char     topicName[0];
    uint16_t topicId;
  } RMSN_STRUCT_PACKED;
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgUnsubAck struct
 */
struct RMSNMsgUnsubAck :  public RMSNMsgHeader
{
  uint16_t messageId;
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgPingReq struct
 */
struct RMSNMsgPingReq :  public RMSNMsgHeader
{
  char clientId[0];
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgDisconnect struct
 */
struct RMSNMsgDisconnect :  public RMSNMsgHeader
{
  uint16_t duration;
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgWillTopicResp struct
 */
struct RMSNMsgWillTopicResp :  public RMSNMsgHeader
{
  uint8_t returnCode; ///< RMSNReturnCode
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNMsgWillMsgResp struct
 */
struct RMSNMsgWillMsgResp :  public RMSNMsgHeader
{
  uint8_t returnCode; ///< RMSNReturnCode
} RMSN_STRUCT_PACKED;

/**
 * @brief The RMSNTopic struct
 */
struct RMSNTopic
{
  const char *name;
  uint16_t    id;
};

#endif // __INCLUDED_FDCE12F8526A11E7AA6EA088B4D1658C
