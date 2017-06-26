#include <Arduino.h>
#include <FlyMqttSN.h>

class MqttClient : public FMSNClient
{
public:
  MqttClient(Stream *stream) : FMSNClient(stream)
  {
  }

protected:
  virtual void
  pubAckHandler(const FMSNMsgPubAck *msg);
  virtual void
  connAckHandler(const FMSNMsgConnAck *msg);
  virtual void
  regAckHandler(const FMSNMsgRegAck *msg);

//  virtual void pubackHandler(const FMSNMsgPubAck *msg);
};

static MqttClient
  sMqttClient(&Serial3);
static volatile int
  sProgress = 0;

void
setup()
{
  // Serial 0 use for print out debug information
  Serial.begin(9600);

  // Serial 3 use for communicate with PC.
  Serial3.begin(9600);

  while(!Serial)  // for the Arduino Leonardo/Micro only
  {
  }

  while(!Serial3)  // for the Arduino Leonardo/Micro only
  {
  }

  Serial.println(F("Fly MQTT-SN library example."));
}

void
loop()
{
  sMqttClient.parseStream();

  if(sMqttClient.waitForResponse())
  {
    return;
  }

  // Finished a response, check progress status
  if(0 == sProgress)
  {
    Serial.println(F("Try connect to broker with these settings : "));
    Serial.println(F("QOS: 0, Keep Alive Interval: 30s, Client ID: Shit007"));

    sMqttClient.setQos(FMSN_FLAG_QOS_1);
    sMqttClient.setClientId("Shit007");
    sMqttClient.connect();
    ++sProgress;
  }
  else if(1 == sProgress)
  {
    Serial.println(F("Failed to connect!"));
    sProgress = 0xff;
  }
  else if(2 == sProgress)
  {
    // Just finished a connection

    Serial.println(F("Try to register topic : arse"));

    // After connected, we try to register a topic, so that we could publish
    // to.
    sMqttClient.registerTopic("arse");

    ++sProgress;
  }
  else if(3 == sProgress)
  {
    Serial.println(F("Failed to register!"));
    sProgress = 0xff;
  }
  else if(4 == sProgress)
  {
    Serial.println(F("Try to publish topic : arse"));

    auto topic = sMqttClient.getTopicByName("arse");

    if(NULL == topic)
    {
      Serial.println(F("Failed to register topic : arse"));
    }
    else
    {
      static const char *info = "Kick your arse!";

      Serial.print(F("Topic id for \"arse\": "));
      Serial.println(topic->id);

      sMqttClient.publish(topic->id, info, static_cast<uint8_t>(strlen(info)));

      ++sProgress;
    }
  }
  else if(5 == sProgress)
  {
    Serial.println(F("Failed to publish!"));
    sProgress = 0xff;
  }
  else if(6 == sProgress)
  {
    Serial.println(F("Publish finished."));

    sMqttClient.disconnect(5);

    Serial.println(F("Disconnect"));
    sProgress = 0xff;
  }
}

void
MqttClient::pubAckHandler(const FMSNMsgPubAck *msg)
{
  FMSNClient::pubAckHandler(msg);

  Serial.println(F("Published!"));
  ++sProgress;
}

void
MqttClient::connAckHandler(const FMSNMsgConnAck *msg)
{
  FMSNClient::connAckHandler(msg);

  Serial.println(F("Connected!"));
  ++sProgress;
}

void
MqttClient::regAckHandler(const FMSNMsgRegAck *msg)
{
  FMSNClient::regAckHandler(msg);

  Serial.println(F("Registerred!"));
  ++sProgress;
}
