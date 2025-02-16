# Smart Bench Power Supply

This project is a DIY Smart Bench Power Supply designed to regulate voltage and current using an old 45W laptop adapter, an LM2596 buck converter, an INA219 current sensor, a 16x2 I2C LCD, and an Arduino Nano. The PSU features software-based constant current control by manipulating the LM2596 feedback pin via a high-frequency PWM signal.  

---

## Working  
The PSU takes input from a laptop adapter and steps it down to a user-defined voltage while ensuring a controlled current limit. The INA219 sensor continuously monitors the voltage and current, displaying the values on an I2C LCD. A rotary encoder allows the user to adjust the current limit in predefined steps, and an Arduino Nano dynamically regulates the LM2596 buck converter. The constant current mode is achieved by adjusting the PWM duty cycle based on real-time current feedback.  

---

## Connection Diagram  

| Component       | Arduino Nano Pin | Additional Info |
|---------------|:---------------:|----------------|
| INA219 (SDA)  | A4              | I2C Communication at address 0x40 |
| INA219 (SCL)  | A5              | I2C Communication at address 0x40 |
| LCD I2C (SDA) | A4              | Shared with INA219 at address 0x27 |
| LCD I2C (SCL) | A5              | Shared with INA219 at address 0x27 |
| Rotary Encoder A (CLK) | 7             | Quadrature input |
| Rotary Encoder B (DT) | 8             | Quadrature input |
| Encoder Button (SW) | 9              | User input for current limit adjustment |
| PWM Output (LM2596 Feedback Pin) | 10 | Limits output current of the LM2596 buck converter |
| Power Input   | VIN, GND         | Connected to laptop adapter using a Mini360 buck converter |

---

## Code Variants
This repository includes two versions of the code. One uses hardware-based PWM by directly manipulating timers for precise control. The other uses the PWM.h library, which abstracts the process and allows for high-frequency software-based PWM, making it easier to implement.

---

## Challenges Faced  
One of the main challenges encountered was PWM interference affecting the voltage adjustment potentiometer of the LM2596. Since the PWM signal was directly modifying the feedback pin, it introduced unwanted fluctuations and made precise voltage control difficult.  

### Solution  
To mitigate this issue, an alternative approach using the `PWM.h` library was implemented, allowing for high-frequency PWM control. This significantly reduced noise and improved the stability of the voltage regulation. Future iterations will replace the PWM-based control with a proper DAC to ensure finer adjustments without interference.  

---

## Future Improvements  
- Replacing the 16x2 LCD with a small OLED display for better display flexibility and a smaller footprint.  
- Using a more powerful buck converter since the LM2596 cannot handle currents above 2A.  
- Reducing the overall project footprint to save desk space and improve portability.

---

## Acknowledgment  
I have taken inspiration from Electronoobs' great video on how to achieve constant voltage or constant current using an Arduino with voltage/current feedback.  
**Watch the video here:** [Electronoobs YouTube Video](https://www.youtube.com/watch?v=rwqY0rYPlVE&ab_channel=Electronoobs)  

To better understand how buck converters work and how they adjust voltage and limit current, I highly recommend checking out GreatScott!'s video.  
**Watch the video here:** [GreatScott! YouTube Video](https://www.youtube.com/watch?v=8uoo5pAeWZI&t=458s&ab_channel=GreatScott%21) 

---

## License  
This project is open-source and released under the [MIT License](LICENSE).
