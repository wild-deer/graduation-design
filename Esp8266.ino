#include <ESP8266WiFi.h>
/* 依赖 PubSubClient 2.4.0 */
#include <PubSubClient.h>
/* 依赖 ArduinoJson 5.13.4 */
#include <ArduinoJson.h>
#include <SimpleDHT.h>

#define SENSOR_PIN    LED_BUILTIN  //定义的板载LED灯

/* 修改1 ------------------------------------------ */
/* 连接您的WIFI SSID和密码 */
#define WIFI_SSID         "esp8266test"   //自己wifi名称
#define WIFI_PASSWD       "12345678"    //自己wifi密码
/* 修改1 end--------------------------------------- */

/* 修改2 ------------------------------------------ */
/* 设备证书信息*/
#define PRODUCT_KEY       "a11ZjsUhJzH" //在自己阿里云里面查看
#define DEVICE_NAME       "car_status"  //创建设备名称，修改成自己的
#define DEVICE_SECRET     "6d2b0d607b03a3d9946dbb98888558b7" //ProductSecret 修改成自己的
#define REGION_ID         "cn-shanghai"  //如果使用的是华东2（上海）服务器，不需要修改
/* 修改2 end--------------------------------------- */

/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER       "139.224.42.2"
#define MQTT_PORT         1883
#define MQTT_USRNAME      DEVICE_NAME "&" PRODUCT_KEY

/* 修改3 ------------------------------------------ */
#define CLIENT_ID    "a11ZjsUhJzH.car_status|securemode=2,signmethod=hmacsha256,timestamp=1651835981664|"//可以不用修改使用，也可以修改成自己想要的
// 请使用以上说明中的加密工具或参见MQTT-TCP连接通信文档加密生成password。
// 加密明文是参数和对应的值（clientIdesp8266deviceName${deviceName}productKey${productKey}timestamp1234567890）按字典顺序拼接
// 密钥是设备的DeviceSecret
#define MQTT_PASSWD       "3cc7b0a14733eef35b52f6870ed38ee2604538eecfe318a4941fccb2d1864ee1"  //修改成自己的，生成方式后面简介。
/* 修改3 end--------------------------------------- */

#define ALINK_BODY_FORMAT         "{\"id\":\"QingYeMuRong\",\"version\":\"1.0\", \"sys\":{\"ack\":0},\"method\":\"thing.event.property.post\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST     "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"



//温湿度定义
int pinDHT11 = 14;
SimpleDHT11 dht11(pinDHT11);
byte dht_temp = 0;
byte dht_humi = 0;
byte dht_sucess = 0;
//超声波测距定义
int echo = 13;
int triger = 12;
byte echoRead = 0;

unsigned long lastMs = 0;
WiFiClient espClient;
PubSubClient  client(espClient);


void callback(char *topic, byte *payload, unsigned int length)  //接收数据
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  Serial.println((char *)payload);
  //直接使用函数，对收到的服务器数据解析 ，这部分函数使用可以见我上一篇博客。
  DynamicJsonDocument doc(1024);
  deserializeJson(doc,(char *)payload);
  JsonObject obj = doc.as<JsonObject>();

  String sensor = obj["params"];

  //digitalWrite(LED_BUILTIN, LOW);
  //Serial.println("Motion absent!");
  Serial.print("SENSOR:");//测试代码
  Serial.println(sensor);

  //DynamicJsonDocument doc(1024);
  deserializeJson(doc,sensor);
  String LightSwitch = obj["LightSwitch"];

  Serial.print("PAYLOADSENSER:");//测试代码
  Serial.println(LightSwitch);
  
  if(LightSwitch=="1")  //根据收到的值，控制LED灯
  {
    digitalWrite(LED_BUILTIN, LOW);  
     Serial.println("开灯");
    }
   if(LightSwitch=="0")
  {
    digitalWrite(LED_BUILTIN, HIGH);Serial.println("关灯");
    } 
}
//读取轮胎磨损程度
double readDistance(int Echo,int Trig){
        digitalWrite(Trig, LOW);                                 
        delayMicroseconds(2);                                   
        digitalWrite(Trig, HIGH);                               
        delayMicroseconds(10);                                  //产生一个10us的高脉冲去触发SR04
        digitalWrite(Trig, LOW);                                
        
        double stime = pulseIn(Echo, HIGH);                              // 检测脉冲宽度，注意返回值是微秒us
        double distance = stime /58 ;                                  //计算出距离,输出的距离的单位是厘米cm
 
        return distance;                            //把得到的距离值通过串口通信返回给电脑
}
void wifiInit()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("WiFi not Connect");
  }

  Serial.println("Connected to AP");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  Serial.print("espClient [");


  client.setServer(MQTT_SERVER, MQTT_PORT);   /* 连接WiFi之后，连接MQTT服务器 */
  client.setCallback(callback);//接收数据
}


void mqttCheckConnect()
{
  while (!client.connected())
  {
    Serial.println("Connecting to MQTT Server ...");
    if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD))

    {
      
      Serial.println("MQTT Connected!");

    }
    else
    {
      Serial.print("MQTT Connect err:");
      Serial.println(client.state()); //重新连接
      delay(5000);
    }
  }
}


void mqttIntervalPost()
{
  char param[64];
  char jsonBuf[128];

  /* 修改5 读取温湿度--------------------------------------- */
 if ((dht11.read(&dht_temp, &dht_humi, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT11 failed"); 
      dht_sucess = 0;
    }else
    {
      dht_sucess = 1;
    }
readDistance



   
    // send report data.
    if (1 == dht_sucess)
    {
      //sprintf(data_now, "#%d,%d,%d#", dht_temp, light_switch, dht_humi);
      /* 修改4 ------------------------------------------ */
  sprintf(param, "{\"LightSwitch\":%d,\"inDoorTemp\":%d,\"CurrentHumidity\":%d}", digitalRead(LED_BUILTIN),dht_temp,dht_humi);//读取LED灯引脚状态和温度
  /* 修改4 end--------------------------------------- */
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
     Serial.print("digitalread:");
    Serial.println(!digitalRead(LED_BUILTIN));
  //Serial.println(jsonBuf);
  boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);   // 上报数据
  //Serial.print("publish:0 失败;1成功");
  //Serial.println(d);
    }
 /* 修改5 end--------------------------------------- */

 
}


void setup()
{
  //初始化保存获取到的数据的数组，并把内存置为0
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  //设置超声波读取
  pinMode(triger, OUTPUT); 
  pinMode(echo, INPUT);           
  /* initialize serial for debugging */
  Serial.begin(115200);
  Serial.println("Demo Start");
  client.setCallback(callback); //用于接收服务器接收的数据

  wifiInit();
  if ((dht11.read(&dht_temp, &dht_humi, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT11 failed"); 
      dht_sucess = 0;
    }else
    {
      dht_sucess = 1;
    }
}


// the loop function runs over and over again forever
void loop()
{
  if (millis() - lastMs >= 5000)
  {
    lastMs = millis();
    mqttCheckConnect();

    /* 上报消息心跳周期 */
    mqttIntervalPost();
  }

  client.loop(); //MUC接收数据的主循环函数。
  /*
  if (digitalRead(LED_BUILTIN) == HIGH) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Motion detected!");
    delay(2000);
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Motion absent!");
    delay(2000);
  }
  */

}

