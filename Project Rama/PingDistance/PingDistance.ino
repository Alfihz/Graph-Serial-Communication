#include <Wire.h>
#define F_CPU 16000000UL


// ---------------------------------------------------------------------------
// Example NewPing library sketch that does a ping about 20 times per second.
// ---------------------------------------------------------------------------

#include <NewPing.h>
#include <TimerOne.h>

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
volatile unsigned int TimPingMs=0;
char CZButton;




//----------------nunchuck------

//
// These are suitable defaults for most nunchuks, including knockoffs.
// If you intend to use the same nunchuk all the time and demand accu-
// racy, it is worth your time to measure these on your own.
//     If you may want to use various nunchuks, you may want to
// calibrate using functions
//   nunchuk_calibrate_joy()
//   nunchuk_calibrate_accelxy()
//   nunchuk_calibrate_accelz()
//
#define DEFAULT_CENTRE_JOY_X 124
#define DEFAULT_CENTRE_JOY_Y 132
#define ACCEL_ZEROX 490
#define ACCEL_ZEROY 500
#define ACCEL_ZEROZ 525

//
// Global vars are kept to a minimum.
//
uint8_t ctrlr_type[6];   // Used externally?
uint8_t nunchuk_buf[6];  // Keeps data payload from nunchuk
// Accelerometer values and callibration centres:
uint16_t accel_zerox, accel_zeroy, accel_zeroz;
// Joystick values and calibration centres:
int joy_x, joy_y, joy_zerox, joy_zeroy;

//
//
// Uses port C (analog in) pins as power & ground for nunchuk
//
void nunchuk_setpowerpins(){
	#define pwrpin PORTC3
	#define gndpin PORTC2
	DDRC |= _BV(pwrpin) | _BV(gndpin);
	PORTC &=~ _BV(gndpin);
	PORTC |=  _BV(pwrpin);
	delay(100); // wait for things to stabilize
	}

//
//
// Initialize and join the I2C bus, and tell the nunchuk we're
// talking to it. This function will work both with Nintendo
// nunchuks, or knockoffs.
//
// See http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1264805255
//

void nunchuk_init(){
	Wire.begin();
	delay(1);
	Wire.beginTransmission(0x52);  // device address
	#if (ARDUINO >= 100)
		Wire.write((uint8_t)0xF0);  // 1st initialisation register
		Wire.write((uint8_t)0x55);  // 1st initialisation value
		Wire.endTransmission();
		delay(1);
		Wire.beginTransmission(0x52);
		Wire.write((uint8_t)0xFB);  // 2nd initialisation register
		Wire.write((uint8_t)0x00);  // 2nd initialisation value
	#else
		Wire.send((uint8_t)0xF0);   // 1st initialisation register
		Wire.send((uint8_t)0x55);   // 1st initialisation value
		Wire.endTransmission();
		delay(1);
		Wire.beginTransmission(0x52);
		Wire.send((uint8_t)0xFB);   // 2nd initialisation register
		Wire.send((uint8_t)0x00);   // 2nd initialisation value
	#endif
	Wire.endTransmission();
	delay(1);
	//
	// Set default calibration centres:
	//
	joy_zerox = DEFAULT_CENTRE_JOY_X;
	joy_zeroy = DEFAULT_CENTRE_JOY_Y;
	accel_zerox = ACCEL_ZEROX;
	accel_zeroy = ACCEL_ZEROY;
	accel_zeroz = ACCEL_ZEROZ;
	}

//
//
// T.T.
// Standard nunchuks use a byte-wise encryption using bit-wise XOR
// with 0x17. This function decodes a byte.
//
// This function is not needed since the way we initialize the nunchuk
// does not XOR encrypt the bits.
//
//static inline char nunchuk_decode_byte (char x)
//	{
//	x = (x ^ 0x17) + 0x17;
//	return x;
//	}

static void nunchuk_send_request()
{
	Wire.beginTransmission(0x52);// transmit to device 0x52
	#if (ARDUINO >= 100)
		Wire.write((uint8_t)0x00);// sends one byte
	#else
		Wire.send((uint8_t)0x00);// sends one byte
	#endif
	Wire.endTransmission();// stop transmitting
}

