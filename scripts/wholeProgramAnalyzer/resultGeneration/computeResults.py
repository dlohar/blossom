#!/usr/bin/env python

import statistics
#import numpy as np
import os
import sys

if len( sys.argv ) == 1:
    print("TACAS21 Tables Generation")
    print("\tusage: python computeResults.py results_directory/tool_name")
    exit()

results_dir = sys.argv[1]
tbl_number = int(sys.argv[2])
benchmarks = next(os.walk(results_dir))[1]

# Print result: name of the benchmark
if tbl_number == 2:
    f_res2 = open(results_dir+'/computeResults_table2.log',"w")
elif tbl_number == 3:
    f_res3 = open(results_dir+'/computeResults_table3.log',"w")

print(' ----- TABLE ',tbl_number,'-----')
    
for benchmark in benchmarks:

    if tbl_number == 2:
        print(benchmark, file=f_res2)
    elif tbl_number == 3:
        print(benchmark, file=f_res3)
    print('\n -----',benchmark,'-----')
    
    nb_runs = len(next(os.walk(results_dir+'/'+benchmark))[1])
    temp_kernels = next(os.walk(results_dir+'/'+benchmark+'/ex1'))[2]

    # Adap result files list if the results come from AFLGo
    # is_aflgo = 0
    # for k in temp_kernels:
    #    if k.find('broad') == 0:
    #        is_aflgo = 1

    # if is_aflgo == 1:
    kernels = []
    for k in temp_kernels:
        if k.find('broad') == 0:
            kernels.append(k)
    # else:
    #    kernels = temp_kernels
        
    # Compute results of kernel1 over several runs and variables
    for kernel in kernels:
        # read in files the ranges of the variables
        f = open(results_dir+'/'+benchmark+'/ex1/'+kernel,"r")
        lines1 = f.readlines()
        if 'Inf' in lines1[0]:
            p_kernel = kernel.split('_')[1]
            if tbl_number == 2:
                print(p_kernel,":\t Not Reachable")
                print(p_kernel,":\t Not Reachable", file=f_res2)
            elif tbl_number == 3:
                print(p_kernel,":\t Not Reachable", file=f_res3)
                print(p_kernel,":\t Not Reachable")
            continue

        nb_variables = len(lines1) #len(f.readlines(  ))
        f.close()
        #t_low, t_high = [], []
        t_low = [[None] * nb_runs for _ in range(nb_variables)]
        t_high = [[None] * nb_runs for _ in range(nb_variables)]
        #t_low = np.empty([nb_variables,nb_runs])
        #t_high = np.empty([nb_variables,nb_runs])
        for run in range(0,nb_runs):
            f = open(results_dir+'/'+benchmark+'/ex'+str(run+1)+'/'+kernel,"r")
            variable = 0
            low_in, high_in = [], []
            for line in f.readlines():
                fields = line.split(' ')
                t_low[variable][run] = float(fields[0])
                t_high[variable][run] = float(fields[1])
                #print(fields[0], t_low[variable][run], fields[1], t_high[variable][run])
                variable += 1
            f.close()

        # Compute the width over variables
        #t_width_per_variable = np.empty([nb_variables,nb_runs])
        #t_mean_over_runs = np.empty([nb_variables])
        #t_rel_change_over_runs = np.empty([nb_variables])
        t_width_per_variable = [[None] * nb_runs for _ in range(nb_variables)]
        t_mean_over_runs = [None for _ in range(nb_variables)]
        #t_mean_over_runs = []
        t_rel_change_over_runs = [None for _ in range(nb_variables)]
        for v in range(0,nb_variables):
            # Check for constants to avoid ZeroDivisionError.
            if t_high[v][0] == t_low[v][0]:
                continue
            for i in range(nb_runs):
                t_width_per_variable[v][i] = t_high[v][i] - t_low[v][i]
                #print(t_width_per_variable[v][i], t_high[v][i], t_low[v][i])
            #print(t_width_per_variable[v])
            t_mean_over_runs[v] = statistics.mean(t_width_per_variable[v])
            #t_mean_over_runs.append(statistics.mean(t_width_per_variable[v]))
            #sprint(t_mean_over_runs[v])
            t_rel_change_over_runs[v] = (max(t_width_per_variable[v])-min(t_width_per_variable[v]))/t_mean_over_runs[v]
        #print(t_mean_over_runs, t_rel_change_over_runs)
        t_mean_over_runs = [x for x in t_mean_over_runs if x is not None]
        t_rel_change_over_runs = [x for x in t_rel_change_over_runs if x is not None]
        mean_over_variables = statistics.mean(t_mean_over_runs)
        rel_change_over_variables = statistics.mean(t_rel_change_over_runs)
        # print results
        p_kernel = kernel.split('_')[1]
        if tbl_number == 2:
            print(p_kernel,":\t %.5f" %mean_over_variables)
            print(p_kernel,":\t %.5f" %mean_over_variables, file=f_res2)
        elif tbl_number == 3:
            print(p_kernel,":\t",rel_change_over_variables,"%", file=f_res3)
            print(p_kernel,":\t",rel_change_over_variables,"%")

if tbl_number == 2:
    f_res2.close()
elif tbl_number == 3:
    f_res3.close()
