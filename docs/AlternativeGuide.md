1. Install dependencies:
```bash
sudo apt install build-essential libbluetooth-dev libglib2.0-dev libdbus-1-dev git
```
2. Clone repo:
```bash
git clone https://github.com/mariob0y/WiimoteEmulator && cd WiimoteEmulator
```
3. Build emulator:
```bash
source ./build-custom.sh
```
3. Stop the already running bluetooth service:
```bash
sudo systemctl disable --now bluetooth
```
4. Reboot.
5. Run:
```bash
sudo hciconfig hci0 reset
```
6. Run:
```bash
cd WiimoteEmulator && sudo ./bluez-4.101/dist/sbin/bluetoothd -d -n
```
7. In second terminal tab run:
```bash
cd WiimoteEmulator && sudo ./wmemulator
```
