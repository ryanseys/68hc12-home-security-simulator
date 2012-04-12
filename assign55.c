/* Final Integration of all Specifications of the Home Security System */
/* AUTHORS: RYAN SEYS AND OSAZUWA OMIGIE */
#include <stdio.h>
#include <hcs12dp256.h>
//macro: false defined as 0
#define FALSE 0
//macro: true is anything that isn't 0 (not false)
#define TRUE !0
//The maximum # of counts used to create a time delay.
#define DELAY_MAX 11000

void DelayNX(int n);
void pacA_isr(void);
void _Timer_ISR(void);
void OC_TIMER(void);

int optCount = 0;
int duty = 0;
int curr_speed;

const int MIN_DUTY = 0;  //the minimum duty cycle (this will determine the minimum speed of the fan)
const int MAX_DUTY = 18;   //the maximum duty cycle (this will determine the maximum speed of the fan)

//defining a boolean type and byte type
typedef int boolean;
typedef unsigned char byte;  //Alias unsigned char data type as byte

// - - - FUNCTION PROTOTYPES - - - //
char checkKey(int);
char getASCII(void);
void printLCD(char); 
void LCD_display(char);
void LCD_instruction(byte);
void Lcd2PP_Init(void);
void Lcd2PP_Init2(void);
void waiting(void);
void initRow(int);
char checkKey(int);
void turnOffBuzzer(void);
void setDownStepperMotor(void);
void turnOnBuzzer(void);
void initializeTimerInterrupts(void);
void soundBuzzer(void);
void runSimulation(void);
void initializeKeypadInterrupts(void);
void initializeTemperatureInterrupts(void);
void initializeKeypadPorts(void);
void STEPPERClkwise(int);
void STEPPERAntiClkwise(int);
void waiting(void);
void initRow(int);
void changeAlarm(void);
void soundBuzzer(void);
void delay (int);
void printLCDSame(char);
void initialPrint(void);
void printTime(void);
void clearPassword(void);
void printLCDString(char *);
void setGreenLED(boolean);
void setRedLED(boolean);
void setYellowLED(boolean);
void STEPPERClkwise(int);
void initializeHeater(void);
void delay1secondAndCheck(void);
void printTemperature(int);
void setUpStepperMotor(void);
void printTemperatureLimitValue(int);
void disableTemperatureInterrupts(void);
int strlen(char *); //gets the length of a character array (from string.h)
// END OF FUNCTION PROTOTYPES //

// - - - GLOBAL VARIABLES - - - //

// BOOLEANS //
boolean alarmArmed = FALSE; //alarm is disarmed to begin
boolean windowSensor = FALSE; //window is not open to begin
boolean InfraredSensor = FALSE; //presense not detected to begin
boolean alarmChanged = FALSE;
boolean passwordCorrect = FALSE;
boolean pressed = FALSE;
boolean timesChanged = FALSE;
boolean blindsUpNext = TRUE;

// BYTES //
byte heatingON = FALSE; //heating is initialized as OFF.
byte ready = FALSE;
byte initPassPos = 0x00;

// INTEGERS //
int temperature_limit = 85; // the temp "too high" value is initialized here. (higher or equal -> blinds closed, lower -> blinds open)
int numInterrupts = 1; // keeps track of the number of seconds that have passed.
int temporary;
int passCount = 0;
int passIndex = 0;
int timer = 10; // the timer countdown length (the user has this many seconds approximately to enter the password)
int ticks = 0;
int passwordTimes = 1; 
int step = 1;
int seconds = 0;
int minutes = 0;
int delayVal;
int divisor = 0; //this divisor is used in conjuction with the interrupt to tell it when the numInterrupts should increase.
int F_found = 0; //F key has not been pressed to begin.
int temperature; //initial temperature is 20 degrees Celcius

// CHARS //
char key;
char next;
char keyPressed;
char password[] = "1111"; //represents the default password to gain access to arming and disarming the alarm.

// - - - END GLOBAL VARIABLES - - - //

