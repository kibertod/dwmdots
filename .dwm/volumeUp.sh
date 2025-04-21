#!/bin/bash

pactl set-sink-volume @DEFAULT_SINK@ +10%
value=$(pactl get-sink-volume @DEFAULT_SINK@ | sed -e 's,.* \([0-9][0-9]*\)%.*,\1,' | head -n 1 )
dunstify "volume 󰕾" -h int:value:$value -t 1000

