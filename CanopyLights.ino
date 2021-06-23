// Adding a 5A fuse to your 12V supply cable is srtongly recommended

// There are two ways to use this module:
// - Run a cable from the DOOR terminal to a door switch that will ground it when the door is open
// or
// - Short the DOOR terminal to one of the GND terminals, and use your own external relay to switch the 12V supply

// LEDs are WS2815 (12V power with 5V logic, ~2kHz refresh reliably [though some datasheets will claim up to 8kHz])

// You'll need to install the FastLED library for this to compile
// Instructions for installing libraries with the library manager here: https://www.arduino.cc/en/guide/libraries#toc3
#include <FastLED.h>  // By Daniel Garcia (you can also get it at https://github.com/FastLED/FastLED if you want to install from zip)


// ========== Pins ========== 
const uint8_t DOOR_SWITCH_PIN = 9;
const uint8_t LED_RELAY_PIN = 10;
const uint8_t LED_DATA_PIN = 11;
const uint8_t BUTTON_PIN = 12;


// ========== LED settings ========== 
const uint16_t LED_COUNT = 300;  // This must be greater than or equal to the number of LEDs
const uint8_t BRIGHTNESS = 255;
// WS2815 LEDs haven't been added to FastLED, but WS2811 is close enough
#define LED_TYPE WS2811 
const uint8_t DIMMER_GROUP_SIZE = 5;
const CRGB WHITE = CRGB::White;
const CRGB INSECT_COLOUR = CRGB(0xFF, 0xDF, 0x00);
const CRGB LEDS_OFF = CRGB::Black;
const uint32_t AUTO_POWER_OFF_TIME_MS = 600000;  // 10 minutes
const uint32_t LED_UPDATE_PERIOD = 50;  // 20 Hz


// ========== Button settings ========== 
const uint32_t BUTTON_DEBOUNCE_MS = 50;
const uint32_t BUTTON_MAX_COLOUR_PRESS_MS = 500; // Holding the button longer than this will dim the LEDs
const uint32_t BUTTON_DIMMER_CYCLE_MS = 500;  // How often to change the LED dimming when the button is held


// ========== Door switch settings =========
const uint32_t DOOR_SWITCH_DEBOUNCE_MS = 50;


// ========== LED variables ========== 
CRGB leds[LED_COUNT];
CRGB current_colour = WHITE;
int8_t current_dimming_level = 0;
uint32_t last_led_update = 0;
bool force_update = true;
uint32_t auto_power_off_timer_start = 0;


// ========== Button variables ========== 
uint32_t last_button_transition = 0;


// ========== Relay variables ==========
bool relay_enabled = true;


void paint_leds(){
	if (millis() - last_led_update > LED_UPDATE_PERIOD || force_update){
		// Set the relay
		if (current_colour == LEDS_OFF){
			digitalWrite(LED_RELAY_PIN, LOW);
			relay_enabled = false;
		}
		else{
			digitalWrite(LED_RELAY_PIN, HIGH);
			if (!relay_enabled){
				delay(50);  // This gives the relay time to close before we send data to the LEDs
			}
			relay_enabled = true;
		}
		
		// current_dimming_level determines how many LEDs in a dimming group should be shut down.
		// DIMMER_GROUP_SIZE defines how many LEDs are in a dimming group. 
		// The maximum dimming_level is DIMMER_GROUP_SIZE - 1 (otherwise you would shut down all LEDs)
	
		// Ensure dimming level isn't accidentally set too high or too low
		if (current_dimming_level > DIMMER_GROUP_SIZE - 1){
			current_dimming_level = DIMMER_GROUP_SIZE - 1;
		}
		if (current_dimming_level < 0){
			current_dimming_level = 0;
		}
	
		// Turn LEDs on/off to achieve the desired dimming level
		for(uint16_t led_num=0; led_num < LED_COUNT; led_num++){
			if (led_num % DIMMER_GROUP_SIZE >= current_dimming_level){
				leds[led_num] = current_colour;
			}
			else{
				leds[led_num] = LEDS_OFF;
			}
		}
		FastLED.show();
		last_led_update = millis();
	}
}


