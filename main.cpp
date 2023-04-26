#include "mbed.h"

DigitalOut trig(D6); 
DigitalOut echo(D7); 
float duration = 0.0f;
int main() { 
    while(1) {
        // Send a 10micro seconds pulse to trigger the ultrasonic sensor 
        trig = 1; // The trigger pin is set to high for 10 microseconds
        wait_us(10); 
        trig = 0; // Then the trigger pin is set to low after 10 microseconds 

        // Measure the time between sending the pulse and receiving the echo 
        Timer t; 
        t.start(); 
        while(!echo) {} // Wait for the echo to be received 
        t.reset(); 
        while(echo) {} // Wait for the echo to stop 
        duration = static_cast<float>(t.elapsed_time().count()); // Read the duration of the pulse in microseconds

        // Convert the time to distance in cm
        float distance = duration / 58.0; 

        // Print the distance to the serial port
        printf("Distance: %.2f cm\n", distance);

        // Wait for a short period before taking the next measurement
    }
}