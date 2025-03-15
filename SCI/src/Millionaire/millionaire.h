/*
Authors: Deevashwer Rathee, Mayank Rathee
Copyright:
Copyright (c) 2021 Microsoft Research
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef MILLIONAIRE_H__
#define MILLIONAIRE_H__
#include "Millionaire/bit-triple-generator.h"
#include "OT/emp-ot.h"
#include "utils/emp-tool.h"
#include <cmath>
#include <omp.h>
#include <chrono>

#define MILL_PARAM 4
#define WAN_EXEC

class MillionaireProtocol {
public:
  sci::IOPack *iopack;
  sci::OTPack *otpack;
  TripleGenerator *triple_gen;
  int party;
  int l, r, log_alpha, beta, beta_pow;
  int num_digits, num_triples_corr, num_triples_std, log_num_digits;
  int num_triples;
  uint8_t mask_beta, mask_r;

  MillionaireProtocol(int party, sci::IOPack *iopack, sci::OTPack *otpack,
                      int bitlength = 32, int radix_base = MILL_PARAM) {
    this->party = party;
    this->iopack = iopack;
    this->otpack = otpack;
    this->triple_gen = new TripleGenerator(party, iopack, otpack);
    configure(bitlength, radix_base);
  }

  void configure(int bitlength, int radix_base = MILL_PARAM) {
    assert(radix_base <= 8);
    assert(bitlength <= 64);
    this->l = bitlength;
    this->beta = radix_base;

    this->num_digits = ceil((double)l / beta);
    this->r = l % beta;
    this->log_alpha = sci::bitlen(num_digits) - 1;
    this->log_num_digits = log_alpha + 1;
    this->num_triples_corr = 2 * num_digits - 2 - 2 * log_num_digits;
    this->num_triples_std = log_num_digits;
    this->num_triples = num_triples_std + num_triples_corr;
    if (beta == 8)
      this->mask_beta = -1;
    else
      this->mask_beta = (1 << beta) - 1;
    this->mask_r = (1 << r) - 1;
    this->beta_pow = 1 << beta;
  }

  ~MillionaireProtocol() { delete triple_gen; }

  void compare(uint8_t *res, uint64_t *data, int num_cmps, int bitlength,
               bool greater_than = true, bool equality = false,
               int radix_base = MILL_PARAM) {
    auto total_start = std::chrono::high_resolution_clock::now();

    auto config_start = std::chrono::high_resolution_clock::now();
    configure(bitlength, radix_base);
    auto config_end = std::chrono::high_resolution_clock::now();

     if (party == sci::ALICE) {
      //std::cout << "Alice - configuration: " << std::chrono::duration_cast<std::chrono::microseconds>(config_end - config_start).count() << " us" << std::endl;
    }
    else {
      //std::cout << "Bob - configuration: " << std::chrono::duration_cast<std::chrono::microseconds>(config_end - config_start).count() << " us" << std::endl;
    }

    // if (bitlength <= beta) {
    //   uint8_t N = 1 << bitlength;
    //   uint8_t mask = N - 1;
    //   if (party == sci::ALICE) {
    //     auto alice_start = std::chrono::high_resolution_clock::now();
        
    //     auto prg_start = std::chrono::high_resolution_clock::now();
    //     sci::PRG128 prg;
    //     prg.random_data(res, num_cmps * sizeof(uint8_t));
    //     auto prg_end = std::chrono::high_resolution_clock::now();

    //     auto alloc_start = std::chrono::high_resolution_clock::now();
    //     uint8_t **leaf_messages = new uint8_t *[num_cmps];
    //     auto alloc_end = std::chrono::high_resolution_clock::now();

    //     auto loop_start = std::chrono::high_resolution_clock::now();
    //     for (int i = 0; i < num_cmps; i++) {
    //       res[i] &= 1;
    //       leaf_messages[i] = new uint8_t[N];
    //       for (int j = 0; j < N; j++) {
    //         if (greater_than) {
    //           leaf_messages[i][j] = ((uint8_t(data[i] & mask) > j) ^ res[i]);
    //         } else {
    //           leaf_messages[i][j] = ((uint8_t(data[i] & mask) < j) ^ res[i]);
    //         }
    //       }
    //     }
    //     auto loop_end = std::chrono::high_resolution_clock::now();

    //     auto ot_start = std::chrono::high_resolution_clock::now();
    //     if (bitlength > 1) {
    //       otpack->kkot[bitlength - 1]->send(leaf_messages, num_cmps, 1);
    //     } else {
    //       otpack->iknp_straight->send(leaf_messages, num_cmps, 1);
    //     }
    //     auto ot_end = std::chrono::high_resolution_clock::now();

    //     auto cleanup_start = std::chrono::high_resolution_clock::now();
    //     for (int i = 0; i < num_cmps; i++)
    //       delete[] leaf_messages[i];
    //     delete[] leaf_messages;
    //     auto cleanup_end = std::chrono::high_resolution_clock::now();

    //     auto alice_end = std::chrono::high_resolution_clock::now();

    //     std::cout << "ALICE TIMING:" << std::endl;
    //     std::cout << "  ALICE - PRG generation: " << std::chrono::duration_cast<std::chrono::microseconds>(prg_end - prg_start).count() << " us" << std::endl;
    //     std::cout << "  ALICE - Memory allocation: " << std::chrono::duration_cast<std::chrono::microseconds>(alloc_end - alloc_start).count() << " us" << std::endl;
    //     std::cout << "  ALICE - Comparison loops: " << std::chrono::duration_cast<std::chrono::microseconds>(loop_end - loop_start).count() << " us" << std::endl;
    //     std::cout << "  ALICE - OT protocol: " << std::chrono::duration_cast<std::chrono::microseconds>(ot_end - ot_start).count() << " us" << std::endl;
    //     std::cout << "  ALICE - Cleanup: " << std::chrono::duration_cast<std::chrono::microseconds>(cleanup_end - cleanup_start).count() << " us" << std::endl;
    //     std::cout << "  Total Alice time: " << std::chrono::duration_cast<std::chrono::microseconds>(alice_end - alice_start).count() << " us" << std::endl;
    //   } else { // party == BOB
    //     auto bob_start = std::chrono::high_resolution_clock::now();

    //     auto alloc_start = std::chrono::high_resolution_clock::now();
    //     uint8_t *choice = new uint8_t[num_cmps];
    //     auto alloc_end = std::chrono::high_resolution_clock::now();

    //     auto loop_start = std::chrono::high_resolution_clock::now();
    //     for (int i = 0; i < num_cmps; i++) {
    //       choice[i] = data[i] & mask;
    //     }
    //     auto loop_end = std::chrono::high_resolution_clock::now();

    //     auto ot_start = std::chrono::high_resolution_clock::now();
    //     if (bitlength > 1) {
    //       otpack->kkot[bitlength - 1]->recv(res, choice, num_cmps, 1);
    //     } else {
    //       otpack->iknp_straight->recv(res, choice, num_cmps, 1);
    //     }
    //     auto ot_end = std::chrono::high_resolution_clock::now();

    //     auto cleanup_start = std::chrono::high_resolution_clock::now();
    //     delete[] choice;
    //     auto cleanup_end = std::chrono::high_resolution_clock::now();

    //     auto bob_end = std::chrono::high_resolution_clock::now();

    //     // Print Bob's timing results
    //     std::cout << "BOB TIMING:" << std::endl;
    //     std::cout << " BOB - Memory allocation: " << std::chrono::duration_cast<std::chrono::microseconds>(alloc_end - alloc_start).count() << " us" << std::endl;
    //     std::cout << "  BOB - Mask operation loop: " << std::chrono::duration_cast<std::chrono::microseconds>(loop_end - loop_start).count() << " us" << std::endl;
    //     std::cout << "  BOB - OT protocol: " << std::chrono::duration_cast<std::chrono::microseconds>(ot_end - ot_start).count() << " us" << std::endl;
    //     std::cout << "  BOB - Cleanup: " << std::chrono::duration_cast<std::chrono::microseconds>(cleanup_end - cleanup_start).count() << " us" << std::endl;
    //     std::cout << "  Total Bob time: " << std::chrono::duration_cast<std::chrono::microseconds>(bob_end - bob_start).count() << " us" << std::endl;
    //   }
    //   auto total_end = std::chrono::high_resolution_clock::now();

    //   if (party == sci::ALICE) {
    //     std::cout << "ALICE - Total comparison time: " << std::chrono::duration_cast<std::chrono::microseconds>(total_end - total_start).count() << " us" << std::endl;
    //   }
    //   else {
    //     std::cout << "BOB - Total comparison time: " << std::chrono::duration_cast<std::chrono::microseconds>(total_end - total_start).count() << " us" << std::endl;
    //   }
    //   return;
    // }

    int old_num_cmps = num_cmps;
    // num_cmps should be a multiple of 8
    num_cmps = ceil(num_cmps / 8.0) * 8;

    auto data_prep_start = std::chrono::high_resolution_clock::now();
    uint64_t *data_ext;
    if (old_num_cmps == num_cmps)
      data_ext = data;
    else {
      data_ext = new uint64_t[num_cmps];
      memcpy(data_ext, data, old_num_cmps * sizeof(uint64_t));
      memset(data_ext + old_num_cmps, 0,
             (num_cmps - old_num_cmps) * sizeof(uint64_t));
    }
    auto data_prep_end = std::chrono::high_resolution_clock::now();

    auto mem_alloc_start = std::chrono::high_resolution_clock::now();
    uint8_t *digits;       // num_digits * num_cmps
    uint8_t *leaf_res_cmp; // num_digits * num_cmps
    uint8_t *leaf_res_eq;  // num_digits * num_cmps

    digits = new uint8_t[num_digits * num_cmps];
    leaf_res_cmp = new uint8_t[num_digits * num_cmps];
    leaf_res_eq = new uint8_t[num_digits * num_cmps];
    auto mem_alloc_end = std::chrono::high_resolution_clock::now();

    auto digit_extract_start = std::chrono::high_resolution_clock::now();
    // Extract radix-digits from data
    for (int i = 0; i < num_digits; i++) // Stored from LSB to MSB
      for (int j = 0; j < num_cmps; j++)
        if ((i == num_digits - 1) && (r != 0))
          digits[i * num_cmps + j] =
              (uint8_t)(data_ext[j] >> i * beta) & mask_r;
        else
          digits[i * num_cmps + j] =
              (uint8_t)(data_ext[j] >> i * beta) & mask_beta;
    auto digit_extract_end = std::chrono::high_resolution_clock::now();

    if (party == sci::ALICE) {
      auto alice_start = std::chrono::high_resolution_clock::now();
      
      auto leaf_alloc_start = std::chrono::high_resolution_clock::now();
      uint8_t **leaf_ot_messages; // (num_digits * num_cmps) X beta_pow (=2^beta)
      leaf_ot_messages = new uint8_t *[num_digits * num_cmps];
      for (int i = 0; i < num_digits * num_cmps; i++)
        leaf_ot_messages[i] = new uint8_t[beta_pow];
      auto leaf_alloc_end = std::chrono::high_resolution_clock::now();

      auto msg_setup_start = std::chrono::high_resolution_clock::now();
      // Set Leaf OT messages
      triple_gen->prg->random_bool((bool *)leaf_res_cmp, num_digits * num_cmps);
      triple_gen->prg->random_bool((bool *)leaf_res_eq, num_digits * num_cmps);
      for (int i = 0; i < num_digits; i++) {
        for (int j = 0; j < num_cmps; j++) {
          if (i == 0) {
            set_leaf_ot_messages(leaf_ot_messages[i * num_cmps + j],
                                digits[i * num_cmps + j], beta_pow,
                                leaf_res_cmp[i * num_cmps + j], 0,
                                greater_than, false);
          } else if (i == (num_digits - 1) && (r > 0)) {
    #ifdef WAN_EXEC
            set_leaf_ot_messages(leaf_ot_messages[i * num_cmps + j],
                                digits[i * num_cmps + j], beta_pow,
                                leaf_res_cmp[i * num_cmps + j],
                                leaf_res_eq[i * num_cmps + j], greater_than);
    #else
            set_leaf_ot_messages(leaf_ot_messages[i * num_cmps + j],
                                digits[i * num_cmps + j], 1 << r,
                                leaf_res_cmp[i * num_cmps + j],
                                leaf_res_eq[i * num_cmps + j], greater_than);
    #endif
          } else {
            set_leaf_ot_messages(leaf_ot_messages[i * num_cmps + j],
                                digits[i * num_cmps + j], beta_pow,
                                leaf_res_cmp[i * num_cmps + j],
                                leaf_res_eq[i * num_cmps + j], greater_than);
          }
        }
      }
      auto msg_setup_end = std::chrono::high_resolution_clock::now();

      auto leaf_ot_start = std::chrono::high_resolution_clock::now();
      // Perform Leaf OTs
    #ifdef WAN_EXEC
      // otpack->kkot_beta->send(leaf_ot_messages, num_cmps*(num_digits), 2);
      otpack->kkot[beta - 1]->send(leaf_ot_messages, num_cmps * (num_digits),
                                  2);
    #else
      // otpack->kkot_beta->send(leaf_ot_messages, num_cmps, 1);
      otpack->kkot[beta - 1]->send(leaf_ot_messages, num_cmps, 1);
      if (r == 1) {
        // otpack->kkot_beta->send(leaf_ot_messages+num_cmps,
        // num_cmps*(num_digits-2), 2);
        otpack->kkot[beta - 1]->send(leaf_ot_messages + num_cmps,
                                    num_cmps * (num_digits - 2), 2);
        otpack->iknp_straight->send(
            leaf_ot_messages + num_cmps * (num_digits - 1), num_cmps, 2);
      } else if (r != 0) {
        // otpack->kkot_beta->send(leaf_ot_messages+num_cmps,
        // num_cmps*(num_digits-2), 2);
        otpack->kkot[beta - 1]->send(leaf_ot_messages + num_cmps,
                                    num_cmps * (num_digits - 2), 2);
        otpack->kkot[r - 1]->send(
            leaf_ot_messages + num_cmps * (num_digits - 1), num_cmps, 2);
      } else {
        // otpack->kkot_beta->send(leaf_ot_messages+num_cmps,
        // num_cmps*(num_digits-1), 2);
        otpack->kkot[beta - 1]->send(leaf_ot_messages + num_cmps,
                                    num_cmps * (num_digits - 1), 2);
      }
    #endif
      auto leaf_ot_end = std::chrono::high_resolution_clock::now();

      auto cleanup_start = std::chrono::high_resolution_clock::now();
      // Cleanup
      for (int i = 0; i < num_digits * num_cmps; i++)
        delete[] leaf_ot_messages[i];
      delete[] leaf_ot_messages;
      auto cleanup_end = std::chrono::high_resolution_clock::now();
      
      auto alice_end = std::chrono::high_resolution_clock::now();
      
      //std::cout << "ALICE TIMING" << std::endl;
      std::cout << "ALICE - Leaf allocation: " << std::chrono::duration_cast<std::chrono::microseconds>(leaf_alloc_end - leaf_alloc_start).count() << " us" << std::endl;
      std::cout << "ALICE - Message setup: " << std::chrono::duration_cast<std::chrono::microseconds>(msg_setup_end - msg_setup_start).count() << " us" << std::endl;
      std::cout << "Alice - Leaf OT protocol: " << std::chrono::duration_cast<std::chrono::microseconds>(leaf_ot_end - leaf_ot_start).count() << " us" << std::endl;
      //std::cout << "  ALICE - Cleanup: " << std::chrono::duration_cast<std::chrono::microseconds>(cleanup_end - cleanup_start).count() << " us" << std::endl;
      std::cout << "Total Alice time (part 1): " << std::chrono::duration_cast<std::chrono::microseconds>(alice_end - alice_start).count() << " us" << std::endl;
      
    } else { // party = sci::BOB
      auto bob_start = std::chrono::high_resolution_clock::now();

      auto leaf_ot_start = std::chrono::high_resolution_clock::now();
      // Perform Leaf OTs
      #ifdef WAN_EXEC
      // otpack->kkot_beta->recv(leaf_res_cmp, digits, num_cmps*(num_digits),
      // 2);
      otpack->kkot[beta - 1]->recv(leaf_res_cmp, digits,
                                    num_cmps * (num_digits), 2);
      #else
      // otpack->kkot_beta->recv(leaf_res_cmp, digits, num_cmps, 1);
      otpack->kkot[beta - 1]->recv(leaf_res_cmp, digits, num_cmps, 1);
      if (r == 1) {
        // otpack->kkot_beta->recv(leaf_res_cmp+num_cmps, digits+num_cmps,
        // num_cmps*(num_digits-2), 2);
        otpack->kkot[beta - 1]->recv(leaf_res_cmp + num_cmps, digits + num_cmps,
                                      num_cmps * (num_digits - 2), 2);
        otpack->iknp_straight->recv(leaf_res_cmp + num_cmps * (num_digits - 1),
                                    digits + num_cmps * (num_digits - 1),
                                    num_cmps, 2);
      } else if (r != 0) {
        // otpack->kkot_beta->recv(leaf_res_cmp+num_cmps, digits+num_cmps,
        // num_cmps*(num_digits-2), 2);
        otpack->kkot[beta - 1]->recv(leaf_res_cmp + num_cmps, digits + num_cmps,
                                      num_cmps * (num_digits - 2), 2);
        otpack->kkot[r - 1]->recv(leaf_res_cmp + num_cmps * (num_digits - 1),
                                  digits + num_cmps * (num_digits - 1),
                                  num_cmps, 2);
      } else {
        // otpack->kkot_beta->recv(leaf_res_cmp+num_cmps, digits+num_cmps,
        // num_cmps*(num_digits-1), 2);
        otpack->kkot[beta - 1]->recv(leaf_res_cmp + num_cmps, digits + num_cmps,
                                      num_cmps * (num_digits - 1), 2);
      }
      #endif
      auto leaf_ot_end = std::chrono::high_resolution_clock::now();

      auto extract_start = std::chrono::high_resolution_clock::now();
      // Extract equality result from leaf_res_cmp
      for (int i = num_cmps; i < num_digits * num_cmps; i++) {
        leaf_res_eq[i] = leaf_res_cmp[i] & 1;
        leaf_res_cmp[i] >>= 1;
      }
      auto extract_end = std::chrono::high_resolution_clock::now();

      auto bob_end = std::chrono::high_resolution_clock::now();

      //std::cout << "BOB TIMING" << std::endl;
      std::cout << "Bob - Leaf OT protocol: " << std::chrono::duration_cast<std::chrono::microseconds>(leaf_ot_end - leaf_ot_start).count() << " us" << std::endl;
      //std::cout << "  BOB - Extract equality results: " << std::chrono::duration_cast<std::chrono::microseconds>(extract_end - extract_start).count() << " us" << std::endl;
      std::cout << "Total Bob time (part 1): " << std::chrono::duration_cast<std::chrono::microseconds>(bob_end - bob_start).count() << " us" << std::endl;
    }

    auto traverse_start = std::chrono::high_resolution_clock::now();
    traverse_and_compute_ANDs(num_cmps, leaf_res_eq, leaf_res_cmp);
    auto traverse_end = std::chrono::high_resolution_clock::now();

    auto results_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < old_num_cmps; i++)
      res[i] = leaf_res_cmp[i];
    auto results_end = std::chrono::high_resolution_clock::now();

    auto final_cleanup_start = std::chrono::high_resolution_clock::now();
    // Cleanup
    if (old_num_cmps != num_cmps)
    delete[] data_ext;
    delete[] digits;
    delete[] leaf_res_cmp;
    delete[] leaf_res_eq;
    auto final_cleanup_end = std::chrono::high_resolution_clock::now();

    auto total_end = std::chrono::high_resolution_clock::now();

    if (party == sci::ALICE) {
      //std::cout << "ALICE(2) - Data preparation: " << std::chrono::duration_cast<std::chrono::microseconds>(data_prep_end - data_prep_start).count() << " us" << std::endl;
      //std::cout << "ALICE(2) - Memory allocation: " << std::chrono::duration_cast<std::chrono::microseconds>(mem_alloc_end - mem_alloc_start).count() << " us" << std::endl;
      //std::cout << "ALICE(2) - Digit extraction: " << std::chrono::duration_cast<std::chrono::microseconds>(digit_extract_end - digit_extract_start).count() << " us" << std::endl;
      std::cout << "Alice - Traverse and compute ANDs: " << std::chrono::duration_cast<std::chrono::microseconds>(traverse_end - traverse_start).count() << " us" << std::endl;
      //std::cout << "ALICE(2) - Copy results: " << std::chrono::duration_cast<std::chrono::microseconds>(results_end - results_start).count() << " us" << std::endl;
      //std::cout << "ALICE(2) - Final cleanup: " << std::chrono::duration_cast<std::chrono::microseconds>(final_cleanup_end - final_cleanup_start).count() << " us" << std::endl;
      std::cout << "TOTAL ALICE TIME: " << std::chrono::duration_cast<std::chrono::microseconds>(total_end - total_start).count() << " us" << std::endl;
    }
    else {
      //std::cout << "BOB(2) - Data preparation: " << std::chrono::duration_cast<std::chrono::microseconds>(data_prep_end - data_prep_start).count() << " us" << std::endl;
      //std::cout << "BOB(2) - Memory allocation: " << std::chrono::duration_cast<std::chrono::microseconds>(mem_alloc_end - mem_alloc_start).count() << " us" << std::endl;
      //std::cout << "BOB(2) - Digit extraction: " << std::chrono::duration_cast<std::chrono::microseconds>(digit_extract_end - digit_extract_start).count() << " us" << std::endl;
      std::cout << "Bob - Traverse and compute ANDs: " << std::chrono::duration_cast<std::chrono::microseconds>(traverse_end - traverse_start).count() << " us" << std::endl;
      //std::cout << "BOB(2) - Copy results: " << std::chrono::duration_cast<std::chrono::microseconds>(results_end - results_start).count() << " us" << std::endl;
      //std::cout << "BOB(2) - Final cleanup: " << std::chrono::duration_cast<std::chrono::microseconds>(final_cleanup_end - final_cleanup_start).count() << " us" << std::endl;
      std::cout << "TOTAL BOB TIME: " << std::chrono::duration_cast<std::chrono::microseconds>(total_end - total_start).count() << " us" << std::endl;
    }
}

  void set_leaf_ot_messages(uint8_t *ot_messages, uint8_t digit, int N,
                            uint8_t mask_cmp, uint8_t mask_eq,
                            bool greater_than, bool eq = true) {
    for (int i = 0; i < N; i++) {
      if (greater_than) {
        ot_messages[i] = ((digit > i) ^ mask_cmp);
      } else {
        ot_messages[i] = ((digit < i) ^ mask_cmp);
      }
      if (eq) {
        ot_messages[i] = (ot_messages[i] << 1) | ((digit == i) ^ mask_eq);
      }
    }
  }

  /**************************************************************************************************
   *                         AND computation related functions
   **************************************************************************************************/

  void traverse_and_compute_ANDs(int num_cmps, uint8_t *leaf_res_eq,
                                 uint8_t *leaf_res_cmp) {
#ifdef WAN_EXEC
    Triple triples_std((num_triples)*num_cmps, true);
#else
    Triple triples_corr(num_triples_corr * num_cmps, true, num_cmps);
    Triple triples_std(num_triples_std * num_cmps, true);
#endif
    // Generate required Bit-Triples
    auto generate_bit_triples_start = std::chrono::high_resolution_clock::now();
#ifdef WAN_EXEC
    // std::cout<<"Running on WAN_EXEC; Skipping correlated triples"<<std::endl;
    triple_gen->generate(party, &triples_std, _16KKOT_to_4OT);
#else
    triple_gen->generate(party, &triples_corr, _8KKOT);
    triple_gen->generate(party, &triples_std, _16KKOT_to_4OT);
#endif
    // std::cout << "Bit Triples Generated" << std::endl;
    auto generate_bit_triples_end = std::chrono::high_resolution_clock::now();

    if (party == sci::ALICE) {
      std::cout << "Alice - Generate bit triples: " << std::chrono::duration_cast<std::chrono::microseconds>(generate_bit_triples_end - generate_bit_triples_start).count() << " us" << std::endl;
    }
    else {
      std::cout << "Bob - Generate bit triples: " << std::chrono::duration_cast<std::chrono::microseconds>(generate_bit_triples_end - generate_bit_triples_start).count() << " us" << std::endl;
    }

    // Combine leaf OT results in a bottom-up fashion
    auto combine_leaf_ot_start = std::chrono::high_resolution_clock::now();
    int counter_std = 0, old_counter_std = 0;
    int counter_corr = 0, old_counter_corr = 0;
    int counter_combined = 0, old_counter_combined = 0;
    uint8_t *ei = new uint8_t[(num_triples * num_cmps) / 8];
    uint8_t *fi = new uint8_t[(num_triples * num_cmps) / 8];
    uint8_t *e = new uint8_t[(num_triples * num_cmps) / 8];
    uint8_t *f = new uint8_t[(num_triples * num_cmps) / 8];
    auto combine_leaf_ot_end = std::chrono::high_resolution_clock::now();

    if (party == sci::ALICE) {
      //std::cout << "Alice - combine leaf OT: " << std::chrono::duration_cast<std::chrono::microseconds>(combine_leaf_ot_end - combine_leaf_ot_start).count() << " us" << std::endl;
    }
    else {
      //std::cout << "Bob - combine leaf OT:  " << std::chrono::duration_cast<std::chrono::microseconds>(combine_leaf_ot_end - combine_leaf_ot_start).count() << " us" << std::endl;
    }

    auto outer_loop_start = std::chrono::high_resolution_clock::now();
    for (int i = 1; i < num_digits; i *= 2) {
      auto inner_loop_1_start = std::chrono::high_resolution_clock::now();
      for (int j = 0; j < num_digits and j + i < num_digits; j += 2 * i) {
        auto AND_step_1_start = std::chrono::high_resolution_clock::now();
        if (j == 0) {
#ifdef WAN_EXEC
          AND_step_1(
              ei + (counter_std * num_cmps) / 8,
              fi + (counter_std * num_cmps) / 8, leaf_res_cmp + j * num_cmps,
              leaf_res_eq + (j + i) * num_cmps,
              (triples_std.ai) + (counter_combined * num_cmps) / 8,
              (triples_std.bi) + (counter_combined * num_cmps) / 8, num_cmps);
          counter_std++;
          counter_combined++;
#else
          AND_step_1(ei + (counter_std * num_cmps) / 8,
                     fi + (counter_std * num_cmps) / 8,
                     leaf_res_cmp + j * num_cmps,
                     leaf_res_eq + (j + i) * num_cmps,
                     (triples_std.ai) + (counter_std * num_cmps) / 8,
                     (triples_std.bi) + (counter_std * num_cmps) / 8, num_cmps);
          counter_std++;
#endif
        } else {
#ifdef WAN_EXEC
          AND_step_1(
              ei + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
              fi + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
              leaf_res_cmp + j * num_cmps, leaf_res_eq + (j + i) * num_cmps,
              (triples_std.ai) + (counter_combined * num_cmps) / 8,
              (triples_std.bi) + (counter_combined * num_cmps) / 8, num_cmps);
          counter_combined++;
          AND_step_1(
              ei + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              fi + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              leaf_res_eq + j * num_cmps, leaf_res_eq + (j + i) * num_cmps,
              (triples_std.ai) + (counter_combined * num_cmps) / 8,
              (triples_std.bi) + (counter_combined * num_cmps) / 8, num_cmps);
          counter_combined++;
          counter_corr++;
#else
          AND_step_1(
              ei + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
              fi + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
              leaf_res_cmp + j * num_cmps, leaf_res_eq + (j + i) * num_cmps,
              (triples_corr.ai) + (2 * counter_corr * num_cmps) / 8,
              (triples_corr.bi) + (2 * counter_corr * num_cmps) / 8, num_cmps);
          AND_step_1(
              ei + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              fi + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              leaf_res_eq + j * num_cmps, leaf_res_eq + (j + i) * num_cmps,
              (triples_corr.ai) + ((2 * counter_corr + 1) * num_cmps) / 8,
              (triples_corr.bi) + ((2 * counter_corr + 1) * num_cmps) / 8,
              num_cmps);
          counter_corr++;
#endif
        }
        auto AND_step_1_end = std::chrono::high_resolution_clock::now();

        if (party == sci::ALICE) {
          //std::cout << "Alice - AND step 1: " << std::chrono::duration_cast<std::chrono::microseconds>(AND_step_1_end - AND_step_1_start).count() << " us" << std::endl;
        }
        else {
          //std::cout << "Bob - AND step 1: " << std::chrono::duration_cast<std::chrono::microseconds>(AND_step_1_end - AND_step_1_start).count() << " us" << std::endl;
        }
      }
      auto inner_loop_1_end = std::chrono::high_resolution_clock::now();

      if (party == sci::ALICE) {
        //std::cout << "Alice - Inner loop step 1: " << std::chrono::duration_cast<std::chrono::microseconds>(inner_loop_1_end - inner_loop_1_start).count() << " us" << std::endl;
      }
      else {
        //std::cout << "Bob - Inner loop step 1: " << std::chrono::duration_cast<std::chrono::microseconds>(inner_loop_1_end - inner_loop_1_start).count() << " us" << std::endl;
      }

      auto calculate_offset_start = std::chrono::high_resolution_clock::now();
      int offset_std = (old_counter_std * num_cmps) / 8;
      int size_std = ((counter_std - old_counter_std) * num_cmps) / 8;
      int offset_corr =
          ((num_triples_std + 2 * old_counter_corr) * num_cmps) / 8;
      int size_corr = (2 * (counter_corr - old_counter_corr) * num_cmps) / 8;
      auto calculate_offset_end = std::chrono::high_resolution_clock::now();

      if (party == sci::ALICE) {
          //std::cout << "Alice - calculate offset: " << std::chrono::duration_cast<std::chrono::microseconds>(calculate_offset_end - calculate_offset_start).count() << " us" << std::endl;
      }
      else {
          //std::cout << "Bob - calculate offset: " << std::chrono::duration_cast<std::chrono::microseconds>(calculate_offset_end - calculate_offset_start).count() << " us" << std::endl;
      }

      auto omp_start = std::chrono::high_resolution_clock::now();
#pragma omp parallel num_threads(2)
      {
        if (omp_get_thread_num() == 1) {
          if (party == sci::ALICE) {
            iopack->io_rev->recv_data(e + offset_std, size_std);
            iopack->io_rev->recv_data(e + offset_corr, size_corr);
            iopack->io_rev->recv_data(f + offset_std, size_std);
            iopack->io_rev->recv_data(f + offset_corr, size_corr);
          } else { // party == sci::BOB
            iopack->io_rev->send_data(ei + offset_std, size_std);
            iopack->io_rev->send_data(ei + offset_corr, size_corr);
            iopack->io_rev->send_data(fi + offset_std, size_std);
            iopack->io_rev->send_data(fi + offset_corr, size_corr);
          }
        } else {
          if (party == sci::ALICE) {
            iopack->io->send_data(ei + offset_std, size_std);
            iopack->io->send_data(ei + offset_corr, size_corr);
            iopack->io->send_data(fi + offset_std, size_std);
            iopack->io->send_data(fi + offset_corr, size_corr);
          } else { // party == sci::BOB
            iopack->io->recv_data(e + offset_std, size_std);
            iopack->io->recv_data(e + offset_corr, size_corr);
            iopack->io->recv_data(f + offset_std, size_std);
            iopack->io->recv_data(f + offset_corr, size_corr);
          }
        }
      }
      auto omp_end = std::chrono::high_resolution_clock::now();

      if (party == sci::ALICE) {
          std::cout << "Alice - omp: " << std::chrono::duration_cast<std::chrono::microseconds>(omp_end - omp_start).count() << " us" << std::endl;
      }
      else {
          std::cout << "Bob - omp: " << std::chrono::duration_cast<std::chrono::microseconds>(omp_end - omp_start).count() << " us" << std::endl;
      }

      auto xor_start = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < size_std; i++) {
        e[i + offset_std] ^= ei[i + offset_std];
        f[i + offset_std] ^= fi[i + offset_std];
      }
      for (int i = 0; i < size_corr; i++) {
        e[i + offset_corr] ^= ei[i + offset_corr];
        f[i + offset_corr] ^= fi[i + offset_corr];
      }
      auto xor_end = std::chrono::high_resolution_clock::now();

      if (party == sci::ALICE) {
          //std::cout << "Alice - xor loops: " << std::chrono::duration_cast<std::chrono::microseconds>(xor_end - xor_start).count() << " us" << std::endl;
      }
      else {
          //std::cout << "Bob - xor loops: " << std::chrono::duration_cast<std::chrono::microseconds>(xor_end - xor_start).count() << " us" << std::endl;
      }

      counter_std = old_counter_std;
      counter_corr = old_counter_corr;
#ifdef WAN_EXEC
      counter_combined = old_counter_combined;
#endif

      auto inner_loop_2_start = std::chrono::high_resolution_clock::now();
      for (int j = 0; j < num_digits and j + i < num_digits; j += 2 * i) {
        auto AND_step_2_start = std::chrono::high_resolution_clock::now();
        if (j == 0) {
#ifdef WAN_EXEC
          AND_step_2(
              leaf_res_cmp + j * num_cmps, e + (counter_std * num_cmps) / 8,
              f + (counter_std * num_cmps) / 8,
              ei + (counter_std * num_cmps) / 8,
              fi + (counter_std * num_cmps) / 8,
              (triples_std.ai) + (counter_combined * num_cmps) / 8,
              (triples_std.bi) + (counter_combined * num_cmps) / 8,
              (triples_std.ci) + (counter_combined * num_cmps) / 8, num_cmps);
          counter_combined++;
#else
          AND_step_2(leaf_res_cmp + j * num_cmps,
                     e + (counter_std * num_cmps) / 8,
                     f + (counter_std * num_cmps) / 8,
                     ei + (counter_std * num_cmps) / 8,
                     fi + (counter_std * num_cmps) / 8,
                     (triples_std.ai) + (counter_std * num_cmps) / 8,
                     (triples_std.bi) + (counter_std * num_cmps) / 8,
                     (triples_std.ci) + (counter_std * num_cmps) / 8, num_cmps);
#endif
          for (int k = 0; k < num_cmps; k++)
            leaf_res_cmp[j * num_cmps + k] ^=
                leaf_res_cmp[(j + i) * num_cmps + k];
          counter_std++;
        } else {
#ifdef WAN_EXEC
          AND_step_2(leaf_res_cmp + j * num_cmps,
                     e + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
                     f + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
                     ei + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
                     fi + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
                     (triples_std.ai) + (counter_combined * num_cmps) / 8,
                     (triples_std.bi) + (counter_combined * num_cmps) / 8,
                     (triples_std.ci) + (counter_combined * num_cmps) / 8,
                     num_cmps);
          counter_combined++;
          AND_step_2(
              leaf_res_eq + j * num_cmps,
              e + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              f + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              ei + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              fi + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              (triples_std.ai) + (counter_combined * num_cmps) / 8,
              (triples_std.bi) + (counter_combined * num_cmps) / 8,
              (triples_std.ci) + (counter_combined * num_cmps) / 8, num_cmps);
          counter_combined++;
#else
          AND_step_2(leaf_res_cmp + j * num_cmps,
                     e + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
                     f + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
                     ei + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
                     fi + ((num_triples_std + 2 * counter_corr) * num_cmps) / 8,
                     (triples_corr.ai) + (2 * counter_corr * num_cmps) / 8,
                     (triples_corr.bi) + (2 * counter_corr * num_cmps) / 8,
                     (triples_corr.ci) + (2 * counter_corr * num_cmps) / 8,
                     num_cmps);
          AND_step_2(
              leaf_res_eq + j * num_cmps,
              e + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              f + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              ei + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              fi + ((num_triples_std + (2 * counter_corr + 1)) * num_cmps) / 8,
              (triples_corr.ai) + ((2 * counter_corr + 1) * num_cmps) / 8,
              (triples_corr.bi) + ((2 * counter_corr + 1) * num_cmps) / 8,
              (triples_corr.ci) + ((2 * counter_corr + 1) * num_cmps) / 8,
              num_cmps);
#endif
          for (int k = 0; k < num_cmps; k++)
            leaf_res_cmp[j * num_cmps + k] ^=
                leaf_res_cmp[(j + i) * num_cmps + k];
          counter_corr++;
        }
        auto AND_step_2_end = std::chrono::high_resolution_clock::now();

        if (party == sci::ALICE) {
          //std::cout << "Alice - AND step 2: " << std::chrono::duration_cast<std::chrono::microseconds>(AND_step_2_end - AND_step_2_start).count() << " us" << std::endl;
        }
        else {
          //std::cout << "Bob - AND step 2: " << std::chrono::duration_cast<std::chrono::microseconds>(AND_step_2_end - AND_step_2_start).count() << " us" << std::endl;
        }
      }
      auto inner_loop_2_end = std::chrono::high_resolution_clock::now();

      if (party == sci::ALICE) {
        //std::cout << "Alice - Inner loop step 2: " << std::chrono::duration_cast<std::chrono::microseconds>(inner_loop_2_end - inner_loop_2_start).count() << " us" << std::endl;
      }
      else {
        //std::cout << "Bob - Inner loop step 2: " << std::chrono::duration_cast<std::chrono::microseconds>(inner_loop_2_end - inner_loop_2_start).count() << " us" << std::endl;
      }

      old_counter_std = counter_std;
      old_counter_corr = counter_corr;
#ifdef WAN_EXEC
      old_counter_combined = counter_combined;
#endif
    }
    auto outer_loop_end = std::chrono::high_resolution_clock::now();

    if (party == sci::ALICE) {
      std::cout << "Alice - outer loop: " << std::chrono::duration_cast<std::chrono::microseconds>(outer_loop_end - outer_loop_start).count() << " us" << std::endl;
    }
    else {
      std::cout << "Bob - outer loop: " << std::chrono::duration_cast<std::chrono::microseconds>(outer_loop_end - outer_loop_start).count() << " us" << std::endl;
    }

