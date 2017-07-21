#include "RMSNPublisher.h"
#include "RMSNClient.h"

RMSNPublisher::RMSNPublisher(RMSNClient *client)
  : mClient(client)
{
}

RMSNPublisher::RMSNPublisher(const RMSNPublisher &other)
  : mClient(other.mClient)
{
  other.mClient = NULL;
}

RMSNPublisher::~RMSNPublisher()
{
  if(mClient)
  {
    mClient->publishEnd();
  }
}

RMSNPublisher &
RMSNPublisher::operator =(const RMSNPublisher &other)
{
  mClient       = other.mClient;
  other.mClient = NULL;
}

RBufferStream *
RMSNPublisher::payloadStream()
{
  return mClient->pubPayloadStream();
}
