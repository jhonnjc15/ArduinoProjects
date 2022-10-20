// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

const unsigned long PERIOD_ALARM = 10000UL;
unsigned long time_now_alarm = 0UL;

const unsigned long PERIOD_DATA = 5000UL;
unsigned long time_now_data = 0UL;

unsigned long evento_tiempo_respuesta_antes = 0UL;
unsigned long evento_tiempo_respuesta_despues = 0UL;
unsigned long evento_tiempo_diferencia = 0UL;

template<class T> inline Print &operator <<(Print &obj, T arg) {
  obj.print(arg);
  return obj;
}

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "CTBot.h" //Unico que hay en la libreria
// ArduinoJson //Instalar el primero

CTBot miBot;
CTBotInlineKeyboard miTeclado;
CTBotReplyKeyboard miTecladoFijo;   // reply keyboard object helper
bool isKeyboardActive;      // store if the reply keyboard is shown

int Led = 2;
#define DHTPIN 4     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

#include "token.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando Bot de Telegram");

  ////////////////////
  // DHT22 PART
  ////////////////////

  // Initialize device.
  dht.begin();
  Serial.println(F("DHT22 Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  //delayMS = sensor.min_delay / 1000;  


  pinMode(Led, OUTPUT);
  digitalWrite(Led, HIGH);

  Serial.println(F("--------------Conectando-----------"));
  miBot.wifiConnect(ssid, password);
  Serial.println(F("--------------Conectando-----------"));
  miBot.setTelegramToken(token);

  if (miBot.testConnection()) {
    Serial.println("\n Conectado");
  }
  else {
    Serial.println("\n Problemas con la conexión, vuelva a intentarlo");
  }

  miTeclado.addButton("Temperatura", "temperatura", CTBotKeyboardButtonQuery);
  miTeclado.addButton("Humedad", "humedad", CTBotKeyboardButtonQuery);
  miTeclado.addRow();
  miTeclado.addButton("Encender Led", "encender_led", CTBotKeyboardButtonQuery);
  miTeclado.addButton("Apagar Led", "apagar_led", CTBotKeyboardButtonQuery);
  //miTeclado.addButton("mira documentación", "https://nocheprogramacion.com/", CTBotKeyboardButtonURL);
  miTecladoFijo.addButton("Opciones");
  miTecladoFijo.enableResize();
	isKeyboardActive = false;
}

void loop() {
  String temperatura, humedad;

  int estado_led = digitalRead(Led);

  // Get temperature event and print its value.
  sensors_event_t eventT;
  sensors_event_t eventH;

  dht.temperature().getEvent(&eventT);
  temperatura = String(eventT.temperature);

  // Get humidity event and print its value.
  dht.humidity().getEvent(&eventH);
  humedad = String(eventH.relative_humidity);
  
  //Funciona sin preocuparse del overflow
  if(millis() - time_now_data > PERIOD_DATA){
    time_now_data = millis();
    if (isnan(eventT.temperature)) {
      Serial.println(F("Error reading temperature!"));
    }
    else {
      Serial.print(F("Temperature: "));
      Serial.print(temperatura);
      Serial.println(F("°C"));
    }

    
    if (isnan(eventH.relative_humidity)) {
      Serial.println(F("Error reading humidity!"));
    }
    else {
      Serial.print(F("Humidity: "));
      Serial.print(humedad);
      Serial.println(F("%"));
    }  
  }


  ////////////////////
  // TELEGRAM PART
  ////////////////////
  TBMessage msg;
  
  //Funciona sin preocuparse del overflow
  if(millis() - time_now_alarm > PERIOD_ALARM){
    time_now_alarm = millis();

    if (isnan(eventT.temperature) || isnan(eventH.relative_humidity))
    {
      miBot.sendMessage(GroupId, "\U0000274C  ERROR  \U0000274C \n\n NO SE PUDO MEDIR LA TEMPERATURA Y HUMEDAD");
    }
    else 
    {
      if (temperatura.toInt() >= limitTemperature)
      {
        miBot.sendMessage(GroupId, "\U0000203C\U0000203C  ALERTA DE TEMPERATURA  \U0000203C\U0000203C \n  Temperatura límite superada\U00002757 \n\n  Temperatura límite: " + String(limitTemperature) + " °C \n  Temperatura actual: " + temperatura + " °C");
        // miBot.sendMessage(GroupId, "**ALERTA** \U0000203C \U0000203C \nTemperatura: " + temperatura + "°C" );
      }
      else
      {
        if (temperatura.toInt() >= warningTemperature)
        { 
          miBot.sendMessage(GroupId, "\U000026A0\U000026A0ADVERTENCIA DE TEMPERATURA\U000026A0\U000026A0 \n  La Temperatura límite está a punto de ser superada \U000026A0 \n\n  Temperatura límite: " + String(limitTemperature) + " °C \n  Temperatura actual: " + temperatura + " °C");
          // miBot.sendMessage(msg.group.id, "\U000026A0\U000026A0ADVERTENCIA DE TEMPERATURA\U000026A0\U000026A0 \n  La Temperatura límite está a punto de ser superada \U000026A0 \n\n  Temperatura límite: " + temperatura + " °C \n  Temperatura actual: " + temperatura + " °C");
        }
      }
    }
  }

  if (miBot.getNewMessage(msg)) 
  {
    evento_tiempo_respuesta_antes = millis();
    if (msg.messageType == CTBotMessageText)
    {

      Serial << "ID: " << (String)msg.sender.id <<
				"\nfirstName: " << msg.sender.firstName <<
				"\nlastName: " << msg.sender.lastName <<
				"\nusername: " << msg.sender.username <<
				"\nMessage: " << msg.text <<
				"\nChat ID: " << msg.group.id <<
				"\nChat title: " << msg.group.title;

      Serial << "\nOpcion escogida: " <<  msg.text << "\n" ;
      if ( msg.text.equalsIgnoreCase("opciones") )
      {
        if (isKeyboardActive)
        { 
          miBot.sendMessage(msg.group.id, "Opciones disponibles", miTeclado);
        }
        else
        {
          miBot.sendMessage(msg.group.id, "Botón de opciones desplegado", miTecladoFijo);
          isKeyboardActive = true;
        }
      }
      else
      {
        miBot.sendMessage(msg.group.id, "Escriba 'opciones' para desplegar la tabla con todas las opciones disponibles. Recuerde que cada petición demora aproximadamente 3 segundos en responderse.");
      }
    } 
    else if (msg.messageType == CTBotMessageQuery) 
    {
      Serial << "ID: " << (String)msg.sender.id <<
				"\nfirstName: " << msg.sender.firstName <<
				"\nlastName: " << msg.sender.lastName <<
				"\nusername: " << msg.sender.username <<
				"\nMessage: " << msg.text <<
				"\nChat ID: " << msg.group.id <<
				"\nChat title: " << msg.group.title;
      Serial << "\nOpcion escogida: " <<  msg.callbackQueryData << "\n" ;

      if (msg.callbackQueryData.equals("temperatura")) 
      {
        // miBot.endQuery(msg.callbackQueryID, "La temperatura es ", true);
        if (isnan(eventT.temperature))
          miBot.sendMessage(msg.group.id, msg.sender.firstName + ": " + "Error! No se pudo obtener la temperatura" );
        else
          miBot.sendMessage(msg.group.id, msg.sender.firstName + ": " + "La temperatura es " + temperatura + "°C" );
      } 
      else if (msg.callbackQueryData.equals("humedad")) 
      {
        // Serial.println(" Humedad");
        //digitalWrite(led, LOW);
        if (isnan(eventH.relative_humidity))
          miBot.sendMessage(msg.group.id, msg.sender.firstName + ": " +"Error! No se pudo obtener la humedad" );
        else
          miBot.sendMessage(msg.group.id, msg.sender.firstName + ": " +"La Humedad es " + humedad + "%" );
        // miBot.endQuery(msg.callbackQueryID, "La humedad es ");
      }
      else if (msg.callbackQueryData.equals("apagar_led")) 
      {
        if (estado_led == 1)
        {
          digitalWrite(Led, LOW);
          miBot.sendMessage(msg.group.id, msg.sender.firstName + ": " +"Led apagado");
          miBot.endQuery(msg.callbackQueryID, "Led apagado");
        }
        else
        {
          miBot.endQuery(msg.callbackQueryID, "El led ya se encuentra apagado");
        }
      }
      else if (msg.callbackQueryData.equals("encender_led"))
      {
        if (estado_led == 0)
        {
          digitalWrite(Led, HIGH);
          miBot.sendMessage(msg.group.id, msg.sender.firstName + ": " + "Led encendido");
          miBot.endQuery(msg.callbackQueryID, "Led encendido");
        }
        else
        {
          miBot.endQuery(msg.callbackQueryID, "El led ya se encuentra encendido");
        }
      }
    }
    evento_tiempo_respuesta_despues = millis();
    evento_tiempo_diferencia = evento_tiempo_respuesta_despues - evento_tiempo_respuesta_antes;
    Serial << "Tiempo de respuesta: " << evento_tiempo_diferencia << " ms" << "\n";

    miBot.sendMessage(msg.group.id, "Tiempo de respuesta: " + String(evento_tiempo_diferencia) + " ms");
  }
  delay(1000);
}
