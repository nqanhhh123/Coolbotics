#include <SoftwareSerial.h>
#include "DHT.h"
#define DHT11Pin 2
#define DHTType DHT11
DHT HT(DHT11Pin, DHTType);

uint32_t lastRead = 0;
uint32_t last_check = 0;
uint32_t lastSendData = 0;

int smoke;  // value -> smoke
float temp;
int ENA = 9;
int IN1 = 10;
int IN2 = 11;
int MQ2 = A0;
int buzz = 13;
SoftwareSerial SIM900(7, 8);  // 7 RX; 8TX (Nối chéo chân TX RX)
bool check = true, check1 = true, check2 = true, check3 = true , check4= true , check5 = true;
// check: Đã đến server chưa, check1: Có khói, check2: Nhiệt độ bất thường và có khói không, check4: Chỉ có nhiệt độ cao bất thường, check5: Bề mặt cảm biến nhiệt độ đã nóng đều chưa

void setup() {
  pinMode(IN2, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(buzz, OUTPUT);
  
  HT.begin();
  Serial.begin(9600);
  SIM900.begin(9600);
}

void message(String str){ // Hết 4 giây
  SIM900.println("AT+CMGF=1");  //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1 second
  SIM900.println("AT+CMGS=\"+84916066543\"\r"); // Replace x with mobile number
  delay(1000);
  SIM900.println(str);  // The SMS text you want to send
  delay(100);
  SIM900.println((char)26);  // ASCII code of CTRL+Z
  delay(1000);
}

void warning(int n) { // Hết n giây
  for (int i = 0; i < n; i++) {
    digitalWrite(buzz, HIGH);
    delay(500);
    digitalWrite(buzz, LOW);
    delay(500);
  }
}

void call() { //Hết 8 giây
  // Tắt máy
  SIM900.println("ATD+84916066543;"); // ATDxxxxxxxxxx; -- watch out here for semicolon at the end!!
  Serial.println("Calling..."); // print response over serial port
  delay(5000);
}

void spray() {  // Hết 16 giây
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, HIGH);
  analogWrite(ENA, 200);
  warning(15);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void loop() {
  if (check == true) {
    delay(5000); // N giây là thời gian từ khi bắt đầu đến server đầu tiên
    check = false;
  }

  if (check5 = true) { //Hết 8 giây
    delay(7500); // Cho bề mặt cảm biến nóng đều rồi mới đọc giá trị nhiệt độ
    temp = abs(HT.readTemperature()); 
    smoke = analogRead(MQ2);
    check5 = false;
  }

  // In các giá trị nhiệt độ và khói ra Serial để kiểm tra
  Serial.print(temp);
  Serial.print("  ");
  Serial.println(smoke);
  delay(500);
  
  if (temp >= 29 && smoke >= 80 && check2 == true) {  // Nhiệt độ bất thường và có khói
  // Hết 45 giây
      warning(5); // Hết 5 giây
      message("Phong server gap su co chay"); // Hết 4 giây
      delay(5000); // Tránh miss tin nhắn // Hết 5 giây
      call(); // Hết 8 giây
      spray();  // Hết 16 giây
      check2 = false;
  }
  else if (temp >= 29 && smoke < 80 && check4 == true) {  // Nhiệt độ bất thường
  // Hết 30 giây
      message("Phat hien server qua nhiet do"); // Hết 4 giây
      delay(2000);  // Tránh miss tin nhắn // Hết 2 giây
      spray(); // Hết 16 giây
      check4 = false;
  }
  else  if (temp < 29 && smoke >= 80 && check1 == true) {  //  Có khói
  // Hết 36 giây
    message("Phat hien phong server co khoi");  // Tránh miss tin nhắn // Hết 4 giây
    delay(5000); // Hết 5 giây
    call(); // Hết 8 giây
    warning(12); // Hết 12 giây
    check1 = false;
  }

  if (millis() - lastSendData > 65000) {  // Sau khi robot xử lí xong 1 server thì phải reset các biến check -> true
    check1 = true;
    check2 = true;
    check3 = true;
    check4 = true;
    check5 = true;
    lastSendData = millis();
  }
}