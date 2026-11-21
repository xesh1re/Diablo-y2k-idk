#pragma once
#include "PluginProcessor.h"

//==============================================================================
// Estilo Y2K: cromo, vidrio brillante, rosa chicle y cian
class Y2KLookAndFeel : public juce::LookAndFeel_V4
{
public:
    Y2KLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;
};

//==============================================================================
class DiabloAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer
{
public:
    explicit DiabloAudioProcessorEditor (DiabloAudioProcessor&);
    ~DiabloAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

    void mouseEnter (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void buildBackground();
    void updateHover (const juce::MouseEvent&);

    struct Knob
    {
        juce::String name, description;
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct Section
    {
        juce::String title;
        juce::Rectangle<int> bounds;
        std::vector<Knob*> knobs;
    };

    void layoutSection (Section&);
    void addKnob (Knob&, const char* paramId, const juce::String& name, const juce::String& desc);

    struct Bubble  { float x, y, r, speed, phase, sway; };
    struct Sparkle { float x, y, size, phase, speed; int colour; };

    DiabloAudioProcessor& processor;
    Y2KLookAndFeel laf;
    juce::TooltipWindow tooltip { this, 600 };

    Knob kInput, kOutput, kPitch, kPitchMix, kDevil, kStutter, kStSize, kChop,
         kRobot, kRobotHz, kCrush, kEchoTime, kEchoMix, kRevMix;

    std::vector<Section> sections;
    juce::Rectangle<int> descBar;
    juce::String currentTitle, currentDesc;

    juce::Image background, scanlines;
    std::vector<Bubble> bubbles;
    std::vector<Sparkle> sparkles;
    float time = 0.0f;
    juce::Random rng { 2000 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiabloAudioProcessorEditor)
};
