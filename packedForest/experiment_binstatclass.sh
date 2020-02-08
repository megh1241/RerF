rm *.csv
rm out*
make
rm /data4/*.bin
./bin/fp 7 7 1 binstatclass 7 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 7 1 binstatclass 7 test > out_test