//
//
// Gets data from the nunchuk and packs it into the nunchuk_buff byte
// aray. That array will be processed by other functions to extract
// the data from the sensors and analyse.
//
int nunchuk_get_data(){
	int cnt=0;// Request six bytes from the chuck.
	Wire.requestFrom (0x52, 6);
	while (Wire.available ()){
	  // receive byte as an integer
		#if (ARDUINO >= 100)
			nunchuk_buf[cnt] = Wire.read();
		#else
			nunchuk_buf[cnt] = Wire.receive();
		#endif
                //debug nunchuck
                //Serial.write(nunchuk_buf[cnt]);

		cnt++;
		}

	Wire.beginTransmission(0x52);// transmit to device 0x52
	#if (ARDUINO >= 100)
		Wire.write((uint8_t)0x00);// sends one byte
	#else
		Wire.send((uint8_t)0x00);// sends one byte
	#endif
	Wire.endTransmission();// stop transmitting

	if (cnt >= 5)
		{
		return 1;   // success
		}
	return 0; // failure
	}

//
//
//
// Calibrate joystick so that we read the centre position as (0,0).
// Otherwise, we use the default values from the header.
//
void nunchuk_calibrate_joy()
	{
	joy_zerox = joy_x;
	joy_zeroy = joy_y;
	}

// Returns c and z button states: 1=pressed, 0=not
// The state is in the two least significant bits of the 6th byte.
// In the data, a 1 is unpressed and 0 is pressed, so this will be
// reversed. These functions use a bitwise AND to determine the value
// and then the () ? true : false; conditional structure to pass out
// the appropriate state.
//
static inline unsigned int nunchuk_zbutton()
	{
	return ((nunchuk_buf[5] >> 0) & 1) ? 0 : 1;
	}

static inline unsigned int nunchuk_cbutton()
	{
	return ((nunchuk_buf[5] >> 1) & 1) ? 0 : 1;
	}

//
//
// Returns the raw x and y values of the the joystick, cast as ints.
//
static inline int nunchuk_joy_x()
	{
	return (int) nunchuk_buf[0];
	}

static inline int nunchuk_joy_y()
	{
	return (int) nunchuk_buf[1];
	}

//
//
// Return calibrated x and y values of the joystick.
//
static inline int nunchuk_cjoy_x()
	{
	return (int)nunchuk_buf[0] - joy_zerox;
	}

static inline int nunchuk_cjoy_y()
	{
	return (int)nunchuk_buf[1] - joy_zeroy;
	}

//
//
// Returns the raw 10-bit values from the 3-axis accelerometer sensor.
// Of the six bytes recieved in a data payload from the nunchuk, bytes
// 2, 3 and 4 are the most significant 8 bits of each 10-bit reading.
// The final two bits are stored in the 6th bit along with the states
// of the c and z button. These functions take the most significant
// 8-bits and stacks it into a 16 bit unsigned integer, and then tacks
// on the least significant bits from the 6th byte of the data
// payload.
// 
// Load the most sig digits into a blank 16-bit unsigned int leaving
// two bits in the bottom ( via a 2-bit shift, << 2) for the least sig
// bits:
// 	0x0000 | nunchuk_buff[*] << 2
// Add to the above, the least sig bits. The code for x:
// 	nunchuk_buf[5] & B00001100
// for example selects the 3rd and 4th bits from the 6th byte of the
// payload to be concatinated with nunchuk_buff[2] to complete the 10-
// bit datum for a given axis.
//
static inline uint16_t nunchuk_accelx(){
	return (  0x0000 | ( nunchuk_buf[2] << 2 ) +
		( ( nunchuk_buf[5] & B00001100 ) >> 2 )  );
	}

static inline uint16_t nunchuk_accely(){
	return (  0x0000 ^ ( nunchuk_buf[3] << 2 ) +( ( nunchuk_buf[5] & B00110000 ) >> 4 )  );
	}

static inline uint16_t nunchuk_accelz(){
	return (  0x0000 ^ ( nunchuk_buf[4] << 2 ) +( ( nunchuk_buf[5] & B11000000 ) >> 6 )  );
}

//
//
// Returns the x,y and z accelerometer values with calibration values
// subtracted.
//
static inline int nunchuk_caccelx(){
		return (int)(nunchuk_accelx() - accel_zerox);
	}

static inline int nunchuk_caccely(){
		return (int)(nunchuk_accely() - accel_zeroy);
	}

static inline int nunchuk_caccelz(){
		return (int)(nunchuk_accelz() - accel_zeroz);
	}

//
//
// Returns joystick angle in degrees. It uses the ratio of calibrated
// x and y potentiometer readings to find the angle, zero being direct
// right (positive x) and measured counter-clockwise from there.
//
// If the atan2 function returns a negative angle, it is rotated back
// into a positive angle. For those unfamiliar, the atan2 function
// is a more inteligent atan function which quadrant the vector <x,y>
// is in, and returns the appropriate angle.
//
static inline int nunchuk_joyangle(){
	double theta;
	theta = atan2( nunchuk_cjoy_y(), nunchuk_cjoy_x() );
	while (theta < 0) theta += 2*M_PI;
	return (int)(theta * 180/M_PI);
}