/*
Runs the simulation in all of its intergrated amazing-ness.
This integrates all the specifications of the entire project.
*/
void main() {
	 // TEMPERATURE INTERRUPT VARIABLES//
	 int localTemperature;
	 // END TEMPERATURE INTERRUPT VARIABLES//
	 
	 // - - - - BEGIN BY WRITING THE MESSAGE TO THE LCD SCREEN - - - - //
	 initialPrint(); //print the initial values for the first time
	 //LCD_instruction(0x0c); //turn off cursor so when we jump around, its invisible
	 
	 // TIMER INTERRUPT INITS //
	 initializeTimerInterrupts();
	 // END TIMER INTERRUPT INITS //
	  
	 // TEMPERATURE INTERRUPT INITS //
	 initializeTemperatureInterrupts();
	 initializeHeater();
	 // END TEMPERATURE INTERRUPT INITS//
	 
	 // KEYPAD INTERRUPT INITS //
	 initializeKeypadInterrupts();
	 // END KEYPAD INTERRUPT INITS //
	 
	 PWMPOL = 0xFF; // Initial Polarity is high
     PWMCLK &= 0x7F; //Select Clock B for channel 7
     PWMPRCLK = 0x70; //Prescale ClockB : busclock/128
     PWMCAE &= 0x7F; //Channel 7 : left aligned
     PWMCTL &= 0xF3; //PWM in Wait and Freeze Modes
     PWMPER7 = 100; //Set period for PWM7
     PWME = 0x80; //Enable PWM Channel 7
     DDRP |= 0x40; //For Motor Direction Control  
     PAFLG |= 1; //Clear out the interrupt flag
     PACTL = 0x51; //Enable PACA for Optical Sensor
     
	 CRGINT |= 0x80;
	 RTICTL |= 0x7F;
     INTR_ON();   //same as cli , globally enable interrupts
	 
	  //for motor.
	 PTM = PTM | 0x08;
	 PTP = 0x0F; //set scan rows
	 //set U7_EN low
	 PTM = PTM & 0xf7;
	 DDRP |= 0x0F; // bitset PP0-3 as outputs (rows)
	 
	 initializeFanInterrupts();
	 
	 asm("cli"); //globally enable interrupts
	 numInterrupts = 0;
	 //printf("starting the main while loop");
	 LCD_instruction(0x0c); //turn off cursor
	 //here is where it will handle EVERYTHING
	 
	 //While the F key has not yet been pressed.
	 while(F_found == FALSE) {
		//Start by initializing ports for keypad detection
		initializeKeypadPorts(); 
		if(pressed == TRUE) {
			 //get the key pressed.
    	 	 next = getASCII();
    		 //reinitialize ports for the keypad.
    		 //if a number is pressed
    		 if((next <= '9') && (next >= '0')) {
			 		//Lcd2PP_Init2();
        			LCD_instruction(initPassPos+0x80); //sets the cursor to 
    				printLCD('*'); //print the '*' in LCD
					if(next == password[passIndex]) passCount +=1; //checks if the password is the same so far
					passIndex++; //increments the password index
    				if(initPassPos==0x07) //checks if the cursor is at the last digit of the password  
    				{
					 	//checks if the password is correct
					 	if(passCount == 4) {
							passwordCorrect = TRUE;
							//LCD_instruction(0xc0); 
							//printLCDString("pass correct!");
						}
						else {
							passwordCorrect = FALSE;
							//LCD_instruction(0xc0); 
							//printLCDString("pass incorrect!");
						}
						initPassPos = 0x04; //sets the cursor to the first digit position of the password
    					clearPassword(); //erases the already entered digits
						passIndex = 0;
						passCount = 0;
    					//break; //originally in a for loop, this is now useless
    				}
    				else initPassPos+=0x01;  //increment track of cursor position
        	 }
    		 //delay(25); // timing
    		 pressed = FALSE;
		}
		//end initialize ports for keypad
	 	//initializeKeypadPorts();
		initializeTemperatureInterrupts(); //commenting out makes the temperature not change.
		
		 //check the alarm status and light the appropriate LEDs
		 if(alarmArmed) {
		 	  setRedLED(TRUE);  //turn on the red led 
			  setGreenLED(FALSE);
		 }
		 else{
		 	  setGreenLED(TRUE); //turn on the green led
			  setRedLED(FALSE);
		 }
		 
		 //if the temperature is less than or equal to its limit (as per the assignment instructions)
         //initializeHeater();
		 if(localTemperature < temperature_limit) {
		 	if((PTM & 0x80) != 0x80) {
				   PTM |= 0x80; //turn on heater
			}
			if((PORTK & 0x04) != 0x04) {
				setYellowLED(TRUE);
			}
			if(heatingON == FALSE) {
					LCD_instruction(0xc5);
    				printLCDString("ON ");
					heatingON = TRUE;
			}
		 }
		 //heater is off now (temperature is greater than the limit now).
		 else {
		 	   	//instead of checking to see if the heater is already off, we just turn it off.
                if(PTM != 0x00) {
				   PTM = 0x00; //turn off heater
				}
				if((PORTK | 0xFB) != 0xFB) {
						setYellowLED(FALSE);
				}
   			  	if(heatingON == TRUE) {
					LCD_instruction(0xc5);
    				printLCDString("OFF");
					heatingON = FALSE;
			}
		 }
		 
		 // - - - - TEMPERATURE ISR WAS CALLED - - - - //
		 if(ready == TRUE) {
		 		 //set the local variable so the global variable doesn't change throughout.
				 if(localTemperature != temperature) {
                	 localTemperature = temperature;
					 printTemperature(localTemperature); //print the temperature
				 }
            	ready = FALSE; //reset the ready flag to wait for another interrupt. 
		 }
		 //disable these interrupts so they don't interfere with the next steps.
		 disableTemperatureInterrupts();
		 
		 //if the user has pressed E key to change the alarm status
		 if(alarmChanged == TRUE) {
		 	//the password must be detected...
			//give the user 10 seconds to input the password.
    		 if(timer != 0) { //as long as the timer hasn't count down to zero yet.
    			 LCD_instruction(0x8b); //set the position of the cursor to where the set temp usually is.
        		 printLCDString("T:"); //print this string where the SET TEMP usually is.
        		 printLCD(timer / 10 + 0x30); //print the timer MSB
        		 printLCD(timer % 10 + 0x30); //print the timer LSB
        		 printLCD(' '); //print a space
    			 delay1secondAndCheck(); //delay 
			 }
			 
			 //if the time has run out and the password hasn't yet been entered correctly.
			 if(timer == 0) {
					turnOnBuzzer(); //set the alarm on!
					//only set the number of passwordTimes once.
					if(timesChanged == FALSE) {
							passwordTimes = 2; //make the user have to enter the password twice now.
							timesChanged = TRUE;
					}
			 }
			 //if the password was correctly entered.
			 if(passwordCorrect == TRUE) {
			 	  	passwordCorrect = FALSE; //reset this so it can be redetected
			 		passwordTimes--; //subtract from the number of times required to login (standard = 1, 2 if alarm is going off).
					if(passwordTimes == 0) {
						turnOffBuzzer(); //turn off the buzzer if its on (for if the alarm is on)
						passwordTimes = 1; //reset to the standard 1 times for number of times required to login
						changeAlarm(); // change the alarm status
    					alarmChanged = FALSE; //set this back to default so changes can be redetected.
						timesChanged = FALSE; //reset this so the next time we miss the password deadline.
    					timer = 10; //reset the timer back to a 10 second countdown.
						//print out the temperature set value (to overwrite the timer countdown)
    					printTemperatureLimitValue(temperature_limit);
					}
			 }
			 //as long as the time still hasn't run out, decrease the timer.
			 if(timer != 0) {
			 		  timer--;
			 }
		 }
		 
		 //Handles the stepper motor steps here.
		 setUpStepperMotor();
		 if(ticks < 10) {
		 		  //determines and executes the next step of the stepper motor.
		 		  if(blindsUpNext == TRUE) {
    		 		if(step == 1) {
                       	PTT = 0x20; //PORT T = 0b01100000 (5th bit set, 6th bit set)
                        delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
                        step = 2;
    				}
    				else if (step == 2) {
    					 PTT = 0x00; //PORT T = 0b00000000 (5th bit cleared, 6th bit cleared)
    					 delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
    					 step = 3;
    				}
    				else if (step == 3) {
    					 PTT = 0x40; //PORT T = 0b00000000 (5th bit cleared, 6th bit cleared)
    					 delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
    					 step = 4;
    		 		}
    				else if (step == 4) {
    					 PTT = 0x60; //PORT T = 0b00000000 (5th bit cleared, 6th bit cleared)
    					 delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
    					 step = 1;
    				}
				}
				//it is either going clockwise or counterclockwise (open or close blinds)
				else if(blindsUpNext == FALSE) {
					if(step == 1) {
						PTT = 0x60; //PORT T = 0b01100000 (5th bit set, 6th bit set)
						delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
						step = 2;
					}
    				else if (step == 2) {
    					 PTT = 0x40; //PORT T = 0b00000000 (5th bit cleared, 6th bit cleared)
    					 delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
    					 step = 3;
    				}
    				else if (step == 3) {
    					 PTT = 0x00; //PORT T = 0b00000000 (5th bit cleared, 6th bit cleared)
    					 delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
    					 step = 4;
    		 		}
    				else if (step == 4) {
    					 PTT = 0x20; //PORT T = 0b00000000 (5th bit cleared, 6th bit cleared)
    					 delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
    					 step = 1;
    				}
				}
		 }
		 //if it has waited enough time between closing and opening the blinds.
		 //this value can be increased to increase the wait time between times of moving the stepper motor.
		 if(ticks == 30) {
		 	ticks = 0;
			//toggle the direction after each move
			if(blindsUpNext == TRUE) blindsUpNext = FALSE;
			else blindsUpNext = TRUE;
		 }
		 if(heatingON == FALSE) {
		 // turn on the fan to cool down
		 PTP = 0x40; //turn  anti-clockwise
		 duty = 10; //default fan value of 10.
		 //printf("setting PWMDTY7 \n");
	   	 PWMDTY7 = duty;
		 }
		 else {
		 // turn off the fan
	   	 PWMDTY7 = 0;
		 }
		 PTP = 0x00;
		 //reset ports.
		 setDownStepperMotor();
		 initializeKeypadPorts();
    	} //end while
		LCD_instruction(0x01);
		printLCDString("  Goodbye! :)");
		LCD_instruction(0xc0);
		printLCDString("By: Ryan/Osazuwa");
} //end main


