sudo mkfs.ext4 -E nodiscard /dev/nvme0n1
sudo mkdir -p /data4
sudo mount -o discard /dev/nvme0n1 "/data4"
sudo chmod o+rwx /data4

sudo apt-get install -y software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-7 -y

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 \
                         --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --config gcc

sudo apt install make
make
