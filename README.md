# partial_weight_bearing_insole
An affordable wearable insole that measures partial weight bearing for rehab

After my mom broke her legs, during her recovery, her doctor prescribed her certain partial weight bearing (ex: "Only put 15% of your bodyweight on your right leg"). The Doctor told her to use the standard "bathroom scale method," where one has to put their legs on a scale to "feel" what certain weights mean. While it is widely recommended as a basic training tool, multiple orthopedic studies show that many patients significantly exceed their weight-bearing limits when actually moving, which can lead to improper healing

My mother started to become very anxious about this method. She tried looking everywhere online for something more accurate that could help with her partial weight bearing, but there was nothing easily accessible to buy or inexpensive. So I set out to make one. I've come a long way with this project, and this is how it works:

Two 150kg force-sensitive resistors sit under the heel and the ball of the foot. A microcontroller (XIAO nRF52840) at the ankle reads them, calibrates to the wearer each time the shoe goes on, and streams the percentage to a phone over Bluetooth. If you cross your limit, a built in alarm sounds.

This project was mentored by Dr. Thomas Jenkyn, and I tested the device in his lab (Wolf Orthopaedic Biomechanics Laboratory) at Western University against a force plate.

However, this project is still ongoing, and I have lots of things I want to add to it in the future, such as an iphone app, better material for the insole, making my own PCB, and testing different FSR sensors. 


**In this repo:**

Engineering Notebook: My entire engineering notebook. Throughout this project, I have documented everything: prototypes, failures, successes, etc.

Code: The code and the MATLAB analysis scripts. These were written from Claude Code

Lab Data: Every trial from the force plate session at Western, both my device’s recordings and the force plate’s, plus the analysis outputs.

Prototype Data: experimental data from earlier phases, recorded at home.



**Contributions**

Myself: identified the problem, researched existing devices and sensing methods, and chose the design. Designed, built, and tested every prototype: the conductive foam, the resistor testing, the drift and foam-sandwich experiments, and the two-sensor insole. Calibrated the sensors, soldered the perfboard, and assembled the final device. I designed the experimental procedure for the 19 tests and 57 trials at the lab. I also did the analysis in MATLAB to check. I found the calibration bug that was making every reading high, and fixed it. Finally, I documented everything in my engineering notebook

Dr. Thomas Jenkyn: Met with me throughout and gave guidance whenever I was stuck, and hosted the force plate testing in his lab at Western University.

Ms. Costello: Dr. Jenkyn’s graduate student, who ran the force plate and recorded the lab’s data during the testing session.

Claude: wrote the code from my descriptions of what each needed to do. Claude also helped generate some of the graphs in my Engineering Notebook



