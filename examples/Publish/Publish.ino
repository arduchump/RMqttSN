#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <FreeRTOSMallocFixer.h>
#include <ArduinouClibcpp.h>
#include <RabirdToolkitThirdParties.h>
#include <RabirdToolkit.h>
#include <RMqttSN.h>

#include <RApplication.h>
#include <RThread.h>
#include <REventLoop.h>

class MqttClient : public RMSNClient
{
public:
  MqttClient(Stream *stream) : RMSNClient(stream)
  {
  }

protected:
  virtual void
  pubAckHandler(const RMSNMsgPubAck *msg);
  virtual void
  connAckHandler(const RMSNMsgConnAck *msg);
  virtual void
  regAckHandler(const RMSNMsgRegAck *msg);

//  virtual void pubackHandler(const RMSNMsgPubAck *msg);
};

static void
mqttClientProcess();

/**
 * @brief sMqttClient
 */
static MqttClient   sMqttClient(&Serial3);
static volatile int sProgress = 0;

int
rMain(int argc, rfchar *argv[])
{
  // Add your initialization code here
  Serial.begin(9600);
  Serial3.begin(9600);

  while((!Serial) || (!Serial3))
  {
  }

  Serial.println(F("RMqttSN library publish example."));

  // Application starts from here!
  RApplication *app = new RApplication();

  auto eventLoop = app->thread()->eventLoop();
  eventLoop->idle.connect(&mqttClientProcess);

  return 0;
}

static void
mqttClientProcess()
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

    sMqttClient.setQos(RMSN_FLAG_QOS_1);
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
MqttClient::pubAckHandler(const RMSNMsgPubAck *msg)
{
  RMSNClient::pubAckHandler(msg);

  Serial.println(F("Published!"));
  ++sProgress;
}

void
MqttClient::connAckHandler(const RMSNMsgConnAck *msg)
{
  RMSNClient::connAckHandler(msg);

  Serial.println(F("Connected!"));
  ++sProgress;
}

void
MqttClient::regAckHandler(const RMSNMsgRegAck *msg)
{
  RMSNClient::regAckHandler(msg);

  Serial.println(F("Registerred!"));
  ++sProgress;
}
