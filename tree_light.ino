/*
* Author: Lukas Žaromskis
* Tree light controller.
* Contains multiple light modes that can be changed by pressing a button.
*/

#include <Adafruit_NeoPixel.h>
#include <stdint.h>
#include "led.h"
#include "hsv_rgb.h"

#define PIN 6
#define NUMBER_OF_LEDS 9
#define NUMBER_OF_MODES 6
#define GLOBAL_DELAY 25

/*
* Function prototypes
*/
void CorrectLEDIndexes();
float lerp(float current, float desired, float t);
void mode0_init();
void mode0();
void mode1_init();
void mode1();
void mode2_init();
void mode2();
void mode3_init();
void mode3();
void mode4_init();
void mode4();
void mode5_init();
void mode5();
void ChangeMode();
void updateLEDMatrixHSV(unsigned long wait);
void updateLEDMatrixRGB(unsigned long wait);

/*
* Global variables
*/
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_LEDS, PIN, NEO_GRB + NEO_KHZ800);

int i = 1;

void (*mode_function)();

LED *LEDs;

int mode = 0;

void setup()
{
	attachInterrupt(digitalPinToInterrupt(2), ChangeMode, RISING);
	mode_function = &mode0;
	LEDs = InitLEDs(NUMBER_OF_LEDS);
	CorrectLEDIndexes();
	SetHSVToAll(LEDs, 0.0f, 1.0f, 1.0f, NUMBER_OF_LEDS);
	strip.begin();
	strip.show();
}

void loop()
{
	(*mode_function)();
}

/*
* Sets the indexes to:
* 6 5 4 
* 7 8 3
* 0 1 2
*/
void CorrectLEDIndexes()
{
	LEDs[0].id = 0;
	LEDs[1].id = 1;
	LEDs[2].id = 2;
	LEDs[3].id = 5;
	LEDs[4].id = 8;
	LEDs[5].id = 7;
	LEDs[6].id = 6;
	LEDs[7].id = 3;
	LEDs[8].id = 4;
}

float lerp(float current, float desired, float t)
{
	if (t <= 0.0f)
	{
		return current;
	}
	if (t >= 1.0f)
	{
		return desired;
	}
	float diff = desired - current;
	float change = diff * t;
	return current + change;
}

/*
* MODE0 
* Colors change through the hue wheel
*/
float M0_HueStep = 0.001f;

void mode0_init()
{
	SetHSVToAll(LEDs, 0.0f, 1.0f, 1.0f, NUMBER_OF_LEDS);
}

void mode0()
{
	LED *led;
	for (i = 0; i < NUMBER_OF_LEDS; i++)
	{
		led = &LEDs[i];
		UpdateColorWheelHSV(led, M0_HueStep);
	}
	updateLEDMatrixHSV(GLOBAL_DELAY);
}

/*
* MODE1
* The outer ring if the LEDs travel through the hue wheel.
* Each LED is shifted by 0.125 in the hue wheel.
*/
float M1_HueStep = 0.001f;

void mode1_init()
{
	SetHSV(&LEDs[0], 0.0f, 1.0f, 1.0f);
	SetHSV(&LEDs[1], 0.125f, 1.0f, 1.0f);
	SetHSV(&LEDs[2], 0.25f, 1.0f, 1.0f);
	SetHSV(&LEDs[3], 0.375f, 1.0f, 1.0f);
	SetHSV(&LEDs[4], 0.5f, 1.0f, 1.0f);
	SetHSV(&LEDs[5], 0.625f, 1.0f, 1.0f);
	SetHSV(&LEDs[6], 0.75f, 1.0f, 1.0f);
	SetHSV(&LEDs[7], 0.875f, 1.0f, 1.0f);
	SetHSV(&LEDs[8], 0.0f, 0.0f, 0.0f);
}

void mode1()
{
	LED *led;
	for (i = 0; i < NUMBER_OF_LEDS; i++)
	{
		led = &LEDs[i];
		UpdateColorWheelHSV(led, M1_HueStep);
	}
	updateLEDMatrixHSV(GLOBAL_DELAY);
}

