/*
* Author: Lukas Žaromskis
* Contains the struct of an LED and has function for setting the data of the struct.
* Also contains functions to send the LED data to the Adafruit NeoPixel LED strip.
*/
#ifndef LED_H
#define LED_H

#include <Adafruit_NeoPixel.h>
#include "hsv_rgb.h"

struct LED
{
    uint16_t id;

    float hue;
    float saturation;
    float value;

    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

LED* InitLEDs(int n)
{
    LED* leds = (LED*)malloc(n * sizeof(struct LED));
    LED* led;
    for (int i = 0; i < n; i++)
    {
        led = &leds[i];
        led->id = i;
        led->hue = 0.0f;
        led->saturation = 1.0f;
        led->value = 1.0f;
        led->red = 255;
        led->blue = 0;
        led->green = 0;
    }
    return leds;
}

void SetHue(LED* led, float hue)
{
    led->hue = hue;
}

void SetSaturation(LED* led, float saturation)
{
    led->saturation = saturation;
}

void SetHSV(LED* led, float hue, float saturation, float value)
{
    led->hue = hue;
    led->saturation = saturation;
    led->value = value;
}

void SetRGB(LED* led, uint8_t red, uint8_t green, uint8_t blue)
{
    led->red = red;
    led->green = green;
    led->blue = blue;
}

void SetHSVToAll(LED* leds, float hue, float saturation, float value, int n)
{
    LED* led;
    for (int i = 0; i < n; i++)
    {
        led = &leds[i];
        SetHSV(led, hue, saturation, value);
    }
}

void SetRGBToAll(LED* leds, uint8_t red, uint8_t green, uint8_t blue, int n)
{
    LED* led;
    for (int i = 0; i < n; i++)
    {
        led = &leds[i];
        SetRGB(led, red, green, blue);
    }
}

void UpdateColorWheelHSV(LED* led, float step)
{
    float hue = led->hue;
    hue += step;
    if (hue > 1.0f)
    {
        hue -= 1.0f;
    }
    led->hue = hue;
}

void UpdateLEDsHSV(Adafruit_NeoPixel* strip, LED* leds, int n)
{
    float* rgb = (float *)malloc(3 * sizeof(float));
    LED* led;
    for (int i = 0; i < n; i++)
    {
        led = &leds[i];
        rgb = hsv2rgb(led->hue, led->saturation, led->value, rgb);
        strip->setPixelColor(led->id, strip->Color(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255));
    }
    strip->show();
    free(rgb);
}

void UpdateLEDsRGB(Adafruit_NeoPixel* strip, LED* leds, int n)
{
    LED* led;
    for (int i = 0; i < n; i++)
    {
        led = &leds[i];
        strip->setPixelColor(led->id, strip->Color(led->red, led->green, led->blue));
    }
    strip->show();
}

#endif