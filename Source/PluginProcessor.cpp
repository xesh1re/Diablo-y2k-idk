#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
namespace
{
    using Attr = juce::AudioParameterFloatAttributes;

    Attr pctAttr()
    {
        return Attr()
            .withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; })
            .withValueFromStringFunction ([] (const juce::String& t) { return t.getFloatValue() * 0.01f; });
    }
    Attr dbAttr()
    {
        return Attr()
            .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " dB"; })
            .withValueFromStringFunction ([] (const juce::String& t) { return t.getFloatValue(); });
    }
    Attr msAttr()
    {
        return Attr()
            .withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v)) + " ms"; })
            .withValueFromStringFunction ([] (const juce::String& t) { return t.getFloatValue(); });
    }
    Attr stAttr()
    {
        return Attr()
            .withStringFromValueFunction ([] (float v, int) { return (v > 0.0f ? "+" : "") + juce::String (v, 1) + " st"; })
            .withValueFromStringFunction ([] (const juce::String& t) { return t.getFloatValue(); });
    }
    Attr hzAttr()
    {
        return Attr()
            .withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v)) + " Hz"; })
            .withValueFromStringFunction ([] (const juce::String& t) { return t.getFloatValue(); });
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout DiabloAudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto add = [&layout] (const char* id, const String& name, NormalisableRange<float> range, float def, Attr attr)
    {
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, range, def, attr));
    };

    add (ParamIDs::input,       es ("Entrada"),     { -24.0f, 12.0f, 0.1f }, 0.0f, dbAttr());

    add (ParamIDs::pitch,       es ("Tono"),        { -12.0f, 12.0f, 0.1f }, 0.0f, stAttr());
    add (ParamIDs::pitchMix,    es ("Mezcla Tono"), { 0.0f, 1.0f, 0.001f },  0.0f, pctAttr());
    add (ParamIDs::devil,       es ("Diablo"),      { 0.0f, 1.0f, 0.001f },  0.35f, pctAttr());

    add (ParamIDs::stutter,     es ("Stutter"),     { 0.0f, 1.0f, 0.001f },  0.25f, pctAttr());
    add (ParamIDs::stutterSize, es ("Tamaño Stutter"), NormalisableRange<float> (30.0f, 400.0f, 1.0f, 0.6f), 120.0f, msAttr());
    add (ParamIDs::chop,        es ("Chop"),        { 0.0f, 1.0f, 0.001f },  0.0f, pctAttr());

    add (ParamIDs::robot,       es ("Robot"),       { 0.0f, 1.0f, 0.001f },  0.0f, pctAttr());
    add (ParamIDs::robotFreq,   es ("Frec. Robot"), NormalisableRange<float> (40.0f, 2000.0f, 1.0f, 0.4f), 300.0f, hzAttr());
    add (ParamIDs::crush,       es ("Crush"),       { 0.0f, 1.0f, 0.001f },  0.0f, pctAttr());

    add (ParamIDs::echoTime,    es ("Eco Tiempo"),  NormalisableRange<float> (60.0f, 1000.0f, 1.0f, 0.6f), 280.0f, msAttr());
    add (ParamIDs::echoMix,     es ("Eco Mezcla"),  { 0.0f, 1.0f, 0.001f },  0.20f, pctAttr());
    add (ParamIDs::reverbMix,   es ("Reverb"),      { 0.0f, 1.0f, 0.001f },  0.25f, pctAttr());

    add (ParamIDs::output,      es ("Salida"),      { -24.0f, 12.0f, 0.1f }, 0.0f, dbAttr());

    return layout;
}

//==============================================================================
DiabloAudioProcessor::DiabloAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    pInput    = apvts.getRawParameterValue (ParamIDs::input);
    pOutput   = apvts.getRawParameterValue (ParamIDs::output);
    pPitch    = apvts.getRawParameterValue (ParamIDs::pitch);
    pPitchMix = apvts.getRawParameterValue (ParamIDs::pitchMix);
    pDevil    = apvts.getRawParameterValue (ParamIDs::devil);
    pStutter  = apvts.getRawParameterValue (ParamIDs::stutter);
    pStSize   = apvts.getRawParameterValue (ParamIDs::stutterSize);
    pChop     = apvts.getRawParameterValue (ParamIDs::chop);
    pRobot    = apvts.getRawParameterValue (ParamIDs::robot);
    pRobotHz  = apvts.getRawParameterValue (ParamIDs::robotFreq);
    pCrush    = apvts.getRawParameterValue (ParamIDs::crush);
    pEchoTime = apvts.getRawParameterValue (ParamIDs::echoTime);
    pEchoMix  = apvts.getRawParameterValue (ParamIDs::echoMix);
    pRevMix   = apvts.getRawParameterValue (ParamIDs::reverbMix);
}

