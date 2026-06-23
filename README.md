sudo apt update && sudo apt install build-essential
g++ -o lora_sender serial_lora_sender.cpp
sudo usermod -a -G dialout $USER
python yolo_renk.py | ./lora_sender
