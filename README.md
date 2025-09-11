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