//
//
// Returns roll angle in degrees. Under the assumption that the
// only acceleration detected by the accelerometer is acceleration due
// to gravity, this function uses the ratio of the x and z
// accelerometer readings to gauge pitch. This only works while the
// nunchuk is being held still or at constant velocity with zero ext-
// ernal force.
//
static inline int nunchuk_rollangle(){
	return (int) (  atan2( (double) nunchuk_caccelx(),(double) nunchuk_caccelz() ) * 180 / M_PI  );
}

//
//
// Returns pitch angle in degrees. Under the assumption that the
// only acceleration detected by the accelerometer is acceleration due
// to gravity, this function uses the ratio of the y and z
// accelerometer readings to gauge pitch.  This only works while the
// nunchuk is being held still or at constant velocity with zero ext-
// ernal force.
//
static inline int nunchuk_pitchangle(){
       return (int) (  atan2( (double) nunchuk_caccely(),(double)nunchuk_caccelz() ) * 180 / M_PI  );
}
void nunchuk_calibrate_accelxy(){
	accel_zerox = nunchuk_accelx();
	accel_zeroy = nunchuk_accely();
}
void nunchuk_calibrate_accelz()	{
     accel_zeroz = nunchuk_accelz();
}

#define MAX_ZIN 40 
unsigned int FIR(unsigned int in){
static unsigned int zIn[MAX_ZIN+1];
long zSum=0;
char i;
     //Collect data
     for(i=MAX_ZIN;i>0;i--){
        zIn[i]=zIn[i-1];
     }
     zIn[0]=in;
     
     //sum
     for(i=0;i<MAX_ZIN;i++){
         zSum=zSum+zIn[i];
     }
     return (zSum/MAX_ZIN);
}

enum eCmd{CMD_NONE,CMD_PING,CMD_BUTTON_C,CMD_BUTTON_Z};

void SendData(unsigned int dataIn){
     Serial.write(0xF2);
     Serial.write(CZButton);     //CMD <0x01> for PING data
     Serial.write(dataIn>>8);
     Serial.write(dataIn);     
     Serial.write(0xF3);     
}


#define MAX_DATA 100
struct TData{
  char Command;
  unsigned int Data;
};

struct TMailbox{
    TData InData[MAX_DATA];
    char Count;
};

TMailbox MailBox;

void InsertMailBox(struct TData DataIn){
     if (MailBox.Count<MAX_DATA){
         MailBox.InData[MailBox.Count].Command=DataIn.Command;
         MailBox.InData[MailBox.Count].Data=DataIn.Data;
         MailBox.Count++;
     }else Serial.println("MailBoxFull ");
}

void SendMailBox(){
     char data_cmd;
     unsigned int data_in;     
     if (MailBox.Count>0){
         data_cmd = MailBox.InData[0].Command;
         data_in  = MailBox.InData[0].Data;  
         Serial.write(0x02);
         Serial.write(data_cmd);     //CMD <0x01> for PING data
         Serial.write(data_in>>8);
         Serial.write(data_in);     
         Serial.write(0x03);      
     }              
}

void ShiftMailBoxQueue(){
     char i;
     if (MailBox.Count>0){
         for(i=0;i<MailBox.Count-1;i++){
             MailBox.InData[i].Command = MailBox.InData[i+1].Command;
             MailBox.InData[i].Data    = MailBox.InData[i+1].Data;
         }MailBox.Count--;
     }     
}


void SendPing(char data_cmd, unsigned int data_in){
         Serial.write(0x02);
         Serial.write(data_cmd);     //CMD <0x01> for PING data
         Serial.write(data_in>>8);
         Serial.write(data_in);     
         Serial.write(0x03);      
}

void InsertPING(unsigned int pingUs){
     //SendPing(CMD_PING,pingUs);

     struct TData AData;
     AData.Command = CMD_PING;
     AData.Data = pingUs;     
     InsertMailBox(AData);

}

void SendNunchuck(char data_cmd, unsigned int data_in){
         Serial.write(0x02);
         Serial.write(data_cmd);     //CMD <0x01> for PING data
         Serial.write(data_in>>8);
         Serial.write(data_in);     
         Serial.write(0x03);      
}

