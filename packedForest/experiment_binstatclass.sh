rm /data4/*
rm *.csv
rm out*
make
./bin/fp 7 3 1 binstatclass 4 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 3 1 binstatclass 4 test > out_test
