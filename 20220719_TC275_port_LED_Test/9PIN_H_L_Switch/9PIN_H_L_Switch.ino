#define PIN 9

void setup() {
  Serial.begin(9600); //  시리얼 설정
  pinMode(PIN, OUTPUT); 
}

void loop() {
  digitalWrite(PIN, LOW);
  Serial.print("L");
  delay(1000);
  digitalWrite(PIN, HIGH);
  Serial.print("H");
  delay(1000);
}
