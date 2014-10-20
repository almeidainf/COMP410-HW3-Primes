LOYOLA UNIVERSITY CHICAGO
COMP 410-001 - OPERATING SYSTEMS - FALL 2014

Tiago de Almeida - tdealmeida@luc.edu
1394611

Homework #3


For this project I decided to rewrite the code in C and run it in Linux environment. This decision was motivated partially because my lack of experience with C# and also because my motivation on working with multithreading with C in Linux.

Two versions of the program were implemented:

- primes_locked: This version was written to be as similar as possible to the code provided by the professor. It creates the threads responsible for verifying whether the numbers are prime, and the read the input file. All numbers are pushed to a queue, from where the other threads pop each number to verify. To control access to the queue, a mutex lock was used. The first thread to aquire the lock is the main thread, which pushes the numbers to the queue. After all numbers all pushed, the remaining threads start poping the numbers from the queue, acquiring the lock for that. It was observed this strategy was better than letting the threads pop numbers from the queue before all numbers were pushed, otherwise they would acquire the lock for that and prevent the main thread from pushing the numbers.

- primes_lockfree: This version changes a bit the algorithm to implement it the way I think is more efficient and allows for a lock-free implementation. In this strategy, every thread has its own queue. First of all, the main thread reads the input file and distributes the numbers for the threads, pushing every number to a different queue. When all the reading is done, each queue has the same (or approximately the same) amount of numbers. Then the counting threads are creating and each one pops numbers from its own queue. Even though we need to read all numbers from the input file before starting to process them, this strategy does not require locks and therefore presents a better performance.

Conclusion:
	The primes_lockfree version offered a general better perfomance. Running both versions on Linux and observing the processors' use through the application htop, it is possible to see how the version primes_locked requires more processing from the system side (showing in red on each core on htop). The version primes_lockfree is not using locks, and therefore can be seen as using almost only user side processing time (green on each core on htop). The lock free version allows for threads to work on the calculations without the hassle of managing locks.


Tools used for this project:
Programming language: C
Compiler: GCC 4.8.1
Operating System: Linux Mint 16 Cinnamon 64 bit
Kernel: 3.11.0-12-generic

- Build project:
	./do

- Run project:
	- Create prime numbers file:
		./primes_locked --createDataFile [file_name]
		./primes_lockfree --createDataFile [file_name]

	- Process file and count the number of primes:
		time ./primes_locked --processDataFile [file_name]
		time ./primes_lockfree --processDataFile [file_name]
