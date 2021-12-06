#include <Arduino.h>
#include <sht1xalt.h>
#include <SFE_BMP180.h>
#include "Wire.h"
#include "Max44009.h"


const float VALORE_NULLO = 9999.9;
#define PIN_VENTOLE 7
#define TEMPERATURA_ATTIVAZIONE_VENTOLE 0

//----------------Settaggio SENSORE SHT10 (temperatura esterna, umidità)----------------
#define dataPin 10
#define clockPin 11
#define clockPulseWidth 1
#define supplyVoltage sht1xalt::VOLTAGE_5V
//#define supplyVoltage sht1xalt::VOLTAGE_3V5
#define temperatureUnits sht1xalt::UNITS_CELCIUS
//#define temperatureUnits sht1xalt::UNITS_FAHRENHEIT
sht1xalt::Sensor sensor( dataPin, clockPin, clockPulseWidth, supplyVoltage, temperatureUnits);
//-------------------------------------------------------------------------------------

//----------------Settaggio SENSORE GY-68 BMP180 (temperatura interna, pressione)----------------
SFE_BMP180 pressure;
#define ALTITUDE 536 // Altitude of Electropeak Co. in meters
//---------------------------------------------------------------------------------------------

//----------------Settaggio SENSORE RILEVATORE UV (raggi uv)----------------
#define MIN_VALORE_NM 200
#define MAX_VALORE_NM 370
#define PIN_UV A1
//--------------------------------------------------------------------------


//----------------Settaggio SENSORE PIOGGIA (quantità di pioggia)----------------
#define MIN_VALORE 0
#define MAX_VALORE 100
#define PIN_PIOGGIA_ANALOGICO A2
#define PIN_PIOGGIA_DIGITALE 6

//--------------------------------------------------------------------------

//----------------Settaggio SENSORE MAX44009 (luminosità)----------------
Max44009 myLux(0x4A);
uint32_t lastDisplay = 0;
//--------------------------------------------------------------------------

String misurazioni ="";

void setup() {
  
  Serial.begin(115200);

  pinMode(PIN_VENTOLE, OUTPUT);
  digitalWrite(PIN_VENTOLE, LOW);

  //----Inizializzazione SENSORE SHT10(temperatura)-----
  sensor.configureConnection();
  // Reset the SHT1x, in case the Arduino was reset during communication:
  sensor.softReset();
  //---------------------------------------------------

  //-----Inizializzazione SENSORE GY-68 BMP180-----
  pressure.begin();
  //-----------------------------------------------

  //-----Inizializzazione SENSORE PIOGGIA-----
  pinMode(PIN_PIOGGIA_DIGITALE, INPUT);
  //------------------------------------------

  //-----Inizializzazione SENSORE max44009-----
  Wire.begin();
  Wire.setClock(100000);
  //-------------------------------------------
 

}

void loop() {
   misurazioni = letturaDatiGY68BMP180();
   if (Serial.available() > 0) {
    // read the incoming byte:
    char c = Serial.read();
    if(c == 's'){
      //Lettura dati
      misurazioni +=letturaDatiSHT10();
      misurazioni +=letturaDatiRaggiUV();
      misurazioni +=letturaDatiMAX44009();
      misurazioni += letturaDatiSensorePioggia();
      Serial.print(misurazioni);
    }
   }

}

//---------Lettura dati del sensore di temperatura e umidita----------
String letturaDatiSHT10(){
  float temp;
  float rh;
  sht1xalt::error_t err;
  
  delay(2000);
  
  err = sensor.measure(temp, rh);
  if (err) {
    sensor.softReset();
    temp = VALORE_NULLO;
    rh = VALORE_NULLO;
  }
  return "temperatura#"+String(temp)+"#umidita#"+String(rh)+"#";
}
//-------------------------------------------------------------------


//---------Lettura dati del sensore di temperatura interna e pressione----------
String letturaDatiGY68BMP180(){
  char status;
  double T=VALORE_NULLO, P, p0=VALORE_NULLO, a;
  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(T);

    if (status != 0)
        {
        status = pressure.startPressure(3);
        if (status != 0)
        {
          delay(status);
          status = pressure.getPressure(P, T);
          if (status != 0)
          {
            p0 = pressure.sealevel(P, ALTITUDE);
            
            if(T >= TEMPERATURA_ATTIVAZIONE_VENTOLE){
              digitalWrite(PIN_VENTOLE, HIGH);
            }
            else{
              digitalWrite(PIN_VENTOLE, LOW);
            }
          }
      }
    }
  }  
  return "temperatura_interna#"+String(T)+"#pressione#"+String(p0)+"#";
}
//-------------------------------------------------------------------


//---------Lettura dati del sensore di raggi ultravioletti----------
String letturaDatiRaggiUV(){
  //Valore del dato in mVolt
  float uv_voltage = analogRead(PIN_UV)*5/1024;
  float uv = map(uv_voltage, 0, 1, MIN_VALORE_NM, MAX_VALORE_NM);
  return "uv#"+String(uv)+"#";
}
//-------------------------------------------------------------------


//---------Lettura dati del sensore di pioggia----------
String letturaDatiSensorePioggia(){
  //Valore del dato in mVolt
  float q_pioggia = map(analogRead(PIN_PIOGGIA_ANALOGICO), 0, 1024, MIN_VALORE, MAX_VALORE);
  q_pioggia = MAX_VALORE - q_pioggia -8;
  if(q_pioggia < 0) {q_pioggia = 0;}
  return "pioggia#"+String(q_pioggia)+"#";
}
//------------------------------------------------------

//---------Lettura dati del sensore di luce----------
//Valori tra 0,045 a 188.000 lux
String letturaDatiMAX44009(){
  uint32_t interval = 1000;
  float v_lux;
  if (millis() - lastDisplay >= interval)
  {
    lastDisplay += interval;
    v_lux = myLux.getLux();
    int err = myLux.getError();
    if (err != 0)
    {
      v_lux = VALORE_NULLO;
    }
  }
  return "luce#"+String(v_lux)+"#";
}
//------------------------------------------------------
