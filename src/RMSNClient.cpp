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

#include <Arduino.h>

#include "RMSNClient.h"
#include "RMSNUtils.h"

RMSNBasicClient::RMSNBasicClient(Stream *stream) :
  mResponseToWaitFor(RMSNMT_INVALID),
  mMessageId(0),
  mTopicCount(0),
  mGatewayId(0),
  mFlags(RMSN_FLAG_QOS_0),
  mKeepAliveInterval(30),
  mStream(stream)
{
  memset(mTopicTable, 0, sizeof(RMSNTopic) * RMSN_MAX_TOPICS);
  memset(mMessageBuffer, 0, RMSN_MAX_BUFFER_SIZE);
  memset(mResponseBuffer, 0, RMSN_MAX_BUFFER_SIZE);
}

RMSNBasicClient::~RMSNBasicClient()
{
}

void
RMSNBasicClient::setQos(uint8_t qos)
{
  mFlags |= (qos & RMSN_QOS_MASK);
}

uint8_t
RMSNBasicClient::qos()
{
  return mFlags & RMSN_QOS_MASK;
}

uint16_t
RMSNBasicClient::bswap(const uint16_t val)
{
  return (val << 8) | (val >> 8);
}

void
RMSNBasicClient::setTopic(const char *name, const uint16_t &id)
{
  auto topic = getTopicByName(name);

  if(topic)
  {
    const_cast<RMSNTopic *>(topic)->id = id;
  }
  else if(mTopicCount <= RMSN_MAX_TOPICS)
  {
    mTopicTable[mTopicCount].name = name;
    mTopicTable[mTopicCount].id   = id;
    ++mTopicCount;
  }
  else
  {
    // Reach the maximum topic support.
  }
}

const RMSNTopic *
RMSNBasicClient::getTopicByName(const char *name) const
{
  for(uint8_t i = 0; i < mTopicCount; ++i)
  {
    if(strcmp(mTopicTable[i].name, name) == 0)
    {
      return &mTopicTable[i];
    }
  }

  return NULL;
}

const RMSNTopic *
RMSNBasicClient::getTopicById(const uint16_t &id) const
{
  for(uint8_t i = 0; i < mTopicCount; ++i)
  {
    if(id == mTopicTable[i].id)
    {
      return &mTopicTable[i];
    }
  }

  return NULL;
}

void
RMSNBasicClient::parseStream()
{
  if(mStream->available() > 0)
  {
    uint8_t *response     = mResponseBuffer;
    uint8_t  packetLength = (uint8_t)mStream->read();

    *response++ = packetLength--;

    while(packetLength > 0)
    {
      while(mStream->available() > 0)
      {
        *response++ = (uint8_t)mStream->read();
        --packetLength;
      }
    }

    dispatch();
  }
}

