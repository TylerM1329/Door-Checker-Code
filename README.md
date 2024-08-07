# Door-Checker-Code
/*

Designed by Tyler McNeil
08/07/2024
This code is for demonstration purposes. It was created using an example code that built the basis of the communication portion of the system.
My additions make it specific to my application.

These are two seperate codes meant to run on their own ESP32 units. If using as reference, make sure to separate them

    {ESP-NOW Broadcast Slave Example}

    This sketch demonstrates how to receive broadcast messages from a master device using the ESP-NOW protocol.

    The master device will broadcast a message every 5 seconds to all devices within the network.

    The slave devices will receive the broadcasted messages. If they are not from a known master, they will be registered as a new master
    using a callback function.
*/
