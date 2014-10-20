#!/bin/bash

g++ primes_locked.cpp -o primes_locked -pthread -lm
g++ primes_lockfree.cpp -o primes_lockfree -pthread -lm