void
RMSNBasicClient::dispatch()
{
  RMSNMsgHeader *responseMessage = (RMSNMsgHeader *)mResponseBuffer;
  bool           handled         = true;

  switch(responseMessage->type)
  {
  case RMSNMT_ADVERTISE:

    if(mResponseToWaitFor == RMSNMT_ADVERTISE)
    {
      advertiseHandler((RMSNMsgAdvertise *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case RMSNMT_GWINFO:
    gwInfoHandler((RMSNMsgGwInfo *)mResponseBuffer);
    break;

  case RMSNMT_CONNACK:

    if(mResponseToWaitFor == RMSNMT_CONNACK)
    {
      connAckHandler((RMSNMsgConnAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case RMSNMT_WILLTOPICREQ:
    willTopicReqHandler(responseMessage);
    break;

  case RMSNMT_WILLMSGREQ:
    willMsgReqHandler(responseMessage);
    break;

  case RMSNMT_REGISTER:
    registerHandler((RMSNMsgRegister *)mResponseBuffer);
    break;

  case RMSNMT_REGACK:

    if(mResponseToWaitFor == RMSNMT_REGACK)
    {
      regAckHandler((RMSNMsgRegAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case RMSNMT_PUBLISH:
    publishHandler((RMSNMsgPublish *)mResponseBuffer);
    break;

  case RMSNMT_PUBACK:

    if(mResponseToWaitFor == RMSNMT_PUBACK)
    {
      pubAckHandler((RMSNMsgPubAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case RMSNMT_SUBACK:

    if(mResponseToWaitFor == RMSNMT_SUBACK)
    {
      subAckHandler((RMSNMsgSubAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case RMSNMT_UNSUBACK:

    if(mResponseToWaitFor == RMSNMT_UNSUBACK)
    {
      unsubAckHandler((RMSNMsgUnsubAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case RMSNMT_PINGREQ:
    pingReqHandler((RMSNMsgPingReq *)mResponseBuffer);
    break;

  case RMSNMT_PINGRESP:

    if(mResponseToWaitFor == RMSNMT_PINGRESP)
    {
      pingRespHandler();
    }
    else
    {
      handled = false;
    }

    break;

  case RMSNMT_DISCONNECT:
    disconnectHandler((RMSNMsgDisconnect *)mResponseBuffer);
    break;

  case RMSNMT_WILLTOPICRESP:

    if(mResponseToWaitFor == RMSNMT_WILLTOPICRESP)
    {
      willTopicRespHandler((RMSNMsgWillTopicResp *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case RMSNMT_WILLMSGRESP:

    if(mResponseToWaitFor == RMSNMT_WILLMSGRESP)
    {
      willMsgRespHandler((RMSNMsgWillMsgResp *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  default:
    return;
  }

  if(handled)
  {
    mResponseToWaitFor = RMSNMT_INVALID;
  }
}

void
RMSNBasicClient::sendMessage()
{
  RMSNMsgHeader *hdr = reinterpret_cast<RMSNMsgHeader *>(mMessageBuffer);

  mStream->write(mMessageBuffer, hdr->length);
  mStream->flush();

  if(RMSNMT_INVALID == mResponseToWaitFor)
  {
    startResponseTimer();

    // Cheesy hack to ensure two messages don't run-on into one send.
//        delay(10);
  }
}

uint8_t
RMSNBasicClient::responseToWaitFor() const
{
  return mResponseToWaitFor;
}

String
RMSNBasicClient::clientId() const
{
  return mClientId;
}

void
RMSNBasicClient::setClientId(const String &clientId)
{
  mClientId = clientId;
}

uint16_t
RMSNBasicClient::keepAliveInterval() const
{
  return mKeepAliveInterval;
}

void
RMSNBasicClient::setKeepAliveInterval(const uint16_t &keepAliveInterval)
{
  mKeepAliveInterval = keepAliveInterval;
}

void
RMSNBasicClient::timeout()
{
  mResponseToWaitFor = RMSNMT_INVALID;
}

void
RMSNBasicClient::advertiseHandler(const RMSNMsgAdvertise *msg)
{
  mGatewayId = msg->gwId;
}

void
RMSNBasicClient::gwInfoHandler(const RMSNMsgGwInfo *msg)
{
}

void
RMSNBasicClient::connAckHandler(const RMSNMsgConnAck *msg)
{
}

void
RMSNBasicClient::willTopicReqHandler(const RMSNMsgHeader *msg)
{
}

void
RMSNBasicClient::willMsgReqHandler(const RMSNMsgHeader *msg)
{
}

void
RMSNBasicClient::regAckHandler(const RMSNMsgRegAck *msg)
{
  if((msg->returnCode == 0)
     && (mTopicCount > 0)
     && (mTopicCount < RMSN_MAX_TOPICS)
     && bswap(msg->messageId) == mMessageId)
  {
    const uint16_t topicId = bswap(msg->topicId);

    if(NULL == getTopicById(topicId))
    {
      // If we don't have that topic id, we reset the last topic id.
      setTopic(mTopicTable[mTopicCount - 1].name, topicId);
    }
  }
}

void
RMSNBasicClient::pubAckHandler(const RMSNMsgPubAck *msg)
{
}

#ifdef USE_QOS2
void
MQTTSN::pubrecHandler(const msg_pubqos2 *msg)
{
}

void
MQTTSN::pubrelHandler(const msg_pubqos2 *msg)
{
}

void
MQTTSN::pubcompHandler(const msg_pubqos2 *msg)
{
}

#endif

void
RMSNBasicClient::pingReqHandler(const RMSNMsgPingReq *msg)
{
  pingResp();
}

void
RMSNBasicClient::subAckHandler(const RMSNMsgSubAck *msg)
{
}

void
RMSNBasicClient::unsubAckHandler(const RMSNMsgUnsubAck *msg)
{
}

void
RMSNBasicClient::disconnectHandler(const RMSNMsgDisconnect *msg)
{
}

void
RMSNBasicClient::pingRespHandler()
{
}

void
RMSNBasicClient::publishHandler(const RMSNMsgPublish *msg)
{
  if(msg->flags & RMSN_FLAG_QOS_1)
  {
    RMSNReturnCode ret = RMSNRC_REJECTED_INVALID_TOPIC_ID;

    if(getTopicById(bswap(msg->topicId)))
    {
      ret = RMSNRC_ACCEPTED;
    }

    pubAck(msg->topicId, msg->messageId, ret);
  }
}

void
RMSNBasicClient::registerHandler(const RMSNMsgRegister *msg)
{
  RMSNReturnCode ret     = RMSNRC_REJECTED_INVALID_TOPIC_ID;
  uint16_t       topicId = bswap(msg->topicId);

  auto topic = getTopicByName(msg->topicName);

  if(topic)
  {
    setTopic(msg->topicName, topicId);

    ret = RMSNRC_ACCEPTED;
  }

  regAck(msg->topicId, msg->messageId, ret);
}

void
RMSNBasicClient::willTopicRespHandler(const RMSNMsgWillTopicResp *msg)
{
}

void
RMSNBasicClient::willMsgRespHandler(const RMSNMsgWillMsgResp *msg)
{
}

void
RMSNBasicClient::searchGw(const uint8_t radius)
{
  RMSNMsgSearchGw *msg = reinterpret_cast<RMSNMsgSearchGw *>(mMessageBuffer);

  msg->length = sizeof(RMSNMsgSearchGw);
  msg->type   = RMSNMT_SEARCHGW;
  msg->radius = radius;

  sendMessage();
  mResponseToWaitFor = fmsnGetRespondType(msg->type);
}

void
RMSNBasicClient::connect()
{
  RMSNMsgConnect *msg = reinterpret_cast<RMSNMsgConnect *>(mMessageBuffer);

  msg->length =
    static_cast<uint8_t>(sizeof(RMSNMsgConnect) + mClientId.length());
  msg->type       = RMSNMT_CONNECT;
  msg->flags      = mFlags;
  msg->protocolId = RMSN_PROTOCOL_ID;
  msg->duration   = bswap(mKeepAliveInterval);

  fmsnSafeCopyText(msg->clientId, mClientId.c_str(),
                   RMSN_GET_MAX_DATA_SIZE(RMSNMsgConnect));

  sendMessage();
  mResponseToWaitFor = fmsnGetRespondType(msg->type);
}

void
RMSNBasicClient::willTopic(const char *willTopic, const bool update)
{
  if(willTopic == NULL)
  {
    RMSNMsgHeader *msg = reinterpret_cast<RMSNMsgHeader *>(mMessageBuffer);

    msg->type   = update ? RMSNMT_WILLTOPICUPD : RMSNMT_WILLTOPIC;
    msg->length = sizeof(RMSNMsgHeader);
  }
  else
  {
    RMSNMsgWillTopic *msg =
      reinterpret_cast<RMSNMsgWillTopic *>(mMessageBuffer);

    msg->type  = update ? RMSNMT_WILLTOPICUPD : RMSNMT_WILLTOPIC;
    msg->flags = mFlags;
    fmsnSafeCopyText(msg->willTopic, willTopic,
                     RMSN_GET_MAX_DATA_SIZE(RMSNMsgWillTopic));
  }

  sendMessage();

//    if (qos() == RMSN_FLAG_QOS_1 || qos() == RMSN_FLAG_QOS_2) {
//        waitingForResponse = true;
//        responseToWaitFor = WILLMSGREQ;
//    }
}

void
RMSNBasicClient::willMsg(const void *willMsg, const uint8_t willMsgLen,
                         const bool update)
{
  RMSNMsgWillMsg *msg = reinterpret_cast<RMSNMsgWillMsg *>(mMessageBuffer);

  msg->length = sizeof(RMSNMsgWillMsg) + willMsgLen;
  msg->type   = update ? RMSNMT_WILLMSGUPD : RMSNMT_WILLMSG;
  memcpy(msg->willmsg, willMsg, willMsgLen);

  sendMessage();
}

void
RMSNBasicClient::disconnect(const uint16_t duration)
{
  RMSNMsgDisconnect *msg =
    reinterpret_cast<RMSNMsgDisconnect *>(mMessageBuffer);

  msg->length = sizeof(RMSNMsgHeader);
  msg->type   = RMSNMT_DISCONNECT;

  if(duration > 0)
  {
    msg->length  += sizeof(RMSNMsgDisconnect);
    msg->duration = bswap(duration);
  }

  sendMessage();
  mResponseToWaitFor = fmsnGetRespondType(msg->type);
}

void
RMSNBasicClient::startResponseTimer()
{
}

bool
RMSNBasicClient::registerTopic(const char *name)
{
  if((RMSNMT_INVALID == mResponseToWaitFor)
     && (mTopicCount < RMSN_MAX_TOPICS))
  {
    ++mMessageId;

    // Fill in the next table entry, but we only increment the counter to
    // the next topic when we get a REGACK from the broker. So don't issue
    // another REGISTER until we have resolved this one.
    setTopic(name, 0);

    RMSNMsgRegister *msg = reinterpret_cast<RMSNMsgRegister *>(mMessageBuffer);

    msg->length    = sizeof(RMSNMsgRegister) + strlen(name);
    msg->type      = RMSNMT_REGISTER;
    msg->topicId   = 0;
    msg->messageId = bswap(mMessageId);
    fmsnSafeCopyText(msg->topicName, name,
                     RMSN_GET_MAX_DATA_SIZE(RMSNMsgRegister));

    sendMessage();
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
    return true;
  }

  return false;
}

void
RMSNBasicClient::regAck(const uint16_t topicId, const uint16_t messageId,
                        const RMSNReturnCode returnCode)
{
  RMSNMsgRegAck *msg = reinterpret_cast<RMSNMsgRegAck *>(mMessageBuffer);

  msg->length     = sizeof(RMSNMsgRegAck);
  msg->type       = fmsnGetRespondType(RMSNMT_REGISTER);
  msg->topicId    = bswap(topicId);
  msg->messageId  = bswap(messageId);
  msg->returnCode = returnCode;

  sendMessage();
}

void
RMSNBasicClient::publish(const uint16_t topicId, const void *data,
                         const uint8_t dataLen)
{
  ++mMessageId;

  RMSNMsgPublish *msg = reinterpret_cast<RMSNMsgPublish *>(mMessageBuffer);

  msg->length    = sizeof(RMSNMsgPublish) + dataLen;
  msg->type      = RMSNMT_PUBLISH;
  msg->flags     = mFlags;
  msg->topicId   = bswap(topicId);
  msg->messageId = bswap(mMessageId);
  memcpy(msg->data, data, dataLen);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(RMSNMT_PUBLISH);
  }
}

#ifdef USE_QOS2
void
MQTTSN::pubrec()
{
  msg_pubqos2 *msg = reinterpret_cast<msg_pubqos2 *>(message_buffer);

  msg->length    = sizeof(msg_pubqos2);
  msg->type      = PUBREC;
  msg->messageId = bswap(_message_id);

  send_message();
}

void
MQTTSN::pubrel()
{
  msg_pubqos2 *msg = reinterpret_cast<msg_pubqos2 *>(message_buffer);

  msg->length    = sizeof(msg_pubqos2);
  msg->type      = PUBREL;
  msg->messageId = bswap(_message_id);

  send_message();
}

void
MQTTSN::pubcomp()
{
  msg_pubqos2 *msg = reinterpret_cast<msg_pubqos2 *>(message_buffer);

  msg->length    = sizeof(msg_pubqos2);
  msg->type      = PUBCOMP;
  msg->messageId = bswap(_message_id);

  send_message();
}

#endif

void
RMSNBasicClient::pubAck(const uint16_t topicId, const uint16_t messageId,
                        const RMSNReturnCode returnCode)
{
  RMSNMsgPubAck *msg = reinterpret_cast<RMSNMsgPubAck *>(mMessageBuffer);

  msg->length     = sizeof(RMSNMsgPubAck);
  msg->type       = fmsnGetRespondType(RMSNMT_PUBLISH);
  msg->topicId    = bswap(topicId);
  msg->messageId  = bswap(messageId);
  msg->returnCode = returnCode;

  sendMessage();
}

void
RMSNBasicClient::subscribeByName(const char *topicName)
{
  ++mMessageId;

  RMSNMsgSubscribe *msg = reinterpret_cast<RMSNMsgSubscribe *>(mMessageBuffer);

  // The -2 here is because we're unioning a 0-length member (topicName)
  // with a uint16_t in the msg_subscribe struct.
  msg->length    = sizeof(RMSNMsgSubscribe) + strlen(topicName) - 2;
  msg->type      = RMSNMT_SUBSCRIBE;
  msg->flags     = qos() | RMSN_FLAG_TOPIC_NAME;
  msg->messageId = bswap(mMessageId);
  fmsnSafeCopyText(msg->topicName, topicName, RMSN_GET_MAX_DATA_SIZE(
                     RMSNMsgSubscribe) - 2);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
  }
}

void
RMSNBasicClient::subscribeById(const uint16_t topicId)
{
  ++mMessageId;

  RMSNMsgSubscribe *msg = reinterpret_cast<RMSNMsgSubscribe *>(mMessageBuffer);

  msg->length    = sizeof(RMSNMsgSubscribe);
  msg->type      = RMSNMT_SUBSCRIBE;
  msg->flags     = qos() | RMSN_FLAG_TOPIC_PREDEFINED_ID;
  msg->messageId = bswap(mMessageId);
  msg->topicId   = bswap(topicId);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
  }
}

void
RMSNBasicClient::unsubscribeByName(const char *topicName)
{
  ++mMessageId;

  RMSNMsgUnsubscribe *msg =
    reinterpret_cast<RMSNMsgUnsubscribe *>(mMessageBuffer);

  // The -2 here is because we're unioning a 0-length member (topicName)
  // with a uint16_t in the msg_unsubscribe struct.
  msg->length    = sizeof(RMSNMsgUnsubscribe) + strlen(topicName) - 2;
  msg->type      = RMSNMT_UNSUBSCRIBE;
  msg->flags     = qos() | RMSN_FLAG_TOPIC_NAME;
  msg->messageId = bswap(mMessageId);
  fmsnSafeCopyText(msg->topicName, topicName,
                   RMSN_GET_MAX_DATA_SIZE(RMSNMsgUnsubscribe) - 2);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
  }
}

void
RMSNBasicClient::unsubscribeById(const uint16_t topicId)
{
  ++mMessageId;

  RMSNMsgUnsubscribe *msg =
    reinterpret_cast<RMSNMsgUnsubscribe *>(mMessageBuffer);

  msg->length    = sizeof(RMSNMsgUnsubscribe);
  msg->type      = RMSNMT_UNSUBSCRIBE;
  msg->flags     = qos() | RMSN_FLAG_TOPIC_PREDEFINED_ID;
  msg->messageId = bswap(mMessageId);
  msg->topicId   = bswap(topicId);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
  }
}

void
RMSNBasicClient::pingReq(const char *clientId)
{
  RMSNMsgPingReq *msg = reinterpret_cast<RMSNMsgPingReq *>(mMessageBuffer);

  msg->length = sizeof(RMSNMsgPingReq) + strlen(clientId);
  msg->type   = RMSNMT_PINGREQ;
  fmsnSafeCopyText(msg->clientId, clientId,
                   RMSN_GET_MAX_DATA_SIZE(RMSNMsgPingReq));

  sendMessage();

  mResponseToWaitFor = fmsnGetRespondType(msg->type);
}

void
RMSNBasicClient::pingResp()
{
  RMSNMsgHeader *msg = reinterpret_cast<RMSNMsgHeader *>(mMessageBuffer);

  msg->length = sizeof(RMSNMsgHeader);
  msg->type   = fmsnGetRespondType(RMSNMT_PINGREQ);

  sendMessage();
}

RMSNClient::RMSNClient(Stream *stream)
  : RMSNBasicClient(stream)
  , mResponseTimer(0)
  , mResponseRetries(0)
{
}

bool
RMSNClient::waitForResponse()
{
  if(responseToWaitFor() != RMSNMT_INVALID)
  {
    // TODO: Watch out for overflow.
    if((millis() - mResponseTimer) > (RMSN_T_RETRY * 1000L))
    {
      mResponseTimer = millis();

      if(mResponseRetries == 0)
      {
        timeout();
        disconnectHandler(NULL);

        // stop retry timer
      }
      else
      {
        sendMessage();
      }

      --mResponseRetries;
    }
  }

  return responseToWaitFor() != RMSNMT_INVALID;
}

void
RMSNClient::startResponseTimer()
{
  mResponseTimer   = millis();
  mResponseRetries = RMSN_N_RETRY;
}
