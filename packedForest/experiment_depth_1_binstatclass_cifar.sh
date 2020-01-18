rm /data4/*
rm *.csv
make
./bin/fp 7 3 1 binstatclass 1 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 3 1 binstatclass 1 test > out_test

rm /data4/*
make
./bin/fp 7 3 1 binstatclass 2 train > out_train2
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 3 1 binstatclass 2 test > out_test2

rm /data4/*
make
./bin/fp 7 3 1 binstatclass 3 train > out_train3
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 3 1 binstatclass 3 test > out_test3

rm /data4/*
make
./bin/fp 7 3 1 binstatclass 4 train > out_train4
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 3 1 binstatclass 4 test > out_test4

rm /data4/*
make
./bin/fp 7 3 1 binstatclass 5 train > out_train5
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 3 1 binstatclass 5 test > out_test5

rm /data4/*
make
./bin/fp 7 3 1 binstatclass 6 train > out_train6
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 3 1 binstatclass 6 test > out_test6
