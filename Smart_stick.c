#include <avr/io.h> 
#include <util/delay.h> 
#include <stdbool.h> // Include this for the bool type 

#define F_CPU 16000000UL // Clock speed 
#define BUZZER_PIN PB2    // Define buzzer pin 
#define VIBRATOR_PIN PB3  // Define vibrator pin 
#define TRIG_PIN PB1      // Define trigger pin for ultrasonic sensor 
#define ECHO_PIN PB0      // Define echo pin for ultrasonic sensor 
#define RAIN_SENSOR_PIN PC0 // Define raindrop sensor analog pin 
#define RAIN_THRESHOLD 900  // Rain threshold value for the buzzer and vibrator 
#define DISTANCE_THRESHOLD 50 // Distance threshold for the ultrasonic sensor in cm 
#define TIMEOUT 2915 // Timeout for echo signal (50 cm / 343 m/s) 

// Function prototypes 
void initBuzzer(void); 
void buzzerOn(void); 
void buzzerOff(void); 
void initVibrator(void); 
void vibratorOn(void); 
void vibratorOff(void); 
void initUltrasonicSensor(void); 
void initRaindropSensor(void); 
uint16_t readRaindropSensor(void); 
unsigned int getDistance(void); 

void initBuzzer(void) { 
DDRB |= (1 << BUZZER_PIN); // Set buzzer pin as output 
} 

void buzzerOn(void) { 
PORTB |= (1 << BUZZER_PIN); // Turn buzzer on 
} 

void buzzerOff(void) { 
PORTB &= ~(1 << BUZZER_PIN); // Turn buzzer off 
} 

void initVibrator(void) { 
DDRB |= (1 << VIBRATOR_PIN); // Set vibrator pin as output 
} 

void vibratorOn(void) { 
PORTB |= (1 << VIBRATOR_PIN); // Turn vibrator on 
} 

void vibratorOff(void) { 
PORTB &= ~(1 << VIBRATOR_PIN); // Turn vibrator off 
} 

void initUltrasonicSensor(void) { 
DDRB |= (1 << TRIG_PIN); // Set trigger pin as output 
DDRB &= ~(1 << ECHO_PIN); // Set echo pin as input 
} 

void initRaindropSensor(void) { 
    // Set the ADC prescaler to 128 (for 16MHz clock) 
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
    // Set reference voltage to AVcc with external capacitor at AREF pin 
    ADMUX |= (1 << REFS0); 
    // Enable ADC 
    ADCSRA |= (1 << ADEN); 
} 
 
uint16_t readRaindropSensor(void) { 
    // Set the appropriate channel (rain sensor on PC0) 
    ADMUX = (ADMUX & 0xF0) | (RAIN_SENSOR_PIN & 0x0F); 
    // Start conversion 
    ADCSRA |= (1 << ADSC); 
    // Wait for conversion to complete 
    while (ADCSRA & (1 << ADSC)); 
    // Return the analog value 
    return ADC; 
} 
 
unsigned int getDistance(void) { 
    uint32_t count = 0; 
 
    // Send trigger pulse 
    PORTB &= ~(1 << TRIG_PIN); 
    _delay_us(2); 
    PORTB |= (1 << TRIG_PIN); 
    _delay_us(10); 
    PORTB &= ~(1 << TRIG_PIN); 
 
    // Wait for echo start (with timeout) 
    count = 0; 
    while (!(PINB & (1 << ECHO_PIN))) { 
        if (count++ > TIMEOUT) return DISTANCE_THRESHOLD; // Timeout 
        _delay_us(1); 
    } 
 
    // Measure echo duration (with timeout) 
    count = 0; 
    while (PINB & (1 << ECHO_PIN)) { 
        if (count++ > TIMEOUT) return DISTANCE_THRESHOLD; // Timeout 
        _delay_us(1); 
    } 
 
    // Calculate distance in cm 
    return (count / 58); // Speed of sound = 343 m/s 
} 
 
int main(void) { 
    uint16_t rainValue; 
    unsigned int distance; 
 
    initBuzzer(); 
    initVibrator(); 
    initUltrasonicSensor(); 
    initRaindropSensor(); 
 
    while (1) { 
        // Read sensor values 
        rainValue = readRaindropSensor(); 
        distance = getDistance(); 
 
        // Determine if sensors are triggered 
        bool rainDetected = (rainValue <= RAIN_THRESHOLD); 
        bool obstacleDetected = (distance < DISTANCE_THRESHOLD); 
 
        // Control buzzer and vibrator based on sensor readings 
        if (rainDetected || obstacleDetected) { 
            if (rainDetected) { 
                buzzerOn();   // Turn on buzzer when rain is detected 
                vibratorOn(); // Turn on vibrator when rain is detected 
            } 
            if (obstacleDetected) { 
                buzzerOn();   // Turn on buzzer when obstacle is detected 
                vibratorOn(); // Turn on vibrator when obstacle is detected 
            } 
        } else { 
            buzzerOff();   // Turn off buzzer when no rain or obstacle 
            vibratorOff(); // Turn off vibrator when no rain or obstacle 
        } 
 
        _delay_ms(10); // Wait for 10 milliseconds before the next reading 
    } 
 
    return 0; 
} 