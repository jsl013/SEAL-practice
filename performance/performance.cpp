
#include "examples.h"

#include "util/ntt.h"

using namespace std;
using namespace seal;

void performance_test(SEALContext context) {
  chrono::high_resolution_clock::time_point time_start, time_end;

  print_parameters(context);
  cout << endl;

  auto &parms = context.first_context_data()->parms();
  auto &plain_modulus = parms.plain_modulus();
  size_t poly_modulus_degree = parms.poly_modulus_degree();

  KeyGenerator keygen(context);
  
  auto scret_key = keygen.secret_key();
  PublicKey public_key;
  keygen.create_public_key(public_key);

  RelinKeys relin_keys;
  chrono::microseconds time_diff;

}
