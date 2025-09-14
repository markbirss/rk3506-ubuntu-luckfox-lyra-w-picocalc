# rk3506-ubuntu-luckfox-lyra-w-picocalc
<img width="1024" height="576" alt="image" src="https://github.com/user-attachments/assets/8aaf2d6b-baaa-4ef9-8d1e-3ec81b291f0e" />
Ubuntu 24.04.3 LTS for Clockwork Picocalc running with Luckfox Lyra Zero W Core

(based off the Luckfox Lyra SDK when it could still build Ubuntu 22.04 OS Images but further modified to include updated Ubuntu 24.04.3 OS and support for the later Luckfox Lyra SBC boards with ability to run luckfox-config) 

PicoCalc Feature | status |
|:--|:--|
| Device Tree | Yes - check out luckfox-config *not sure it working correctly ? |
| Keyboard | Yes |
| Display | Yes | 
| Serial Consle | Yes - baud @ 1500000|
| ADB | Yes |
| USB Networking | Yes |
| BLE | Yes |
| WiFi | Yes * see important note below to setup wifi modules|
| XFCE4 Desktop | Yes * see installation notes below|
| SDCard Slot | todo |

Serial Connectioin
<img width="1672" height="985" alt="image" src="https://github.com/user-attachments/assets/ae392758-8ae8-4499-9e72-c5a48f10f45a" />

```
Connect Serial USB UART
GND         - GND
UART0_TX    - RxD
UART0_RX    - TxD

tio -b 1500000 /dev/ttyUSB0
https://wiki.luckfox.com/Luckfox-Lyra/Login/
```

```
[build instructions]
git clone https://github.com/markbirss/rk3506-ubuntu-luckfox-lyra-w-picocalc.git

cd rk3506-ubuntu-luckfox-lyra-w-picocalc

#cd rk3506-ubuntu-luckfox-lyra-w-picocalc/device/rockchip/.chips/rk3506
#ln -s .chips/rk3506 ../../rk3506
#ln -s .chips/rk3506 ../../.chip
#cd ../../../../

#sha256sum
#d6f58545b0b9c679665a8ff58dd2a7a75aa2b2648871e4be5a2c2288b4261545  ubuntu_24.04.3.tar.gz

git clone https://github.com/markbirss/ubuntu_24.04.3.git
cd ubuntu_24.04.3
rm -fr .git
7z x ubuntu_24.04.3.7z.001
sha256sum ubuntu_24.04.3.tar.gz

rm -f ubuntu_24.04.3.7z.*

mv ubuntu_24.04.3.tar.gz ../
cd ../
mkdir ubuntu
mv ubuntu_24.04.3.tar.gz ubuntu
```

luckfox_lyra_zero-w_ubuntu_sdmmc_defconfig
```
./build.sh lunch
Log colors: message notice warning error fatal

Log saved at /build/rk3506-ubuntu-luckfox-lyra-w-picocalc/output/sessions/2025-09-11_14-20-48
Pick a defconfig:

1. luckfox_lyra_pi-w_ubuntu_emmc_defconfig
2. luckfox_lyra_pi-w_ubuntu_sdmmc_defconfig
3. luckfox_lyra_plus_ubuntu_sdmmc_defconfig
4. luckfox_lyra_ubuntu_sdmmc_defconfig
5. luckfox_lyra_ultra-w_ubuntu_emmc_defconfig
6. luckfox_lyra_zero-w_ubuntu_sdmmc_defconfig
7. rk3506-armsom-forge1_ubuntu_sdmmc_defconfig
Which would you like? [1]: 6

sudo ./build.sh 
```

```
Default User Login Credentials

User:     root
Password: root

User:     lyra
Password: luckfox

ADB shell requires no password and can be used to set or change existing passwords
```

# IMPORTANT NOTE FOR LYRA BOARDS WITH WIFI
```
adb shell "cd /home/lyra/aic800/ && make install; reboot"

#test
adb shell nmcli dev wifi list

#connect
nmtui
```

```
# lsusb
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
Bus 001 Device 002: ID 1a86:8091 QinHeng Electronics USB HUB
Bus 001 Device 004: ID a69c:88dc AICSemi AIC8800DC
# lsusb -tv
/:  Bus 001.Port 001: Dev 001, Class=root_hub, Driver=dwc2/1p, 480M
    ID 1d6b:0002 Linux Foundation 2.0 root hub
    |__ Port 001: Dev 002, If 0, Class=Hub, Driver=hub/4p, 480M
        ID 1a86:8091 QinHeng Electronics 
        |__ Port 001: Dev 004, If 0, Class=Wireless, Driver=btusb, 480M
            ID a69c:88dc  
        |__ Port 001: Dev 004, If 1, Class=Wireless, Driver=btusb, 480M
            ID a69c:88dc  
        |__ Port 001: Dev 004, If 2, Class=Vendor Specific Class, Driver=aic8800_fdrv, 480M
            ID a69c:88dc
```

XFCE4 Desktop Installation Notes
```
dpkg-reconfigure tzdata
timedatectl set-ntp off; timedatectl set-ntp on

apt -y update; apt install -y --no-install-recommends xserver-xorg-input-all xserver-xorg-core xinit xfce4-terminal xserver-xorg-video-fbdev x11-utils;

apt -y install xfce4 dbus-x11 mesa-utils xubuntu-default-settings xfce4-goodies lightdm-gtk-greeter lightdm-gtk-greeter-settings;

rm /etc/systemd/system/default.target;
systemctl set-default graphical.target;
reboot

```

4layer adapter board

my design download here

https://github.com/markbirss/rk3506-ubuntu-luckfox-lyra-w-picocalc/releases/tag/1.0

<img width="1195" height="495" alt="image" src="https://github.com/user-attachments/assets/196d412f-cea0-4caf-8f48-739a9d16bc2f" />


```
Your require the PCB
1 x femaile 40 pins raspberry pi type header
2 x 20 pin male 2.54 header

INSTRCUTIONS
1. Solver the 40 pin female header to the PCB on opposite side of the inner sildscreen (see picutre below)
2. Solder the 20 Pin male header onto the the opposite side as the femaile 40 pin header
3. Isert the assembled board into the back of the Picocalc as pictured below (the #v3 and 5V on facing bottom display fpc connector)
4. Insert the Luckfox Zero W facing down in the female 40 pin conenctor
5. Insert the flashed sd card and boot

```
<img width="1195" height="495" alt="image" src="https://github.com/user-attachments/assets/dc31760e-c04b-49cb-af1d-1e7d2327ab54" />
<img width="1848" height="4000" alt="image" src="https://github.com/user-attachments/assets/fc55bc8c-c5f6-4c1e-b92d-2f695b8aa3fd" />

Case pictured above Credit to n602
https://www.thingiverse.com/thing:6998636#google_vignette

Stand by sn0ren
@sn0ren_1842114

https://www.printables.com/model/1254039-picocalc-stand

PicoCalc Display, Keybaord, SND drivers by hisptoot
https://github.com/hisptoot/picocalc_luckfox_lyra/

LuckFox Lyra SDK by Luckfox

# **JLBPCB Gerber view (online)**
https://jlcpcb.com/RGE

#IMPORTANT NOTE
This SDK is provided for non commercial use only

UBUNTU require official autorization for commerical use

This SDK is provided without any warranty
Use at you own risk

Support my work and consider buying me a coffee

https://buymeacoffee.com/mark.birss
