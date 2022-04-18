#!/bin/bash
gcc user.c -lpthread -o p1
gnome-terminal -- bash -c "./p1; exec bash"
python3 Messenger_GUI.py
