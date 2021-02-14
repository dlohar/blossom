import argparse
import re

parser = argparse.ArgumentParser(description='Process log file results per kernel.')

parser.add_argument('-f', dest='file_name', help='name of log file to process')
parser.add_argument('-e', dest='bench_name', help='name of benchmark')
#parser.add_argument("--broad", type=str2bool, nargs='?',
#                        const=True, default=True,
#                        help="Add broad to filename.")

parser.add_argument('--broad', dest='broad', action='store_true')

args = parser.parse_args()

file_name = args.file_name
bench_name = args.bench_name
broad_flag = args.broad


dict_kernel_ranges = {}

with open(file_name, 'r') as fr:
    for _ in range(6):
        next(fr)
    for line in fr:
        # Split the line by at least 2 white-spaces.
        line_list = re.split(r'\s{2,}', line[:-1])
        # Kernel name is 1st entry.
        kernel = line_list[0]
        # Not reachable means -Inf and Inf
        if line_list[-1] == 'Not Reachable':
            min_range = '-Inf'
            max_range = 'Inf'
        # Otherwise need to take last 2 values.
        else:
            min_range = line_list[-2]
            max_range = line_list[-1]
        # Store in a dictionary.
        if kernel not in dict_kernel_ranges:
            dict_kernel_ranges[kernel] = [min_range + ' ' + max_range]
        else:
            dict_kernel_ranges[kernel].append(min_range + ' ' + max_range)

# There is only one kernel.
if len(list(dict_kernel_ranges.keys())) == 1:
    if broad_flag:
        fname = 'broad_kernel_range.dat'
    else:
        fname = 'kernel_range.dat'
    with open(fname, 'w') as fw:
        for s in dict_kernel_ranges[kernel]:
            fw.write(s+'\n')
# We have many kernels. Need separate files for each!
else:
    for kernel in dict_kernel_ranges.keys():
        k_num = kernel.split('_')[-1]
        if broad_flag:
            fname = 'broad_'+ k_num+'_range.dat'
        else:
            fname = k_num + '_range.dat'
        with open(fname, 'w') as fw:
            for s in dict_kernel_ranges[kernel]:
                fw.write(s+'\n')

# Post-processing results further.
if bench_name in ['molecularDynamics', 'nbody']:
    if broad_flag:
        fnm = 'broad_kernel1_range.dat'
    else:
        fnm = 'kernel1_range.dat'
    with open(fnm, "r") as f:
        lines = f.readlines()
    with open(fnm, "w") as f:
        for line in lines[1:]:
            f.write(line)

if bench_name == 'linearSVC':
    if broad_flag:
        fnm = 'broad_kernel_range.dat'
    else:
        fnm = 'kernel_range.dat'
    with open(fnm, "r") as f:
        lines = f.readlines()
    with open(fnm, "w") as f:
        for line in lines[:-1]:
            f.write(line)

if bench_name == 'linpack':
    if broad_flag:
        fnm = 'broad_kernel1_range.dat'
    else:
        fnm = 'kernel1_range.dat'
    with open(fnm, "r") as f:
        lines = f.readlines()
    with open(fnm, "w") as f:
        for line in lines[6:]:
            f.write(line)
    if broad_flag:
        fnm = 'broad_kernel2_range.dat'
    else:
        fnm = 'kernel2_range.dat'
    with open(fnm, "r") as f:
        lines = f.readlines()
    with open(fnm, "w") as f:
        for line in lines[1:-1]:
            f.write(line)


print("Kernel ranges are generated!")