// Delay1Second is used for the timer interrupt which delays 1 second.
void delay1second() {
	 int i;
	 TSCR1 = 0x90; // movb #$90,TSCR ; enable TCNT and fast timer
	 TIE = 0x03; // movb #$05,TMSK2 ; set prescale factor to 8 (3 = 8, 5 = 32)
	 TIOS |= 0x01; //enable OC0
	 TC0 = TCNT;
	 //by experiment it shows that the frequency is actually low
	 for(i = 0; i < 100; i++) { 
	 	   TC0 += 50000;
		   while((TFLG1 & 0x01) == 0);
	 }
}

void initializeFanInterrupts() {
	 PIFH = 0xFF; // Clear previous interrupt flags
	 PPSH = 0xF0; // Rising Edge
	 PERH = 0x00; // Disable pulldowns
	 PIEH |= 0xF0; // Local enable on columns inputs
	 duty = 0;
	 PWMDTY7 = 0; //duty = 0 initially. note we are using channel 7
}

// Prints the temperature limit to the LCD in the specific spot
void printTemperatureLimitValue(int temperature_limit) {
    LCD_instruction(0x8a); 
    printLCDString(" S:");
    printLCD(temperature_limit / 10 + 0x30); //get the MSB and convert to ascii. then print it out.
    printLCD(temperature_limit % 10 + 0x30); //get LSB and convert to ascii. then print it out.
    printLCD('C');
}

// Prints the temperature value passed to the LCD screen in the default position.
void printTemperature(int temperature) {
	 LCD_instruction(0xcd); //end of the first line on LCD
	 LCD_instruction(0x0c); //turn off cursor
	 //Print out the value of the temperature onto the LCD screen
	 printLCD(temperature / 10 + 0x30); //get the MSB and convert to ascii. then print it out.
	 printLCD(temperature % 10 + 0x30); //get LSB and convert to ascii. then print it out.
	 printLCD('C'); //Celcius
}

