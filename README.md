# partial_weight_bearing_insole
An affordable wearable insole that measures partial weight bearing for rehab

After my mom broke her legs, during her recovery, her doctor prescribed her certain partial weight bearing (ex: "Only put 15% of your bodyweight on your right leg"). The Doctor told her to use the standard "bathroom scale method," where one has to put their legs on a scale to "feel" what certain weights mean. While it is widely recommended as a basic training tool, multiple orthopedic studies show that most patients significantly exceed their weight-bearing limits when actually moving, which can lead to improper healing

My mother started to become very anxious about this method. She tried looking everywhere online for something more accurate that could help with her partial weight bearing, but there was nothing easily accessible to buy or inexpensive. So I set out to make one. I've come a long way with this project, and this is how it works:

Two 150kg force-sensitive resistors sit under the heel and the ball of the foot. A microcontroller (XIAO nRF52840) at the ankle reads them, calibrates to the wearer each time the shoe goes on, and streams the percentage to a phone over Bluetooth. If you cross your limit, a built in alarm sounds.

This project was mentored by Dr. Thomas Jenkyn, and I tested the device in his lab (Wolf Orthopaedic Biomechanics Laboratory) at Western University against a force plate. The main thing I found was not a problem with my sensors. It was a bug in my own at-home calibration code, which was inflating every reading by 37%. Fixed, the device lands in the correct clinical weight-bearing band in 19 of 21 standing trials.

However, this project is still ongoing, and I have lots of things I want to add to it in the future, such as an iphone app, better material for the insole, and making my own PCB. 


In this repo:

Engineering Notebook: My entire engineering notebook. Throughout this project, I have documented everything: prototypes, failures, successes, etc.

summary: My engineering notebook is pretty long, so I have summarized it here

firmware: I used Claude Code to write the code for this project. I have included all versions of the code, along with the prompts I used
