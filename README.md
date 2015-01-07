# nrpages
Display information about memory use of an application for the program data

I use this tool to optimize applications.  Changing code a bit might mean
one less page of data.  It also helps to see whether data is moved from
pure data segments to relro segments which are more secure.  Example output:

  $ nrpages ~/bin/nrpages 
       /home/drepper/bin/nrpages:    1 read-only,    2 writable,     2 COW,     1 relro