#ifdef WAN_EXEC
    assert(counter_combined == num_triples);
#else
    assert(counter_std == num_triples_std);
    assert(2 * counter_corr == num_triples_corr);
#endif

    // cleanup
    delete[] ei;
    delete[] fi;
    delete[] e;
    delete[] f;
  }

  void AND_step_1(uint8_t *ei, // evaluates batch of 8 ANDs
                  uint8_t *fi, uint8_t *xi, uint8_t *yi, uint8_t *ai,
                  uint8_t *bi, int num_ANDs) {
    assert(num_ANDs % 8 == 0);
    for (int i = 0; i < num_ANDs; i += 8) {
      ei[i / 8] = ai[i / 8];
      fi[i / 8] = bi[i / 8];
      ei[i / 8] ^= sci::bool_to_uint8(xi + i, 8);
      fi[i / 8] ^= sci::bool_to_uint8(yi + i, 8);
    }
  }
  void AND_step_2(uint8_t *zi, // evaluates batch of 8 ANDs
                  uint8_t *e, uint8_t *f, uint8_t *ei, uint8_t *fi, uint8_t *ai,
                  uint8_t *bi, uint8_t *ci, int num_ANDs) {
    assert(num_ANDs % 8 == 0);
    for (int i = 0; i < num_ANDs; i += 8) {
      uint8_t temp_z;
      if (party == sci::ALICE)
        temp_z = e[i / 8] & f[i / 8];
      else
        temp_z = 0;
      temp_z ^= f[i / 8] & ai[i / 8];
      temp_z ^= e[i / 8] & bi[i / 8];
      temp_z ^= ci[i / 8];
      sci::uint8_to_bool(zi + i, temp_z, 8);
    }
  }
};

#endif // MILLIONAIRE_H__
