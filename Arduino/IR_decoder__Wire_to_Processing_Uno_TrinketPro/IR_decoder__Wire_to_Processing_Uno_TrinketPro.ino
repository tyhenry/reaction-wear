/*
* BeamBreak and IR Code detector for LoVid Reaction Bubble Project
* by Tyler Henry
*
* Input Capture function
* uses timer hardware to measure pulses on pin 8 on 168/328
* 
* Input capture timer code based on Input Capture code found in Arduino Cookbook
*/


/* HEADER CODE AND IR CODES GO HERE */
/*--------------------------------------------------------*/

const char HEADER = 'A'; //the header ID value for this IR receiver (i.e. A, B, C, D...)

//value of IR CODEs to be checked for (these are length of IR burst in microseconds)
const int codeArrayLength = 4; //length of array below
const int codeArray[codeArrayLength] = {1000, 2000, 3000, 4000};

/*--------------------------------------------------------*/

/*WIRE LIBRARY INPUT*/
/*--------------------------------------------------------*/

#include <Wire.h>
const int address = 4; //this needs to match the sender arduino

/*--------------------------------------------------------*/



const int inputCapturePin = 8; // Input Capture pin fixed to internal Timer - MUST BE 8 on ATMega168/328
const int ledPin = 13;
const int prescale = 64; // prescale factor 64 needed to count to 62 ms for beam break
const byte prescaleBits = B011; // see Table 18-1 or data sheet

// calculate time per counter tick in ns
const long precision = (1000000/(F_CPU/1000)) * prescale ;
const long compareMatch = (F_CPU/(16 * prescale)) - 1; //compareMatch interrupt value for beam break detection

const int pulseSampleSize = 6; //denotes the number of pulse readings that need to be close before declaring a pulse

volatile unsigned long pulseCountCurrent; //counter for current pulse
unsigned long pulseCount[pulseSampleSize]; // storage array of pulse counts with size of "pulseSampleSize" constant
unsigned int pulseIndex = 0; // index to the pulse count storage array
unsigned long pulseTime[pulseSampleSize];

volatile unsigned long pauseCount; // counter for current pause

volatile boolean pulsed = false;
volatile boolean paused = false;
volatile boolean beamBroken = false;
boolean beamBrokenNew = true;



/* ICR interrupt vector */
/* runs when pin 8 detects an input change (rising or falling trigger depends on ICES1 value) */
ISR(TIMER1_CAPT_vect){
  TCNT1 = 0; // reset counter
  
  if(bitRead(TCCR1B, ICES1)){ // rising edge was detected
      pulseCountCurrent = ICR1;
      pulsed = true;
      beamBroken = false;
  } else {
      pauseCount = ICR1; // save ICR as break
      paused = true;
   }
   
  TCCR1B ^= _BV(ICES1); // toggle bit to trigger on the other edge
}

//beam break interrupt vector
ISR(TIMER1_COMPA_vect){ //timer1 interrupt at 16Hz (~62 milliseconds)
    beamBroken = true;
}


void setup() {
  
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(inputCapturePin, INPUT); // ICP pin (digital pin 8 on Arduino) as input
  
  cli(); //stop interrupts temporarily
  
  //input capture timer:
    TCCR1A = 0; // Normal counting mode
    TCCR1B = 0;
    TCCR1B = prescaleBits ; // set prescale bits
    TCNT1 = 0; //reset counter
    TCCR1B |= _BV(ICES1); // enable input capture: counter value written to ICR1 on rising edge
    bitSet(TIMSK1,ICIE1); // enable input capture interrupt for timer 1: runs ISR
    //TIMSK1 |= _BV(TOIE1); // Add this line to enable overflow interrupt
  
  //beam break interrupt:
    TCCR1B |= (1 << WGM12); //Timer 1 CTC mode: TCNT1 clears when OCR1A reached
    OCR1A = compareMatch; //set compare match interrupt for beam break to 62ms (~16Hz)
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
    
  sei(); //turn interrupts back on
  
  /* WIRE LIBRARY */
  Wire.begin(address); // join I2C bus using this address
  Wire.onReceive(receiveEvent); // register event to handle requests
  
}


