#include "ultrasonic_node.h"
#include "mbed.h"

/* 
    The constructor initalizes the UltrasonicSensor objects with their respective trigger and 
    echo pins, and also initializes the distance publishers for each sensor. 
*/
UltrasonicNode::UltrasonicNode(int publish_rate, int num_sensors, PinName* trig_pins, PinName* echo_pins, float threshold)
    : node_handler("/ultrasonic_node"),
      last_publish_time(0)
{
    this->num_sensors = num_sensors; 
    this->threshold = threshold; 
    this->publish_rate = publish_rate; 

    for (int i = 0; i < num_sensors; ++i) {
        UltrasonicSensor& sensor = sensors[i];
        sensor.echo = InterruptIn(echo_pins[i]);
        sensor.trig = DigitalOut(trig_pins[i]);
        sensor.echo.rise(callback(this, &UltrasonicNode::echo_rising, echo_pins[i]));
        sensor.echo.fall(callback(this, &UltrasonicNode::echo_falling, &sensor));
        sensor.distance = 0.0f;
        sensor.duration = 0.0f;
        sensor.new_distance_available = false;
        sensor.rising_timestamp = 0;
        distance_pubs[i] = ros::Publisher("distance_" + std::to_string(i), &distances[i]);
    }
}

/*
    The setup() function initializes the ROS node and advertises the distance publisher for the entire node. 
*/
void UltrasonicNode::setup() {
}

/*
    The update function runs periodically and publishes the distance values for each sensor if a new distance 
    value is available. 
*/
void UltrasonicNode::update() {
    node_handler.spinOnce();

    ros::Time now = node_handler.now();
    if ((now - last_publish_time).toSec() >= 1.0 / publish_rate) {
        for (int i = 0; i < num_sensors; ++i) {
            UltrasonicSensor& sensor = sensors[i];
            if (sensor.new_distance_available) {
                sensor.distance = microseconds_to_cm(sensor.duration);
                distances[i].data = sensor.distance;
                distance_pubs[i].publish(&distances[i]);
                sensor.new_distance_available = false;
            }
        }
        last_publish_time = now;
    }
}

void UltrasonicNode::echo_rising(PinName echo_pin) {
    // Loop through all the sensors and find the one that triggered the interrupt
    for(auto& sensor : sensors) {
        // Get a reference to the sensor that triggered the interrupt
        UltrasonicSensor& sensor = sensor; 
        // Check if this sensor is the one that triggered the interrupt
        if (sensor.echo == echo_pin) {
            // Get the time in microseconds since the start of the program using the echo_timer
            sensor.rising_timestamp = sensor.echo_timer.read_us(); 
            // Set the flag to indicate that a new distance measurement is not yet available
            sensor.new_distance_available = false; 
            // Exit the loop since we've found the sensor that triggered the interrupt
            break; 
        }
    }
}

void UltrasonicNode::echo_falling(UltrasonicSensor* sensor) {
    // Get the timestamp of the falling edge of the echo pulse
    uint32_t falling_timestamp = echo_timer.read_high_resolution_us();
    // Calculate the duration of the pulse by subtracting the rising timestamp from the falling timestamp
    uint32_t duration = falling_timestamp - sensor->rising_timestamp;
    // Calculate the distance based on the duration of the pulse 
    // Divide by 58 to convert the duration to distance in cm (based on the speed of sound)
    sensor->distance = static_cast<float>(duration) / 58.0f;
    // Set the flag to indicate that new distance measurement is available 
    sensor->new_distance_available = true;
}

float UltrasonicNode::microseconds_to_cm(float duration_us) {
    // Speed of sound in air at sea level = 343 meters/second = 34300 cm/second
    // The sound travels to the object and back, so divide by 2
    return duration_us / 2.0f * 0.0343f;
}

void UltrasonicNode::publish_distance(int sensor_index) {
    if (node_handler.connected() && distance_pubs[sensor_index].getTopic()) {
        std_msgs::Float32 msg;
        msg.data = distances[sensor_index];
        distance_pubs[sensor_index].publish(&msg);
    }
}