/*
* MODE2
* Colors change through the color wheel and saturation periodicaly goes up and down.
*/
uint8_t M2_IsSaturationIncreasing = 0;
float M2_SaturationMax = 1.0f;
float M2_SaturationMin = 0.8f;
float M2_SaturationStep = 0.0001f;
float M2_HueStep = 0.001f;

void mode2_init()
{
	SetHSVToAll(LEDs, 0.0f, 1.0f, 1.0f, NUMBER_OF_LEDS);
	M2_IsSaturationIncreasing = 0;
}

void mode2()
{
	LED *led;
	for (i = 0; i < NUMBER_OF_LEDS; i++)
	{
		led = &LEDs[i];
		UpdateColorWheelHSV(led, M2_HueStep);
		if (M2_IsSaturationIncreasing == 1)
		{
			led->saturation += M2_SaturationStep;
			if (led->saturation >= M2_SaturationMax)
			{
				M2_IsSaturationIncreasing = 0;
			}
		}
		else
		{
			led->saturation -= M2_SaturationStep;
			if (led->saturation <= M2_SaturationMin)
			{
				M2_IsSaturationIncreasing = 1;
			}
		}
	}
	updateLEDMatrixHSV(GLOBAL_DELAY);
}

/*
* MODE3
* Only two LEDs are on at the same time and they are traveling in a circle.
* The color changes through the hue wheel.
*/
float M3_CurrentHue = 0.0f;
int M3_CurrentLED = 0;
int M3_LastLEDIndex = 7;
float M3_HueStep = 0.001f;
float M3_ValueStep = 0.05f;

void mode3_init()
{
	SetHSVToAll(LEDs, 0.0f, 1.0f, 0.0f, NUMBER_OF_LEDS);
	SetHSV(&LEDs[0], 0.0f, 1.0f, 0.5f);
	SetHSV(&LEDs[1], 0.0f, 1.0f, 0.5f);
	M3_CurrentLED = 0;
	M3_CurrentHue = 0.0f;
}

void mode3()
{
	int nextLED;
	LED *current, *next;
	if (M3_CurrentLED == M3_LastLEDIndex)
	{
		nextLED = 0;
	}
	else
	{
		nextLED = M3_CurrentLED + 1;
	}

	current = &LEDs[M3_CurrentLED];
	next = &LEDs[nextLED];

	M3_CurrentHue += M3_HueStep;
	if (M3_CurrentHue >= 1.0f)
	{
		M3_CurrentHue -= 1.0f;
	}

	current->hue = M3_CurrentHue;
	next->hue = M3_CurrentHue;

	float *currentLEDValue = &current->value;
	float *nextLEDValue = &next->value;

	*currentLEDValue -= M3_ValueStep;
	*nextLEDValue += M3_ValueStep;

	if (*currentLEDValue <= 0.0f)
	{
		*currentLEDValue = 0.0f;
		*nextLEDValue = 1.0f;
		M3_CurrentLED = nextLED;
	}

	updateLEDMatrixHSV(GLOBAL_DELAY);
}

/*
* MODE4
* Every LED is given a random hue and saturation value every n miliseconds.
*/
float M4_SaturationMin = 0.85f;
float M4_SaturationRand = 150;
int M4_Delay = 1000;

void mode4_init()
{
	SetHSVToAll(LEDs, 0.0f, 0.0f, 1.0f, NUMBER_OF_LEDS);
}

void mode4()
{
	LED *led;
	for (int i = 0; i < NUMBER_OF_LEDS; i++)
	{
		led = &LEDs[i];
		float hue = (float)random(1000) / 1000.0f;
		float saturation = ((float)random(M4_SaturationRand) / 1000.0f) + M4_SaturationMin;
		led->hue = hue;
		led->saturation = saturation;
	}
	updateLEDMatrixHSV(M4_Delay);
}