//delay1second is used for the timer interrupt which delays 1 second
//and checks if a keyt has been pressed during this time.
void delay1secondAndCheck() {
    	 int i;
    	 TSCR1 = 0x90; // movb #$90,TSCR ; enable TCNT and fast timer
    	 TIE = 0x03; // movb #$05,TMSK2 ; set prescale factor to 8 (3 = 8, 5 = 32)
    	 TIOS |= 0x01; //enable OC0
    	 TC0 = TCNT;
    	 //by experiment it shows that the frequency is actually low
    	 for(i = 0; i < 15000; i++) { 
    	 	   TC0 += 50000;
			   //set the ports appropriately.
			   initializeKeypadPorts();
			   		if(pressed == TRUE) {
            	 	 next = getASCII();
            		 //reinitialize ports for the keypad.
            		 //if a number is pressed
            		 if((next <= '9') && (next >= '0')) {
        			 		Lcd2PP_Init2(); //initialize the ports for writing to the LCD.
                			LCD_instruction(initPassPos+0x80); //sets the cursor to 
            				printLCD('*'); //print the '*' in LCD
        					if(next == password[passIndex]) passCount +=1; //checks if the password is the same so far
        					passIndex++; //increments the password index
            				if(initPassPos==0x07) //checks if the cursor is at the last digit of the password  
            				{
        					 	//checks if the password is correct
								LCD_instruction(0x84);
								//if the sum is 4, it means all values were correct.
        					 	if(passCount == 4) {
        							printLCDString("PASS"); //display momentarily this message for quick visual feedback. 
									passwordCorrect = TRUE;
        						}
        						else {
        							printLCDString("FAIL"); //display momentarily this message for quick visual feedback. 
									passwordCorrect = FALSE;
        						}
        						initPassPos = 0x04; //sets the cursor to the first digit position of the password
            					clearPassword(); //erases the already entered digits
        						passIndex = 0;
        						passCount = 0;
            					//break; //originally in a for loop, this is now useless
            				}
            				else initPassPos+=0x01;  //increment track of cursor position
                	 }
            		 delay(10); // timing
            		 pressed = FALSE;
    		   while((TFLG1 & 0x01) == 0);
			}	
		}
}

// This gets the key pressed from the keypad and returns it.
// It also runs the specific commands depending on the certain key pressed.
char getASCII(void) {
	 int i = 1;
	 //default to see if it ever changed.
	 char temp = 0x00;
	 //for each row of keys on the keypad.
	 while(temp == 0x00) {
	 	   initRow(i); //initialize the row
		   temp = checkKey(i); //check the keys in that row
		   i++; //increase to go to the next row.
		   if(i > 4) break; //does it for four rows
	 }
	 return temp; //return the character if changed from the default.
}

// Prints out the time to the LCD (assumes the LCD has been initialized)
void printTime() {
	 printLCD(minutes / 10 + 0x30); //get the MSB and convert to ascii. then print it out.
	 printLCD(minutes % 10 + 0x30); //get LSB and convert to ascii. then print it out.
	 printLCD(':');
	 //Print out the value of the seconds onto the LCD
	 printLCD(seconds / 10 + 0x30); //get the MSB and convert to ascii. then print it out.
	 printLCD(seconds % 10 + 0x30); //get LSB and convert to ascii. then print it out.
}

/*
Setup the ports DDRP,DDRH,and PTM to be able to detect a 
key press on the specified row. The row intialization is used in addition with a column check
to get the status of specific keys and whether they have 
been pressed or not. 
*/
void initRow(int rowid) {
	SPI1CR1 = 0x00;	  //clear Serial Peripheral Interface port
	DDRP = DDRP | 0x0f;	   //set outputs to key1-4, avoids turning on motor
	DDRH = DDRH & 0x0f;	//sets columns as inputs //clear DDRH
	PTM = PTM | 0x08; //set u7 tp high
	//load PTP with key
	if(rowid == 1) PTP = 0x01;   //if row 1 is passed in the parameter, set PTP accordingly 
	else if(rowid == 2) PTP = 0x02; //if row 2 is passed in the parameter, set PTP accordingly
	else if(rowid == 3) PTP = 0x04; //if row 3 is passed in the parameter, set PTP accordingly
	else if(rowid == 4) PTP = 0x08; //if row 4 is passed in the parameter, set PTP accordingly
	PTM = PTM & 0xf7;  //set U7_EN low
}

//Turns ON and OFF the Red LED depending on the value passed in boolean.
void setRedLED(boolean val){
  DDRK |= 0x0F; //setup portk with bits 0..3 as output pins (1 as output) 
  if(val == FALSE) {
		//turn off red led
 		PORTK &= 0xFE;
  }
  else PORTK |= 0x01; //turn it on
}

//Turns ON and OFF the Yellow LED depending on the value passed in boolean.
void setYellowLED(boolean val) {
  DDRK |= 0x0F; //setup portk with bits 0..3 as output pins (1 as output) 
  if(val == FALSE) {
		//turn off yellow led
 		PORTK &= 0xFB;
  }
  else PORTK |= 0x04; //turn it on
}

//Turns ON and OFF the Green LED depending on the value passed in boolean.
void setGreenLED(boolean val){

  DDRK |= 0x0F; //setup portk with bits 0..3 as output pins (1 as output) 
  if(val == FALSE) {
		//turn off green led
 		PORTK &= 0xF7;
  }
  else PORTK |= 0x08; //turn it on.
}

/*
Print the character on the board's LCD screen and increase 
the cursor to the next position.
*** This has been changed from question one. ***
*/
void printLCD(char c) {
	 LCD_display(c); //load the character
	 LCD_instruction(0x06); //write character and shift to next position
}

