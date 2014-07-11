/*
 * Robot 2WD Firmware (with ORCP2)
 *
 * v.0.0.1
 *
 * http://robocraft.ru
 */

// define type of sensors connected to the board

// motors + encoders + bumper + voltage
#define DRIVE_BOARD

// debug  (use pins 3, 2  for Software Serial)
//#define DEBUG

#include "orcp2.h"
#include "robot_2wd.h"
#include "robot_sensors.h"

//#include <Wire.h>
//#include <Ultrasonic.h>

#define BAUDRATE 57600

// telemetry rate (Hz)
#define TELEMETRY_RATE 		50

// Convert the rate into an interval
const int TELEMETRY_INTERVAL = 1000 / TELEMETRY_RATE;

// Track the next time we send telemetry
unsigned long telemetry_time = 0;

#define MODE_FIRST_0D       0
#define MODE_SECOND_0A      1
#define MODE_TOPIC_L        2   // waiting for topic id
#define MODE_TOPIC_H        3
#define MODE_SIZE_L         4   // waiting for message size
#define MODE_SIZE_H         5
#define MODE_MESSAGE        6
#define MODE_CHECKSUM       7

int mode_ = 0;
int bytes_ = 0;
int topic_ = 0;
int index_ = 0;
int checksum_ = 0;

uint8_t message_in[128];
uint8_t message_out[256];

Robot_2WD robot_data;

//Ultrasonic ultrasonic(13, 12); // Trig - 123, Echo - 12
//int IRpin[4] = { 0, 1, 2, 3 };
//int battVoltPin = A7;

int send_sensors_telemetry();

int bumperPin[] = {5, 4};
int bumperState[] = {0, 0};

MOTOR motors[MOTORS_COUNT] = {
    { 12, 11, 10 },
    { 7,  8,  9 }
};

static const int FORWARD = HIGH;
static const int BACKWARD = LOW;

void motor_drive(int motor_id, int dir, int pwm);

void motor_drive(int motor_id, int pwm) 
{
    if(pwm >= 0) {
        motor_drive(motor_id, FORWARD, pwm);
    }
    else {
        motor_drive(motor_id, BACKWARD, (-1)*pwm);
    }
}

#if defined(DEBUG)
#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 2); // RX, TX
#endif //#if defined(DEBUG)

int send_message(int id, unsigned char* src, int src_size) 
{
    int l = orcp2::to_buffer(id, src, src_size, message_out, sizeof(message_out));
    int res = Serial.write(message_out, l);
    return res;
}

void make_message() 
{
    int i = 0;
    uint8_t val8 = 0;
    uint16_t val16 = 0;
    unsigned char* msg=0;
    uint16_t len = 0;

#if defined(DEBUG)
    mySerial.print("[i] msg: ");
    mySerial.println(topic_, HEX);
#endif //#if defined(DEBUG)  

    switch(topic_) {
    case ORCP2_PIN_MODE:
        pinMode(message_in[0], message_in[1]);
        break;
    case ORCP2_DIGITAL_READ:
        val8 = digitalRead ( message_in[0] );
        msg = message_out+ORCP2_PACKET_HEADER_LENGTH;
        msg[0] = message_in[0];
        msg[1] = val8;
        len = 2;
        send_message(ORCP2_DIGITAL_READ, message_out+ORCP2_PACKET_HEADER_LENGTH, len);
        break;
    case ORCP2_ANALOG_READ:
        val16 = analogRead(message_in[0]);
        msg = message_out+ORCP2_PACKET_HEADER_LENGTH;
        msg[0] = message_in[0];
        len = 1;
        orcp2::copy_int16(msg+1, val16);
        len += sizeof(val16);
        send_message(ORCP2_ANALOG_READ, message_out+ORCP2_PACKET_HEADER_LENGTH, len);
        break;
    case ORCP2_DIGITAL_WRITE:
        // digitalWrite
        digitalWrite(message_in[0], message_in[1]);
        break;
    case ORCP2_ANALOG_WRITE:
        val16 = message_in[1];
        val16 += (uint16_t)message_in[2] << 8;
        analogWrite(message_in[0], val16);
        break;
        //------------------------------------------------
    case ORCP2_MOTOR_WRITE:
        val8 = message_in[0]; // motor number
        val16 = message_in[1]; // value
        val16 += (uint16_t)message_in[2] << 8;
        motor_drive(val8, val16);
        break;
    case ORCP2_MOTORS_WRITE:
        val16 = message_in[0]; // value
        val16 += (uint16_t)message_in[1] << 8;
        for(i=0; i<MOTORS_COUNT; i++) {
            motor_drive(i, val16);
        }
        break;
    case ORCP2_MESSAGE_ROBOT_2WD_DRIVE:
        val8=0;
        for(i=0; i<MOTORS_COUNT; i++) {
            val16 = message_in[val8]; // value
            val16 += (uint16_t)message_in[val8+1] << 8;
            motor_drive(i, val16);
            val8 += 2;
        }
        break;
        //------------------------------------------------
    default:
        break;
    }
}

