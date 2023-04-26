#include "mbed-os/drivers/include/drivers/AnalogIn.h"
#include "mbed-os/drivers/include/drivers/DigitalInOut.h"
#include "mbed-os/drivers/include/drivers/DigitalOut.h"
#include "mbed-os/mbed.h"




int correction = 0;
Timer sonar;

int distancecalc(DigitalOut trigger_pin,DigitalIn echo_pin,int i){
    DigitalOut trigger(trigger_pin);
    DigitalIn echo(echo_pin);

    int distance = 0;
    sonar.reset();

    // measure actual software polling timer delays
    // delay used later in time correction
    // start timer
    sonar.start();
    // min software polling delay to read echo pin
    while (echo==2) {};
    // stop timer
    sonar.stop();
    // read timer
    correction = sonar.elapsed_time().count();
    printf("Approximate software overhead timer delay is %d uS\n\r",correction);

    //Loop to read Sonar distance values, scale, and print
    while(1) {
    // trigger sonar to send a ping
        trigger = 1;

        sonar.reset();
        wait_us(10.0);
        trigger = 0;
    //wait for echo high
        while (echo==0) {};
    //echo high, so start timer
        sonar.start();

    //wait for echo low
        while (echo==1) {};

    //stop timer and read value
        sonar.stop();

    //subtract software overhead timer delay and scale to cm
        distance = (sonar.elapsed_time().count()-correction)/58.0;
        printf("%d: %d cm \n\r",i,distance);
        return distance;
    //wait so that any echo(s) return before sending another ping
        wait_us(1);
}
}


int main(){
    while(1){
        distancecalc(D6,D7,1);
        distancecalc(D3,D2,2);
    }
}
