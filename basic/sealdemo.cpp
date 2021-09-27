#include "seal/seal.h"
#include "examples.h"
#include <iostream>

using namespace std;
using namespace seal;

void bfv()
{
  EncryptionParameters parms(scheme_type::bfv);

  size_t poly_modulus_degree = 4096;

  parms.set_poly_modulus_degree(poly_modulus_degree);
  parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
  parms.set_plain_modulus(1024);

  SEALContext context(parms);

  print_line(__LINE__);
  cout << "Set encryption paramters and check" << endl;
  print_parameters(context);

  cout << "Paramter Validation(success): " << context.parameter_error_message() << endl;
  cout << endl;

  KeyGenerator keygen(context);
  SecretKey secret_key = keygen.secret_key();
  PublicKey public_key;
  keygen.create_public_key(public_key);

  Encryptor encryptor(context, public_key);
  Evaluator evaluator(context);
  Decryptor decryptor(context, secret_key);

  uint64_t x = 6;
  Plaintext x_plain(uint64_to_hex_string(x));
  cout << "Express x = " + to_string(x) + " as a plaintext polynomial 0x" + x_plain.to_string() + "." << endl;

  Ciphertext x_encrypted;
  cout << "Encrypt x_plain to x_encrypted" << endl;
  encryptor.encrypt(x_plain, x_encrypted);
  cout << "     + size of freshly encrypted x: " << x_encrypted.size() << endl;
  cout << "     + noise budget in freshly encrypted x: " << decryptor.invariant_noise_budget(x_encrypted) << " bits" << endl;

  Plaintext x_decrypted;
  decryptor.decrypt(x_encrypted, x_decrypted);
  cout << "     + decryption of x_encrypted: 0x" << x_decrypted.to_string() << " ... Correct" << endl;

  // Method 1. Naive
  cout << "Compute x^2+1" << endl;
  Ciphertext x_sq_plus_one;
  evaluator.square(x_encrypted, x_sq_plus_one);
  Plaintext plain_one("1");
  evaluator.add_plain_inplace(x_sq_plus_one, plain_one);
  cout << "     + size of x_sq_plus_one: " << x_sq_plus_one.size() << endl;
  cout << "     + noise budget in x_sq_plus_one: " << decryptor.invariant_noise_budget(x_sq_plus_one) << " bits" << endl;

  Plaintext decrypted_result;
  decryptor.decrypt(x_sq_plus_one, decrypted_result);
  cout << "     + decryption of x_encrypted: 0x" << decrypted_result.to_string() << " ... Correct" << endl;

  cout << "Compute (x+1)^2" << endl;
  Ciphertext x_plus_one_sq;
  evaluator.add_plain(x_encrypted, plain_one, x_plus_one_sq);
  evaluator.square_inplace(x_plus_one_sq);
  cout << "    + size of x_plus_one_sq: " << x_plus_one_sq.size() << endl;
  cout << "    + noise budget in x_plus_one_sq: " << decryptor.invariant_noise_budget(x_plus_one_sq) << " bits" << endl;
  cout << "    + decryption of x_plus_one_sq: ";
  decryptor.decrypt(x_plus_one_sq, decrypted_result);
  cout << "0x" << decrypted_result.to_string() << " ... Correct" << endl;

  cout << "Compute encrypted_result (4(x^2+1)(x+1)^2)." << endl;
  Ciphertext encrypted_result;
  Plaintext plain_four("4");
  evaluator.multiply_plain_inplace(x_sq_plus_one, plain_four);
  evaluator.multiply(x_sq_plus_one, x_plus_one_sq, encrypted_result);
  cout << "    + size of encrypted_result: " << encrypted_result.size() << endl;
  cout << "    + noise budget in encrypted_result: " << decryptor.invariant_noise_budget(encrypted_result) << " bits" << endl;
  cout << "NOTE: Decryption can be incorrect if noise budget is zero." << endl;

  cout << endl;
  cout << "~~~~~~ A better way to calculate 4(x^2+1)(x+1)^2. ~~~~~~" << endl;


  // Method 2. Using Relinearization
  cout << "Generate relinearization keys." << endl;
  RelinKeys relin_keys;
  keygen.create_relin_keys(relin_keys);

  cout << "Compute and relinearize x^2, then compute x^2+1" << endl;
  Ciphertext x_squared;
  evaluator.square(x_encrypted, x_squared);
  cout << "    + size of x_squared: " << x_squared.size() << endl;
  evaluator.relinearize_inplace(x_squared, relin_keys);
  cout << "    + size of x_squared (after relinearization): " << x_squared.size() << endl;
  evaluator.add_plain(x_squared, plain_one, x_sq_plus_one);
  cout << "    + noise budget in x_sq_plus_one: " << decryptor.invariant_noise_budget(x_sq_plus_one) << " bits"<< endl;
  cout << "    + decryption of x_sq_plus_one: ";
  decryptor.decrypt(x_sq_plus_one, decrypted_result);
  cout << "0x" << decrypted_result.to_string() << " ... Correct" << endl;

  cout << "Compute x+1, then compute and relinearize (x+1)^2" << endl;
  Ciphertext x_plus_one;
  evaluator.add_plain(x_encrypted, plain_one, x_plus_one);
  evaluator.square(x_plus_one, x_plus_one_sq);
  cout << "    + size of x_plus_one_sq: " << x_plus_one_sq.size() << endl;
  evaluator.relinearize_inplace(x_plus_one_sq, relin_keys);
  cout << "    + noise budget in x_plus_one_sq: " << decryptor.invariant_noise_budget(x_plus_one_sq) << " bits" << endl;
  cout << "    + decryption of x_plus_one_sq: ";
  decryptor.decrypt(x_plus_one_sq, decrypted_result);
  cout << "0x" << decrypted_result.to_string() << " ... Correct" << endl;

  cout << "Compute and relinearize encrypted_result (4(x^2+1)(x+1)^2)." << endl;
  evaluator.multiply_plain_inplace(x_sq_plus_one, plain_four);
  evaluator.multiply(x_sq_plus_one, x_plus_one_sq, encrypted_result);
  cout << "    + size of encrypted_result: " << encrypted_result.size() << endl;
  evaluator.relinearize_inplace(encrypted_result, relin_keys);
  cout << "    + size of encrypted_result (after relinearization): " << encrypted_result.size() << endl;
  cout << "    + noise budget in encrypted_result: " << decryptor.invariant_noise_budget(encrypted_result) << " bits" << endl;

  cout << endl;
  cout << "NOTE: Notice the increase in remaining noise budget." << endl;

  cout << "Decrypt encrypted_result (4(x^2+1)(x+1)^2)." << endl;
  decryptor.decrypt(encrypted_result, decrypted_result);
  cout << "    + decryption of 4(x^2+1)(x+1)^2 = 0x" << decrypted_result.to_string() << " ...... Correct." << endl;
  /*
    For x=6, 4(x^2+1)(x+1)^2 = 7252. Since the plaintext modulus is set to 1024,
    this result is computed in integers modulo 1024. Therefore the expected output
    should be 7252 % 1024 == 84, or 0x54 in hexadecimal.
  */

  // Invalid Parameters
  cout << "An example of invalid parameters" << endl;
  parms.set_poly_modulus_degree(2048);
  context = SEALContext(parms);
  print_parameters(context);
  cout << "Parameter validation (failed): " << context.parameter_error_message() << endl << endl;

}

int main() {
  bfv();

  return 0;
}
