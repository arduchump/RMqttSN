#ifndef __INCLUDED_895A8BE057E211E7AA6EA088B4D1658C
#define __INCLUDED_895A8BE057E211E7AA6EA088B4D1658C

#include "FlyMqttSNTypes.h"

///
/// Get respond type from a reqeuest type.
///
/// @arg requestType
FMSNMsgType
fmsnGetRespondType(FMSNMsgType requestType);

///
/// @brief Check if qos level is high qos
/// @param qos
/// @return
///
bool
fmsnIsHighQos(uint8_t qos);

#endif // __INCLUDED_895A8BE057E211E7AA6EA088B4D1658C