int send_string(char *src) 
{
    if(!src)
        return -1;
    return send_message(ORCP2_SEND_STRING, (unsigned char*)src, strlen(src));
}

int send_telemetry() 
{
    uint16_t len=0;
    len = serialize_robot_2wd( &robot_data,
                               message_out+ORCP2_PACKET_HEADER_LENGTH,
                               sizeof(message_out)-ORCP2_PACKET_HEADER_LENGTH );
    return send_message(ORCP2_MESSAGE_ROBOT_2WD_TELEMETRY, message_out+ORCP2_PACKET_HEADER_LENGTH, len);
}

#if 0

void Read_US() 
{
    robot_data.US[0] = ultrasonic.Ranging(CM);       // get distance
    if ((robot_data.US[0] < 1) || (robot_data.US[0] > 350)) {
        robot_data.US[0] = 350;
    }
}

void Read_IR() 
{
    for(int j=0; j<INFRARED_COUNT; j++) {
        robot_data.IR[j] = 65*pow(analogRead(IRpin[j])*0.0048828125, -1.10);
    }
}

void Read_Voltage() 
{
    //$TODO

    // 4,883mV & divider 1/6
    float val = (float)analogRead(battVoltPin)*4.883*6.000;

    robot_data.Voltage = (uint32_t)(val*1000);
}

#endif

#if defined(DRIVE_BOARD)

void motor_drive(int motor_id, int dir, int pwm)
{
    if(motor_id < 0 || motor_id >= MOTORS_COUNT)
        return;

#if defined(DEBUG)
    mySerial.print("[i] drive: id: ");
    mySerial.print(motor_id, DEC);
    mySerial.print(" dir: ");
    mySerial.print(dir, DEC);
    mySerial.print(" pwm: ");
    mySerial.println(pwm, DEC);
#endif //#if defined(DEBUG) 	

    if(dir == FORWARD) {
        digitalWrite(motors[motor_id].in1, HIGH);
        digitalWrite(motors[motor_id].in2, LOW);
    }
    else {
        digitalWrite(motors[motor_id].in1, LOW);
        digitalWrite(motors[motor_id].in2, HIGH);
    }
    analogWrite(motors[motor_id].enable, pwm);

    robot_data.PWM[motor_id] = pwm;
    if(dir == BACKWARD) {
        robot_data.PWM[motor_id] *= (-1);
    }
}

void Read_Bampers() 
{
    robot_data.Bamper = 0;
    for (int i=0; i<BAMPER_COUNT; i++) {
        bumperState[i] = digitalRead ( bumperPin[i]);
        if(bumperState[i]) {
            robot_data.Bamper = robot_data.Bamper | (1 << i);
        }
    }
}

#endif //#if defined(DRIVE_BOARD)

