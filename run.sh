#!/bin/bash
gnome-terminal -- bash -c "python3 Messenger_GUI.py; exec bash"
gcc userGUI.c -lpthread -o p1
./p1
