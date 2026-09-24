#ifndef FASTRNG_hh
#define FASTRNG_hh

// Fast Randon Number Generator
class FastRNG {
  
private:

  // internal state
  uint64_t s; 

public:
  
  // --- Constructor / Seeder ---
  explicit FastRNG(uint64_t seed = 1) noexcept : s(seed) {}

  // --- Core wyRand generator (64-bit) ---
  inline uint64_t next() noexcept {
    s += 0xA0761D6478BD642FULL;
    __uint128_t t = (__uint128_t)s * (s ^ 0xE7037ED1A0B428DBULL);
    return (uint64_t)((t >> 64) ^ t);
  }

  // --- Get a random integer in [1, n] (unbiased) ---
  inline uint64_t range(uint64_t n) noexcept {
    __uint128_t p = (__uint128_t)next() * n;
    uint64_t lo = (uint64_t)p;
    if (lo < n) {                      // remove modulo bias
      uint64_t t = -n % n;
      while (lo < t) {
        p = (__uint128_t)next() * n;
        lo = (uint64_t)p;
      }
    }
    return (uint64_t)(p >> 64);
  }

  // --- Get a random double in [0, 1) ---
  inline double uniform01() noexcept {
    return (next() >> 11) * (1.0 / (1ULL << 53));
  }

  // --- Reseed if needed ---
  inline void reseed(uint64_t seed) noexcept { s = seed; }
};

#endif
