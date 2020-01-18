rm /data4/*
rm *.csv
make
./bin/fp 7 8 2 binstatclass 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 2 binstatclass 1 test

rm /data4/*
mv *.csv res2/

sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 3 binstatclass 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 3 binstatclass 1 test3
rm /data4/
mv *.csv res3/

sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 4 binstatclass 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 4 binstatclass 1 test

rm /data4/*
mv *.csv res4/


sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 5 binstatclass 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 5 binstatclass 1 test

rm /data4/*
mv *.csv res5/

sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 6 binstatclass 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 6 binstatclass 1 test

rm /data4/*
mv *.csv res6/

sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 7 binstatclass 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 7 binstatclass 1 test

rm /data4/*
mv *.csv res7/

sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 8 binstatclass 1 train
sudo echo 3 > sudo /proc/sys/vm/drop_caches
make clean
make
./bin/fp 7 8 8 binstatclass 1 test

rm /data4/*
mv *.csv res8/
