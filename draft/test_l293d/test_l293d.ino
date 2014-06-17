/*
 * test CraftDuino and L293 or L298 module
 */

struct MOTOR    // структура для хранения номеров pin-ов, к которым подключены моторчики
{
  int in1;      // INPUT1
  int in2;      // INPUT2
  int enable;   // ENABLE1
};

// определяем порты, к которым подключены моторчики
MOTOR MOTOR1 = { 12, 11, 10 };
MOTOR MOTOR2 = { 7, 8, 9 };

void drive1(int speed=254);
void drive2(int speed=254);
void forward1(int speed=254);
void forward2(int speed=254);
void back1(int speed=254);
void back2(int speed=254);

void setup()
{
  Serial.begin(9600);
  pinMode(MOTOR1.in1, OUTPUT); // настраиваем выводы
  pinMode(MOTOR1.in2, OUTPUT); // на ВЫВОД
  pinMode(MOTOR2.in1, OUTPUT);
  pinMode(MOTOR2.in2, OUTPUT);
}

void loop()
{
  forward1(50);   // вращаем моторчик вперёд
  forward2(50);
  delay(2000);
  back1(50);      // вращаем моторчик назад
  back2(50);
  delay(2000);
  //forward2();   // а теперь опять вращаем второй моторчик вперёд
}

void drive1(int speed)
{
  if(speed > 0) {
    // forward
    digitalWrite(MOTOR1.in1, HIGH);
    digitalWrite(MOTOR1.in2, LOW);
  }
  else {
    // back
    digitalWrite(MOTOR1.in1, LOW);
    digitalWrite(MOTOR1.in2, HIGH);
  }
   analogWrite(MOTOR1.enable, speed);
}

void drive2(int speed)
{
  if(speed > 0) {
    // forward
    digitalWrite(MOTOR2.in1, HIGH);
    digitalWrite(MOTOR2.in2, LOW);
  }
  else {
    // back
    digitalWrite(MOTOR2.in1, LOW);
    digitalWrite(MOTOR2.in2, HIGH);
  }
   analogWrite(MOTOR2.enable, speed);
}

void forward1(int speed) // первый вперёд
{
  digitalWrite(MOTOR1.in1, HIGH);
  digitalWrite(MOTOR1.in2, LOW);
  analogWrite(MOTOR1.enable, speed);
}

void forward2(int speed) // второй вперёд
{
  digitalWrite(MOTOR2.in1, HIGH);
  digitalWrite(MOTOR2.in2, LOW);
  analogWrite(MOTOR2.enable, speed);
}

void back1(int speed) // первый назад
{
  digitalWrite(MOTOR1.in1, LOW);
  digitalWrite(MOTOR1.in2, HIGH);
  analogWrite(MOTOR1.enable, speed);
}

void back2(int speed) // второй назад
{
  digitalWrite(MOTOR2.in1, LOW);
  digitalWrite(MOTOR2.in2, HIGH);
  analogWrite(MOTOR2.enable, speed);
}
