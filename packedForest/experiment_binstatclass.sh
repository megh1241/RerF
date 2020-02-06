rm /data4/*
rm *.csv
rm out*
make
./bin/fp 7 3 1 binstatclass 4 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 3 1 binstatclass 4 test > out_test

./bin/fp 7 7 1 binstat 4 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstat 4 test > out_test

./bin/fp 7 7 1 binbfs 4 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binbfs 4 test > out_test

./bin/fp 7 7 1 bfs 4 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 bfs 4 test > out_test

./bin/fp 7 7 1 stat 4 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 stat 4 test > out_test

./bin/fp 7 7 1 binstatclass 3 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstatclass 3 test > out_test

./bin/fp 7 7 1 binstat 3 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstat 3 test > out_test
