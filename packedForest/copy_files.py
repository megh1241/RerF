import shutil 
import os

num_of_files = 900
base_dir = '/data4'
base_filename = 'bfsars0.bin'

for i in range(num_of_files): 
    shutil.copy2(os.path.join(base_dir, base_filename), '/data4/bfars{}.bin'.format(i+1))
