#include <Arduino.h>
#include <RabirdToolkitConfigNonOS.h>
#include <RabirdToolkitThirdParties.h>
#include <RabirdToolkit.h>
#include <RCoRoutine.h>
#include <RMqttSN.h>

#include <RApplication.h>
#include <RThread.h>
#include <REventLoop.h>

static PT_THREAD(mqttClientProcess(struct pt *pt));

class MqttClient : public RMSNClient
{
public:
  MqttClient() : RMSNClient()
  {
  }

protected:
  void
  onReceived(const RMSNMsgHeader *msg);
};

static struct pt sMqttClientPt;

/**
 * @brief sMqttClient
 */
static MqttClient *sMqttClient = NULL;

/**
 * @brief onApplicationIdle
 */
static void
onApplicationIdle()
{
  static bool isFinished = false;

  if(!isFinished)
  {
    if(PT_EXITED <= mqttClientProcess(&sMqttClientPt))
    {
      isFinished = true;
    }
  }
}

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

  PT_INIT(&sMqttClientPt);

  sMqttClient = new MqttClient();
  sMqttClient->begin(&Serial3);

  auto eventLoop = app->thread()->eventLoop();
  eventLoop->idle.connect(onApplicationIdle);

  return 0;
}

static PT_THREAD(mqttClientProcess(struct pt *pt))
{
  PT_BEGIN(pt);

  // Finished a response, check progress status
  Serial.println(F("Try connect to broker with these settings : "));
  Serial.println(F("QOS: 0, Keep Alive Interval: 30s, Client ID: Shit007"));

  sMqttClient->setQos(RMSN_FLAG_QOS_1);
  sMqttClient->setClientId("Shit007");
  sMqttClient->connect();

  PT_WAIT_UNTIL(pt, sMqttClient->isResponsedOrTimeout());

  if(sMqttClient->isTimeout())
  {
    Serial.println(F("Failed to connect!"));
    PT_EXIT(pt);
  }

  // Just finished a connection

  Serial.println(F("Try to register topic : arse"));

  // After connected, we try to register a topic, so that we could publish
  // to.
  sMqttClient->registerTopic("arse");

  PT_WAIT_UNTIL(pt, sMqttClient->isResponsedOrTimeout());

  if(sMqttClient->isTimeout())
  {
    Serial.println(F("Failed to register!"));
    PT_EXIT(pt);
  }

  Serial.println(F("Try to publish topic : arse"));

  {
    auto topic = sMqttClient->getTopicByName("arse");

    if(NULL == topic)
    {
      Serial.println(F("Failed to register topic : arse"));
      PT_EXIT(pt);
    }
    else
    {
      static const char *info = "Kick your arse!";

      Serial.print(F("Topic id for \"arse\": "));
      Serial.println(topic->id);

      sMqttClient->publish(topic->id, info, static_cast<uint8_t>(strlen(info)));
    }
  }

  PT_WAIT_UNTIL(pt, sMqttClient->isResponsedOrTimeout());

  if(sMqttClient->isTimeout())
  {
    Serial.println(F("Failed to publish!"));
    PT_EXIT(pt);
  }

  Serial.println(F("Publish finished."));

  sMqttClient->disconnect(5);

  PT_WAIT_UNTIL(pt, sMqttClient->isResponsedOrTimeout());

  Serial.println(F("Disconnect"));

  PT_END(pt);
}

void
MqttClient::onReceived(const RMSNMsgHeader *msg)
{
  if(msg->type == RMSNMT_PUBACK)
  {
    Serial.println(F("Published!"));
  }
  else if(msg->type == RMSNMT_CONNACK)
  {
    Serial.println(F("Connected!"));
  }
  else if(msg->type == RMSNMT_REGACK)
  {
    Serial.println(F("Registerred!"));
  }
}
