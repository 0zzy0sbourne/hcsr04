#include "ultrasonic_node.h"
#include "mbed.h"

UltrasonicNode::UltrasonicNode() {
    node_handler("/ultrasonic"); 
    distance_publisher("distance", &distance_msg); 
    publish_count(0) {}
}

UltrasonicNode::UltrasonicNode(int publish_rate, int num_sensors, PinName* trig_pins, PinName* echo_pins, float threshold)
    : node_handler("ultrasonic_node"),
      this->num_sensors(num_sensors),
      this->threshold(threshold),
      this->publish_rate(publish_rate),
      last_publish_time(0)
{
    for (int i = 0; i < num_sensors; ++i) {
        UltrasonicSensor& sensor = sensors[i];
        sensor.echo = InterruptIn(echo_pins[i]);
        sensor.trig = DigitalOut(trig_pins[i]);
        sensor.echo.rise(callback(this, &UltrasonicNode::echo_rising));
        sensor.echo.fall(callback(this, &UltrasonicNode::echo_falling, &sensor));
        sensor.distance = 0.0f;
        sensor.duration = 0.0f;
        sensor.new_distance_available = false;
        sensor.rising_timestamp = 0;
        distance_pubs[i] = ros::Publisher("distance_" + std::to_string(i), &distances[i]);
    }
}

void UltrasonicNode::setup() {
    node_handler.initNode(); 
    node_handler.advertise(distance_publisher); 
    for(auto& distance_pub : distance_pubs) {
        node_handler.advertise<std_msgs::Float32>(); 
    }
}

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

void UltrasonicNode::echo_rising() {
    
}