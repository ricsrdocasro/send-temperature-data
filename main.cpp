//importação de biblioteca
#include <Arduino.h> //caso não utilizar Arduino IDE
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <DHT.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

const int pinoDHT = 13;

DHT dht{pinoDHT, DHT11};

//configuração de padrão de segurança
//descomentar a configuração desejada
#define USE_OTAA
//#define USE_ABP

//#ifdef USE_ABP
//  static const PROGMEM u1_t /NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//  static const u1_t PROGMEM APPSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//  static const u4_t DEVADDR = 0x0000000;
//  void os_getArtEui (u1_t* buf) { }
//  void os_getDevEui (u1_t* buf) { }
//  void os_getDevKey (u1_t* buf) { }
//#endif

#ifdef USE_OTAA
  static const u1_t PROGMEM APPEUI[8] = { 0x78, 0x56, 0x64, 0x58, 0x17, 0x42, 0x45, 0x00 }; //lsb format
  void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
  static const u1_t PROGMEM DEVEUI[8]  = { 0x4A, 0x62, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 }; // lsb format
  void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
  static const u1_t PROGMEM APPKEY[16] = { 0x86, 0x31, 0xA0, 0x86, 0x46, 0xD4, 0x9D, 0xCE, 0x06, 0xCA, 0xD4, 0x17, 0x93, 0xA1, 0x14, 0xC5 }; //msb format
  void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }
#endif

//objeto que será mandado para a função do_send()
static osjob_t sendjob;

//intervalo de envio
const unsigned TX_INTERVAL = 40;

// Mapa de pinos Heltec ESP32Lora v2
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14, 
    .dio = {26, 35, 34},
};

void do_send(osjob_t *j);

// Callback de evento: todo evento do LoRaAN irá chamar essa
// callback, de forma que seja possível saber o status da
// comunicação com o gateway LoRaWAN.
void onEvent(ev_t ev)
{
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev)
    {
    case EV_SCAN_TIMEOUT:
        Serial.println(F("EV_SCAN_TIMEOUT"));
        break;
    case EV_BEACON_FOUND:
        Serial.println(F("EV_BEACON_FOUND"));
        break;
    case EV_BEACON_MISSED:
        Serial.println(F("EV_BEACON_MISSED"));
        break;
    case EV_BEACON_TRACKED:
        Serial.println(F("EV_BEACON_TRACKED"));
        break;
    case EV_JOINING:
        Serial.println(F("EV_JOINING"));
        break;
    case EV_JOINED:
        Serial.println(F("EV_JOINED"));
        break;
    case EV_JOIN_FAILED:
        Serial.println(F("EV_JOIN_FAILED"));
        LMIC_setLinkCheckMode(0);
        break;
    case EV_REJOIN_FAILED:
        Serial.println(F("EV_REJOIN_FAILED"));
        break;
    case EV_TXCOMPLETE:
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        if (LMIC.txrxFlags & TXRX_ACK)
            Serial.println(F("Received ack"));
        if (LMIC.dataLen)
        {
            Serial.print(F("Received "));
            Serial.print(LMIC.dataLen);
            Serial.println(F(" bytes of payload"));
        }
        if (LMIC.dataLen == 1) 
        {
            uint8_t dados_recebidos = LMIC.frame[LMIC.dataBeg + 0];
            Serial.print(F("Dados recebidos: "));
            Serial.write(dados_recebidos);
        }
        // Agenda a transmissão automática com intervalo de TX_INTERVAL
        os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
        break;
    case EV_LOST_TSYNC:
        Serial.println(F("EV_LOST_TSYNC"));
        break;
    case EV_RESET:
        Serial.println(F("EV_RESET"));
        break;
    case EV_RXCOMPLETE:
        Serial.println(F("EV_RXCOMPLETE"));
        break;
    case EV_LINK_DEAD:
        Serial.println(F("EV_LINK_DEAD"));
        break;
    case EV_LINK_ALIVE:
        Serial.println(F("EV_LINK_ALIVE"));
        break;
    case EV_TXSTART:
        Serial.println(F("EV_TXSTART"));
        break;
    case EV_TXCANCELED:
        Serial.println(F("EV_TXCANCELED"));
        break;
    case EV_RXSTART:
        break;
    case EV_JOIN_TXCOMPLETE:
        Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
        break;
    default:
        Serial.print(F("Unknown event: "));
        Serial.println((unsigned)ev);
        break;
    }
}

void do_send(osjob_t *j)
{    
    // Verifica se não está ocorrendo uma transmissão no momento TX/RX
    if (LMIC.opmode & OP_TXRXPEND){
        Serial.println(F("OP_TXRXPEND, not sending"));
    }
    else{
        uint8_t payload[4];
        //uint32_t humidity = dht.readHumidity(false) * 100;
        float temperature = dht.readTemperature();

        Serial.println(temperature);

        if(0 < temperature < 100){
            temperature = temperature * 100;
        }

        uint lng = temperature;
        payload[0] = (byte) ((lng & 0xFF000000) >> 24 );
        payload[1] = (byte) ((lng & 0x00FF0000) >> 16 );
        payload[2] = (byte) ((lng & 0x0000FF00) >> 8  );
        payload[3] = (byte) ((lng & 0X000000FF)       );
        Serial.println(payload[0]);
        Serial.println(payload[1]);
        Serial.println(payload[2]);
        Serial.println(payload[3]);
        
        //envio
        LMIC_setTxData2(1, payload, sizeof(payload), 0);
        Serial.println(F("Sended"));
    }
}

void setup()
{
    #ifdef USE_ABP
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession(0x13, DEVADDR, nwkskey, appskey);
    #endif

    Serial.begin(9600);
    Serial.println(F("Starting"));

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    do_send(&sendjob); //Start
}

void loop()
{
    os_runloop_once();

}