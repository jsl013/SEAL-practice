#include "examples.h"

using namespace std;
using namespace seal;

void batch_encoder() {
  
  // Parameters
  EncryptionParameters parms(scheme_type::bfv);
  size_t poly_modulus_degree = 8192;
  parms.set_poly_modulus_degree(poly_modulus_degree);
  parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));

  parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

  SEALContext context(parms);
  print_parameters(context);
  cout << endl;

  // Check Batching enabled
  auto qualifier = context.first_context_data()->qualifiers();
  cout << "Batching enabled: " << boolalpha << qualifier.using_batching << endl;

  // Key & Functions
  KeyGenerator keygen(context);
  SecretKey secret_key = keygen.secret_key();
  PublicKey public_key;
  keygen.create_public_key(public_key);

  RelinKeys relin_keys;
  keygen.create_relin_keys(relin_keys);
  Encryptor encryptor(context, public_key);
  Evaluator evaluator(context);
  Decryptor decryptor(context, secret_key);

  // Batching
  BatchEncoder batch_encoder(context);

  size_t slot_count = batch_encoder.slot_count(); // == N
  size_t row_size = slot_count / 2;
  cout << "Plaintext matrix row size: " << row_size << endl;

  // Batch 1
  /*
    The matrix plaintext is simply given to BatchEncoder as a flattened vector
    of numbers. The first `row_size' many numbers form the first row, and the
    rest form the second row. Here we create the following matrix:

        [ 0,  1,  2,  3,  0,  0, ...,  0 ]
        [ 4,  5,  6,  7,  0,  0, ...,  0 ]
  */
  vector<uint64_t> pod_matrix(slot_count, 0ULL);
  pod_matrix[0] = 0ULL;
  pod_matrix[1] = 1ULL;
  pod_matrix[2] = 2ULL;
  pod_matrix[3] = 3ULL;
  pod_matrix[row_size] = 4ULL;
  pod_matrix[row_size + 1] = 5ULL;
  pod_matrix[row_size + 2] = 6ULL;
  pod_matrix[row_size + 3] = 7ULL;

  cout << "Input plaintext matrix:" << endl;
  print_matrix(pod_matrix, row_size);

  Plaintext plain_matrix;
  cout << "Encode plaintext matrix:" << endl;
  batch_encoder.encode(pod_matrix, plain_matrix);

  vector<uint64_t> pod_result;
  cout << "    + Decode plaintext matrix ... Correct" << endl;
  batch_encoder.decode(plain_matrix, pod_result);
  print_matrix(pod_result, row_size);

  Ciphertext encrypted_matrix;
  cout << "Encrypt plain_matrix to encrypted_matrix." << endl;
  encryptor.encrypt(plain_matrix, encrypted_matrix);
  cout << "    + Noise budget in encrypted_matrix: " << decryptor.invariant_noise_budget(encrypted_matrix) << " bits" << endl;

  // Batch 2
  /*
    Operating on the ciphertext results in homomorphic operations being performed
    simultaneously in all 8192 slots (matrix elements). To illustrate this, we
    form another plaintext matrix

        [ 1,  2,  1,  2,  1,  2, ..., 2 ]
        [ 1,  2,  1,  2,  1,  2, ..., 2 ]

    and encode it into a plaintext.
  */
  vector<uint64_t> pod_matrix2;
  for (size_t i = 0; i < slot_count; i++) {
    pod_matrix2.push_back((i & size_t(0x1)) + 1);
  }

  Plaintext plain_matrix2;
  batch_encoder.encode(pod_matrix2, plain_matrix2);
  cout << endl;
  cout << "Second input plaintext matrix:" << endl;
  print_matrix(pod_matrix2, row_size);

  // Computation
  // (Enc(pod_matrix) + pod_matrix2)^2
  cout << "Sum, square, and relinearize." << endl;
  evaluator.add_plain_inplace(encrypted_matrix, plain_matrix2);
  evaluator.square_inplace(encrypted_matrix);
  evaluator.relinearize_inplace(encrypted_matrix, relin_keys);

  cout << "    + Noise budget in result: " << decryptor.invariant_noise_budget(encrypted_matrix) << " bits" << endl;

  // Decryption
  Plaintext plain_result;
  cout << "Decrypt and decode result." << endl;
  decryptor.decrypt(encrypted_matrix, plain_result);
  batch_encoder.decode(plain_result, pod_result);
  cout << "    + Result plaintext matrix ... Correct" << endl;
  print_matrix(pod_result, row_size);
}

int main() {
  batch_encoder();
  return 0;
}