void loop(){
  
  if (pulsed){ // run if a pulse was detected
  
    pulseCount[pulseIndex] = pulseCountCurrent; // save the current pulse value in the array 
    
    if(pulseIndex / (pulseSampleSize-1) == 1){ //if array is newly filled - should run every five pulses
            
      //calculate time (microsecond) values of pulseCount array
      for(int i = 0 ; i < pulseSampleSize ; i++){
        pulseTime[i] = calcTime(pulseCount[i]);
      }
      
      //bubble sort (i.e. from small to large) the time value array
      bubbleSort(pulseTime, pulseSampleSize);
      
      //check for close matches in array
      for(int i=0; i<(pulseSampleSize-1); i++){
        
        unsigned long dif = pulseTime[i+1] - pulseTime[i];
        if (dif < 120) { //check to see if the two values are close (120 is fairly arbitrary)
                  
          unsigned long avg = (pulseTime[i] + pulseTime[i+1]) / 2; //use average to check for matches to CODE values

          for (int j=0; j<codeArrayLength; j++){ //check the CODE value array for matches
            if ((avg > codeArray[j]-80) && avg < (codeArray[j]+80)){ //if the avg of the two values is close to a CODE

              //SEND A HEADER AND MATCHED CODE VALUE TO SERIAL              
              Serial.write(HEADER); //header for this IR Receiver ('A')
              sendBinary(codeArray[j]); //CODE value received
            }
          }
        }
      }
      
    }
    
    //increment or reset index:
    pulseIndex = (pulseIndex + 1) % pulseSampleSize;
    
    pulsed = false; //reset pulse boolean
    digitalWrite(ledPin, LOW);
    beamBrokenNew = true;
  }
  
  if (paused){    
    paused = false; //reset pause boolean
    beamBrokenNew = true;
  }
  
  if (beamBroken){
    Serial.print(HEADER);
    sendBinary(65000); //send beam break value of 65000
    
    if (beamBrokenNew){
      beamBrokenNew = false;
    }
    beamBroken = false;
    pulseIndex = 0;
  }
    
  
}

//converts a timer count to microseconds
long calcTime(unsigned long count){

  long durationNanos = count * precision; // pulse duration in nanoseconds
  
  if(durationNanos > 0){
    long durationMicros = durationNanos / 1000;
    return (durationMicros); // duration in microseconds
  } else {
    return 0;
  }
}



/* min/max/mean of array functions pulled from Average library */
unsigned long minimum(unsigned long *data,int count)
{
	int l;
	unsigned long minval;

	minval = 2147483647L;

	for(l=0; l<count; l++)
	{
		if(data[l] < minval)
		{
			minval = data[l];
		}
	}
	return minval;
}
unsigned long maximum(unsigned long *data,int count)
{
	int l;
	unsigned long maxval;

	maxval = 0;

	for(l=0; l<count; l++)
	{
		if(data[l] > maxval)
		{
			maxval = data[l];
		}
	}
	return maxval;
}
unsigned long mean(unsigned long *data, int count)
{
	int i;
	unsigned long total;
	unsigned long result;

	total = 0;
	for(i=0; i<count; i++)
	{
		total = total + data[i];
	}
	result = total / count;
	return result;
}


/* lifted from http://www.hackshed.co.uk/arduino-sorting-array-integers-with-a-bubble-sort-algorithm/ */
void bubbleSort(unsigned long a[], int aSize) {
    for(int i=0; i<(aSize-1); i++) {
        for(int j=0; j<(aSize-(i+1)); j++) {
                if(a[j] > a[j+1]) {
                    int t = a[j];
                    a[j] = a[j+1];
                    a[j+1] = t;
                }
        }
    }
}

//sends integer to serial port (i.e. in 2 bytes)
void sendBinary(int value){
  //split into 2 bytes and send each
  Serial.write(lowByte(value)); //send low byte
  Serial.write(highByte(value));
}



/* WIRE LIBRARY */
void receiveEvent(int howMany){
  while(Wire.available() > 0){
    char c = Wire.read(); // receive byte as a character
    Serial.write(c); // echo
  }
}

