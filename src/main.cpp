#include <MIDI.h>
#include <Arduino.h>
#include <FastLED.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

//  First 16 are MIDI footswitches, last two are bank switching
const int midiButtonCount = 16;
const int extraSwitches = 2;
const int numButtons = midiButtonCount + extraSwitches;

//  Physical Button tracking
byte currButtonState[numButtons];
byte prevButtonState[numButtons];

//  The pin each button is connected to
//  Note: The last three are out of order as they are bank switches rather than cmd 
int buttons[] = {3, 4, 5, 6, 7, 14, 15, 16, 17, 18, 27, 28, 29, 30, 31, 8, 19, 32};
unsigned long lastPressed[numButtons];
unsigned long buttonTimeout = 150;

//  MIDI variables
//  Commands to send, multiples of 16 in banks
const int channel = 1;
int commands[] =
    { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };


enum ButtonStates { OFF, ACTIVE, PLAYING, RECORDING };
enum ButtonTypes { RECDUB, PLAYSTOP, TOGGLE, MOMENTARY };

//  Need arrays of colours, matching the layout of ButtonStates
CRGB RecDub[4]    = { CRGB::Black, CRGB::Black, CRGB::Red, CRGB::Yellow };
CRGB PlayStop[4]  = { CRGB::Black, CRGB::Black, CRGB::Red, CRGB::Yellow }; 
CRGB Toggle[4]    = { CRGB::Black, CRGB::Black, CRGB::Red, CRGB::Yellow }; 
CRGB Momentary[4] = { CRGB::Black, CRGB::Red, CRGB::Black, CRGB::Black }; 

ButtonStates buttonStates[numButtons];

//  Logical Button States, stored by bank
byte midiState[numButtons];

void SetCC(int button, bool value)
{
    midiState[button] = value;
    int velocity = midiState[button] == HIGH ? 127 : 0;
    MIDI.sendControlChange(commands[button], velocity, channel);

    Serial.print("Sent ");
    Serial.print(button);
    Serial.print(", ");
    Serial.print(commands[button]);
    Serial.print(", ");
    Serial.println(velocity);
}

void setup()
{
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);

    for (int i = 0; i < numButtons; i++)
    {
        pinMode(buttons[i], INPUT_PULLUP);
        lastPressed[i] = 0;
    }

    for(int i = 0; i < numButtons; i++)
    {
        buttonStates[i] = OFF;
    }

    MIDI.begin(channel);

    Serial.println("Begun!");
}

void loop()
{
    // read digital pins and use them for the buttons
    for (int i = 0; i < numButtons; i++)
    {
        bool buttonValue = digitalRead(buttons[i]) == LOW;
        // Serial.print(buttonValue);
        // Serial.print(",");

        if ((buttonValue != currButtonState[i]) & (millis() - lastPressed[i] > buttonTimeout))
        {
            currButtonState[i] = buttonValue;
            SetCC(i, buttonValue);
            lastPressed[i] = millis();
        }
    }

    // Serial.println();

    //  run at 200hz
    delay(5);
}