//The initial print that needs to be executed for the LCD display.
void initialPrint() {
	 //initialize the LCD display
	 Lcd2PP_Init();
	 
	 // FIRST LINE OF THE LCD PRINTING STARTS HERE
	 printLCDString("PWD:     ");
	 
	 if(alarmArmed == 0) //if the alarm is disarmed 
	 {
	 	  printLCD('D'); //print character 'D' on the LCD
	 }
	 else printLCD('A');  //else print character 'A' on the LCD
	 //delay(5);
	 
	 printLCDString(" S:");
	 printLCD(temperature_limit / 10 + 0x30); //get the MSB and convert to ascii. then print it out.
	 printLCD(temperature_limit % 10 + 0x30); //get LSB and convert to ascii. then print it out.
	 printLCD('C');
	 
	 //Shifts the cursor to the next line
	 LCD_instruction(0xc0);  
	 
	 // SECOND LINE OF PRINTING ON THE LCD FOLLOWS HERE
	 
	 //display the string 'Heat:' on the LCD
	 printLCDString("Heat:");

	 //print out whether the heat is on or off.
	 if(heatingON) //if heating is on, print 'ON' on the LCD 
	 {
	 	  printLCDString("ON");
	 }
	 else {
	 	  //else print 'OFF' on the LCD
	 	  printLCDString("OFF");
	 }
	 
	 //print out " Tmp:" afterwards
	 printLCDString(" TMP:");
	 
	 //Print out the value of the temperature onto the LCD screen
	 printLCD(temperature / 10 + 0x30); //get the MSB and convert to ascii. then print it out.
	 printLCD(temperature % 10 + 0x30); //get LSB and convert to ascii. then print it out.
	 printLCD('C'); //Celcius
}

// Clears the values at the password field on the LCD display.
void clearPassword() {
	 LCD_instruction(0x84); //write character and shift to prev position
	 printLCDString("    "); //replaces the '*' characters with spaces
	 LCD_instruction(0x84); //write character and shift to prev position
}

// Turns on the buzzer.
void turnOnBuzzer() {
	DDRK |= 0x2F; // outputs for the buzzer set
	PORTK |= 0x20; //turning on the buzzer bit
}

// Turns off the buzzer.
void turnOffBuzzer() {
	DDRK |= 0x2F; // outputs
	PORTK &= 0xDF; //turning on the buzzer bit
	
}

// Sets up the Stepper Motor so it may be controlled with ports P and T 
void setUpStepperMotor() {

	 //set up the directions for both Port T and Port P.
	 DDRT = 0x60;  //setting the portT's direction register to indicate the output pins
	 DDRP = 0x20; //setting the portP's direction register to indicate the output pins
	 
	 /*6th bit in portT is the output sent to the Bottom inductor coid of the stepper
	 5th bit in portT is the output sent to the Left inductor coil of the stepper
	  */
	 //clear PT5 and PT6
	 PTT = PTT & (~(1<<5)); //clearing the 5th bit of Port T
	 PTT = PTT & (~(1<<6)); //clearing the 6th bit of Port T
	 
	 /*
	   5th bit in portP is the enable(EN) bit for the stepper
	 */ 
	 PTP = PTP | (1<<5); //setting the 5th bit in port P.
}

// Set the stepper motor ports back to more reasonable values.
void setDownStepperMotor() {
	 DDRP = 0xFF;
	 DDRT = 0x0F;
}
/*
  This function rotates the stepper motor clockwise a specific number
  of moves. In testing, with the delays given, approximately 5 moves
  are required for a full rotation of the stepper motor. Therefore 2
  rotations would require an input value of stepMoves = 10.
*/
void STEPPERClkwise(int stepMoves){
	 int i; //used for the 'delay()' macro.
	 
	 setUpStepperMotor(); //set up the appropriate bits for using the stepper.
	 
	 while(stepMoves > 0) //will iterate stepMoves number of times
	 {
	 	PTT = 0x60; //PORT T = 0b01100000 (5th bit set, 6th bit set)
		delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
		PTT = 0x40; //PORT T = 0b01000000 (6th bit set, 5th bit cleared)
		delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
  		PTT = 0x00; //PORT T = 0b00000000 (5th bit cleared, 6th bit cleared)
		delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
		PTT = 0x20; //PORT T = 0b00100000 (5th bit set, 6th bit cleared)
		delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
	    --stepMoves; //keep reducing stepMoves every iteration
	 }
}

/*
  This function rotates the stepper motor counter-clockwise a specific 
  number of moves. In testing, with the delays given, approximately 
  5 moves are required for a full rotation of the stepper motor. 
  Therefore 2 rotations would require an input value of stepMoves = 10.
*/
void STEPPERAntiClkwise(int stepMoves){
	 int i; //set up for the delay macro to use
	 
	 setUpStepperMotor(); //set up the appropriate bits for using the stepper.
	 
	 while(stepMoves > 0) //perfoms the following sequences on PORT stepMoves number of times
	 {
	 	PTT = 0x20; //PORT T = 0b00100000 (5th bit set, 6th bit cleared)
 		delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
		PTT = 0x00; //PORT T = 0b00000000 (5th bit cleared, 6th bit cleared)
		delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
  		PTT = 0x40; //PORT T = 0b01000000 (6th bit set, 5th bit cleared)
		delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
		PTT = 0x60; //PORT T = 0b01100000 (5th bit set, 6th bit set)
		delay(DELAY_MAX); //delays for approx. 10ms so the stepper has time to move.
	    --stepMoves; 
	 }
}

