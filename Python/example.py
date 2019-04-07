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
	thresh = 0.0001
	filename = '17977_4_00002_spikes_and_times_v2.h5'
	f = h5py.File(filename, 'r')

	print("Keys: %s" % f.keys())
	a_group_key = list(f.keys())[0]

	data = list(f[a_group_key])
	cell_traces = f['cell_traces'].value
	cell_traces_subset = cell_traces[:, :100]
	num_cells = cell_traces_subset.shape[0]
	num_ts = cell_traces_subset.shape[1]

	cell_traces_thresholded = np.zeros(cell_traces_subset.shape) 	
		
	for i in range(num_cells):
		for j in range(num_ts):
			if cell_traces_subset[i, j] > thresh:	
				cell_traces_thresholded[i, j] = cell_traces_subset[i, j]

	labels = np.ones(num_cells)
	labels[3] = 0
	labels[113] = 0
	labels[312] = 0
	labels[37] = 0
	labels[13] = 0
	labels[32] = 0
	return cell_traces_thresholded, labels


#dat, labs = get_inp_mat()

#datatype = "iris"
datatype = "mnist"
#datatype = "bcm"

if datatype == "iris":
    datafile = "../packedForest/res/iris.csv"
    label_col = 4
    X = np.genfromtxt(datafile, delimiter=",")
    feat_data = X[:, 0:4]
    labels = X[:, label_col]
elif datatype == "mnist":
    datafile = "../packedForest/res/mnist.csv"
    label_col = 0
    X = np.genfromtxt(datafile, delimiter=",")
    feat_data = X[:, 1:]  # mnist
    labels = X[:, label_col]
else:
    feat_data, labels = get_inp_mat()
#print("loading data...")
#X = np.genfromtxt(datafile, delimiter=",")
#print("data loaded")

#if datatype == "iris":
#    feat_data = X[:, 0:4]  # iris
#elif datatype == "mnist":
#    feat_data = X[:, 1:]  # mnist


print ("shape: ")
print (feat_data.shape)

# forest = fastRerF(
#     CSVFile=datafile,
#     Ycolumn=label_col,
#     forestType="binnedBaseRerF",
#     trees=500,
#     numCores=cpu_count() - 1,
# )

start = time.time()
forest = fastRerF(
    X=feat_data,
    Y=labels,
    forestType="urerf",
    trees=100,
    numCores=1,
)

end = time.time()
forest.printParameters()
predictions = fastPredict(feat_data, forest)
# print(predictions)

#print("Error rate", np.mean(predictions != labels))

print("loading test data...")

#if datatype == "iris":
 #   data_fname = "../packedForest/res/iris.csv"  # iris
#elif datatype == "mnist":
#    data_fname = "../packedForest/res/mnist_test.csv"  # mnist
#test_X = np.genfromtxt(data_fname, delimiter=",")

print("data loaded")

#if datatype == "iris":
#    test_data = test_X[:, 0:4]  # iris
# elif datatype == "mnist":
#    test_data = test_X[:, 1:]  # mnist

#test_pred = fastPredict(test_data, forest)
pairMat = retSimMat(forest)

tupList = []
dataCounts = []

for key, value in pairMat.items():
	tupList.append(key)
	dataCounts.append(value)

#tupList = pairMat.keys()
#dataCounts = pairMat.items()

rows,cols = map(list,zip(*tupList))

n_obs = feat_data.shape[0]
dataCounts = np.array(dataCounts)
rows = np.array(rows)
cols = np.array(cols)
print ("number of observations: ")
print (n_obs)
print (dataCounts.shape)
print (rows.shape)
print (cols.shape)
pairMatSparse = csc_matrix((dataCounts, (rows, cols)), shape=(n_obs, n_obs), dtype=np.float64)
s, u = eigs(pairMatSparse)
print ("eigenvalues: ")
print (s)
num_comp = min(s.size, 10)

df = pd.DataFrame({'var':s,
             'PC':[i for i in range(num_comp)]})
fig = sns.barplot(x='PC',y="var", 
           data=df, color="c");
Fig = fig.get_figure()
plt.savefig("scree_example.png")

s_real = np.real(s)
q_list, q_indx = get_one_elbow(s_real)

print("q_indx: ")
print(q_list)


print("TIME ELAPSED: !!!!!!!")
print(end - start)

