#!/bin/bash
artist=$(playerctl metadata artist)
title=$(playerctl metadata title)
status=$(playerctl status)

if [[ "$status" == "Playing" ]]; then
    echo " $artist - $title"
elif [[ "$status" == "Paused" ]]; then
    echo " $artist - $title"
else
    echo "No players found"
fi
