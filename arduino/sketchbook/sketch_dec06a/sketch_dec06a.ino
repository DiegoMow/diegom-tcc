
int j = LOW;
  int i;

void setup() {
  Serial.begin(9600);
  for (i=2; i< 10; i++) {
    pinMode(i, OUTPUT);
  }
}

void loop() {
  if (j == LOW) {
    j = HIGH;
  }else{
    j = LOW;
  }
  for (i=2; i< 10; i++) {
    digitalWrite(i, j);
  }
  delay(1000);
}

