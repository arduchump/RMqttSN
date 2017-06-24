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

#include "FMSNTypes.h"

#define FMSN_MAX_TOPICS      10
#define FMSN_MAX_BUFFER_SIZE 66
#define FMSN_GET_MAX_DATA_SIZE(headerClass) \
  ((size_t)(FMSN_MAX_BUFFER_SIZE - sizeof(headerClass)))

class FMSNClient
{
public:
  FMSNClient(Stream *stream);
  virtual
  ~FMSNClient();

  void
  setQos(uint8_t qos);

  uint8_t
  qos();

  uint16_t
  findTopicId(const char *name,
              const uint16_t &defaultId=FMSN_INVALID_TOPIC_ID);
  bool
  waitForResponse();

  void
  parseStream();

  void
  searchgw(const uint8_t radius);
  void
  connect();
  void
  willtopic(const char *willTopic, const bool update=false);
  void
  willmsg(const void *willMsg, const uint8_t willMsgLen,
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
  pingreq(const char *clientId);
  void
  pingresp();
  void
  disconnect(const uint16_t duration);

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

protected:
  virtual void
  advertiseHandler(const FMSNMsgAdvertise *msg);
  virtual void
  gwinfoHandler(const FMSNMsgGwinfo *msg);
  virtual void
  connackHandler(const FMSNMsgConnack *msg);
  virtual void
  willtopicreqHandler(const FMSNMsgHeader *msg);
  virtual void
  willmsgreqHandler(const FMSNMsgHeader *msg);
  virtual void
  regackHandler(const FMSNMsgRegack *msg);
  virtual void
  publishHandler(const FMSNMsgPublish *msg);
  virtual void
  registerHandler(const FMSNMsgRegister *msg);
  virtual void
  pubackHandler(const FMSNMsgPuback *msg);

#ifdef USE_QOS2
  virtual void
  pubrecHandler(const msg_pubqos2 *msg);
  virtual void
  pubrelHandler(const msg_pubqos2 *msg);
  virtual void
  pubcompHandler(const msg_pubqos2 *msg);

#endif
  virtual void
  subackHandler(const FMSNMsgSuback *msg);
  virtual void
  unsubackHandler(const FMSNMsgUnsuback *msg);
  virtual void
  pingreqHandler(const FMSNMsgPingreq *msg);
  virtual void
  pingrespHandler();
  virtual void
  disconnectHandler(const FMSNMsgDisconnect *msg);
  virtual void
  willtopicrespHandler(const FMSNMsgWilltopicresp *msg);
  virtual void
  willmsgrespHandler(const FMSNMsgWillmsgresp *msg);

  void
  regack(const uint16_t topicId, const uint16_t messageId,
         const FMSNReturnCode returnCode);
  void
  puback(const uint16_t topicId, const uint16_t messageId,
         const FMSNReturnCode returnCode);

private:
  struct Topic
  {
    const char *name;
    uint16_t    id;
  };

  void
  dispatch();
  uint16_t
  bswap(const uint16_t val);
  void
  sendMessage();

  /// Set to valid message type when we're waiting for some sort of
  /// acknowledgement from the server that will transition our state.
  uint8_t  mResponseToWaitFor;
  uint16_t mMessageId;
  uint8_t  mTopicCount;
  uint8_t  mMessageBuffer[FMSN_MAX_BUFFER_SIZE];
  uint8_t  mResponseBuffer[FMSN_MAX_BUFFER_SIZE];
  Topic    mTopicTable[FMSN_MAX_TOPICS];
  uint8_t  mGatewayId;
  /// Default flags
  uint8_t  mFlags;
  uint16_t mKeepAliveInterval;
  uint32_t mResponseTimer;
  uint8_t  mResponseRetries;

  /// Target stream we will send to.
  Stream *mStream;
  String  mClientId;
};

#endif // __INCLUDED_E457D8FE526A11E7AA6EA088B4D1658C
