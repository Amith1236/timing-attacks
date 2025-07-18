import random
import timeit
from statistics import median, stdev, mean
from rsa import power, generateKeys, encrypt, decrypt

def create_timing_oracle(ciphertext):
    """
    Creates a timing oracle that measures decryption time for different keys
    """
    def oracle(candidate_key):
        times = []
        for _ in range(timing_samples):
            start_time = timeit.default_timer()
            _ = decrypt(ciphertext, candidate_key, n)
            end_time = timeit.default_timer()
            times.append(end_time - start_time)
        # return median(times)
        return mean(times), median(times), stdev(times)
    return oracle

def create_test_data():
    test_pairs = []
    for _ in range(5):
        plaintext = random.randint(1, n-1)
        ciphertext = encrypt(plaintext, e, n)
        test_pairs.append((ciphertext, plaintext))
    return test_pairs

# GA Parameters
timing_samples = 1000
# https://en.wikipedia.org/wiki/List_of_prime_numbers#The_first_1000_prime_numbers
prime_pqs = [(7823, 7841), (5209, 6917), (7853, 6703), (3931, 5387), (5839, 7829)]

for pq in prime_pqs:
    # RSA Setup
    e, d, n = generateKeys(*pq)

    target_private_key = d
    target_priv_key_str = str(bin(target_private_key)[2:]) # Remove '0b' prefix
    print(f"Key length of {len(target_priv_key_str)} bits: {target_priv_key_str}")
    print(f"Number of '1's: {target_priv_key_str.count('1')}")
    print(f"Number of '0's: {target_priv_key_str.count('0')}")

    test_pairs = create_test_data()
    ciphertexts = [pair[0] for pair in test_pairs]
    timing_oracles = [create_timing_oracle(ct) for ct in ciphertexts]
    # timing_oracle = create_timing_oracle(ciphertexts[0])
    # print("Median")
    print("Mean, Median, Std. Dev")
    # print(timing_oracle(d))
    for index, oracle in enumerate(timing_oracles):
        print(oracle(d))
    print()