void change_dimmer(){
	Serial.println(F("change_dimmer() called"));
	static int8_t dimmer_direction = 1;
	current_dimming_level += dimmer_direction;
	if (current_dimming_level >= DIMMER_GROUP_SIZE - 1){
		dimmer_direction = -1;
	}
	else if (current_dimming_level == 0){
		dimmer_direction = 1;
	}
	force_update = true;
	auto_power_off_timer_start = millis();
}


void change_led_colour(){
	Serial.println(F("change_led_colour() called"));
	if (current_colour == WHITE){
		current_colour = INSECT_COLOUR;
	}
	else if (current_colour == INSECT_COLOUR){
		current_colour = LEDS_OFF;
	}
	else{
		current_colour = WHITE;
	}
	force_update = true;
	auto_power_off_timer_start = millis();
}


void check_button(){
	// Variables to keep track of the button through time
	// Note that a button state of HIGH means it's not pressed, 
	// and LOW means that is is pressed because we're using the INPUT_PULLUP pin mode
	static uint8_t old_button_state = HIGH;
	static uint32_t last_dimmer_change = 0;
	
	// Read the button
	uint8_t new_button_state = digitalRead(BUTTON_PIN);

	// Check if the button has changed state
	if (new_button_state != old_button_state){
		old_button_state = new_button_state;
		// Debounce the button: https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
		if (millis() - last_button_transition >= BUTTON_DEBOUNCE_MS){
			// Change colour if the button was only pressed for a short time
			if (new_button_state == HIGH){
				if (last_dimmer_change == 0){
					Serial.println(F("Short button press"));
					change_led_colour();
				}
				last_dimmer_change = 0;				
			}
			last_button_transition = millis();
		}
	}
	// Check if the button is being held
	else if (new_button_state == LOW && millis() - last_button_transition >= BUTTON_MAX_COLOUR_PRESS_MS){
		// Cycle the dimmer every so often
		if (millis() - last_dimmer_change >= BUTTON_DIMMER_CYCLE_MS){
			last_dimmer_change = millis();
			change_dimmer();
		}
	}
}


bool check_door_switch(){
	// This is very similar to the button code above
	static uint8_t old_switch_state = LOW;
	static uint32_t last_switch_transition = 0;
	
	uint8_t new_switch_state = digitalRead(DOOR_SWITCH_PIN);
	if (new_switch_state != old_switch_state){
		if (millis() - last_switch_transition >= DOOR_SWITCH_DEBOUNCE_MS){
			last_switch_transition = millis();
			old_switch_state = new_switch_state;
			
			if (new_switch_state == LOW){
				Serial.println(F("Door opened"));
				current_colour = WHITE;
				auto_power_off_timer_start = millis();
			}
			else{
				Serial.println(F("Door closed"));
				current_colour = LEDS_OFF;
			}
			force_update = true;
		}
	}
}


void power_off_timer(){
	if (millis() - auto_power_off_timer_start > AUTO_POWER_OFF_TIME_MS && current_colour != LEDS_OFF){
		current_colour = LEDS_OFF;
	}
}


void setup(){
	// Set up inputs and outputs
	pinMode(DOOR_SWITCH_PIN, INPUT_PULLUP);
	pinMode(LED_RELAY_PIN, OUTPUT);
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	if (digitalRead(DOOR_SWITCH_PIN) == HIGH){
		current_colour = LEDS_OFF;  // Prevent the LEDs from flashing on bootup if the door is closed
	}
	
	// Set up LEDs
	FastLED.addLeds<LED_TYPE, LED_DATA_PIN>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
	FastLED.setBrightness(BRIGHTNESS);
	FastLED.clear();

	
	Serial.begin(115200);
	Serial.println(F("Setup finished"));
}


void loop(){
	check_button();
	check_door_switch();
	power_off_timer();
	paint_leds();
}
