#include "seal/seal.h"
#include "seal/modulus.h"
#include "seal/util/ntt.h"
#include <chrono>
#include <iostream>

using namespace std;
using namespace seal;
using namespace seal::util;

void ntt(int coeff_count_power) {
  bool verbose = false;

  chrono::high_resolution_clock::time_point time_start, time_end;
  chrono::nanoseconds time_diff;
  
  MemoryPoolHandle pool = MemoryPoolHandle::Global();
  Pointer<NTTTables> tables;
//  Modulus modulus(0xffffffffffc0001ULL);
//  Modulus modulus(0x1e01ULL);
  
  vector<int> pow_twelve_bit_sizes = { 54, 55 };
  vector<int> pow_thirteen_bit_sizes = { 54, 54, 55, 55 };
  vector<int> pow_fourteen_bit_sizes = { 54, 54, 55, 55, 55, 55, 55, 55 };

  cout << "N = " << (uint64_t(1) << coeff_count_power) << endl;
  
  switch(coeff_count_power) {
    case 10: {
      Modulus modulus(get_prime(uint64_t(1) << coeff_count_power, 40)); //40 == bit_size
      tables = allocate<NTTTables>(pool, coeff_count_power, modulus, pool);
      break;
    }
    case 11: {
      Modulus modulus(get_prime(uint64_t(1) << coeff_count_power, 55));
      tables = allocate<NTTTables>(pool, coeff_count_power, modulus, pool);
      break;
    }
    case 12: {
      vector<Modulus> modulus = CoeffModulus::Create(uint64_t(1) << coeff_count_power, pow_twelve_bit_sizes);
      CreateNTTTables(coeff_count_power, modulus, tables, pool);
      break;
    }
    case 13: {
      vector<Modulus> modulus = CoeffModulus::Create(uint64_t(1) << coeff_count_power, pow_thirteen_bit_sizes);
      CreateNTTTables(coeff_count_power, modulus, tables, pool);
      break;
    }
    case 14: {
      vector<Modulus> modulus = CoeffModulus::Create(uint64_t(1) << coeff_count_power, pow_fourteen_bit_sizes);
      CreateNTTTables(coeff_count_power, modulus, tables, pool);
      break;
    }
    default:
      cout << "Invalid coeff_count_power" << endl;
      return;
  }

  /* CreateNTTTables(coeff_count_power, CoeffModulus::Create(uint64_t(1) << coeff_count_power, { 20, 20, 20, 20, 20 }), tables, pool); */
  
  auto poly(allocate_poly(uint64_t(1) << coeff_count_power, 1, pool));
  
  for (size_t i = 0; i < (uint64_t(1) << coeff_count_power); i++) {
    poly[i] = i;
  }

  if (verbose) {
    cout << "poly = " << endl << "[";
    for (size_t i = 0; i < (uint64_t(1) << coeff_count_power); i++) {
      cout << poly[i] << ", ";
    }
    cout << "]" << endl;
  }

  time_start = chrono::high_resolution_clock::now();
  ntt_negacyclic_harvey_lazy(poly.get(), *tables);
  time_end = chrono::high_resolution_clock::now();

  time_diff = chrono::duration_cast<chrono::nanoseconds>(time_end - time_start);
  float time_diff_micro = time_diff.count() * 1.000 / 1000;

  cout << "NTT is done [" << time_diff_micro << " microseconds]" << endl;

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

  time_diff = chrono::duration_cast<chrono::nanoseconds>(time_end - time_start);
  time_diff_micro = time_diff.count() * 1.000 / 1000;

  cout << "INTT is done [" << time_diff_micro << " microseconds]" << endl;

  if (verbose) {
    cout << "intt_ntt_poly = " << endl << "[";
    for (size_t i = 0; i < (uint64_t(1) << coeff_count_power); i++) {
      cout << poly[i] << ", ";
    }
    cout << "]" << endl;
  }
}

int main(int argc, char** argv) {

  int coeff_count_power;
  stringstream ssint_coeff_count_power(argv[1]);

  if (ssint_coeff_count_power.fail()) {
    cout << "Invalid coeff_count_power" << endl;
    return 0;
  }
  ssint_coeff_count_power >> coeff_count_power;

  ntt(coeff_count_power);

  return 0;
}
