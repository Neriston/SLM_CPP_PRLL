import time

def calculate_pi_leibniz(n_terms: int) -> float:
    pi = 0.0
    numerator = 1.0
    for i in range(n_terms):
        denominator = 2 * i + 1
        term = numerator / denominator
        if i % 2 == 0:
            pi += term
        else:
            pi -= term
    return 4 * pi

n_terms = 100_000_000
start = time.time()
pi = calculate_pi_leibniz(n_terms)
end = time.time()
print(f"π ≈ {pi:.15f}, время: {end - start:.3f} сек")
