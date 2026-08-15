#include "SPI.h" 
#include "RF24.h" 
#include "nRF24L01.h" 

// Define Joystick Connections
#define JoyStick_1_X_PIN     A0 
#define JoyStick_1_Y_PIN     A1
#define JoyStick_2_X_PIN     A2 
#define JoyStick_2_Y_PIN     A3
//#define JoyStick_3_X_PIN     A4 
//#define JoyStick_3_Y_PIN     A5

// definice CE a CSN pinu modulu RF v RF-NANO
#define CE_PIN 7 
#define CSN_PIN 8

#define INTERVAL_MS_TRANSMISSION 50
// Nastaveni CE a SCN pinu v RF-NANO
RF24 radio(CE_PIN, CSN_PIN); 
// Adresa FR modulu
const byte address[6] = "00001"; 
//NRF24L01 buffer limit is 32 bytes (max struct size) 

const int TlApin = 2; //Prirazeni tlacitek k pinum
const int TlBpin = 3;
const int TlCpin = 4;
const int TlDpin = 5;

unsigned long debDelay = 50; //Zakmiy 50ms

struct tlacitko {
	int soucasny_platny; // Co bereme jako debounced stav 
	unsigned long debounce_start; // Kdy jsme zacali cekat na nejakou zmenu od soucasny_platny NEBO 0
	int kandidat; // Co jsme videli pri debounce_start, jen pokud deboucne_start != 0
};

void tlacitko_init(struct tlacitko *tl) {
	tl->soucasny_platny = HIGH;
	tl->debounce_start = 0;
}

// tlacitko_precteno zprocesuje primy vstup na pinu a vrati, jestli se zmenila hodnota tlacika.
// Pokud ano, vrati true a tl->soucasny_platny je aktualizovan.
bool tlacitko_precteno(struct tlacitko *tl, int precteno) {
	 // Zaciname debounce?
	 if (tl->debounce_start == 0) {
  	 if (precteno != tl->soucasny_platny) {
	  	tl->debounce_start = millis(); 
			tl->kandidat = precteno;
		 }
	 } else { // jsme v debounce
		 if ((millis() - tl->debounce_start) > debDelay) {
			tl->debounce_start = 0;
			if (precteno == tl->kandidat) {
				// debounce detekoval platnou zmenu.
				tl->soucasny_platny = precteno;
				return true;
			}
		}
	 }
	 return false;
}

struct tlacitko tlA;
struct tlacitko tlB;
struct tlacitko tlC;
struct tlacitko tlD;

int readTlA = HIGH; //Promenne pro precteni tlacitek
int readTlB = HIGH;
int readTlC = HIGH;
int readTlD = HIGH;


