# ESP32-8-Octave-Audio-Spectrum-Display
Using an ESP32 and OLED display together with an FFT to display audio as 8-octaves in the frequency domain.

An improved version solely for the ESP32 with its significantly faster ADC to enable an increase in the resolution of the frequency domain by a factor of 4, whilst also giving an improved dynamic range. Code improvements have also been made to speed up the processing together with a shorter code length.

You must place the font.h file in the same location as the sketch. The Font file is needed to provide a font that can display enough characters across the screen to enable each band to be clearly denoted. The bands are:

125Hz
250Hz
500Hz
1KHz
2Khz
4KHz
8Khz
16kHz

Please note, if you use a microphone - speaker test environment it is highly likely (unless of very high bandwidth and quality) that both the microphone and speakers used have insufficient quality to ensure you are getting a flat frequency response for the Fast Fourier Transform to convert and display. The only way to ensure you see the correct result is to feed approximately 50mV to 100mV pk-pk of audio directly into the ADC port with no DC offset. This is not a fault with the software or display or FFT - it’s the laws of physics!





