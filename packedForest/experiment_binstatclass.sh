sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 1 binstatclass 6 test 1 > out_test
mv elapsed* batch1/
sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 1 binstatclass 6 test 100 > out_test
mv elapsed* batch100/
sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 1 binstatclass 6 test 500 > out_test
mv elapsed* batch500/
sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 1 binstatclass 6 test 1000 > out_test
mv elapsed* batch1000/
sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 1 binstatclass 6 test 2000 > out_test
mv elapsed* batch2000/
sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 1 binstatclass 6 test 5000 > out_test
mv elapsed* batch5000/
sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 1 binstatclass 6 test 10000 > out_test
mv elapsed* batch10000/
sudo echo 3 > sudo /proc/sys/vm/drop_caches
./bin/fp 7 8 1 binstatclass 6 test 20000 > out_test
mv elapsed* batch20000/
