#include "ultrasonic_node.h"

int main() {
  UltrasonicNode node;
  node.init();

  while (1) {
    node.update();
  }
}
