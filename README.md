# rk3506-ubuntu-luckfox-lyra-w-picocalc
<img width="1024" height="576" alt="image" src="https://github.com/user-attachments/assets/8aaf2d6b-baaa-4ef9-8d1e-3ec81b291f0e" />
Ubuntu 24.04.3 LTS for Clockwork Picocalc

(based off the Luckfox Lyra SDK when it could still build Ubuntu 22.04 OS Images but further modified to include updated Ubuntu 24.04.3 OS and support for the later Luckfox Lyra SBC boards with ability to run luckfox-config) 

PicoCalc Feature | status |
|:--|:--|
| Keyboard | Yes |
| Display | Yes | 
| Serial Consle | Yes - baud @ 1500000|
| ADB | Yes |
| USB Networking | Yes |
| BLE | Yes |
| WiFi | Yes * see important note below to setup wifi modules|
| XFCE4 Desktop | Yes * see installation notes below|

```
[build instructions]
git clone https://github.com/markbirss/rk3506-ubuntu-luckfox-lyra-w-picocalc.git

cd rk3506-ubuntu-luckfox-lyra-w-picocalc/device/rockchip/.chips/rk3506
ln -s .chips/rk3506 ../../rk3506
#ln -s .chips/rk3506 ../../.chip
cd ../../../../

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
