#include "examples.h"

using namespace std;
using namespace seal;

void levels() {
  EncryptionParameters parms(scheme_type::bfv);

  size_t poly_modulus_degree = 8192;
  parms.set_poly_modulus_degree(poly_modulus_degree);

  parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 50, 30, 30, 50, 50 }));

  parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

  SEALContext context(parms);
  print_parameters(context);
  cout << endl;

}

int main() {
  levels();
  return 0;
}
