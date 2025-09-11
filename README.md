# rk3506-ubuntu-luckfox-lyra-w-picocalc
Ubuntu 24.04.3 LTS for Clockwork Picocalc

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
