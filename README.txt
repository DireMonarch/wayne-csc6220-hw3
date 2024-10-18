Jim Haslett's submission for Wayne State University's Fall 2024 CSC6220
Homework 3 assignment.

This program will generate two 256 x 256 matricies (A and B), and fill them with
the integer progression 1, 2, 3, ... 256 on each row.  It will also generate a
destination matrix (C) and fill it with all zeros.  It will then spawn 8 threads
where each thread will do an elementwise additon of 32 rows from A and B and
put the result directly into C.  Each thread is responsible for rows i * 32 
to (i * 32) + 31, where i is the zero indexed thread number.

After each worker thread is complete, it will print the line "Thread i: Done",
where i is the zero indexed thread number.  These result lines are ensured
to print in decreasing order of i (7, 6, ... 0) by use of a mutex and condition
variable barrier synchronization scheme.

Once each thread completes, the default thread will contiue and write the 
result matrix to a file named result.txt in the current directory.