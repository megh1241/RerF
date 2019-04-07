from RerF import fastRerF, fastPredict, retSimMat
import numpy as np
from multiprocessing import cpu_count
from scipy.sparse import *
from scipy.sparse.linalg import svds, eigs
import h5py
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from getElbows import get_elbows, get_one_elbow
import time
def get_inp_mat():
	filename = '17977_4_00002_spikes_and_times_v2.h5'
	f = h5py.File(filename, 'r')

	print("Keys: %s" % f.keys())
	a_group_key = list(f.keys())[0]

	data = list(f[a_group_key])
	cell_traces = f['cell_traces'].value
	cell_traces_subset = cell_traces[:, :100]
	labels = np.ones(cell_traces_subset.shape[0])
	labels[3] = 0
	labels[113] = 0
	labels[312] = 0
	labels[37] = 0
	labels[13] = 0
	labels[32] = 0
	return cell_traces_subset, labels


dat, labs = get_inp_mat()
count_tots = 0
count_non_zero = 0
for i in dat:
	for j in i:
		count_tots +=1
		if (j> 0.003):
			count_non_zero+=1


print("NUMBER OF NON ZEROS: ")
print(count_non_zero)
print("TOTAL COUNT: ")
print(count_tots)
			#print (j)
