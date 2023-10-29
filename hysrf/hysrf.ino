// Define the pin connections
#define echoPin 4 // Use the appropriate GPIO number for the echo pin
#define triggerPin 0 // Use the appropriate GPIO number for the trigger pin

long duration;
int distance;

void setup() {
  Serial.begin(9600);
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);
}

void loop() {
  // Ultrasonic Sensor
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.017 / 2;
  Serial.println("Jarak: " + String(distance) + " cm");

  delay(1000);
}
