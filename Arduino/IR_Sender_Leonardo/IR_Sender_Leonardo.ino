#define CODE 3000 //length of "mark" in usecs (1 cycle of IR mark @ 38khz ==  26.32 usecs // CODE x 0.038 = # of cycles)

int irLedPin = 9; //IR on Pin 9

//volatile boolean toggleIR = 1; //toggles HIGH/LOW @ 38kHz IR pulse

//counters for millis()
unsigned long time0 = 0;
unsigned long time1 = 0;

void setup(){
  
  pinMode(irLedPin, OUTPUT);
  
  cli(); //turn off interrupts
  
  //set timer1 (16-bit) interrupt at 76000 Hz  (i.e. 38kHz x 2 for full square wave)
  TCCR1A = 0;// set entire TCCR1A (Timer 1 Control Register A) register to 0
  TCCR1B = 0;// same for TCCR1B (Timer 1 Control Register B)
  TCNT1  = 0;//initialize Timer 1 counter value to 0
  // set compare match register for 38khz increments
  OCR1A = 210;// = (16000000 / (76000*1)) - 1 == i.e. (16MHz / (76Khz*prescaler) - 1 [because count begins with 0] -  [value must be <65536 for 16bit]
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 bit for no prescaler
  TCCR1B |= (1 << CS10);
  // enable timer compare interrupt
  //TIMSK1 |= (1 << OCIE1A);
  
  //enable toggle OC1A (Flora Pin 9) on compare match
  TCCR1A |= (1 << COM1A0);
  
  sei(); //turn interrupts back on
  
  //set time0 to current micros()
  time0 = micros();
}

void loop(){
  
  time1 = micros();
  
  //if CODE microseconds have elapsed...
  if ((time1 - time0) >= CODE){
    
     //disable the timer compare interrupt
     //TIMSK1 &= ~(1 << OCIE1A);
     
     //disable compare output mode, Pin 9 now accessible
     TCCR1A &= ~(1 << COM1A0);
     
     //make sure pin 3 is off
     digitalWrite(irLedPin, LOW);
     
     //tell the serial monitor the code was sent
     Serial.print("sent pulse for ");
     Serial.print((time1 - time0), DEC);
     Serial.print("microseconds, according to ");
     Serial.print(CODE, DEC);
     Serial.print(" CODE");
     Serial.print("\n\r");
     
     //delay 50ms between bursts
     delay(50);
     
     //reset time0 to current micros()
     time0 = micros();
     
     //re-enable timer compare interrupt
     //TIMSK1 |= (1 << OCIE1A);
     
     //re-enable compare output mode
     TCCR1A |= (1 << COM1A0);

  }
  
}


/*ISR(TIMER1_COMPA_vect) {

      digitalWrite(irLedPin, toggleIR);
      toggleIR = !toggleIR;

}*/

  
