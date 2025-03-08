#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <iostream>

#ifdef DEBUG
    #define DEBUG_PRINT(msg) std::cout << "DEBUG: " << msg << std::endl;
#else
    #define DEBUG_PRINT(msg)
#endif

class FFT {
public:
    using Complex = std::complex<float>;

    // Declaration of static member
    static std::vector<Complex> audioData;

    /*
        Respoinsible for performing Fast Fourier Transformation
        MUST be called after setup (Setup initally loads data)

        Start >> where the data has left off
        Inc >> The amount to increase / end
    
    */
   static std::vector<Complex> compute(int start, int inc) {
        printf("Beginning FFT\n");

        if (audioData.empty()) {
            DEBUG_PRINT("Is Empty!");
            return {}; // Early return for empty input
        }

        // Ensure the 'start' and 'inc' values are valid
        if (start < 0 || start >= audioData.size() || inc <= 0) {
            std::cerr << "Invalid start or increment value" << std::endl;
            return {};
        }

        int end = std::min(start + inc, static_cast<int>(audioData.size()));
        int n = end - start; // Length of the data to process

        // Slice the audioData to process only the segment from start to start+inc
        std::vector<Complex> segment(audioData.begin() + start, audioData.begin() + end);

        // Bit-reversal reordering on the segment
        bitReverseReorder(segment);

        // Iterative Cooley-Tukey FFT on the segment
        int m = 1;
        while (m < n) {
            Complex wm = std::polar(1.0f, static_cast<float>(-2.0f * 3.1415f / m)); // Twiddle factor
            printf("m = %d, wm = (%f, %f)\n", m, wm.real(), wm.imag());

            // FFT computation for this segment
            for (int k = 0; k < n; k += m) {
                Complex w = 1;
                for (int j = 0; j < m / 2; j++) {
                    // Skip some calculations here, e.g., for j > some threshold
                    if (j < m / 4) {
                        Complex t = w * segment[k + j + m / 2];
                        Complex u = segment[k + j];
                        segment[k + j] = u + t;
                        segment[k + j + m / 2] = u - t;
                        w *= wm;
                    }
                }
            }

            m *= 2;  // Double m at each stage (equivalent to 2^s)
        }

        printf("Complete computation\n");
        return segment;  // Return only the FFT result for this segment
    }


    // Set up for FFT, converting real input to complex and reordering
    static void setup(std::vector<float>& input) {
        int n = input.size();
        if (n <= 1) {
            printf("Input size too small for FFT, returning early.\n");
            return;
        }
        
        audioData = realToComplex(input);  // Convert to complex
        DEBUG_PRINT("Converted to Complex");
        bitReverseReorder(audioData);  // Bit-reversal reorder
        DEBUG_PRINT("bitReverseReorder Complete");
    }

private:
    // Convert real input to complex (imaginary part = 0)
    static std::vector<Complex> realToComplex(const std::vector<float>& input) {
        std::vector<Complex> complexInput(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            complexInput[i] = Complex(input[i], 0.0f);
        }
        return complexInput;
    }

    // Bit-reversal reorder (for Cooley-Tukey FFT)
    static void bitReverseReorder(std::vector<Complex>& data) {
        int n = data.size();
        int bits = 0;
        
        // Find the number of bits needed (for power of 2, would be log2(n))
        while ((1 << bits) < n) bits++;
    
        int j = 0;
        for (int i = 0; i < n; i++) {
            if (i < j) std::swap(data[i], data[j]);
    
            int mask = n >> 1; 
            while (j & mask) { // Find next bit-reversed index
                j &= ~mask; 
                mask >>= 1;
            }
            j |= mask; 
        }
    }
    
    

    // Return the next power of two for padding
    static int nextPowerOfTwo(int n) {
        if (n == 0) return 1;
        return 1 << (int)std::ceil(std::log2(n));
    }

    // Pad input to the next power of two (for FFT efficiency)
    static std::vector<float> padToPowerOfTwo(const std::vector<float>& input) {
        int n = input.size();
        int nextPow2 = nextPowerOfTwo(n);

        std::vector<float> paddedInput(input);
        paddedInput.resize(nextPow2, 0.0f);

        std::cout << "Padding data to size: " << nextPow2 << std::endl;
        return paddedInput;
    }
};

// Define the static member outside the class
std::vector<FFT::Complex> FFT::audioData;  // Define audioData outside the class
