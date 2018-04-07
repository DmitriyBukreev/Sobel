import os
import sys
import errno
import random
import matplotlib.pyplot as plt

def get_test_result(threadnum):
	fifo_path = "/tmp/stat_fifo"
	bin_path = "../bin/sobel"
	in_file = "../img/in/src_eighth.pgm"
	out_file = "../img/out/src_eighth.pgm"
	
	try:
		os.mkfifo(fifo_path)
	except FileExistsError:
		pass
	# Open the same named pipe in testing program
	fifo = os.open(fifo_path, os.O_RDONLY | os.O_NONBLOCK)
	cmd = bin_path+" -j "+str(threadnum)+" "+in_file+" > "+out_file
	os.system(cmd)
	time = int.from_bytes(os.read(fifo, 4), byteorder = 'little')
	os.close(fifo)
	return time


def collect_stats(threadmin=1, threadmax=1, testnum=1):
	stats = {}
	tests = []
	for threadnum in range(threadmin, threadmax+1):
		print("Number of threads: " + str(threadnum))
		for test in range(0, testnum):
			tests.append(get_test_result(threadnum))
		stats[threadnum] = sum(tests)/len(tests)
		print("Mean value: " + str(stats[threadnum]))
		tests.clear()
	return stats

def main(threadmin=1, threadmax=1, testnum=1, savepng="test.png"):

	x_vals = []
	y_vals = []
	time_per_thread = collect_stats(1, 3, 2)
	for key in time_per_thread:
		x_vals.append(key)
		y_vals.append(time_per_thread[key])
	

	print("Times per thread: " + str(time_per_thread))

	plt.plot(x_vals, y_vals)
	plt.xlabel("Number of threads")
	plt.ylabel("Time, s")
	plt.grid()
	plt.savefig("test.png")
	
	return

# Todo: proper args parsing
if __name__ == "__main__":
	main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])