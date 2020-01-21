rm /data4/*
make
<<<<<<< HEAD
./bin/fp 7 7 1 stat 4 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 stat 4 test

=======
./bin/fp 7 7 1 binstat 3 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstat 3 test

rm /data4/*
make
./bin/fp 7 7 1 binstatclass 3 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean


rm /data4/*
make
./bin/fp 7 7 1 binstatclass 3 test
make
./bin/fp 7 7 1 bfs 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 bfs 1 test


rm /data4/*
make
./bin/fp 7 7 1 stat 3 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 stat 3 test

rm /data4/*
rm *.bin
rm *.txt
make
./bin/fp 7 7 1 binbfs 3 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binbfs 3 test
>>>>>>> e565377297f288fae1d0d9e580ef9cf0231785ac
