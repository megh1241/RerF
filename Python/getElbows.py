from RerF import fastRerF, fastPredict, retSimMat

import numpy as np
from multiprocessing import cpu_count
from scipy.sparse import *
import h5py
from scipy.stats import norm 
import types


def get_elbows_inner(d, threshold=False, plot=True, main=""):
	np.sort(-d)
	max_q_list = []
	if type(threshold) != bool:
		d = d[np.argwhere(d>threshold)]

	siz = d.size
	if siz <= 0:
		print("must have elements larger than threshold")

	lq = np.zeros(siz)
	for q in range(1, siz):
		mu1 = np.mean(d[0:q])
		mu2 = np.mean(d[q:siz])
		if q<siz:
			tv = 1
		else:
			tv = 0
		print("mean1: ", end='')
		print(mu1)
		print("mean2: ", end='')
		print(mu2)
		sum_c1 = 0
		for i in range(0, q):
			sum_c1 += (d[i]- mu1)**2
		sum_c2 = 0
		for i in range(q+1, siz):
			sum_c2 += (d[i]- mu2)**2
		num_comp_1 = sum_c1
		num_comp_2 = sum_c2
		den = siz-1 -tv
		print("num_comp_1: ", end='')
		print (num_comp_1)
		print("num_comp_2: ", end='')
		print (num_comp_2)
		sigma2 = float(num_comp_1 + num_comp_2) / float(den)
		if sigma2 == 0:
			continue
		#sigma2 = float(sum(d[0:q] - mu1) + sum(d[q:siz] - mu2)) / float(siz-1 - tv)
		print("d vec: ", end='')
		print(d)
		print("sigma 2: ", end='')
		print(sigma2)
		component_1 = sum(norm.pdf(d[0:q], loc=mu1, scale=np.sqrt(sigma2)))
		component_2 = sum(norm.pdf(d[q:siz], loc=mu2, scale=np.sqrt(sigma2)))
		lq[q] = component_1 + component_2
	
	max_q_indx = np.argmax(lq)
	max_q = np.max(lq)
	return max_q_indx, d, siz, max_q


def get_elbows(dat, n=3, threshold=False, plot=True, main=""):
	count_n = n
	q_indx_list = []
	q_list = []
	q = 0
	siz = dat.size
	while n > 0:
		if q >= siz-2: 
			break
		q, d, siz, max_q = get_elbows_inner(dat[q:(siz)], threshold, plot, main)
		q_indx_list.append(q)
		q_list.append(max_q)
		n = n-1

	return q_list, q_indx_list
		
		
def get_one_elbow(dat, threshold=False, plot=True, main=""):
	q, d, siz, max_q = get_elbows_inner(dat, threshold, plot, main)
	return q, max_q
		
