rm out*
make
./bin/fp 7 3 64 binstatclass 7 train > out_train
sudo echo 3 > sudo /proc/sys/vm/drop_caches

./bin/fp 7 3 1 binstatclass 7 test > out_test
mv *.csv res1/
sudo echo 3 > sudo /proc/sys/vm/drop_caches

./bin/fp 7 3 2 binstatclass 7 test > out_test
mv *.csv res2/
sudo echo 3 > sudo /proc/sys/vm/drop_caches

./bin/fp 7 3 3 binstatclass 7 test > out_test
mv *.csv res3/
sudo echo 3 > sudo /proc/sys/vm/drop_caches

./bin/fp 7 3 4 binstatclass 7 test > out_test
mv *.csv res4/
sudo echo 3 > sudo /proc/sys/vm/drop_caches

./bin/fp 7 3 5 binstatclass 7 test > out_test
mv *.csv res5/
sudo echo 3 > sudo /proc/sys/vm/drop_caches

./bin/fp 7 3 6 binstatclass 7 test > out_test
mv *.csv res6/
sudo echo 3 > sudo /proc/sys/vm/drop_caches

./bin/fp 7 3 7 binstatclass 7 test > out_test
mv *.csv res7/
sudo echo 3 > sudo /proc/sys/vm/drop_caches

./bin/fp 7 3 8 binstatclass 7 test > out_test
mv *.csv res8/
sudo echo 3 > sudo /proc/sys/vm/drop_caches
