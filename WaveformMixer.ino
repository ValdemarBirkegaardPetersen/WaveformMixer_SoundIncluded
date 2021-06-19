#include <U8g2lib.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include <math.h>

//Declaring int's for the diffrent button pins on the Teensy, and making int conditionals that will save the state of each button as either "0" or "1" 
int button1 = 2; 
int button2 = 12;
int button3 = 11; 
int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;

#include <Audio.h>
AudioSynthWaveform waveform_one;
AudioSynthWaveform waveform_two;
AudioOutputI2S i2s1;
AudioOutputAnalogStereo dacs1;
//AudioOutputUSB usb1;             //if the audio should be outputted to the PC instead of the jackstick in the audio shield
AudioConnection patchCord1(waveform_one, 0, i2s1, 0);
AudioConnection patchCord2(waveform_one, 0, dacs1, 0);
AudioConnection patchCord3(waveform_two, 0, i2s1, 1);
AudioConnection patchCord4(waveform_two, 0, dacs1, 1);
AudioControlSGTL5000 sgtl5000_1;
int active_waveform=0;

int analogPin = 14; //Fill this out correctly
int analogValue = 0; 

int xspacing = 2;  //The spacing between lines
float theta = 0.0;  //Used to increment x, so that the screen moves horizontally
float amplitude = 10.0;  //Particulary used in the square() function.
float freq;  //The frequency of the audio wave being played
float period = 40.0;  //The period/frequency of the wave being displayed on the OLED
float dx;
float yvalues [72];  //Array for storing y-values of the different waveforms. Calculated differently for each wave. 


//Pin 18 and 19 (analog) are specified, and so is the I2C communication method
U8G2_SSD1306_128X64_ALT0_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 19, /* data=*/ 18, /* reset=*/ U8X8_PIN_NONE);    //Low spped I2C


void setup() {
  //Defining the pins for each button and setting it as input. 
  pinMode(button1, INPUT); 
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(analogPin, INPUT);

  //Starts the audio memory.
  AudioMemory(10);       

  //Enables and sets the volume for the audio board.
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8); // caution: very loud - use oscilloscope only!

  //Set frequency 440Hz and max Amplitude 1.0
  waveform_one.frequency(440);
  waveform_two.frequency(440);

  //Implement this into the loop() or the application in general. Sets waveform and begins it. 
  active_waveform = WAVEFORM_SINE;
  
    //Starting the u8g2 library
  u8g2.begin();
  //Starting the serial on baud rate 115200
  Serial.begin(115200);

}

void loop() {
  waveform_one.begin(active_waveform);
  //Using the built-in digital read function in order to read the state of button connected to the breadboard. When the button is held down it will receive 5v and stay HIGH. When their is no action on the button the state is LOW
  buttonState1 = digitalRead(button1);
  buttonState2 = digitalRead(button2);
  buttonState3 = digitalRead(button3);

  //Used to increment x one time around the unit cirlce (TWO_PI). The lines are spaced by 2 stored in xspacing
  dx = (TWO_PI / period) *xspacing;

  //Reading the value from the potientiometer by using analogRead()
  analogValue = analogRead(analogPin);

  //Reading the value from the potientiometer by using analogRead()
  analogValue = analogRead(analogPin);
  //Mapping the value to convert from analog range to a range that is reasonable to use on our waveforms (we are changning frequency)
  period = map(analogValue, 0, 1023, 80, 10);
  //Mapping the value to convert analog range to the freq range used (0.0 - 1.0)
  freq = map(analogValue, 0, 1023, 280, 900);

  //Initating the buffer for the OLED and graphics library. Everything will be drawn between clearBuffer() and sendBuffer() 
  
  u8g2.clearBuffer();
  //Drawing a black box, filling the whole screen in order to get a stabile background
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 128, 64);

  //If all buttons are LOW (0V) then we will display the main page and not play any sounds.
  if(buttonState1 == LOW && buttonState2 == LOW && buttonState3 == LOW) {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_DigitalDisco_tf);
    u8g2.drawStr(18 ,23,"Sound Mixer");
}
  
  theta += 0.22;    //Incrementing by 0.22 in order to horizontally move the screen real-time
  float x = theta;    



  if (buttonState1 == HIGH) {           //SINUS WAVE                    
    active_waveform = WAVEFORM_SINE;
    waveform_one.amplitude(1);
    waveform_one.frequency(freq);
    for (int i = 0; i < 72; i++) {              //For-loop for filling the y-position array with sinus wave numbers. Also multiplying by amplitude
      yvalues[i] = sin(x)*amplitude;
      x += dx;          //Increment x in a full cirlcle TWO_PI / period 
  }
  //For loop for rendering  
  for (int ii = 0; ii < 72; ii++) {    
    u8g2.setDrawColor(1);
    u8g2.drawLine(ii*xspacing, 15-yvalues[ii], (ii+1)*xspacing, 15-yvalues[ii+1]);
  }

  
}  else if(buttonState2 == HIGH) {      //SQUARE-WAVE
   waveform_one.amplitude(1);
   waveform_one.frequency(freq);
   active_waveform = WAVEFORM_SQUARE;
   square(x);
   for (int ii = 0; ii < 72; ii++) {     //For loop for rendering
      u8g2.setDrawColor(1);
      u8g2.drawLine(ii*xspacing, 15-yvalues[ii], (ii+1)*xspacing, 15-yvalues[ii+1]);
  } 

  
} else if (buttonState3 == HIGH) {     // SAWTOOTH - WAVE
  active_waveform = WAVEFORM_SAWTOOTH;
  waveform_one.amplitude(1); 
  waveform_one.frequency(freq);
  for(int i = 0; i < 72; i++) {
    double nx = fmod(x, TWO_PI)  * dx;      
    yvalues[i] = (-1 + nx) * amplitude;
    x += dx;  
        }
  for (int ii = 0; ii < 72; ii++) {   //For loop for rendering
    u8g2.setDrawColor(1);
    u8g2.drawLine(ii*xspacing, 15-yvalues[ii], (ii+1)*xspacing, 15-yvalues[ii+1]);
    }
}    else {
  waveform_one.amplitude(0);
}
    //Sending the buffer and displaying on OLED
    u8g2.sendBuffer();
}


void square(float input) {      //Square-Wave Function
  for(int i = 0; i < 72; i++) {
  float c = sin(input) * amplitude;
      if(c < 0) {
        yvalues[i] = -amplitude;
  } else {
    yvalues[i] = amplitude;
    }
    input += dx;
  }
}
