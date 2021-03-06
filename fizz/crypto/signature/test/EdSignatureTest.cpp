/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/signature/Signature.h>
#include <fizz/crypto/signature/test/EdSignatureTest.h>
#include <fizz/crypto/test/TestUtil.h>
#include <folly/String.h>

#define ED25519_FIXTURE(num)                                               \
  Params {                                                                 \
    kEd25519PrivateKey##num, kEd25519PublicKey##num, kEd25519Message##num, \
        kEd25519Signature##num                                             \
  }

using namespace folly;
using namespace folly::ssl;

namespace fizz {
namespace testing {

#if FIZZ_OPENSSL_HAS_ED25519
TEST_P(EdDSATest, TestSignature) {
  auto privateKey = fizz::test::getPrivateKey(GetParam().privateKey);
  auto message = unhexlify(GetParam().hexMessage);
  auto generatedSignature =
      detail::edSign(IOBuf::copyBuffer(message)->coalesce(), privateKey);
  EXPECT_EQ(hexlify(generatedSignature->coalesce()), GetParam().hexSignature);
}

TEST_P(EdDSATest, TestVerify) {
  auto publicKey = fizz::test::getPublicKey(GetParam().publicKey);
  auto message = unhexlify(GetParam().hexMessage);
  auto signature = unhexlify(GetParam().hexSignature);

  // 1. Verification should pass for the message-signature pair in our fixtures
  detail::edVerify(
      IOBuf::copyBuffer(message)->coalesce(),
      folly::ByteRange(folly::StringPiece(signature)),
      publicKey);

  // 2. Verification should fail if the message is modified
  auto modifiedMessage = modifyMessage(message);
  EXPECT_THROW(
      detail::edVerify(
          IOBuf::copyBuffer(modifiedMessage)->coalesce(),
          folly::ByteRange(folly::StringPiece(signature)),
          publicKey),
      std::runtime_error);

  // 3. Verification should fail if the signature is modified
  auto modifiedSignature = modifySignature(signature);
  EXPECT_THROW(
      detail::edVerify(
          IOBuf::copyBuffer(message)->coalesce(),
          folly::ByteRange(folly::StringPiece(modifiedSignature)),
          publicKey),
      std::runtime_error);
}

// Test vectors from RFC8032
INSTANTIATE_TEST_CASE_P(
    TestVectors,
    EdDSATest,
    ::testing::Values(
        ED25519_FIXTURE(1),
        ED25519_FIXTURE(2),
        ED25519_FIXTURE(3),
        ED25519_FIXTURE(4),
        ED25519_FIXTURE(5)));
#endif

std::string modifyMessage(const std::string& input) {
  if (input.size() == 0) {
    return "a";
  }
  auto modifiedMessage = std::string(input);
  modifiedMessage[0] ^= 1; // Flip a  bit in the first character
  return modifiedMessage;
}

std::string modifySignature(const std::string& input) {
  CHECK_GT(input.size(), 0) << "Signatures should have positive length";
  auto modifiedMessage = std::string(input);
  modifiedMessage[0] ^= 1; // Flip a  bit in the first character
  return modifiedMessage;
}
} // namespace testing
} // namespace fizz
