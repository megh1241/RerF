rm /data4/*
make
./bin/fp 7 8 1 bfs 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 1 bfs 1 test

rm /data4/*
make
./bin/fp 7 8 1 binstat 5 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 1 binstat 5 test

rm /data4/*
make
./bin/fp 7 8 1 binstatclass 5 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 1 binstatclass 5 test

rm /data4/*
make
./bin/fp 7 8 1 stat 5 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 1 stat 5 test

rm /data4/*
make
./bin/fp 7 8 1 binbfs 5 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 1 binbfs 5 test
