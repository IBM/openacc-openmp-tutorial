#!/usr/bin/env python
import numpy as np
from tabulate import tabulate

log_file = "out/log.txt"
log_file_ref = "out_reference/log.txt"
log_file_timings = "out/log-timings.txt"

vals = np.loadtxt(log_file,skiprows=1)
vals_ref = np.loadtxt(log_file_ref,skiprows=1)
vals_fields = open(log_file_ref,'r').readline().replace("\n",'').split(" ")
vals_fields.pop(0)
vals = np.delete(vals,0,1);
vals_ref = np.delete(vals_ref,0,1);

vals_ref = np.resize(vals_ref,np.shape(vals))
deviation = np.abs(vals-vals_ref)/vals_ref
deviation = np.insert(deviation,0,np.arange(vals.shape[0]),1)

timings = np.loadtxt(log_file_timings,skiprows=1)
timings_fields = open(log_file_timings,'r').readline().replace("\n",'').split(" ")
timings_fields.pop(0)

def print_table(table, fields, header):
	tablestring = tabulate(table, fields)
	tablewidth = tablestring.index('\n')	
	print
	print '{:*^{width}}'.format(" "+header+" ",width=tablewidth)
	print tablestring

print_table(deviation, vals_fields, "Relative Deviations from Reference Output")
print_table(timings, timings_fields, "Runtimes and Call-Counts")