bool DiabloAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

//==============================================================================
void DiabloAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;

    auto init = [this] (juce::SmoothedValue<float>& s, float v, double secs = 0.03)
    {
        s.reset (sr, secs);
        s.setCurrentAndTargetValue (v);
    };

    init (smInput,   juce::Decibels::decibelsToGain (pInput->load()));
    init (smOutput,  juce::Decibels::decibelsToGain (pOutput->load()));
    init (smPitch,   pPitch->load(), 0.06);
    init (smPitchMix, pPitchMix->load());
    init (smCrush,   pCrush->load());
    init (smEchoMs,  pEchoTime->load());
    init (smEchoMix, pEchoMix->load());
    init (smRobot,   pRobot->load());

    delayBuf.setSize (2, (int) (sr * 1.2) + 8);
    delayBuf.clear();
    delayWrite = 0;

    stSize = (int) (sr * 0.8);
    for (auto& b : stBuf)
        b.assign ((size_t) stSize, 0.0f);
    stWrite = 0; stCounter = 0; stRepeats = 0; stNextCheck = 1;
    stuttering = false; stEnv = 0.0f;
    chopCount = 1; chopTarget = chopEnv = 1.0f;
    robotPhase = 0.0;

    for (int ch = 0; ch < 2; ++ch)
    {
        pitchMain[ch].prepare (sr);
        pitchDevil[ch].prepare (sr);
        devilLp[ch].reset(); echoLp[ch].reset();
        devilLp[ch].a = coefFor (3500.0, sr);   // voz demoníaca más oscura
        echoLp[ch].a  = coefFor (4500.0, sr);   // ecos que se apagan
        held[ch] = 0.0f;
        holdCount[ch] = 0;
    }

    reverb.setSampleRate (sampleRate);
    reverb.reset();
}

