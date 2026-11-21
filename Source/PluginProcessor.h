#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Parameters.h"

class DiabloAudioProcessor : public juce::AudioProcessor
{
public:
    DiabloAudioProcessor();
    ~DiabloAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    struct OnePole
    {
        float z = 0.0f, a = 0.0f;
        float lp (float x) noexcept { z += a * (x - z); return z; }
        void reset() noexcept { z = 0.0f; }
    };

    // Cambio de tono con dos lecturas cruzadas (ventana Hann). ratio < 1 = más grave.
    struct PitchShifter
    {
        std::vector<float> buf;
        int size = 0, w = 0, windowLen = 1;
        float phase = 0.0f;

        void prepare (double sr)
        {
            windowLen = juce::jmax (64, (int) (sr * 0.06));
            size = windowLen * 2 + 8;
            buf.assign ((size_t) size, 0.0f);
            w = 0;
            phase = 0.0f;
        }

        float readAt (float delay) const noexcept
        {
            float rp = (float) w - delay;
            if (rp < 0.0f) rp += (float) size;
            int i0 = (int) rp;
            if (i0 >= size) i0 -= size;
            int i1 = i0 + 1;
            if (i1 >= size) i1 -= size;
            const float f = rp - (float) i0;
            return buf[(size_t) i0] + f * (buf[(size_t) i1] - buf[(size_t) i0]);
        }

        float process (float x, float ratio) noexcept
        {
            buf[(size_t) w] = x;

            phase += (1.0f - ratio) / (float) windowLen;
            phase -= std::floor (phase);
            float phase2 = phase + 0.5f;
            phase2 -= std::floor (phase2);

            const float g1 = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * phase);
            const float g2 = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * phase2);
            const float y  = readAt (phase * (float) windowLen) * g1
                           + readAt (phase2 * (float) windowLen) * g2;

            if (++w >= size) w = 0;
            return y;
        }
    };

    static float coefFor (double hz, double sr) noexcept
    {
        return (float) (1.0 - std::exp (-juce::MathConstants<double>::twoPi * hz / sr));
    }

    double sr = 44100.0;

    std::atomic<float>* pInput = nullptr;     std::atomic<float>* pOutput = nullptr;
    std::atomic<float>* pPitch = nullptr;     std::atomic<float>* pPitchMix = nullptr;
    std::atomic<float>* pDevil = nullptr;     std::atomic<float>* pStutter = nullptr;
    std::atomic<float>* pStSize = nullptr;    std::atomic<float>* pChop = nullptr;
    std::atomic<float>* pRobot = nullptr;     std::atomic<float>* pRobotHz = nullptr;
    std::atomic<float>* pCrush = nullptr;     std::atomic<float>* pEchoTime = nullptr;
    std::atomic<float>* pEchoMix = nullptr;   std::atomic<float>* pRevMix = nullptr;

    juce::SmoothedValue<float> smInput, smOutput, smPitch, smPitchMix, smCrush,
                               smEchoMs, smEchoMix, smRobot;

    PitchShifter pitchMain[2], pitchDevil[2];
    OnePole devilLp[2], echoLp[2];

    // Crush
    float held[2] { 0.0f, 0.0f };
    int   holdCount[2] { 0, 0 };

    // Stutter (tartamudeo): graba, congela un pedacito y lo repite
    std::vector<float> stBuf[2];
    int   stSize = 1, stWrite = 0, stCounter = 0, stSliceLen = 64, stSliceStart = 0,
          stRepeats = 0, stNextCheck = 1;
    bool  stuttering = false;
    float stEnv = 0.0f;

    // Chop (cortes al azar)
    int   chopCount = 1;
    float chopTarget = 1.0f, chopEnv = 1.0f;

    // Robot (modulación en anillo)
    double robotPhase = 0.0;

    // Eco
    juce::AudioBuffer<float> delayBuf;
    int delayWrite = 0;

    juce::Reverb reverb;
    juce::Random rng { 1234 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiabloAudioProcessor)
};