// based on IsTime() function - David Fowler, AKA uCHobby, http://www.uchobby.com 01/21/2012
// http://www.uchobby.com/index.php/2012/01/21/replacing-delay-in-arduino-sketches-istime-to-the-rescue/
//

#define TIMECTL_MAXTICKS  4294967295L
#define TIMECTL_INIT      0

bool is_Time(unsigned long &timeMark, unsigned long timeInterval)
{
    unsigned long timeCurrent;
    unsigned long timeElapsed;

    bool result = false;

    timeCurrent = millis();
    if( timeCurrent < timeMark ) { // Rollover detected
        // elapsed = all the ticks to overflow + all the ticks since overflow
        timeElapsed = (TIMECTL_MAXTICKS - timeMark) + timeCurrent;
    }
    else {
        timeElapsed = timeCurrent - timeMark;
    }

    if(timeElapsed >= timeInterval) {
        timeMark = timeCurrent;
        result = true;
    }

    return (result);
}

void setup() 
{                
    Serial.begin(BAUDRATE);

    memset(&robot_data, 0 , sizeof(robot_data));

#if defined(DRIVE_BOARD)

    int i;
    for(i=0; i<MOTORS_COUNT; i++) {
        pinMode(motors[i].in1, OUTPUT);
        pinMode(motors[i].in2, OUTPUT);
    }

    for(i=0; i<MOTORS_COUNT; i++) {
        motor_drive(i, FORWARD, 0);
    }

    for(i=0; i<BAMPER_COUNT; i++) {
        pinMode(bumperPin[i], INPUT);
    }

#endif //#if defined(DRIVE_BOARD)

#if defined(DEBUG)
    mySerial.begin(9600);
    mySerial.println(F("[i] Start"));
#endif //#if defined(DEBUG)
}

void loop() 
{
    // read message from serial
    if(Serial.available()) {
        int data = Serial.read();
        if(data < 0 ) {
            return;
        }

        if(mode_ != MODE_CHECKSUM)
            checksum_ ^= data;
        if( mode_ == MODE_MESSAGE ) {        /* message data being recieved */
            message_in[index_++] = data;
            bytes_--;
            if(bytes_ == 0) {                   /* is message complete? if so, checksum */
                mode_ = MODE_CHECKSUM;
            }
        }
        else if( mode_ == MODE_FIRST_0D ) {
            if(data == 0x0D) {
                mode_++;
            }
        }
        else if( mode_ == MODE_SECOND_0A ) {
            if(data == 0x0A) {
                checksum_ = 0;
                mode_++;
            }
            else {
                mode_ = MODE_FIRST_0D;
            }
        }
        else if( mode_ == MODE_TOPIC_L ) {  /* bottom half of topic id */
            topic_ = data;
            mode_++;
        }
        else if( mode_ == MODE_TOPIC_H ) {  /* top half of topic id */
            topic_ += data << 8;
            mode_++;
        }
        else if( mode_ == MODE_SIZE_L ) {   /* bottom half of message size */
            bytes_ = data;
            index_ = 0;
            mode_++;
        }
        else if( mode_ == MODE_SIZE_H ) {   /* top half of message size */
            bytes_ += data << 8;
            mode_ = MODE_MESSAGE;
            if(bytes_ == 0)
                mode_ = MODE_CHECKSUM;
        }
        else if( mode_ == MODE_CHECKSUM ) { /* do checksum */
            if(checksum_ == data) { // check checksum
                make_message();
            }
            mode_ = MODE_FIRST_0D;
        }
    } // if(Serial.available())

    if( is_Time(telemetry_time, TELEMETRY_INTERVAL) ) {
#if 0
        Read_US();
        Read_IR();
        Read_Voltage();
#endif

#if defined(DRIVE_BOARD)
        Read_Bampers();

        send_telemetry();
#endif	//#if defined(DRIVE_BOARD)
    }
}