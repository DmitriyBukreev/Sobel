import os
import sys
import matplotlib.pyplot as plt

fifo_path = '/tmp/stat_fifo'
bin_path = '../bin/sobel'
LONG_INT = 8

# Reads time from pipe
def get_test_result(cmd):
	fifo = os.open(fifo_path, os.O_RDONLY | os.O_NONBLOCK)
	os.system(cmd)
	time = int.from_bytes(os.read(fifo, LONG_INT), byteorder = 'little')
	os.close(fifo)
	print('Time in Python: ' + str(time))
	return time

# Makes <testnum> tests for <threadmax> threads using <infile> 
def collect_stats(threadmax, testnum, infile):
	out_file = './' + str(threadmax) + '_' + str(testnum) + '_' + infile.split('/')[-1]
	stats = {}
	tests = []

	for threadnum in range(1, threadmax+1):
		print('Number of threads: ' + str(threadnum))
		# cmd = bin_path -j N infile 1> outfile 2> /dev/null
		cmd = bin_path+' -j '+str(threadnum)+' '+infile+' 1> '+out_file+' 2> /dev/null'
		for test in range(0, testnum):
			tests.append(get_test_result(cmd))
		# Calculating mean value
		stats[threadnum] = sum(tests)/len(tests)
		print('Mean value: ' + str(stats[threadnum]))
		tests.clear()
	return stats

def build_subgraph(xvals, yvals, xlabel, ylabel):
	plt.plot(xvals, yvals, 'kx-')

	# Markers annotation	
	# for x in xvals:
	# 	plt.annotate(s='{:.0f}'.format(yvals[x-1]),
	# 		xy=(x, yvals[x-1]), xytext=(0, 5),
	# 		textcoords='offset points')

	plt.xlabel(xlabel)
	plt.ylabel(ylabel)
	plt.grid()

def build_table(xvals, yvals, xlabel, ylabel):
	cell_text = [[xlabel, ylabel]]
	for x in xvals:
		cell_text.append([str(x), str(yvals[x-1])])
	plt.axis('off')
	plt.table(cellText=cell_text,
			cellLoc='left', loc='center')


def build_graphs(stats):
	x_vals = []
	y_vals = []
	xlabel = 'Number of threads'
	ylabel_time = 'Time, ns'
	ylabel_acl = 'Acceleration'

	for key in stats:
		x_vals.append(key)
		y_vals.append(stats[key])

	plt.subplot(2,2,1)
	build_subgraph(x_vals, y_vals, 
		xlabel, ylabel_time)

	plt.subplot(2,2,3)
	build_table(x_vals, y_vals,
		xlabel, ylabel_time)

	for y in range(1, len(y_vals)):
		y_vals[y] = y_vals[y]/y_vals[0]
	y_vals[0] = 0

	plt.subplot(2,2,2)
	build_subgraph(x_vals, y_vals,
		xlabel, ylabel_acl)
	
	plt.subplot(2,2,4)
	build_table(x_vals, y_vals,
		xlabel, ylabel_acl)

	plt.subplots_adjust(wspace=0.5, hspace=0.5)

def save_stats(stats, outfile):
	f = open(outfile, 'w')
	for x in stats:
		f.write(str(x)+'|'+str(stats[x])+'\n')
	f.close()

def read_stats(infile):
	stats = {}
	f = open(infile, 'r')
	for line in f:
		key, value = line.split('|')
		stats[int(key)] = float(value.rstrip())
	f.close()
	return stats


def main():
	if len(sys.argv) == 4:
		threadmax = int(sys.argv[1])
		testnum = int(sys.argv[2])
		infile = sys.argv[3]
		outfile = './' + sys.argv[1] + '_' + sys.argv[2] \
		+ '_' + infile.split('/')[-1].split('.')[0]

		try:
			os.mkfifo(fifo_path)
		except FileExistsError:
			pass

		stats = collect_stats(threadmax, testnum, infile)
		build_graphs(stats)
		plt.savefig(outfile + '.png')
		save_stats(stats, outfile + '.txt')

	elif len(sys.argv) == 2:
		stats = read_stats(sys.argv[1])
		build_graphs(stats)
		plt.show()
	else:
		print('Wrong usage')


if __name__ == '__main__':
	main()