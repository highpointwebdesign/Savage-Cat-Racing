# Savage Cat Racing

After many sources of inspiration including my IRL VW GTI with dynamic suspension, this is my take on creating a dynamic suspension system for radio-controlled vehicles, specifically  trucks. I know there are other options but it would be fun to see what I can develop.

The platform I'm targeting is the TRX4 and eventually the TRX6. I'm also hoping to adapt this to a radio-controlled RV whose scale is likely around 1/8th scale as well as the TRX6 with only the steer axle and the rearmost axle with the Cen Racing F350 body (the body is better suited for the TRX6 with the removal of the intermediate axle). Another option is to make it a tandem axle F350. 

The goal for this project right now:
1. test for gyro
2. test servos (currently only testing for 4)

Future goals
1. Flip the switch on the 6-channel radio to lower the truck or raise the truck to preset settings with dynamic suspension off. When settings are high or low, dynamic suspension is off. When the switch is in the middle, the dynamic suspension is off. (you could adapt that if you did not have a multi-channel remote, you could perform a series of inputs from the radio to trigger specific actions). I will not be exploring those, at least not right now.
2. Incorporate light control too as the esp32 is fast enough it can handle the additional workload
3. Develop a Bluetooth LE app to adjust settings while out in the field
4. Add the ability to save/load profiles for different terrains
