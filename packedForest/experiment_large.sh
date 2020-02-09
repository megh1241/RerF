rm *.csv
rm out*
make
./bin/fp 7 9 64 binstat 2 train > out_train
