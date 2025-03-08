#include "Visualizer.h"


//For Debug (tis the name)
#ifdef DEBUG
    #define DEBUG_PRINT(msg) std::cout << "DEBUG: " << msg << std::endl;
#else
    #define DEBUG_PRINT(msg)
#endif


//Its a stylistic thing more than anything practical
const int BAR_HEIGHT_MIN = 5;


    //Constructor
    Visualizer::Visualizer(int bins, int width, int height, int sampleRate, int fftSize)
    : numBins(bins), screenWidth(width), screenHeight(height), sampleRate(sampleRate), fftSize(fftSize)
    {

        this->screenHeight = static_cast<float>(height);
        DEBUG_PRINT("height " << screenHeight);
        BAR_HEIGHT_MAX = screenHeight * 0.9f; //Make it reach till the top 10%
        DEBUG_PRINT("BAR_HEIGHT_MAX " << BAR_HEIGHT_MAX);

        // Calculate bin width in Hz (useful for mapping frequencies)
        binWidth = static_cast<float>(sampleRate) / fftSize;

        DEBUG_PRINT("Creating Scene");
        //Creates Scene (simplifys work)
        if (!createScene()) {
            throw std::runtime_error("Failed to Create Scene (Missing or Invalid Data).");
        } 
    }


    /*
        Allocates proper size to bins and creates soundbars
        Returns: true if created, false otherwise
    */
    bool Visualizer::createScene()
    {   
        // Return error if necessary values are undefined or invalid
        if (sampleRate <= 0 || fftSize <= 0 || screenWidth <= 0 || screenHeight <= 0 || numBins <= 0)
            return false;
    

        DEBUG_PRINT("Creating Log Bins / Soundbars");
        generateLogBins();
        return true; // Success
    }

    void Visualizer::generateLogBins()
    {
        float fMin = 20.0f;  // Minimum frequency we care about
        float fMax = sampleRate / 2.0f;  // Nyquist limit (limit calcuable)
        
        //Clean start
        logBins.clear();
        soundBars.clear();
        float binPixels = (screenWidth / numBins);

        //I like a little bit of seperation
        float binSeparation = binPixels * 0.1f;  // 10% spacing
        binPixels *= 0.9f;  // Adjust bin width accordingly
    
        DEBUG_PRINT("Entering Logbin Loop");
        for (int i = 0; i < numBins; i++)
        {
            //Associated frequencies for each box
            float binFreq = fMin * pow(fMax / fMin, static_cast<float>(i) / (numBins - 1));
            binFreq = std::min(binFreq, sampleRate / 2.0f); //I think?
            logBins.push_back(binFreq);
    
            // Create the regions for each bar (evenly spaced)
            SDL_FRect bar = { i * (binPixels + binSeparation) + binSeparation / 2, 0, binPixels, BAR_HEIGHT_MIN };
            soundBars.push_back(bar);
        }


        DEBUG_PRINT("Successfully created sound bars and logbins");
        DEBUG_PRINT("Size of SoundBars: " << soundBars.size());
        DEBUG_PRINT("Size of logBins: " << logBins.size());
        
    }
    



    /*
        This uses the bin index to sum the amplitudes of the frequencies that belong to that bin.
        Args: Bin index we want to process.
        Returns: Amplitude of the bin.
    */
    float Visualizer::sumAmplitude(int binIndex) {
        DEBUG_PRINT("Performing Sum Amplitude");
        // Error checking: Ensure binIndex is valid
        if (binIndex < 0 || binIndex >= numBins || freqData.empty()) {
            return 0.0f;  // Return zero if binIndex is out of range or FFT data is unavailable.
        }

        // Get the frequency range for this bin using logBins
        float freqStart = logBins[binIndex];
        float freqEnd = (binIndex < numBins - 1) ? logBins[binIndex + 1] : sampleRate / 2.0f;

        float amplitude = 0.0f;

        // Convert frequency range to FFT index range
        int startIdx = static_cast<int>(freqStart / binWidth);
        int endIdx = static_cast<int>(freqEnd / binWidth);

        // Ensure index bounds
        startIdx = std::max(0, std::min(startIdx, fftSize / 2 - 1));
        endIdx = std::max(0, std::min(endIdx, fftSize / 2 - 1));
        DEBUG_PRINT("Index Range " << startIdx << " " << endIdx );

        // Sum the magnitudes of FFT bins within the range
        for (int i = startIdx; i <= endIdx; i++) {
            DEBUG_PRINT("Real: " << freqData[i].real() << " Imaginary: " << freqData[i].imag());
            amplitude += std::abs(freqData[i]); // Take magnitude from complex number
        }

        DEBUG_PRINT("Amplitude: " << amplitude);
        return amplitude;
    }


    /*
        Responsible for updating the visualizer. Pulls updated frequency from 'insertFreq()'
        Returns: 0 on success, else error 
    */
    int Visualizer::update(const std::vector<Complex>& input)
    {
        

        DEBUG_PRINT("Updating Visualizer");
        float maxAmplitude = 1.0f; // Prevent division by zero, can be dynamically updated
        std::vector<float> amplitudes(numBins, 0.0f); // Store computed amplitudes

        DEBUG_PRINT("INPUT ------ Real: " << input[0].real() << " Imaginary: " << input[0].imag());
    
        freqData = input;

        // First pass: Compute amplitudes and track max
       for (int i = 0; i < numBins; i++)
        {
            amplitudes[i] = sumAmplitude(i);
            DEBUG_PRINT("Amplitude at " << i << " ---- " << amplitudes[i]);
            if (amplitudes[i] > maxAmplitude)
            {
                maxAmplitude = amplitudes[i];
            }
        }

        DEBUG_PRINT("BAR HEITGHT MAX: " << BAR_HEIGHT_MAX);
        
        // Second pass: Normalize and update bars
        for (int i = 0; i < numBins; i++)
        {
            float normalizedAmplitude = (amplitudes[i] / (maxAmplitude + 0.1f));  // Add small constant to prevent division by zero

            // You may want to remove the DEBUG_PRINT here for performance reasons in production
            DEBUG_PRINT("Current Amplitude: " << normalizedAmplitude)

            soundBars[i].h = BAR_HEIGHT_MIN + (normalizedAmplitude) * BAR_HEIGHT_MAX;
            DEBUG_PRINT("HEIGHT FOR " << i << " = " << soundBars[i].h);
        }

        


        return 0;
    }

    //Render the updated graphics
    void Visualizer::render(SDL_Renderer* renderer)
    {
        // 🎨 Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        // 🎶 Draw each sound bar
        SDL_SetRenderDrawColor(renderer, 155, 30, 200, 255); // ourple
        for (const auto& bar : soundBars) {
            SDL_RenderFillRect(renderer, &bar); // Draw bar
        }

        // 📤 Present the final frame
        SDL_RenderPresent(renderer);
    }

    //Destructor
    Visualizer::~Visualizer() {
        //doesnt do anything
    }
    

   

