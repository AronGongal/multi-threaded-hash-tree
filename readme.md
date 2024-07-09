# Multi-threaded Hash Tree

## Description
This C program was written for a project in a systems programming course. It calculates the hash value of a file, implementing Jenkin's one-at-a-time hash function using multiple threads in a binary tree. The threads at the "edge" of the tree will perform the hash funtion using the blocks of the file assigned to it. Parent threads then concatenate these values together into a string and perform the hash function on it.

## Installation
This program is for use on Unix-based systems, and will need gcc for compilation.

To compile the program, enter this command:
```
% gcc htree.c -o htree -std=gnu99 -pthread
```

## Usage
To use the program, enter this command:
```
% ./htree <filename> <num_threads>
```

Where `<filename>` is the name of the file to get a hash value from, and `<num_threads` is the amount of threads to compute the hash value with.

## Credit
This was created by Aron Gongal, using starter code provided by Professor Sridhar Alagar from The University of Texas at Dallas.

## License
Licensed under the GNU General Public License v3.0.
