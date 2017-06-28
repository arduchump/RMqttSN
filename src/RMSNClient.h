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

#ifndef __INCLUDED_E457D8FE526A11E7AA6EA088B4D1658C
#define __INCLUDED_E457D8FE526A11E7AA6EA088B4D1658C

#include "RMSNTypes.h"
#include <RTimer.h>

#define RMSN_MAX_TOPICS      10
#define RMSN_MAX_BUFFER_SIZE 66
#define RMSN_GET_MAX_DATA_SIZE(headerClass) \
  ((size_t)(RMSN_MAX_BUFFER_SIZE - sizeof(headerClass)))

class RMSNBasicClient : public RObject
{
public:
  RMSNBasicClient(Stream *stream);

  virtual
  ~RMSNBasicClient();

  void
  setQos(uint8_t qos);

  uint8_t
  qos();

  const RMSNTopic *
  getTopicByName(const char *name) const;
  const RMSNTopic *
  getTopicById(const uint16_t &id) const;

  void
  setTopic(const char *name, const uint16_t &id);

  void
  parseStream();

  void
  searchGw(const uint8_t radius);
  void
  connect();
  void
  willTopic(const char *willTopic, const bool update=false);
  void
  willMsg(const void *willMsg, const uint8_t willMsgLen,
          const bool update=false);
  bool
  registerTopic(const char *name);
  void
  publish(const uint16_t topicId, const void *data, const uint8_t dataLen);
#ifdef USE_QOS2
  void
  pubrec();
  void
  pubrel();
  void
  pubcomp();
#endif
  void
  subscribeByName(const char *topicName);
  void
  subscribeById(const uint16_t topicId);
  void
  unsubscribeByName(const char *topicName);
  void
  unsubscribeById(const uint16_t topicId);
  void
  pingReq(const char *clientId);
  void
  pingResp();
  void
  disconnect(const uint16_t duration);

  virtual void
  startResponseTimer();
  virtual void
  timeout();

  uint16_t
  keepAliveInterval() const;
  void
  setKeepAliveInterval(const uint16_t &keepAliveInterval);

  String
  clientId() const;
  void
  setClientId(const String &clientId);

  uint8_t
  responseToWaitFor() const;

protected:
  virtual void
  advertiseHandler(const RMSNMsgAdvertise *msg);
  virtual void
  gwInfoHandler(const RMSNMsgGwInfo *msg);
  virtual void
  connAckHandler(const RMSNMsgConnAck *msg);
  virtual void
  willTopicReqHandler(const RMSNMsgHeader *msg);
  virtual void
  willMsgReqHandler(const RMSNMsgHeader *msg);
  virtual void
  regAckHandler(const RMSNMsgRegAck *msg);
  virtual void
  publishHandler(const RMSNMsgPublish *msg);
  virtual void
  registerHandler(const RMSNMsgRegister *msg);
  virtual void
  pubAckHandler(const RMSNMsgPubAck *msg);

#ifdef USE_QOS2
  virtual void
  pubrecHandler(const msg_pubqos2 *msg);
  virtual void
  pubrelHandler(const msg_pubqos2 *msg);
  virtual void
  pubcompHandler(const msg_pubqos2 *msg);

#endif
  virtual void
  subAckHandler(const RMSNMsgSubAck *msg);
  virtual void
  unsubAckHandler(const RMSNMsgUnsubAck *msg);
  virtual void
  pingReqHandler(const RMSNMsgPingReq *msg);
  virtual void
  pingRespHandler();
  virtual void
  disconnectHandler(const RMSNMsgDisconnect *msg);
  virtual void
  willTopicRespHandler(const RMSNMsgWillTopicResp *msg);
  virtual void
  willMsgRespHandler(const RMSNMsgWillMsgResp *msg);

  void
  regAck(const uint16_t topicId, const uint16_t messageId,
         const RMSNReturnCode returnCode);
  void
  pubAck(const uint16_t topicId, const uint16_t messageId,
         const RMSNReturnCode returnCode);

  void
  dispatch();
  uint16_t
  bswap(const uint16_t val);
  void
  sendMessage();

private:
  /// Set to valid message type when we're waiting for some sort of
  /// acknowledgement from the server that will transition our state.
  uint8_t   mResponseToWaitFor;
  uint16_t  mMessageId;
  uint8_t   mTopicCount;
  uint8_t   mMessageBuffer[RMSN_MAX_BUFFER_SIZE];
  uint8_t   mResponseBuffer[RMSN_MAX_BUFFER_SIZE];
  RMSNTopic mTopicTable[RMSN_MAX_TOPICS];
  uint8_t   mGatewayId;
  /// Default flags
  uint8_t  mFlags;
  uint16_t mKeepAliveInterval;

  /// Target stream we will send to.
  Stream *mStream;
  String  mClientId;
};

class RMSNClient : public RMSNBasicClient
{
public:
  RMSNClient(Stream *stream);

  void
  startResponseTimer();

private:
  void
  onResponseTimerTimeout();

private:
  RTimer  mResponseTimer;
  uint8_t mResponseRetries;
};

#endif // __INCLUDED_E457D8FE526A11E7AA6EA088B4D1658C
