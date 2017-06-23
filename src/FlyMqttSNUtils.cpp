#include "FlyMqttSNUtils.h"

FMSNMsgType
fmsnGetRespondType(FMSNMsgType requestType)
{
  // Mqtt-SN message have a request-respond pattern, respond message types are
  // numbers followed after request message types except types list below:
  //
  // FMSNMT_ADVERTISE, FMSNMT_DISCONNECT
  if((FMSNMT_ADVERTISE == requestType) || (FMSNMT_DISCONNECT == requestType))
  {
    return requestType;
  }

  return static_cast<FMSNMsgType>(((requestType) + 1));
}

bool
fmsnIsHighQos(uint8_t qos)
{
  return (qos == FMSN_FLAG_QOS_1) || (qos == FMSN_FLAG_QOS_2);
}