void InsertWII(char buttonCMD, unsigned int data){
     //SendNunchuck(buttonCMD,data);
     
     struct TData AData;
     AData.Command = buttonCMD;
     AData.Data = data;     
     InsertMailBox(AData);
}

enum eMailBox{mbNone,mbIble,mbSendItem};
void FMailBox(){
static char stMailbox=mbIble;

     switch(stMailbox){
         case mbIble:
              if (MailBox.Count>0)
                  stMailbox=mbSendItem;
              break;
         case mbSendItem:
              SendMailBox();
              ShiftMailBoxQueue();
              stMailbox=mbIble;
              break;
     }
}


enum eSTLoop{pingNone,pingRead,pingInitDelay,pingDelay};
#define PING_DELAY 5
#define PING_CNT 100

void FPing(){
  static char stPING=pingRead;
     static unsigned int  iTimExp,TimPingMs=0;
     
     iTimExp++;
     if( iTimExp>PING_CNT){
         iTimExp=0;
         TimPingMs++;
     }
  
  switch(stPING){
    case pingRead:
         //InsertPING(FIR(sonar.ping()));
         SendData(FIR(sonar.ping()));
         TimPingMs=0;
         digitalWrite( 13, digitalRead( 13 ) ^ 1 );   
         stPING=pingDelay;
         break;
    case pingDelay:
         if (TimPingMs>PING_DELAY)
             stPING=pingRead;         
         break;         
  }
}


#define WII_MAX_DELAY 60
enum eNunchuck{ncNone,ncInit,ncDelay,ncReadNunChuck};



void FNunChuck(){
     static char zstNunchuck,stNunchuck=ncInit;
     static unsigned int  iTimExp,TimWiiDelay=0;
     static char ButtonC,ButtonZ;
     static char zButtonC,zButtonZ;
     
     /*if (zstNunchuck!=stNunchuck){
         zstNunchuck=stNunchuck;
         Serial.println(stNunchuck);
     }*/
     
     iTimExp++;
     if( iTimExp>100){
         iTimExp=0;
         TimWiiDelay++;
     }
     
     switch(stNunchuck){
        case ncInit:
             //Serial.println("ncInit");
             nunchuk_setpowerpins();
             nunchuk_init();
             stNunchuck=ncReadNunChuck;
             break;
        case ncReadNunChuck:
             //Serial.println("ncReadNunChuck");              
             nunchuk_get_data();
             
             CZButton= (nunchuk_cbutton()<<4) | (nunchuk_zbutton());
           /*  
             if (zButtonZ!=ButtonZ){
                 zButtonZ=ButtonZ;
                 InsertWII(CMD_BUTTON_Z,zButtonZ);
                 Serial.println("Z button");                 
             }ButtonZ=nunchuk_zbutton();
             
             if (zButtonC!=ButtonC){
                 zButtonC=ButtonC;
                 InsertWII(CMD_BUTTON_C,1);
                 Serial.println("C button");
             }ButtonC=nunchuk_cbutton();
             
             */
             
            
             if (nunchuk_zbutton()==1){
                 //Serial.println("Z button");                 
                 //InsertWII(CMD_BUTTON_Z,1);
                 ButtonZ=1;
             }else ButtonZ=0;//InsertWII(CMD_BUTTON_Z,0);             
             
             if (nunchuk_cbutton()==1){
                 ButtonC=1;
                 //Serial.println("C button");
                 //InsertWII(CMD_BUTTON_C,1);
             }else ButtonC=0;//InsertWII(CMD_BUTTON_C,0);
             
             
             CZButton = (ButtonC<<1) | ButtonZ ;
             
             TimWiiDelay=0;
             stNunchuck=ncDelay;             
             break;
        case ncDelay:
             //Serial.println("ncDelay");
             if (TimWiiDelay>WII_MAX_DELAY) 
                 stNunchuck=ncReadNunChuck;
             break;
             
     }
}


void TimEvent(){
     TimPingMs++;
     //TimWiiDelay++;       
     
}


void setup() {  
  Serial.begin(115200); // Open serial monitor at 9600 baud to see ping results.
  pinMode(13, OUTPUT);    
  
  //Timer1.initialize(100000);//InitTimer 1ms
  //Timer1.attachInterrupt(TimEvent);
  //Timer1.start();  
  
}

void loop(){ 
 //static unsigned long iTim=0; 
 //delay(40);                      // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  //unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  //unsigned int uS = FIR(sonar.ping()); // Send ping, get ping time in microseconds (uS).
  //SendData(uS);    
  FPing();
  FNunChuck();
  //FMailBox();
}


