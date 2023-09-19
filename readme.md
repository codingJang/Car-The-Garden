# Car-The-Garden 🚗

**Team**: Yejun Jang, Sangmok Yeom

Repository for the Car-The-Garden project, an autonomous robotic car designed to navigate a miniature version of the standard Korean driving test using left-first search and right-first search algorithms.

## Overview 📖

The goal of the Car-The-Garden project was to design, develop, and test an autonomous robotic car capable of navigating complex driving test tracks inspired by the standard Korean driving test. By incorporating a rich set of sensors and leveraging rule-based algorithms, the robot exhibits dynamic behavior, responding efficiently to its surroundings and adjusting its actions based on the received inputs.

### Highlights:

- **Dynamic Navigation**: The robot employs left-first and right-first search algorithms to determine its path and navigate the test track.
- **Multimodal Sensing**: Equipped with sensors like RFID, IR, light, ultrasonic, and more, the robot perceives and understands its environment in real time.
- **Communication**: Bluetooth module enables remote control and dynamic command updates.
- **Interactive Feedback**: With LCD displays and serial communication, the robot offers valuable feedback for real-time monitoring and debugging.
- **Emergency Handling**: A dedicated emergency mode ensures safety and rapid response to unforeseen obstacles or conditions.

## Challenges & Learnings 💡

- **Imperfect Hardware**: Dealing with real-world imperfections of sensors and hardware was a challenge, teaching the importance of calibration, error handling, and robust algorithm design.
- **Rule-Based Algorithms**: Implementing rule-based algorithms was essential for the car to make smart decisions autonomously, adapting its behavior based on sensor inputs and predefined rules.
- **Modularity in Code**: With multiple Arduino modules integrated, ensuring a modular and clean codebase was pivotal for seamless integration and future scalability.

## Tools & Technologies 🛠

- **Platform**: Arduino
- **Modules**: 
  - RFID Reader
  - Bluetooth Communication
  - IR Sensor
  - Light Sensor
  - Ultrasonic Sensor

## Getting Started 🚀

1. Clone this repository: 
```
git clone https://github.com/codingJang/Car-The-Garden.git
```
2. Set up your Arduino environment.
3. Connect the required modules as detailed in the project documentation.
4. Upload the code to your Arduino board and observe the magic of Car-The-Garden!

## Credits & Acknowledgements 👏

Special thanks to the TAs of Creative Engineering Design course, for providing the miniature driving test map and missions.

## License 📄

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