// Datová struktura pro 3 joysticky
struct payload { 
	 byte X1tx; 
	 byte Y1tx; 
	 byte J1tx; 
	 byte X2tx; 
	 byte Y2tx; 
	 byte J2tx; 
	 byte TlAtx;
	 byte TlBtx;
	 byte TlCtx;
	 byte TlDtx;
}; 
payload payload; 
void setup() 
{ 
	 Serial.begin(115200); 

	 pinMode(TlApin, INPUT_PULLUP); //Prirazeni INPUT_PULLUP pinum tlacitek
	 pinMode(TlBpin, INPUT_PULLUP);
	 pinMode(TlCpin, INPUT_PULLUP);
	 pinMode(TlDpin, INPUT_PULLUP);

   tlacitko_init(&tlA);
	 tlacitko_init(&tlB);
	 tlacitko_init(&tlC);
	 tlacitko_init(&tlD);

	 radio.begin(); 
	 //Append ACK packet from the receiving radio back to the transmitting radio 
	 radio.setAutoAck(false); //(true|false) 
	 //Set the transmission datarate 
	 radio.setDataRate(RF24_1MBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS) 
	 //Greater level = more consumption = longer distance 
	 Serial.println("Data rate = 1 Mbps");
	 // radio.setPALevel(RF24_PA_MAX); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX) 
	 radio.setPALevel(RF24_PA_HIGH);
	 Serial.println("Sila signalu = HIGH");
	 //Default value is the maximum 32 bytes 
	 radio.setPayloadSize(sizeof(payload)); 
	 Serial.print("Velikost datoveho bufferu: ");
	 Serial.print(sizeof(payload));
	 Serial.println();
	 //Act as transmitter 
	 radio.openWritingPipe(address); 
	 delay(1000);
	 radio.stopListening(); 
} 
void loop() 
{ 
	// Nacteni dat ze 3 joysticku a namapovani na hodnoty 0 - 255
	 payload.X1tx = map(analogRead(JoyStick_1_X_PIN), 0, 1023, 0, 255); 
	 payload.Y1tx = map(analogRead(JoyStick_1_Y_PIN), 0, 1023, 0, 255); 
	 payload.J1tx = 1; 
	 payload.X2tx = map(analogRead(JoyStick_2_X_PIN), 0, 1023, 0, 255);
	 payload.Y2tx = map(analogRead(JoyStick_2_Y_PIN), 0, 1023, 0, 255); 
	 payload.J2tx = 2; 
  
	 readTlA = digitalRead(TlApin); //Cteni stavu tlacitek na pinech
	 readTlB = digitalRead(TlBpin);
	 readTlC = digitalRead(TlCpin);
	 readTlD = digitalRead(TlDpin);

	 //Pokud se stav tlacitka zmenil (stisk)
	 if (tlacitko_precteno(&tlA, readTlA)) {
		Serial.print("Tlacitko A: ");
		Serial.println(tlA.soucasny_platny);
	 }

	 if (tlacitko_precteno(&tlB, readTlB)) {
		Serial.print("Tlacitko B: ");
		Serial.println(tlB.soucasny_platny);
	 }

	 if (tlacitko_precteno(&tlC, readTlC)) {
		Serial.print("Tlacitko C: ");
		Serial.println(tlC.soucasny_platny);
	 }

	 if (tlacitko_precteno(&tlD, readTlD)) {
		Serial.print("Tlacitko D: ");
		Serial.println(tlD.soucasny_platny);
	 }

	 payload.TlAtx = tlA.soucasny_platny;
	 payload.TlBtx = tlB.soucasny_platny;
	 payload.TlCtx = tlC.soucasny_platny;
	 payload.TlDtx = tlD.soucasny_platny;


// Odeslani dat joysticku na nrf24
	 radio.write(&payload, sizeof(payload)); 
	 Serial.println("DATA: ");
   Serial.print("Joystick č.:");
   Serial.println(payload.J1tx);
   Serial.print("X1:");
   Serial.println(payload.X1tx);
   Serial.print("Y1:");
   Serial.println(payload.Y1tx);

   Serial.println("-----------");
   Serial.print("Joystick č.:");
   Serial.println(payload.J2tx);
   Serial.print("X2:");
   Serial.println(payload.X2tx);
   Serial.print("Y2:");
   Serial.println(payload.Y2tx);

	 Serial.println("-----------");
   Serial.println("Tlacitka:");
   Serial.print("A: ");
	 Serial.println(tlA.soucasny_platny);
	 Serial.print("B: ");
   Serial.println(tlB.soucasny_platny);
	 Serial.print("C: ");
   Serial.println(tlC.soucasny_platny);
	 Serial.print("D: ");
   Serial.println(tlD.soucasny_platny);

   //Serial.println("-----------");
   //Serial.print("Joystick č.:");
   //Serial.println(payload.data9);
   //Serial.print("X3:");
   //Serial.println(payload.data7);
   //Serial.print("Y3:");
   //Serial.println(payload.data8);

	 Serial.println("Odeslana"); 
	 delay(INTERVAL_MS_TRANSMISSION); 
} 
