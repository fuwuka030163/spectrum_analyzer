#include <arduinoFFT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define ANALOG_PIN A0

#define SAMPLES 64
#define SAMPLING_FREQ 24000
#define FREQUENCY_BANDS 8

double vImag[SAMPLES];
double vReal[SAMPLES];
double bands[FREQUENCY_BANDS];
unsigned long sampling_period_us;

LiquidCrystal_I2C lcd(0x27, 16, 2);
arduinoFFT fft = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);

float reference = log10(50.0);

unsigned long newTime;
int analogValue;

byte vol0[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
};

byte vol1[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
};

byte vol2[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
};

byte vol3[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
};

byte vol4[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

byte vol5[8] = {
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

byte vol6[8] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

byte vol7[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

void setup(){
  Serial.begin(9600);
  Serial.println("Sampling... please, wait. ");
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, vol0);
  lcd.createChar(1, vol1);
  lcd.createChar(2, vol2);
  lcd.createChar(3, vol3);
  lcd.createChar(4, vol4);
  lcd.createChar(5, vol5);
  lcd.createChar(6, vol6);
  lcd.createChar(7, vol7);
  lcd.begin(16, 2);
  
  lcd.setCursor(0, 0);
  lcd.print("SpectrumAnalyzer");  
  for(int i = 0; i < 8; i++){
    lcd.setCursor(i, 1);
    lcd.write(byte(i));
    delay(1000);
  }  

  sampling_period_us = ((1.0 / SAMPLING_FREQ ) * 1000000);
  
}

void loop(){
  //sampling
  for (int i = 0; i < SAMPLES; i++) {   
    int value = analogRead(ANALOG_PIN);
    vReal[i] = value;
    vImag[i] = 0;
    delayMicroseconds(sampling_period_us);
    //while (micros() < (newTime + sampling_period_us)) {
    //}    
  }

  //FFT computation
  fft.DCRemoval();
  fft.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  fft.Compute(FFT_FORWARD);
  fft.ComplexToMagnitude();

  //arrange into bands  
  int steps = (SAMPLES / 2) / FREQUENCY_BANDS; 
  int idx = 0;
   
  for(int i = 0; i < (SAMPLES/2); i += steps){
    bands[idx] = 0;
    
    for(int j = 0; j < steps; j++){
      bands[idx] += vReal[i + j];
    }

    bands[idx] = bands[idx] / steps;
    bands[idx] = 20.0 * (log10(bands[idx]) - reference);
    
    idx++;        
  }
  
  
  //display
  lcd.clear();
  int bar_h;
  
  for(int i = 0; i < FREQUENCY_BANDS; i++){
    bands[i] = constrain(bands[i], 0, 16);    
    bar_h = bands[i];

    if(bar_h == 0){
      lcd.setCursor(i + 4, 0);
      lcd.write("");
      lcd.setCursor(i + 4, 1);    
      lcd.write("");
    }
    else if(bar_h <= 8){
      lcd.setCursor(i + 4, 0);
      lcd.write("");
      lcd.setCursor(i + 4, 1);    
      lcd.write(byte(bar_h - 1));
    }
    else{
      lcd.setCursor(i + 4, 0);
      lcd.write(byte(bar_h - 9));
      lcd.setCursor(i + 4, 1);    
      lcd.write(byte(7));
    }    
  }
  
}   
