#!/bin/sh

echo "Change dir..."
cd /usr/src/robot_2wd/pet

screen -dmS speecher sh -c './speecher ; exec bash'
screen -dmS elwin sh -c './face_detect_woof ; exec bash'

