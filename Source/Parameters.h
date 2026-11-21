#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Convierte texto UTF-8 (acentos, ñ) a juce::String de forma segura.
inline juce::String es (const char* utf8) { return juce::String::fromUTF8 (utf8); }

namespace ParamIDs
{
    inline constexpr const char* input       = "input";
    inline constexpr const char* output      = "output";

    inline constexpr const char* pitch       = "pitch";
    inline constexpr const char* pitchMix    = "pitchMix";
    inline constexpr const char* devil       = "devil";

    inline constexpr const char* stutter     = "stutter";
    inline constexpr const char* stutterSize = "stutterSize";
    inline constexpr const char* chop        = "chop";

    inline constexpr const char* robot       = "robot";
    inline constexpr const char* robotFreq   = "robotFreq";
    inline constexpr const char* crush       = "crush";

    inline constexpr const char* echoTime    = "echoTime";
    inline constexpr const char* echoMix     = "echoMix";
    inline constexpr const char* reverbMix   = "reverbMix";
}