//==============================================================================
void DiabloAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const int n     = buffer.getNumSamples();

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, n);

    if (numCh == 0 || n == 0)
        return;

    float* data[2] { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };

    smInput.setTargetValue   (juce::Decibels::decibelsToGain (pInput->load()));
    smOutput.setTargetValue  (juce::Decibels::decibelsToGain (pOutput->load()));
    smPitch.setTargetValue   (pPitch->load());
    smPitchMix.setTargetValue (pPitchMix->load());
    smCrush.setTargetValue   (pCrush->load());
    smEchoMs.setTargetValue  (pEchoTime->load());
    smEchoMix.setTargetValue (pEchoMix->load());
    smRobot.setTargetValue   (pRobot->load());

    const float devil    = pDevil->load();
    const float stAmt    = pStutter->load();
    const float chopAmt  = pChop->load();
    const double robotHz = pRobotHz->load();
    const float echoFb   = 0.4f;

    const int sliceLen = juce::jlimit (64, stSize - 2, (int) (pStSize->load() * 0.001 * sr));
    const float envCoef  = 1.0f - std::exp (-1.0f / (0.004f * (float) sr));
    const float chopCoef = 1.0f - std::exp (-1.0f / (0.003f * (float) sr));
    const int   fadeLen  = juce::jmax (8, juce::jmin (128, sliceLen / 4));
    const int   echoSize = delayBuf.getNumSamples();

    for (int i = 0; i < n; ++i)
    {
        const float inG    = smInput.getNextValue();
        const float st     = smPitch.getNextValue();
        const float pMix   = smPitchMix.getNextValue();
        const float cr     = smCrush.getNextValue();
        const float eMs    = smEchoMs.getNextValue();
        const float eMix   = smEchoMix.getNextValue();
        const float robot  = smRobot.getNextValue();

        const float ratio  = std::exp2 (st / 12.0f);
        const float levels = std::exp2 (15.0f - cr * 12.0f);
        const int   hold   = 1 + (int) (cr * cr * 11.0f);

        // ---- control compartido entre canales
        // Chop: cada ~60 ms decide al azar si corta la voz
        if (--chopCount <= 0)
        {
            chopCount  = (int) (0.06 * sr * (0.5 + rng.nextFloat()));
            chopTarget = (rng.nextFloat() < chopAmt * 0.6f) ? 0.0f : 1.0f;
        }
        chopEnv += (chopTarget - chopEnv) * chopCoef;

        // Stutter: al final de cada pedacito decide si se traba
        if (! stuttering && --stNextCheck <= 0)
        {
            stNextCheck = sliceLen;
            if (stAmt > 0.001f && rng.nextFloat() < stAmt * 0.7f)
            {
                stuttering   = true;
                stSliceLen   = sliceLen;
                stSliceStart = (stWrite - stSliceLen + stSize) % stSize;
                stCounter    = 0;
                stRepeats    = 2 + rng.nextInt (5);
            }
        }
        stEnv += ((stuttering ? 1.0f : 0.0f) - stEnv) * envCoef;

        // Robot: portadora senoidal
        const float rmod = (float) std::sin (robotPhase);
        robotPhase += juce::MathConstants<double>::twoPi * robotHz / sr;
        if (robotPhase > juce::MathConstants<double>::twoPi)
            robotPhase -= juce::MathConstants<double>::twoPi;

        const float delaySamples = juce::jlimit (1.0f, (float) (echoSize - 2), eMs * 0.001f * (float) sr);

        for (int ch = 0; ch < numCh; ++ch)
        {
            float x = data[ch][i] * inG;

            // Cambio de tono
            const float shifted = pitchMain[ch].process (x, ratio);
            x += pMix * (shifted - x);

            // Diablo: una octava abajo, sucia y oscura
            const float low = pitchDevil[ch].process (x, 0.5f);
            if (devil > 0.001f)
            {
                const float d = devilLp[ch].lp (std::tanh (low * (1.0f + 4.0f * devil)));
                x = x * (1.0f - 0.35f * devil) + devil * 1.2f * d;
            }

            // Robot
            if (robot > 0.001f)
                x += robot * (x * rmod - x);

            // Chop
            x *= chopEnv;

            // Stutter
            {
                auto& sb = stBuf[ch];
                float slice = x;
                if (stuttering)
                {
                    const int idx = (stSliceStart + stCounter) % stSize;
                    const int distEdge = juce::jmin (stCounter, stSliceLen - 1 - stCounter);
                    const float fade = juce::jmin (1.0f, (float) distEdge / (float) fadeLen);
                    slice = sb[(size_t) idx] * fade + x * (1.0f - fade);
                }
                else
                {
                    sb[(size_t) stWrite] = x;
                }
                x += stEnv * (slice - x);
            }

            // Crush
            if (cr > 0.002f)
            {
                if (--holdCount[ch] <= 0)
                {
                    held[ch] = std::round (x * levels) / levels;
                    holdCount[ch] = hold;
                }
                x = held[ch];
            }
            else
            {
                holdCount[ch] = 0;
            }

            // Eco
            {
                float* buf = delayBuf.getWritePointer (ch);
                float rp = (float) delayWrite - delaySamples;
                if (rp < 0.0f) rp += (float) echoSize;
                int i0 = (int) rp;
                if (i0 >= echoSize) i0 -= echoSize;
                int i1 = i0 + 1;
                if (i1 >= echoSize) i1 -= echoSize;
                const float frac = rp - (float) i0;
                const float delayed = buf[i0] + frac * (buf[i1] - buf[i0]);

                buf[delayWrite] = x + echoLp[ch].lp (delayed) * echoFb;
                x += delayed * eMix;
            }

            data[ch][i] = x;
        }

        // avanzar contadores (una vez por muestra, no por canal)
        if (stuttering)
        {
            if (++stCounter >= stSliceLen)
            {
                stCounter = 0;
                if (--stRepeats <= 0)
                {
                    stuttering  = false;
                    stNextCheck = 1;
                }
            }
        }
        else if (++stWrite >= stSize)
        {
            stWrite = 0;
        }

        if (++delayWrite >= echoSize)
            delayWrite = 0;
    }

    //==========================================================================
    // Reverb
    {
        juce::Reverb::Parameters rp;
        rp.roomSize   = 0.7f;
        rp.damping    = 0.45f;
        rp.wetLevel   = 0.33f * pRevMix->load();
        rp.dryLevel   = 0.5f;
        rp.width      = 1.0f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        if (numCh >= 2) reverb.processStereo (data[0], data[1], n);
        else            reverb.processMono   (data[0], n);
    }

    // Volumen de salida + límite de seguridad
    for (int i = 0; i < n; ++i)
    {
        const float og = smOutput.getNextValue();
        for (int ch = 0; ch < numCh; ++ch)
            data[ch][i] = juce::jlimit (-2.0f, 2.0f, data[ch][i] * og);
    }
}

//==============================================================================
juce::AudioProcessorEditor* DiabloAudioProcessor::createEditor()
{
    return new DiabloAudioProcessorEditor (*this);
}

void DiabloAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void DiabloAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DiabloAudioProcessor();
}
