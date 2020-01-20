rm /data4/*
make
./bin/fp 7 7 1 binstatclass 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstatclass 1 test

rm /data4/*
make
./bin/fp 7 7 1 binstatclass 2 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstatclass 2 test

rm /data4/*
make
./bin/fp 7 7 1 binstatclass 3 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstatclass 3 test

rm /data4/*
make
./bin/fp 7 7 1 binstatclass 4 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstatclass 4 test

rm /data4/*
make
./bin/fp 7 7 1 binstatclass 5 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstatclass 5 test

rm /data4/*
make
./bin/fp 7 7 1 binstatclass 6 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstatclass 6 test
