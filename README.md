# Exploring Timing attacks

## Overview
This project implements a naive password-cracking approach based on timing variations in password validation. It includes a graphical interface under development and a timing attack demonstration.

## GCD Analysis Branch

In this branch of the Timing Attacks Repo, we are exploring a potential weak point in the nave implementation of the RSA, the GCD attacks. 

### Current Progress:

**gcd_timing_analysis_1.ipynb**
In this script we attempt to measure the timing of the gcd function for gcd(e, phi) for various bit lengths of 5, 6, 7, 8, and 9 of phi.

The idea behind this approach started from noticing that the phi and n often share the same amount of bits, or phi is one less. e and n are public information. Therefore, since we know the bit length of phi and e, we wanted to test the various timings of gcd(e, phi) for all possible phi of corresponding bit lengths.

For example, consider 2 prime numbers of length 5, and the resultant phi is 10 bit length. We would then attempt gcd(e, phi) for all phi from 1000000000 to 1111111111.

This script does it for primes of bit length 5, 6, 7, 8 and 9

Some evidence for the idea that phi and n often share the same amount of bits, or phi is one less can be found [here](https://www.researchgate.net/publication/332323068_Number_of_Digits_in_Two_Integers_and_Their_Multiplication).