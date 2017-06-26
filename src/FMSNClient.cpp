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

#include "FMSNClient.h"
#include "FMSNUtils.h"

FMSNBasicClient::FMSNBasicClient(Stream *stream) :
  mResponseToWaitFor(FMSNMT_INVALID),
  mMessageId(0),
  mTopicCount(0),
  mGatewayId(0),
  mFlags(FMSN_FLAG_QOS_0),
  mKeepAliveInterval(30),
  mStream(stream)
{
  memset(mTopicTable, 0, sizeof(FMSNTopic) * FMSN_MAX_TOPICS);
  memset(mMessageBuffer, 0, FMSN_MAX_BUFFER_SIZE);
  memset(mResponseBuffer, 0, FMSN_MAX_BUFFER_SIZE);
}

FMSNBasicClient::~FMSNBasicClient()
{
}

void
FMSNBasicClient::setQos(uint8_t qos)
{
  mFlags |= (qos & FMSN_QOS_MASK);
}

uint8_t
FMSNBasicClient::qos()
{
  return mFlags & FMSN_QOS_MASK;
}

uint16_t
FMSNBasicClient::bswap(const uint16_t val)
{
  return (val << 8) | (val >> 8);
}

void
FMSNBasicClient::setTopic(const char *name, const uint16_t &id)
{
  auto topic = getTopicByName(name);

  if(topic)
  {
    const_cast<FMSNTopic *>(topic)->id = id;
  }
  else if(mTopicCount <= FMSN_MAX_TOPICS)
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

const FMSNTopic *
FMSNBasicClient::getTopicByName(const char *name) const
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

const FMSNTopic *
FMSNBasicClient::getTopicById(const uint16_t &id) const
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
FMSNBasicClient::parseStream()
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
FMSNBasicClient::dispatch()
{
  FMSNMsgHeader *responseMessage = (FMSNMsgHeader *)mResponseBuffer;
  bool           handled         = true;

  switch(responseMessage->type)
  {
  case FMSNMT_ADVERTISE:

    if(mResponseToWaitFor == FMSNMT_ADVERTISE)
    {
      advertiseHandler((FMSNMsgAdvertise *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case FMSNMT_GWINFO:
    gwInfoHandler((FMSNMsgGwInfo *)mResponseBuffer);
    break;

  case FMSNMT_CONNACK:

    if(mResponseToWaitFor == FMSNMT_CONNACK)
    {
      connAckHandler((FMSNMsgConnAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case FMSNMT_WILLTOPICREQ:
    willTopicReqHandler(responseMessage);
    break;

  case FMSNMT_WILLMSGREQ:
    willMsgReqHandler(responseMessage);
    break;

  case FMSNMT_REGISTER:
    registerHandler((FMSNMsgRegister *)mResponseBuffer);
    break;

  case FMSNMT_REGACK:

    if(mResponseToWaitFor == FMSNMT_REGACK)
    {
      regAckHandler((FMSNMsgRegAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case FMSNMT_PUBLISH:
    publishHandler((FMSNMsgPublish *)mResponseBuffer);
    break;

  case FMSNMT_PUBACK:

    if(mResponseToWaitFor == FMSNMT_PUBACK)
    {
      pubAckHandler((FMSNMsgPubAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case FMSNMT_SUBACK:

    if(mResponseToWaitFor == FMSNMT_SUBACK)
    {
      subAckHandler((FMSNMsgSubAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case FMSNMT_UNSUBACK:

    if(mResponseToWaitFor == FMSNMT_UNSUBACK)
    {
      unsubAckHandler((FMSNMsgUnsubAck *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case FMSNMT_PINGREQ:
    pingReqHandler((FMSNMsgPingReq *)mResponseBuffer);
    break;

  case FMSNMT_PINGRESP:

    if(mResponseToWaitFor == FMSNMT_PINGRESP)
    {
      pingRespHandler();
    }
    else
    {
      handled = false;
    }

    break;

  case FMSNMT_DISCONNECT:
    disconnectHandler((FMSNMsgDisconnect *)mResponseBuffer);
    break;

  case FMSNMT_WILLTOPICRESP:

    if(mResponseToWaitFor == FMSNMT_WILLTOPICRESP)
    {
      willTopicRespHandler((FMSNMsgWillTopicResp *)mResponseBuffer);
    }
    else
    {
      handled = false;
    }

    break;

  case FMSNMT_WILLMSGRESP:

    if(mResponseToWaitFor == FMSNMT_WILLMSGRESP)
    {
      willMsgRespHandler((FMSNMsgWillMsgResp *)mResponseBuffer);
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
    mResponseToWaitFor = FMSNMT_INVALID;
  }
}

void
FMSNBasicClient::sendMessage()
{
  FMSNMsgHeader *hdr = reinterpret_cast<FMSNMsgHeader *>(mMessageBuffer);

  mStream->write(mMessageBuffer, hdr->length);
  mStream->flush();

  if(FMSNMT_INVALID == mResponseToWaitFor)
  {
    startResponseTimer();

    // Cheesy hack to ensure two messages don't run-on into one send.
//        delay(10);
  }
}

uint8_t
FMSNBasicClient::responseToWaitFor() const
{
  return mResponseToWaitFor;
}

String
FMSNBasicClient::clientId() const
{
  return mClientId;
}

void
FMSNBasicClient::setClientId(const String &clientId)
{
  mClientId = clientId;
}

uint16_t
FMSNBasicClient::keepAliveInterval() const
{
  return mKeepAliveInterval;
}

void
FMSNBasicClient::setKeepAliveInterval(const uint16_t &keepAliveInterval)
{
  mKeepAliveInterval = keepAliveInterval;
}

void
FMSNBasicClient::timeout()
{
  mResponseToWaitFor = FMSNMT_INVALID;
}

void
FMSNBasicClient::advertiseHandler(const FMSNMsgAdvertise *msg)
{
  mGatewayId = msg->gwId;
}

void
FMSNBasicClient::gwInfoHandler(const FMSNMsgGwInfo *msg)
{
}

void
FMSNBasicClient::connAckHandler(const FMSNMsgConnAck *msg)
{
}

void
FMSNBasicClient::willTopicReqHandler(const FMSNMsgHeader *msg)
{
}

void
FMSNBasicClient::willMsgReqHandler(const FMSNMsgHeader *msg)
{
}

void
FMSNBasicClient::regAckHandler(const FMSNMsgRegAck *msg)
{
  if((msg->returnCode == 0)
     && (mTopicCount > 0)
     && (mTopicCount < FMSN_MAX_TOPICS)
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
FMSNBasicClient::pubAckHandler(const FMSNMsgPubAck *msg)
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
FMSNBasicClient::pingReqHandler(const FMSNMsgPingReq *msg)
{
  pingResp();
}

void
FMSNBasicClient::subAckHandler(const FMSNMsgSubAck *msg)
{
}

void
FMSNBasicClient::unsubAckHandler(const FMSNMsgUnsubAck *msg)
{
}

void
FMSNBasicClient::disconnectHandler(const FMSNMsgDisconnect *msg)
{
}

void
FMSNBasicClient::pingRespHandler()
{
}

void
FMSNBasicClient::publishHandler(const FMSNMsgPublish *msg)
{
  if(msg->flags & FMSN_FLAG_QOS_1)
  {
    FMSNReturnCode ret = FMSNRC_REJECTED_INVALID_TOPIC_ID;

    if(getTopicById(bswap(msg->topicId)))
    {
      ret = FMSNRC_ACCEPTED;
    }

    pubAck(msg->topicId, msg->messageId, ret);
  }
}

void
FMSNBasicClient::registerHandler(const FMSNMsgRegister *msg)
{
  FMSNReturnCode ret     = FMSNRC_REJECTED_INVALID_TOPIC_ID;
  uint16_t       topicId = bswap(msg->topicId);

  auto topic = getTopicByName(msg->topicName);

  if(topic)
  {
    setTopic(msg->topicName, topicId);

    ret = FMSNRC_ACCEPTED;
  }

  regAck(msg->topicId, msg->messageId, ret);
}

void
FMSNBasicClient::willTopicRespHandler(const FMSNMsgWillTopicResp *msg)
{
}

void
FMSNBasicClient::willMsgRespHandler(const FMSNMsgWillMsgResp *msg)
{
}

void
FMSNBasicClient::searchGw(const uint8_t radius)
{
  FMSNMsgSearchGw *msg = reinterpret_cast<FMSNMsgSearchGw *>(mMessageBuffer);

  msg->length = sizeof(FMSNMsgSearchGw);
  msg->type   = FMSNMT_SEARCHGW;
  msg->radius = radius;

  sendMessage();
  mResponseToWaitFor = fmsnGetRespondType(msg->type);
}

void
FMSNBasicClient::connect()
{
  FMSNMsgConnect *msg = reinterpret_cast<FMSNMsgConnect *>(mMessageBuffer);

  msg->length =
    static_cast<uint8_t>(sizeof(FMSNMsgConnect) + mClientId.length());
  msg->type       = FMSNMT_CONNECT;
  msg->flags      = mFlags;
  msg->protocolId = FMSN_PROTOCOL_ID;
  msg->duration   = bswap(mKeepAliveInterval);

  fmsnSafeCopyText(msg->clientId, mClientId.c_str(),
                   FMSN_GET_MAX_DATA_SIZE(FMSNMsgConnect));

  sendMessage();
  mResponseToWaitFor = fmsnGetRespondType(msg->type);
}

void
FMSNBasicClient::willTopic(const char *willTopic, const bool update)
{
  if(willTopic == NULL)
  {
    FMSNMsgHeader *msg = reinterpret_cast<FMSNMsgHeader *>(mMessageBuffer);

    msg->type   = update ? FMSNMT_WILLTOPICUPD : FMSNMT_WILLTOPIC;
    msg->length = sizeof(FMSNMsgHeader);
  }
  else
  {
    FMSNMsgWillTopic *msg =
      reinterpret_cast<FMSNMsgWillTopic *>(mMessageBuffer);

    msg->type  = update ? FMSNMT_WILLTOPICUPD : FMSNMT_WILLTOPIC;
    msg->flags = mFlags;
    fmsnSafeCopyText(msg->willTopic, willTopic,
                     FMSN_GET_MAX_DATA_SIZE(FMSNMsgWillTopic));
  }

  sendMessage();

//    if (qos() == FMSN_FLAG_QOS_1 || qos() == FMSN_FLAG_QOS_2) {
//        waitingForResponse = true;
//        responseToWaitFor = WILLMSGREQ;
//    }
}

void
FMSNBasicClient::willMsg(const void *willMsg, const uint8_t willMsgLen,
                         const bool update)
{
  FMSNMsgWillMsg *msg = reinterpret_cast<FMSNMsgWillMsg *>(mMessageBuffer);

  msg->length = sizeof(FMSNMsgWillMsg) + willMsgLen;
  msg->type   = update ? FMSNMT_WILLMSGUPD : FMSNMT_WILLMSG;
  memcpy(msg->willmsg, willMsg, willMsgLen);

  sendMessage();
}

void
FMSNBasicClient::disconnect(const uint16_t duration)
{
  FMSNMsgDisconnect *msg =
    reinterpret_cast<FMSNMsgDisconnect *>(mMessageBuffer);

  msg->length = sizeof(FMSNMsgHeader);
  msg->type   = FMSNMT_DISCONNECT;

  if(duration > 0)
  {
    msg->length  += sizeof(FMSNMsgDisconnect);
    msg->duration = bswap(duration);
  }

  sendMessage();
  mResponseToWaitFor = fmsnGetRespondType(msg->type);
}

void
FMSNBasicClient::startResponseTimer()
{
}

bool
FMSNBasicClient::registerTopic(const char *name)
{
  if((FMSNMT_INVALID == mResponseToWaitFor)
     && (mTopicCount < FMSN_MAX_TOPICS))
  {
    ++mMessageId;

    // Fill in the next table entry, but we only increment the counter to
    // the next topic when we get a REGACK from the broker. So don't issue
    // another REGISTER until we have resolved this one.
    setTopic(name, 0);

    FMSNMsgRegister *msg = reinterpret_cast<FMSNMsgRegister *>(mMessageBuffer);

    msg->length    = sizeof(FMSNMsgRegister) + strlen(name);
    msg->type      = FMSNMT_REGISTER;
    msg->topicId   = 0;
    msg->messageId = bswap(mMessageId);
    fmsnSafeCopyText(msg->topicName, name,
                     FMSN_GET_MAX_DATA_SIZE(FMSNMsgRegister));

    sendMessage();
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
    return true;
  }

  return false;
}

void
FMSNBasicClient::regAck(const uint16_t topicId, const uint16_t messageId,
                        const FMSNReturnCode returnCode)
{
  FMSNMsgRegAck *msg = reinterpret_cast<FMSNMsgRegAck *>(mMessageBuffer);

  msg->length     = sizeof(FMSNMsgRegAck);
  msg->type       = fmsnGetRespondType(FMSNMT_REGISTER);
  msg->topicId    = bswap(topicId);
  msg->messageId  = bswap(messageId);
  msg->returnCode = returnCode;

  sendMessage();
}

void
FMSNBasicClient::publish(const uint16_t topicId, const void *data,
                         const uint8_t dataLen)
{
  ++mMessageId;

  FMSNMsgPublish *msg = reinterpret_cast<FMSNMsgPublish *>(mMessageBuffer);

  msg->length    = sizeof(FMSNMsgPublish) + dataLen;
  msg->type      = FMSNMT_PUBLISH;
  msg->flags     = mFlags;
  msg->topicId   = bswap(topicId);
  msg->messageId = bswap(mMessageId);
  memcpy(msg->data, data, dataLen);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(FMSNMT_PUBLISH);
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
FMSNBasicClient::pubAck(const uint16_t topicId, const uint16_t messageId,
                        const FMSNReturnCode returnCode)
{
  FMSNMsgPubAck *msg = reinterpret_cast<FMSNMsgPubAck *>(mMessageBuffer);

  msg->length     = sizeof(FMSNMsgPubAck);
  msg->type       = fmsnGetRespondType(FMSNMT_PUBLISH);
  msg->topicId    = bswap(topicId);
  msg->messageId  = bswap(messageId);
  msg->returnCode = returnCode;

  sendMessage();
}

void
FMSNBasicClient::subscribeByName(const char *topicName)
{
  ++mMessageId;

  FMSNMsgSubscribe *msg = reinterpret_cast<FMSNMsgSubscribe *>(mMessageBuffer);

  // The -2 here is because we're unioning a 0-length member (topicName)
  // with a uint16_t in the msg_subscribe struct.
  msg->length    = sizeof(FMSNMsgSubscribe) + strlen(topicName) - 2;
  msg->type      = FMSNMT_SUBSCRIBE;
  msg->flags     = qos() | FMSN_FLAG_TOPIC_NAME;
  msg->messageId = bswap(mMessageId);
  fmsnSafeCopyText(msg->topicName, topicName, FMSN_GET_MAX_DATA_SIZE(
                     FMSNMsgSubscribe) - 2);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
  }
}

void
FMSNBasicClient::subscribeById(const uint16_t topicId)
{
  ++mMessageId;

  FMSNMsgSubscribe *msg = reinterpret_cast<FMSNMsgSubscribe *>(mMessageBuffer);

  msg->length    = sizeof(FMSNMsgSubscribe);
  msg->type      = FMSNMT_SUBSCRIBE;
  msg->flags     = qos() | FMSN_FLAG_TOPIC_PREDEFINED_ID;
  msg->messageId = bswap(mMessageId);
  msg->topicId   = bswap(topicId);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
  }
}

void
FMSNBasicClient::unsubscribeByName(const char *topicName)
{
  ++mMessageId;

  FMSNMsgUnsubscribe *msg =
    reinterpret_cast<FMSNMsgUnsubscribe *>(mMessageBuffer);

  // The -2 here is because we're unioning a 0-length member (topicName)
  // with a uint16_t in the msg_unsubscribe struct.
  msg->length    = sizeof(FMSNMsgUnsubscribe) + strlen(topicName) - 2;
  msg->type      = FMSNMT_UNSUBSCRIBE;
  msg->flags     = qos() | FMSN_FLAG_TOPIC_NAME;
  msg->messageId = bswap(mMessageId);
  fmsnSafeCopyText(msg->topicName, topicName,
                   FMSN_GET_MAX_DATA_SIZE(FMSNMsgUnsubscribe) - 2);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
  }
}

void
FMSNBasicClient::unsubscribeById(const uint16_t topicId)
{
  ++mMessageId;

  FMSNMsgUnsubscribe *msg =
    reinterpret_cast<FMSNMsgUnsubscribe *>(mMessageBuffer);

  msg->length    = sizeof(FMSNMsgUnsubscribe);
  msg->type      = FMSNMT_UNSUBSCRIBE;
  msg->flags     = qos() | FMSN_FLAG_TOPIC_PREDEFINED_ID;
  msg->messageId = bswap(mMessageId);
  msg->topicId   = bswap(topicId);

  sendMessage();

  if(fmsnIsHighQos(qos()))
  {
    mResponseToWaitFor = fmsnGetRespondType(msg->type);
  }
}

void
FMSNBasicClient::pingReq(const char *clientId)
{
  FMSNMsgPingReq *msg = reinterpret_cast<FMSNMsgPingReq *>(mMessageBuffer);

  msg->length = sizeof(FMSNMsgPingReq) + strlen(clientId);
  msg->type   = FMSNMT_PINGREQ;
  fmsnSafeCopyText(msg->clientId, clientId,
                   FMSN_GET_MAX_DATA_SIZE(FMSNMsgPingReq));

  sendMessage();

  mResponseToWaitFor = fmsnGetRespondType(msg->type);
}

void
FMSNBasicClient::pingResp()
{
  FMSNMsgHeader *msg = reinterpret_cast<FMSNMsgHeader *>(mMessageBuffer);

  msg->length = sizeof(FMSNMsgHeader);
  msg->type   = fmsnGetRespondType(FMSNMT_PINGREQ);

  sendMessage();
}

FMSNClient::FMSNClient(Stream *stream)
  : FMSNBasicClient(stream)
  , mResponseTimer(0)
  , mResponseRetries(0)
{
}

bool
FMSNClient::waitForResponse()
{
  if(responseToWaitFor() != FMSNMT_INVALID)
  {
    // TODO: Watch out for overflow.
    if((millis() - mResponseTimer) > (FMSN_T_RETRY * 1000L))
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

  return responseToWaitFor() != FMSNMT_INVALID;
}

void
FMSNClient::startResponseTimer()
{
  mResponseTimer   = millis();
  mResponseRetries = FMSN_N_RETRY;
}
