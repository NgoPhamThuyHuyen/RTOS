PRODUCT COUNTING SYSTEM USING FREERTOS
1. Introduce

This project develops a product counting system for conveyor belts using the ESP32 microcontroller. It effectively handles two common industrial scenarios: stacked products and attached products. A box is defined as containing 2 products. 

3. System Architecture
   
The system employs a multi-tasking architecture powered by FreeRTOS to ensure real-time performance: 
- Microcontroller: ESP32 acts as the central processing unit. 
- Sensors: Utilizes 3 Infrared (IR) sensors for product detection across two conveyor belts. 
- Outputs: Features a 16x2 LCD, LEDs, and Buzzers for real-time status alerts. 
- Memory Management: Implements Mutex Semaphores to synchronize data and prevent race conditions between tasks when accessing shared counters.

3. Feature
   
- Conveyor 1 (Stacking Logic): Uses a dual-sensor (top and bottom) setup. If both sensors are triggered simultaneously, the system identifies 02 stacked products. If only the bottom sensor is triggered, it counts as 01 single product.
- Conveyor 2 (Attachment Logic): Measures the duration of sensor activation. If the duration exceeds 3 seconds, the system detects 02 attached products. 
- Packaging Management: Every 2 products are automatically calculated as 01 box, triggering the buzzer for completion. 
- Real-time Visualization: Continuously updates the product and box counts for both conveyor belts on the LCD screen.



