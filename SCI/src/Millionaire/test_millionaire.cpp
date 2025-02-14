#include "Millionaire/millionaire.h"
#include "OT/emp-ot.h"
#include "utils/emp-tool.h"
#include <iostream>

int main(int argc, char **argv) {
    int port = 8000;
    int party = atoi(argv[1]);

    sci::IOPack *iopack = new sci::IOPack(party, port);
    sci::OTPack *otpack = new sci::OTPack(iopack, party);

    // Create instances of MillionaireProtocol for both parties
    MillionaireProtocol alice(1, iopack, otpack);
    MillionaireProtocol bob(2, iopack, otpack);

    // Define the values to compare
    uint64_t alice_value = 123456;
    uint64_t bob_value = 654321;
    uint8_t result;

    // Perform the comparison
    if (party == 1) {
        alice.compare(&result, &alice_value, 1, 64);
    } else {
        bob.compare(&result, &bob_value, 1, 64);
    }

    // Synchronize to ensure both parties have completed the protocol
    iopack->io->flush();

    // Output the result
    if (party == 1) {
        std::cout << "Alice's value: " << alice_value << std::endl;
        std::cout << "Bob's value: " << bob_value << std::endl;
        std::cout << "Comparison result (1 if Alice > Bob, 0 otherwise): " << (int)result << std::endl;
    }

    // Clean up
    delete iopack;
    delete otpack;

    return 0;
}
