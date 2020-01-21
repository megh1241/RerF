rm /data4/*
make
./bin/fp 7 7 1 stat 4 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 stat 4 test

