# Bruteforce FTP tool

This is a command-line tool for performing brute-force attacks against remote services that require authentication.


## Requirements

***

To compile and use this tool, you will need the following:

C compiler (GCC or Clang)
pthread library
A remote service to test against (e.g., SSH server)

## Compilation

***

To compile the tool, run the following command:


```
$ make
```

This will create an executable file called bruteforce.

## Usage

***

The tool takes the following arguments:


```
$ ./bruteforce -h

Usage: ./bruteforce [options] <target>

Options:
  -u, --userfile FILE     File containing usernames to try
  -p, --passfile FILE     File containing passwords to try
  -t, --threads N         Number of threads to use (default: 1)
  -v, --verbose           Verbose output
  -s, --silent            Silent mode (only output successful attempts)
  -h, --help              Show this help message and exit

```

For example, to try all combinations of usernames and passwords in the files users.txt and passwords.txt, using 4 threads, and connecting to an SSH server on the local machine, run:

```
$ ./bruteforce -u users.txt -p passwords.txt -t 4 localhost
```

## Implementation Details

***

The tool uses a multi-threaded approach to perform the brute-force attack. The user and password files are divided into chunks, and each thread is assigned a chunk to work on. The number of threads can be controlled using the `-t` option.

The tool uses the `connectToServer` function to establish a connection with the target server and attempt authentication. This function should be implemented in a separate file called `connect.c`

