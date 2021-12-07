#include "CipherFactory.hpp"
#include "CipherMode.hpp"
#include "CipherType.hpp"
#include "ProcessCommandLine.hpp"
#include "TransformChar.hpp"
#include "Exceptions.hpp"

#include <exception>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <thread>

int main(int argc, char* argv[])
{
    // Convert the command-line arguments into a more easily usable form
    const std::vector<std::string> cmdLineArgs{argv, argv + argc};

    // Options that might be set by the command-line arguments
    ProgramSettings settings{
        false, false, "", "", "", CipherMode::Encrypt, CipherType::Caesar};

    // Process command line arguments
    // Any failure in the argument processing means we can't continue
    // Use a non-zero return value to indicate failure
    try {
        processCommandLine(cmdLineArgs, settings);
    }
    catch (const MissingArgument& e ) {
        std::cerr << "[error] Missing argument: " << e.what() << "\n";
        return 1;
    }
    catch (const UnknownArgument& e ) {
        std::cerr << "[error] " << e.what() << "\n";
        return 1;
    }

    // Handle help, if requested
    if (settings.helpRequested) {
        // Line splitting for readability
        std::cout
            << "Usage: mpags-cipher [-h/--help] [--version] [-i <file>] [-o <file>] [-c <cipher>] [-k <key>] [--encrypt/--decrypt]\n\n"
            << "Encrypts/Decrypts input alphanumeric text using classical ciphers\n\n"
            << "Available options:\n\n"
            << "  -h|--help        Print this help message and exit\n\n"
            << "  --version        Print version information\n\n"
            << "  -i FILE          Read text to be processed from FILE\n"
            << "                   Stdin will be used if not supplied\n\n"
            << "  -o FILE          Write processed text to FILE\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "  -c CIPHER        Specify the cipher to be used to perform the encryption/decryption\n"
            << "                   CIPHER can be caesar, playfair, or vigenere - caesar is the default\n\n"
            << "  -k KEY           Specify the cipher KEY\n"
            << "                   A null key, i.e. no encryption, is used if not supplied\n\n"
            << "  --encrypt        Will use the cipher to encrypt the input text (default behaviour)\n\n"
            << "  --decrypt        Will use the cipher to decrypt the input text\n\n"
            << std::endl;
        // Help requires no further action, so return from main
        // with 0 used to indicate success
        return 0;
    }

    // Handle version, if requested
    // Like help, requires no further action,
    // so return from main with zero to indicate success
    if (settings.versionRequested) {
        std::cout << "0.5.0" << std::endl;
        return 0;
    }

    // Initialise variables
    char inputChar{'x'};
    std::string inputText;

    // Read in user input from stdin/file
    if (!settings.inputFile.empty()) {
        // Open the file and check that we can read from it
        std::ifstream inputStream{settings.inputFile};
        if (!inputStream.good()) {
            std::cerr << "[error] failed to create istream on file '"
                      << settings.inputFile << "'" << std::endl;
            return 1;
        }

        // Loop over each character from the file
        while (inputStream >> inputChar) {
            inputText += transformChar(inputChar);
        }

    } else {
        // Loop over each character from user input
        // (until Return then CTRL-D (EOF) pressed)
        while (std::cin >> inputChar) {
            inputText += transformChar(inputChar);
        }
    }

    // Request construction of the appropriate cipher
    std::string outputText;
    try {

        // set nThreads = 10
        size_t nThreads = 10;
        size_t subStrLength = inputText.size()/nThreads + 1;

        auto cipher = cipherFactory(settings.cipherType, settings.cipherKey);
        // Check that the cipher was constructed successfully
        if (!cipher) {
            std::cerr << "[error] problem constructing requested cipher"
                      << std::endl;
            return 1;
        }

        // lambda to apply cipher
        auto applyTheCipher = [&] (const std::string inText, const CipherMode mode) {
            return cipher->applyCipher(inText, mode);
        };

        std::vector<std::future< std::string>> futureVec = {};

        std::string subStrInputText;
        for (size_t t = 0; t < nThreads - 1; t++) {
            subStrInputText = inputText.substr(t*subStrLength,
                                                 subStrLength);

            futureVec.push_back( std::async(std::launch::async, applyTheCipher,
                                          subStrInputText, settings.cipherMode));
        }

        // wait for each thread to finish
        std::future_status status{std::future_status::ready};
        for (std::future<std::string>& fut : futureVec) {
            do {
                status = fut.wait_for(std::chrono::seconds(1));
                if (status == std::future_status::timeout) {
                    std::cout << "[main] waiting...\n";
                }
            } while (status != std::future_status::ready);
        }

        // concatenate results
        for (std::future<std::string>& fut : futureVec) {
            outputText += fut.get();
        }
    }
    catch (const InvalidKey& e) {
        std::cerr << "[error] problem encountered with given key:\n" 
                  << e.what();
        return 1;
    }
    
    // Output the encrypted/decrypted text to stdout/file
    if (!settings.outputFile.empty()) {
        // Open the file and check that we can write to it
        std::ofstream outputStream{settings.outputFile};
        if (!outputStream.good()) {
            std::cerr << "[error] failed to create ostream on file '"
                      << settings.outputFile << "'" << std::endl;
            return 1;
        }

        // Print the encrypted/decrypted text to the file
        outputStream << outputText << std::endl;

    } else {
        // Print the encrypted/decrypted text to the screen
        std::cout << outputText << std::endl;
    }

    // No requirement to return from main, but we do so for clarity
    // and for consistency with other functions
    return 0;
}