/*
This checks the appropriate key on the board's keypad to see if it was pressed.
It does this only once, so it must be run in a loop [see function waiting() above].
Once it determines that a key has been pressed it runs the appropriate action
and returns the character which was pressed.
*/
char checkKey(int i) {

	 byte a = PTH & 0xf0;   //activates the columns in the keypad. (needed when polling the keys in a row) 
	 
	 //represent the columns of the array of keys (column 1 to 4)
	 byte col1 = 0x10;  //column 1
	 byte col2 = 0x20; //column 2
	 byte col3 = 0x40;	//column 3
	 byte col4 = 0x80;  //column 4
	 
	 //row one of keys on keypad
	 if(i == 1){
	 	  //key '1' pressed
	 	  if (a==col1) return '1';
		  
		  //key '2'pressed
	 	  else if (a==col2)return '2';
		  
		  //key '3' pressed 
	 	  else if (a==col3)return '3';
		  
		  //key 'A' pressed
	 	  else if (a==col4){
		  	   windowSensor = TRUE; //emulates opening the window
   			   
			    //if the window sensor is on/true, turn on the buzzer
			   if((windowSensor)&& (alarmArmed)) {
			   	 soundBuzzer(); //sound buzzer
			   }
			   return 'A';
			  }
	 }
	 
	 //row two of keys on keypad
	 else if(i == 2){
	 	  //key '4' pressed
	 	  if (a==col1)return '4';
		  
		  //key '5' pressed
	 	  else if (a==col2)return '5';
		 
		 //key '6' pressed
	 	  else if (a==col3)return '6';
		  
		  //key 'B' pressed
	 	  else if (a==col4){
		  	   InfraredSensor = TRUE; //emulate presence (an infrared sensor is connected to this circuit)
			   
			   //if the infrared sensor is on/true, turn on the buzzer
			   if((InfraredSensor)&& (alarmArmed)) //if the infrared sensor is on/true and the alarm is armed,
			   {
			        soundBuzzer(); //sound the buzzer
			   }
			   
			   return 'B';
		  }
	 }
	 
	  //row three of keys on keypad
	 else if(i == 3){
	 	  
		  ///key '7' pressed
	 	  if (a==col1)return '7';
		  
		  //key '8' pressed
	 	  else if (a==col2)return '8';
		  
		  //key '9' pressed
	 	  else if (a==col3)return '9';
		  
		  //key 'C' pressed
	  	  else if (a==col4){
		       	  LCD_instruction(0x8d);
				  ++temperature_limit; //increase temperature by 1 degree C		   
				   //Print out the value of the temperature onto the LCD screen
	 			   printLCD(temperature_limit / 10 + 0x30); //get the MSB and convert to ascii. then print it out.
	 			   printLCD(temperature_limit % 10 + 0x30); //get LSB and convert to ascii. then print it out.
				   printLCD('C');
				   duty++;
			   return 'C';
		  }
	 }
	 
	 //row four of keys on keypad
	 else if(i == 4){
	  	  //letter E
	  	  if (a==col1) {
		  	 alarmChanged = TRUE;
			 return 'E';
		  }
		  //letter 0
	 	  else if (a==col2)return '0';
		  //letter F
	 	  else if (a==col3){
			   //The line below is supposed to stop the program (i.e when F key is pressed, but the question required that 
			   //"THE PROGRAM RUNS FOREVER UNTIL WE RESET THE BOARD")
			   
			   F_found = 1;
			   
			   return 'F';
		  }
		  //letter D
	  	  else if (a==col4){
		  	   	  LCD_instruction(0x8d);
				  --temperature_limit; //decrease temperature by 1 degree C
				   //Print out the value of the temperature onto the LCD screen
	 			   printLCD(temperature_limit / 10 + 0x30); //get the MSB and convert to ascii. then print it out.
	 			   printLCD(temperature_limit % 10 + 0x30); //get LSB and convert to ascii. then print it out.
				   printLCD('C');
				   duty--;
			   return 'D';
		  }
	  }
	 return ''; //always going to be 1 of 4 rows.
}

// Sound the buzzer on the board for one second.
void soundBuzzer() {
	int i; //for the delay macro
	int x; //for the for-loop
	DDRK = 0xFF; // outputs for the buzzer set
	PORTK |= 0x20; //turning on the buzzer bit
	
	//delay for around 1 second
	for(x = 0; x < 4; x++) {
		  delay(DELAY_MAX); 
	}
	
	PORTK = 0x00;  //turn off the buzzer afterwards
	DDRK = 0x00; // outputs
}

// Initialize the timer interrupts.
void initializeTimerInterrupts() {
CRGINT |= 0x80; //enable timer interrupts
RTICTL |= 0x7F; //this sets the prescale factor as 16 x 2^16 (from observation, we found the clock is 4MHz)
}

// Initialize the temperature interrupts
void initializeTemperatureInterrupts() {
	 DDRM = 0x80; //set direction on Port M so heater can turn on and off.
	 ATD0CTL2 = 0xFA; // Enable ATD & ATint.
	 ATD0CTL3 = 0x00; // Continue conversions
	 ATD0CTL4 = 0x60; // Same as previous example
	 ATD0CTL5 = 0x86; // Right justified. Unsigned. Scan mode
}

// Initialize the heater.
void initializeHeater() {
	 DDRM = 0xF0;
}

// Disables the ports used for the temperature interrupts so as not to 
// disturb other devices on the same ports.
void disableTemperatureInterrupts(void) {
	 DDRM = 0x00; //clear direction on Port M so heater can turn on and off.
	 ATD0CTL2 = 0x00; // Disable ATD & ATint.
	 ATD0CTL3 = 0x00; // Continue conversions
	 ATD0CTL4 = 0x00; // Clearing this port
	 ATD0CTL5 = 0x00; // Clearing the control.
}

// Initialize the keypad interrupt ports.
void initializeKeypadInterrupts() {
	 DDRP |= 0x0F; // bitset PP0-3 as outputs (rows)  
	 DDRH &= 0x0F; // bitclear PH4..7 as inputs (columns)
	 PTP = 0x0F; // Set scan row(s)
	 PIFH = 0xFF; // Clear previous interrupt flags
	 PPSH = 0xF0; // Rising Edge
	 PERH = 0x00; // Disable pulldowns
	 PIEH |= 0xF0; // Local enable on columns inputs
	 initPassPos = 0x04;  //keeps track of cursor position; intialised to 1st digit position
	 key = 0xFF;
}

