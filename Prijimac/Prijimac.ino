#include <Wire.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#define CE_PIN 8
#define CSN_PIN 10 
#define INTERVAL_MS_SIGNAL_LOST 1000 
#define INTERVAL_MS_SIGNAL_RETRY 500 
RF24 radio(CE_PIN, CSN_PIN); 
const byte address[6] = "00001"; 

// Motor A Connections
int enA = 9;
int in1 = 3;
int in2 = 4;
 
// Motor B Connections
int enB = 6;
int in3 = 5;
int in4 = 7;

uint8_t buf[2];

int X1mapped = 0;
int Y1mapped = 0;

//NRF24L01 buffer limit is 32 bytes (max struct size) 
struct payload { 
	 byte X1rx; 
	 byte Y1rx; 
	 byte J1rx; 
	 byte X2rx; 
	 byte Y2rx; 
	 byte J2rx; 
	 byte TlArx;
	 byte TlBrx;
	 byte TlCrx;
	 byte TlDrx;
}; 
payload payload; 
/*
struct Arduino2 {
	 byte iX2tx; 
	 byte iY2tx; 
	 byte iJ2tx; 
	 byte iTlAtx;
	 byte iTlBtx;
	 byte iTlCtx;
	 byte iTlDtx;
};
Arduino2 Arduino2;
*/
unsigned long lastSignalMillis = 0;
unsigned long interval = 0; 

void setup() 
{ 
	 //Wire.begin(); //Spustí I2C jako master
	 Serial.begin(115200); 
	 pinMode(CE_PIN, OUTPUT);

	 // Set all the motor control pins to outputs
   pinMode(enA, OUTPUT);
   pinMode(enB, OUTPUT);
   pinMode(in1, OUTPUT);
   pinMode(in2, OUTPUT);
   pinMode(in3, OUTPUT);
   pinMode(in4, OUTPUT);
 
   buf[0] = 0; // Rychlost B
   buf[1] = 0; // Rychlost A

   buf[2] = 0; // Vpred = 0, Vzad = 1, Vpravo = 2, Vlevo = 3

	 radio.begin(); 
	 //Append ACK packet from the receiving radio back to the transmitting radio 
	 radio.setAutoAck(false); //(true|false) 
	 //Set the transmission datarate 
	 radio.setDataRate(RF24_1MBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS) 
	 //Greater level = more consumption = longer distance 
	 Serial.println("Data rate = 1 Mbps");
	 // radio.setPALevel(RF24_PA_MIN); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX) 
	 radio.setPALevel(RF24_PA_HIGH);
	 Serial.println("Sila signalu = HIGH");
	 //Default value is the maximum 32 bytes1 
	 radio.setPayloadSize(sizeof(payload)); 
	 Serial.print("Velikost datoveho bufferu: ");
   Serial.print(sizeof(payload));
	 Serial.println();
	 //Act as receiver 
	 radio.openReadingPipe(0, address); 
	 radio.startListening(); 
} 
void loop() 
{ 
	 unsigned long currentMillis = millis(); 
	 if (radio.available() > 0) { 
	   radio.read(&payload, sizeof(payload)); 
/*
		 Arduino2.iX2tx = payload.X2rx;
		 Arduino2.iY2tx = payload.Y2rx; 
		 Arduino2.iJ2tx = payload.J2rx; 
		 Arduino2.iTlAtx = payload.TlArx;
		 Arduino2.iTlBtx = payload.TlBrx;
		 Arduino2.iTlCtx = payload.TlCrx;
		 Arduino2.iTlDtx = payload.TlDrx;

		 Wire.beginTransmission(2);
		 Wire.write(Arduino2.iX2tx);
		 Wire.write(Arduino2.iY2tx);
		 Wire.write(Arduino2.iJ2tx);
		 Wire.write(Arduino2.iTlAtx);
		 Wire.write(Arduino2.iTlBtx);
		 Wire.write(Arduino2.iTlCtx);
		 Wire.write(Arduino2.iTlDtx);
		 Wire.endTransmission();*/

		 //Odeslat pres I2C Joystick 2 + Tlacitka
		 if (payload.X1rx > 110 && payload.X1rx < 135) {
			X1mapped = 0;
			buf[0] = 0;
			buf[1] = 0;
		 }
		 if (payload.Y1rx > 110 && payload.Y1rx < 135) {
			Y1mapped = 0;
			buf[0] = 0;
			buf[1] = 0;
		 }
		 if (payload.X1rx < 110) {
			X1mapped = map(payload.X1rx, 0, 110, 255, 0);
			buf[2] = 2;
			buf[0] = X1mapped;
			buf[1] = X1mapped;
		 }
		 if (payload.X1rx > 135) {
			X1mapped = map(payload.X1rx, 135, 255, 0, 255);
			buf[2] = 3;
			buf[0] = X1mapped;
			buf[1] = X1mapped;
		 }
		 if (payload.Y1rx < 110) {
			Y1mapped = map(payload.Y1rx, 0, 110, 255, 0);
			buf[2] = 0;
			buf[0] = Y1mapped;
			buf[1] = Y1mapped;
		 }
		 if (payload.Y1rx > 135) {
			Y1mapped = map(payload.Y1rx, 135, 255, 0, 255);
			buf[2] = 1;
			buf[0] = Y1mapped;
			buf[1] = Y1mapped;
		 }


	   Serial.println("Prijata data: "); 
	   Serial.print("Joystick č.:"); 
	   Serial.println(payload.J1rx); 
	   Serial.print("X1:"); 
	   Serial.println(payload.X1rx); 
		 Serial.print("Y1:"); 
	   Serial.println(payload.Y1rx); 
		 Serial.print("Namapovane X1:");
		 Serial.println(X1mapped);
		 Serial.print("Namapovane Y1:");
		 Serial.println(Y1mapped);
		 Serial.print("Stav buf[2]:");
		 Serial.println(buf[2]);
		 Serial.print("Stav buf[0]:");
		 Serial.println(buf[0]);
		 Serial.print("Stav buf[1]:");
		 Serial.println(buf[1]);
	   Serial.print("Joystick č.:"); 
	   Serial.println(payload.J2rx); 
		 Serial.print("X2:"); 
	   Serial.println(payload.X2rx); 
	   Serial.print("Y2:"); 
	   Serial.println(payload.Y2rx); 
		 Serial.print("Tlacitko A: ");
		 Serial.println(payload.TlArx);
		 Serial.print("Tlacitko B: ");
		 Serial.println(payload.TlBrx);
		 Serial.print("Tlacitko C: ");
		 Serial.println(payload.TlCrx);
		 Serial.print("Tlacitko D: ");
		 Serial.println(payload.TlDrx);
		
		 // Drive Motors
     analogWrite(enA, buf[1]);
     analogWrite(enB, buf[0]);

		 if (buf[2] == 0) {
      // Motory dopredu
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
     }
     if (buf[2] == 1) {
      // Motory dozadu
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
     }
     if (buf[2] == 2) {
      // Motory doprava
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
     }
     if (buf[2] == 3) {
      // Motory doleva
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
     }

	   lastSignalMillis = currentMillis; 
		 interval = currentMillis - lastSignalMillis;
     Serial.print("Interval:"); 
	   Serial.println(interval); 
	 } 
	 if (currentMillis != 0 && currentMillis - lastSignalMillis > INTERVAL_MS_SIGNAL_LOST) { 
	   lostConnection(); 
	 } 
} 
void lostConnection() 
{ 
	 Serial.println("We have lost connection, preventing unwanted behavior"); 
	 delay(INTERVAL_MS_SIGNAL_RETRY); 
} 
