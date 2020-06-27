#define PIN_SIGNAL 15 //pin where locate Signal LED
#define PIN_OWIRE  13 //pin where locate temperature sensor
#define PIN_RESET  02 //reset pin
#define PIN_ADC    A0 //ADC pin

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(PIN_OWIRE);
DallasTemperature sensors(&oneWire);

#include <Homie.h>

const int TEMPERATURE_INTERVAL = 30;

unsigned long lastTemperatureSent = 0;
unsigned long lastSupplySent = 0;

HomieNode temperatureNode("temperature", "Temperature", "temperature");
HomieNode supplyNode("supply", "PowerSupply", "power-supply");


void loopHandler() {
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0) {
    sensors.requestTemperatures(); // получаем значение температуры от датчика
    float temperature = sensors.getTempCByIndex(0);
    Homie.getLogger() << "Temperature: " << temperature << " °C" << endl;
    temperatureNode.setProperty("degrees").send(String(temperature));
    lastTemperatureSent = millis();
  }
  if (millis() - lastSupplySent >= TEMPERATURE_INTERVAL * 1000UL || lastSupplySent == 0) {
    float volts = (analogRead(PIN_ADC)/1023.0)*5; // Volts from ADC
    Homie.getLogger() << "Supply voltage: " << volts << " V" << endl;
    supplyNode.setProperty("supply-voltage").send(String(volts));

    bool value = (volts > 3);
    supplyNode.setProperty("supply-present").send(value ? "true" : "false");
    Homie.getLogger() << "Supply present: " << (value ? "on" : "off") << endl;
        
    lastSupplySent = millis();
  }  
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("temperature-sensor", "1.0.0");
  Homie.setLoopFunction(loopHandler);
  Homie.setLedPin(PIN_SIGNAL, HIGH);
  
  temperatureNode.advertise("degrees").setName("Degrees")
                                      .setDatatype("float")
                                      .setUnit("ºC");
  
  pinMode(PIN_ADC, INPUT); // Initialize ADC for voltage measurement
  
  supplyNode.advertise("supply-voltage").setName("Volts")
                                      .setDatatype("float")
                                      .setUnit("V");
  
  supplyNode.advertise("supply-present").setName("SupplyPresent")
                                      .setDatatype("boolean");
  
  Homie.setResetTrigger(PIN_RESET, LOW, 2000); // before Homie.setup()
  Homie.setup();
  //Homie.reset();
  randomSeed(micros());
}

void loop() {
  Homie.loop();
}
