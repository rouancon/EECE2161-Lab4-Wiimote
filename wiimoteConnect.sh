#!/bin/bash
# wiimoteConnect.sh

WII_MAC=8C:56:C5:40:04:EF

echo "Put the Wii-mote in pairing mode then press enter."
read variable

$(sudo bluez-simple-agent hci0 $WII_MAC)
$(sudo bluez-test-device trusted $WII_MAC yes)
echo "Make Sure the Wii-Mote is still in pairing mode then press enter."
read variable2
$(sudo bluez-test-input connect $WII_MAC)