//Initialize the keypad ports for use by the keypad detection interrupts.
void initializeKeypadPorts() {
	 	DDRP |= 0x0F; // bitset PP0-3 as outputs (rows)  
	 	DDRH &= 0x0F; // bitclear PH4..7 as inputs (columns)
	 	PTM = PTM | 0x08;
	 	PTP = 0x0F; //set scan rows
	 	//set U7_EN low
	 	PTM = PTM & 0xf7;
	 	DDRP |= 0x0F; // bitset PP0-3 as outputs (rows)
}

//Prints a given string at the current location of the cursor
// on the LCD screen. 
void printLCDString(char string[]) {
	 int i;
	 int length = strlen(string);
	 for(i = 0; i < length; i++) {
	  	   printLCD(string[i]);
	 }
}

void DelayNX(int n)
{
	int x;
	int y;
	while(n > 0)
	{
	  for(y=0; y<9900; y++)
	  {
		//for(x=0; x<9990; x++)
		//{
			asm("pshx");
			asm("pulx");
			asm("pshx");
			asm("pulx");
		//}
	  }
		
	  n--;
	}
}

//Delay a specified value. The value does not represent seconds, but just an arbitary wait time.
void delay(int val) {
	 for(delayVal = 0; delayVal < val; delayVal++);
}

// Changes the alarm from disarmed to armed or vise-versa.
void changeAlarm() {
	 alarmArmed = !alarmArmed; //toggles the alarm
	 LCD_instruction(0x89);
	 //if its armed now...
	 if(alarmArmed == TRUE){
	 		printLCD('A'); // A for Armed
	 }
	 else printLCD('D'); // D for Disarmed
	 if(alarmArmed == TRUE) {
	 	soundBuzzer(); //alarm armed so play beep for 1 sec.
	 }
}

// - - - - INTERRUPT SERVICE ROUTINES START HERE - - - - //

// Temperature ISR is used for when the temperature is read from the sensor.
#pragma interrupt_handler TEMPERATURE_ISR
void TEMPERATURE_ISR(void) {
	 temporary =  ATD0DR0 & 0x03FF; //read temperature and ensure 10-bit resolution
	 temporary = temporary >> 3; //divide by 2, three times.
	 temporary = temporary - 5; //offset here for the bias
	 temperature = temporary; //set to temperature variable
	 ready = TRUE; //now ready to be read!
	 delay(10); // delay so it can be read
	 ATD0CTL5 = 0x86; // Start a new conversion
}

// Allows a delay for a specific value of time.
#pragma interrupt_handler OC_TIMER_ISR
void OC_TIMER_ISR(void) {
	 TC0 = TCNT;
}

// The Keypad ISR changes the global variable "key" when a key
// is pressed on the keypad. //
#pragma interrupt_handler KEYPAD_ISR
void KEYPAD_ISR(void) {
	 pressed = TRUE;
	 PIFH = PIFH; // Acknowledge (all) interrupts
}

/* The Timer ISR is called every time the real time interrupt occurs! 
   In this case it counts how many times it has occurred. 
   numInterrupts is increased by 1 approximately once per second.
   The interrupt handler is used in causing a 3 second delay.
*/
#pragma interrupt_handler _Timer_ISR
void _Timer_ISR(void) {
	 divisor += 1;   //increment the divisor by 1 every interrupt
	 if(divisor == 2) { //do action only 1 per 4 interrupt.
	 		divisor = 0x00; //reset dummy.
			numInterrupts += 1; //increment tick
			ticks+=1;
	 }
	 CRGFLG |= 0x80; //Clear the RTI flag
}

#pragma interrupt_handler pacA_isr()
void pacA_isr(void) {
     INTR_OFF();
     //printf("count:%x \n", optCount);
    // printf("duty:%d \n", duty);	 
     PAFLG |= 1;
	 //curr_speed = motor_speed();
     INTR_ON();
}

// - - - - INTERRUPT SERVICE ROUTINES END HERE - - - - //

// ***** START OF INTERRUPT VECTOR TABLE HERE ***** //

/* To use, either add this file to the project file list (preferred), or 
 * #include it in a single source file if you are just using 
 * File->CompileToOutput (this works but using the Project feature is better
 */

/* Vector Table for Dx256 S12 CPU
 * As is, all interrupts except reset jumps to 0xffff, which is most
 * likely not going to useful. To replace an entry, declare your function,
 * and then change the corresponding entry in the table. For example,
 * if you have a SCI handler (which must be defined with 
 * #pragma interrupt_handler ...) then in this file:
 * add
 *	extern void SCIHandler();
 * before th table.
 * In the SCI entry, change:
 *	DUMMY_ENTRY,
 * to
 *  SCIHandler, 
 */
#pragma nonpaged_function _start
extern void _start(void);	/* entry point in crt??.s */

//tells the compiler that _Timer_ISR is an interrupt and cannot be called directly
#pragma interrupt_handler _Timer_ISR
void _Timer_ISR(void);

#pragma interrupt_handler TEMPERATURE_ISR
void TEMPERATURE_ISR(void);

#pragma interrupt_handler KEYPAD_ISR
void KEYPAD_ISR(void);

#pragma interrupt_handler OC_TIMER_ISR
void OC_TIMER_ISR(void);

#pragma interrupt_handler pacA_isr
void pacA_isr(void);

#define NOICE_DUMMY_ENTRY (void (*)(void))0xF8CF

