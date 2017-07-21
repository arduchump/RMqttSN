#ifndef __INCLUDED_018164CE6DDA11E7AA6EA088B4D1658C
#define __INCLUDED_018164CE6DDA11E7AA6EA088B4D1658C

#include "RMSNTypes.h"

class RBufferStream;
class RMSNClient;
class RMSNPublisher
{
public:
  RMSNPublisher(RMSNClient *client);
  RMSNPublisher(const RMSNPublisher &other);
  ~RMSNPublisher();

  RMSNPublisher &
  operator =(const RMSNPublisher &other);
  RBufferStream *
  payloadStream();

private:
  mutable RMSNClient *mClient;
};

#endif // __INCLUDED_018164CE6DDA11E7AA6EA088B4D1658C
