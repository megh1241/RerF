make
./bin/fp 7 8 1 binstatclass 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 1 binstatclass 1 test
