#ifndef ULTRASONIC_NODE_H_
#define ULTRASONIC_NODE_H_ 

#include "ros.h"
#include "std_msgs/Float32.h"
#include "mbed.h"

struct UltrasonicSensor {
    DigitalOut trig;
    InterruptIn echo; 
    Timer echo_timer; 
    float duration; 
    float distance;
    uint32_t rising_timestamp; 
    bool new_distance_available;  

    UltrasonicSensor(PinName trig_pin, PinName echo_pin) {
        : trig(trig_pin), echo(echo_pin), echo_timer() {
        distance = 0.0f;
        duration = 0.0f;
        new_distance_available = false;
        rising_timestamp = 0;
    }
}

class UltrasonicNode {
public: 
    UltrasonicNode (
        int publish_rate, 
        int num_sensors, 
        PinName* trig_pins, 
        PinName* echo_pins, 
        float threshold
    ); 
    void setup(); 
    void update(); 
    void publish_distance(float); 

private: 
    static const int publishRate = 10;
    static const int thresholdCm = 40;

    void echo_rising();
    void echo_falling(UltrasonicSensor* sensor); 
    float microseconds_to_cm(float microseconds);

    ros::NodeHandle node_handler; 
    std_msgs::Float32 distances[8];
    ros::Publisher distance_pubs[8];
    UltrasonicSensor sensors[8];
    int num_sensors;
    float threshold;
    int publish_rate;
    ros::Time last_publish_time;
}

#endif  // ULTRASONIC_NODE_H_
