#include "seal/seal.h"
#include "seal/modulus.h"
#include "seal/util/ntt.h"
#include <chrono>
#include <iostream>

using namespace std;
using namespace seal;
using namespace seal::util;

void ntt() {
  chrono::high_resolution_clock::time_point time_start, time_end;
  chrono::microseconds time_diff;
  bool verbose = false;
  
  MemoryPoolHandle pool = MemoryPoolHandle::Global();
  Pointer<NTTTables> tables;
  int coeff_count_power = 10;
/* Modulus modulus(0xffffffffffc0001ULL); */
/* Modulus modulus(0x1e01ULL); */
  Modulus modulus(get_prime(uint64_t(1) << coeff_count_power, 40)); //40 == bit_size

  tables = allocate<NTTTables>(pool, coeff_count_power, modulus, pool);

  /* CreateNTTTables(coeff_count_power, CoeffModulus::Create(uint64_t(1) << coeff_count_power, { 20, 20, 20, 20, 20 }), tables, pool); */
  
  auto poly(allocate_poly(uint64_t(1) << coeff_count_power, 1, pool));
  
  for (size_t i = 0; i < (uint64_t(1) << coeff_count_power); i++) {
    poly[i] = i;
  }

  cout << "poly = " << endl << "[";
  for (size_t i = 0; i < (uint64_t(1) << coeff_count_power); i++) {
    cout << poly[i] << ", ";
  }
  cout << "]" << endl;

  time_start = chrono::high_resolution_clock::now();
  ntt_negacyclic_harvey_lazy(poly.get(), *tables);
  time_end = chrono::high_resolution_clock::now();

  time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);

  cout << "NTT is done [" << time_diff.count() << " microseconds]" << endl;

  if (verbose) {
    cout << "ntt_poly = " << endl << "[";
    for (size_t i = 0; i < (uint64_t(1) << coeff_count_power); i++) {
      cout << poly[i] << ", ";
    }
    cout << "]" << endl;
  }

  time_start = chrono::high_resolution_clock::now();
  inverse_ntt_negacyclic_harvey_lazy(poly.get(), *tables);
  time_end = chrono::high_resolution_clock::now();

  time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);

  cout << "INTT is done [" << time_diff.count() << " microseconds]" << endl;

  if (verbose) {
    cout << "intt_ntt_poly = " << endl << "[";
    for (size_t i = 0; i < (uint64_t(1) << coeff_count_power); i++) {
      cout << poly[i] << ", ";
    }
    cout << "]" << endl;
  }
}

int main() {

  ntt();

  return 0;
}
