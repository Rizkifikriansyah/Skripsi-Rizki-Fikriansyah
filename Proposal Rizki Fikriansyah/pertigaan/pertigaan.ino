int ledmA = 2;
int ledkA = 3;
int ledhA = 4;

int ledmB = 5;
int ledkB = 6;
int ledhB = 7;

int ledmC = 8;
int ledkC = 9;
int ledhC = 10;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledmA, OUTPUT);
  pinMode(ledkA, OUTPUT);
  pinMode(ledhA, OUTPUT);

  pinMode(ledmB, OUTPUT);
  pinMode(ledkB, OUTPUT);
  pinMode(ledhB, OUTPUT);

  pinMode(ledmC, OUTPUT);
  pinMode(ledkC, OUTPUT);
  pinMode(ledhC, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  // SIKLUS ARAH A MENYALA
  digitalWrite(ledhA, HIGH);
  digitalWrite(ledmB, HIGH);
  digitalWrite(ledmC, HIGH);
  Serial.println("Arah A: HIJAU - Jalan");
  delay(4000);
  
  digitalWrite(ledhA, LOW);
  digitalWrite(ledkA, HIGH);
  Serial.println("Arah A: KUNING - Bersiap");
  delay(1000);
  
  digitalWrite(ledkA, LOW);
  digitalWrite(ledmA, HIGH);
  digitalWrite(ledmB, LOW);
  digitalWrite(ledmC, HIGH);

    // SIKLUS ARAH B MENYALA
  digitalWrite(ledhB, HIGH);
  Serial.println("Arah B: HIJAU - Jalan");
  delay(4000);
  
  digitalWrite(ledhB, LOW);
  digitalWrite(ledkB, HIGH);
  Serial.println("Arah B: KUNING - Bersiap");
  delay(1000);
  
  digitalWrite(ledkB, LOW);
  digitalWrite(ledmB, HIGH);
  digitalWrite(ledmC, LOW);

  // SIKLUS ARAH C MENYALA
  digitalWrite(ledhC, HIGH);
  Serial.println("Arah C: HIJAU - Jalan");
  delay(4000);
  
  digitalWrite(ledhC, LOW);
  digitalWrite(ledkC, HIGH);
  Serial.println("Arah C: KUNING - Bersiap");
  delay(1000);
  
  digitalWrite(ledkC, LOW);
  digitalWrite(ledmC, HIGH);
  digitalWrite(ledmA, LOW);

}