#define NOICE_XIRQ	(void (*)(void))0xF8C7
#define NOICE_SWI	(void (*)(void))0xF8C3
#define NOICE_TRAP	(void (*)(void))0xF8CB
#define NOICE_COP	(void (*)(void))0xF805
#define NOICE_CLM	(void (*)(void))0xF809

//#pragma abs_address:0xFF80 		//Used for Standalone Apps.
#pragma abs_address:0x3F8C 		//Used with the monitor

/* change the above address if your vector starts elsewhere
 */
void (*interrupt_vectors[])(void) = 
	{
	/* to cast a constant, say 0xb600, use
	   (void (*)())0xb600
	 */
	//NOICE_DUMMY_ENTRY, /*Reserved $FF80*/  //Not used under Monitor
	//NOICE_DUMMY_ENTRY, /*Reserved $FF82*/  //Not used under Monitor
	//NOICE_DUMMY_ENTRY, /*Reserved $FF84*/  //Not used under Monitor
	//NOICE_DUMMY_ENTRY, /*Reserved $FF86*/  //Not used under Monitor
	//NOICE_DUMMY_ENTRY, /*Reserved $FF88*/  //Not used under Monitor
	//NOICE_DUMMY_ENTRY, /*Reserved $FF8A*/  //Not used under Monitor
	NOICE_DUMMY_ENTRY, /*PWM Emergency Shutdown*/
	NOICE_DUMMY_ENTRY, /*Port P Interrupt*/
	NOICE_DUMMY_ENTRY, /*MSCAN 4 Transmit*/
	NOICE_DUMMY_ENTRY, /*MSCAN 4 Receive*/
	NOICE_DUMMY_ENTRY, /*MSCAN 4 Error*/
	NOICE_DUMMY_ENTRY, /*MSCAN 4 Wake-up*/
	NOICE_DUMMY_ENTRY, /*MSCAN 3 Transmit*/
	NOICE_DUMMY_ENTRY, /*MSCAN 3 Receive*/
	NOICE_DUMMY_ENTRY, /*MSCAN 3 Error*/
	NOICE_DUMMY_ENTRY, /*MSCAN 3 Wake-up*/
	NOICE_DUMMY_ENTRY, /*MSCAN 2 Transmit*/
	NOICE_DUMMY_ENTRY, /*MSCAN 2 Receive*/
	NOICE_DUMMY_ENTRY, /*MSCAN 2 Error*/
	NOICE_DUMMY_ENTRY, /*MSCAN 2 Wake-up*/
	NOICE_DUMMY_ENTRY, /*MSCAN 1 Transmit*/
	NOICE_DUMMY_ENTRY, /*MSCAN 1 Receive*/
	NOICE_DUMMY_ENTRY, /*MSCAN 1 Error*/
	NOICE_DUMMY_ENTRY, /*MSCAN 1 Wake-up*/
	NOICE_DUMMY_ENTRY, /*MSCAN 0 Transmit*/
	NOICE_DUMMY_ENTRY, /*MSCAN 0 Receive*/
	NOICE_DUMMY_ENTRY, /*MSCAN 0 Error*/
	NOICE_DUMMY_ENTRY, /*MSCAN 0 Wake-up*/
	NOICE_DUMMY_ENTRY, /*Flash*/
	NOICE_DUMMY_ENTRY, /*EEPROM*/
	NOICE_DUMMY_ENTRY, /*SPI2*/
	NOICE_DUMMY_ENTRY, /*SPI1*/
	NOICE_DUMMY_ENTRY, /*IIC Bus*/
	NOICE_DUMMY_ENTRY, /*DLC*/
	NOICE_DUMMY_ENTRY, /*SCME*/
	NOICE_DUMMY_ENTRY, /*CRG Lock*/
	NOICE_DUMMY_ENTRY, /*Pulse Accumulator B Overflow*/
	NOICE_DUMMY_ENTRY, /*Modulus Down Counter Underflow*/
	KEYPAD_ISR, /*Port H Interrupt*/
	NOICE_DUMMY_ENTRY, /*Port J Interrupt*/
	NOICE_DUMMY_ENTRY, /*ATD1*/
	TEMPERATURE_ISR, /*ATD0*/
	NOICE_DUMMY_ENTRY, /*SCI1*/
	NOICE_DUMMY_ENTRY, /*SCI0*/
	NOICE_DUMMY_ENTRY, /*SPI0*/
	pacA_isr, /*Pulse Accumulator A Input Edge*/
	NOICE_DUMMY_ENTRY, /*Pulse Accumulator A Overflow*/
	NOICE_DUMMY_ENTRY, /*Timer Overflow*/
	NOICE_DUMMY_ENTRY, /*Timer Channel 7*/
	NOICE_DUMMY_ENTRY, /*Timer Channel 6*/
	NOICE_DUMMY_ENTRY, /*Timer Channel 5*/
	NOICE_DUMMY_ENTRY, /*Timer Channel 4*/
	NOICE_DUMMY_ENTRY, /*Timer Channel 3*/
	NOICE_DUMMY_ENTRY, /*Timer Channel 2*/
	NOICE_DUMMY_ENTRY, /*Timer Channel 1*/
	OC_TIMER_ISR, /*Timer Channel 0*/
	_Timer_ISR, /*Real Time Interrupt
	NOICE_DUMMY_ENTRY, /*IRQ*/
	NOICE_XIRQ, /*XIRQ*/
	NOICE_SWI, /*SWI*/
	NOICE_TRAP, /*Unimplement Intruction Trap*/
	NOICE_COP, /*COP failure reset*/
	NOICE_CLM, /*Clock monitor fail reset*/
	_start, /*Reset*/
	};
#pragma end_abs_address
// ***** END OF INTERRUPT VECTOR TABLE HERE ***** //