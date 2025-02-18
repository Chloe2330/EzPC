#include "Millionaire/millionaire.h"
#include "OT/emp-ot.h"
#include "utils/emp-tool.h"
#include <iostream>

int main(int argc, char **argv) {
    int port = 12345;
    int party = atoi(argv[1]);

    sci::IOPack *iopack = new sci::IOPack(party, port);
    sci::OTPack *otpack = new sci::OTPack(iopack, party);

    // Create instances of MillionaireProtocol for both parties
    MillionaireProtocol alice(sci::ALICE, iopack, otpack);
    MillionaireProtocol bob(sci::BOB, iopack, otpack);

    // Define the values to compare
    uint64_t alice_value = 5;
    uint64_t bob_value = 4;
    
    uint8_t alice_result = 0;
    uint8_t bob_result = 0;

    // Perform the comparison
    if (party == sci::ALICE) {
        alice.compare(&alice_result, &alice_value, 1, 4);
    } else {
        bob.compare(&bob_result, &bob_value, 1, 4);
    }

    // Synchronize to ensure both parties have completed the protocol
    iopack->io->flush();

    // Output the results
    if (party == sci::ALICE) {
        if (alice_result) {
            std::cout << "Alice's value (" << alice_value << ") is greater than Bob's value (" << bob_value << ")." << std::endl;
        } else {
            std::cout << "Alice's value (" << alice_value << ") is NOT greater than Bob's value (" << bob_value << ")." << std::endl;
        }
    } else {
        if (bob_result) {
            std::cout << "Bob's value (" << bob_value << ") is greater than Alice's value (" << alice_value << ")." << std::endl;
        } else {
            std::cout << "Bob's value (" << bob_value << ") is NOT greater than Alice's value (" << alice_value << ")." << std::endl;
        }
    }

    // Clean up
    delete iopack;
    delete otpack;

    return 0;
}
