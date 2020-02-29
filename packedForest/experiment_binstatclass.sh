rm *.csv
rm out*
make
rm /data4/*.bin

./bin/fp 7 3 1 align 7 train > out_train
./bin/fp 7 3 1 align 7 test
