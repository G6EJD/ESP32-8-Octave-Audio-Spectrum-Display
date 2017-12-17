/* ESP32 Audio Spectrum Analyser on an SSD1306/SH1106 Display, 8-bands 125, 250, 500, 1k, 2k, 4k, 8k, 16k
 * Improved noise performance and speed and resolution.
 * The MIT License (MIT) Copyright (c) 2017 by David Bird. 
 * The formulation and display of an AUdio Spectrum using an ESp8266 or ESP32 and SSD1306 or SH1106 OLED Display using a Fast Fourier Transform
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files 
 * (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, but not to use it commercially for profit making or to sub-license and/or to sell copies of the Software or to 
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:  
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * See more at http://dsbird.org.uk 
*/

#include <Wire.h>
#include "arduinoFFT.h" // Standard Arduino FFT library https://github.com/kosme/arduinoFFT
arduinoFFT FFT = arduinoFFT();
/////////////////////////////////////////////////////////////////////////
// Comment out the display your NOT using e.g. if you have a 1.3" display comment out the SSD1306 library and object
#include "SH1106.h"     // https://github.com/squix78/esp8266-oled-ssd1306
SH1106 display(0x3c, 17,16); // 1.3" OLED display object definition (address, SDA, SCL) Connect OLED SDA , SCL pins to ESP SDA, SCL pins

//#include "SSD1306.h"  // https://github.com/squix78/esp8266-oled-ssd1306
//SSD1306 display(0x3c, 16,17);  // 0.96" OLED display object definition (address, SDA, SCL) Connect OLED SDA , SCL pins to ESP SDA, SCL pins
/////////////////////////////////////////////////////////////////////////
#include "font.h" // The font.h file must be in the same folder as this sketch
/////////////////////////////////////////////////////////////////////////
#define SAMPLES 1024             // Must be a power of 2
#define SAMPLING_FREQUENCY 40000 // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define amplitude 150            // Depending on your audio source level, you may need to increase this value
unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[] = {0,0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;
int dominant_value;
/////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Wire.begin(17,16); // SDA, SCL
  display.init();
  display.setFont(Dialog_plain_8);
  //display.setFont(ArialMT_Plain_10);
  display.flipScreenVertically(); // Adjust to suit or remove
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

void loop() {
  display.clear();        
  display.drawString(0,0,"125 250 500 1K  2K 4K 8K 16K");
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros()-oldTime;
    oldTime = newTime;
    vReal[i] = analogRead(A0); // Using Arduino ADC nomenclature. A conversion takes about 1uS on an ESP32
  //vReal[i] = analogRead(VP); // Using logical name fo ADC port
  //vReal[i] = analogRead(36); // Using pin number for ADC port
    vImag[i] = 0;
    while (micros() < (newTime + sampling_period_us)) { /* do nothing to wait */ }
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  for (int i = 2; i < (SAMPLES/2); i++){ // Don't use sample 0 and only the first SAMPLES/2 are usable.
    // Each array element represents a frequency and its value, is the amplitude. Note the frequencies are not discrete.
    if (vReal[i] > 1500) { // Add a crude noise filter, 10 x amplitude or more
      if (i<=2 )             displayBand(0,(int)vReal[i]); // 125Hz
      if (i >2   && i<=4 )   displayBand(1,(int)vReal[i]); // 250Hz
      if (i >4   && i<=7 )   displayBand(2,(int)vReal[i]); // 500Hz
      if (i >7   && i<=15 )  displayBand(3,(int)vReal[i]); // 1000Hz
      if (i >15  && i<=40 )  displayBand(4,(int)vReal[i]); // 2000Hz
      if (i >40  && i<=70 )  displayBand(5,(int)vReal[i]); // 4000Hz
      if (i >70  && i<=288 ) displayBand(6,(int)vReal[i]); // 8000Hz
      if (i >288           ) displayBand(7,(int)vReal[i]); // 16000Hz
      //Serial.println(i);
    }
    for (byte band = 0; band <= 7; band++) display.drawHorizontalLine(1+16*band,64-peak[band],14);
  }
  if (millis()%4 == 0) {for (byte band = 0; band <= 7; band++) {if (peak[band] > 0) peak[band] -= 1;}} // Decay the peak
  display.display();
}

void displayBand(int band, int dsize){
  int dmax = 50;
  dsize /= amplitude;
  if (dsize > dmax) dsize = dmax;
  for (int s = 0; s <= dsize; s=s+2){display.drawHorizontalLine(1+16*band,64-s, 14);}
  if (dsize > peak[band]) {peak[band] = dsize;}
}