/*
* MODE5
* All of the LEDs are given the same random hue and saturation value every n miliseconds.
* The color change goes through a linear interpolation through the RGB values.
*/
float M5_SaturationMin = 0.85f;
int M5_SaturationRand = 150;
int M5_Delay = 8000;
int M5_LerpDuration = 2000;
uint8_t M5_NextRed = 0;
uint8_t M5_NextGreen = 0;
uint8_t M5_NextBlue = 0;
uint8_t M5_CurrentRed = 0;
uint8_t M5_CurrentGreen = 0;
uint8_t M5_CurrentBlue = 0;
float M5_LerpTime = 0;
uint8_t M5_IsLerping = 0;

void mode5_init()
{
	float hue = (float)random(1000) / 1000.0f;
	float saturation = ((float)random(M5_SaturationRand) / 1000.0f) + M5_SaturationMin;
	float* rgb = (float *)malloc(3 * sizeof(float));
	rgb = hsv2rgb(hue, saturation, 1.0f, rgb);
	M5_NextRed = rgb[0] * 255;
	M5_NextGreen = rgb[1] * 255;
	M5_NextBlue = rgb[2] * 255;
	free(rgb);
	M5_CurrentRed = 255;
	M5_CurrentGreen = 255;
	M5_CurrentBlue = 255;
	M5_IsLerping = 1;
	M5_LerpTime = 0;
	SetRGBToAll(LEDs, M5_CurrentRed, M5_CurrentGreen, M5_CurrentBlue, NUMBER_OF_LEDS);
}

void mode5()
{
	if (M5_IsLerping)
	{
		float t = M5_LerpTime / M5_LerpDuration;
		float r = lerp(M5_CurrentRed, M5_NextRed, t);
		float g = lerp(M5_CurrentGreen, M5_NextGreen, t);
		float b = lerp(M5_CurrentBlue, M5_NextBlue, t);
		SetRGBToAll(LEDs, r, g, b, NUMBER_OF_LEDS);
		M5_LerpTime += GLOBAL_DELAY;
		if (M5_LerpTime >= M5_LerpDuration)
		{
			M5_IsLerping = 0;
			M5_LerpTime = 0;
			M5_CurrentRed = M5_NextRed;
			M5_CurrentGreen = M5_NextGreen;
			M5_CurrentBlue = M5_NextBlue;
			float hue = (float)random(1000) / 1000.0f;
			float saturation = ((float)random(M5_SaturationRand) / 1000.0f) + M5_SaturationMin;
			float* rgb = (float *)malloc(3 * sizeof(float));
			rgb = hsv2rgb(hue, saturation, 1.0f, rgb);
			M5_NextRed = rgb[0] * 255;
			M5_NextGreen = rgb[1] * 255;
			M5_NextBlue = rgb[2] * 255;
			free(rgb);
		}
		updateLEDMatrixRGB(GLOBAL_DELAY);
	}
	else
	{
		SetRGBToAll(LEDs, M5_CurrentRed, M5_CurrentGreen, M5_CurrentBlue, NUMBER_OF_LEDS);
		M5_IsLerping = 1;
		updateLEDMatrixRGB(M5_Delay);
	}
}


/*
* This function changes the current mode of the lights.
*/
void ChangeMode()
{
	mode++;
	if (mode == NUMBER_OF_MODES)
	{
		mode = 0;
	}
	switch (mode)
	{
	case 0:
		mode_function = &mode0;
		mode0_init();
		break;
	case 1:
		mode_function = &mode1;
		mode1_init();
		break;
	case 2:
		mode_function = &mode2;
		mode2_init();
		break;
	case 3:
		mode_function = &mode3;
		mode3_init();
		break;
	case 4:
		mode_function = &mode4;
		mode4_init();
		break;
	case 5:
		mode_function = &mode5;
		mode5_init();
		break;
	default:
		break;
	}
	delay(100);
}

/*
* Updates the lights using their HSV values.
*/
void updateLEDMatrixHSV(unsigned long wait)
{
	UpdateLEDsHSV(&strip, LEDs, NUMBER_OF_LEDS);
	delay(wait);
}

/*
* Updates the lights using their RGB values.
*/ 
void updateLEDMatrixRGB(unsigned long wait)
{
	UpdateLEDsRGB(&strip, LEDs, NUMBER_OF_LEDS);
	delay(wait);
}