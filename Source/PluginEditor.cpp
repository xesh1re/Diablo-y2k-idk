#include "PluginEditor.h"

namespace
{
    const juce::Colour kPurple  { 0xFF4B1E78 };
    const juce::Colour kHotPink { 0xFFFF2E9A };
    const juce::Colour kCyan    { 0xFF19D3F3 };
    const juce::Colour kLilac   { 0xFFD9B8FF };
    const juce::Colour kSilver  { 0xFFB8BFD8 };

    constexpr int kW = 920, kH = 600;

    juce::Path makeSparkle()
    {
        juce::Path p;
        p.startNewSubPath (0.0f, -1.0f);
        p.quadraticTo (0.1f, -0.1f, 1.0f, 0.0f);
        p.quadraticTo (0.1f, 0.1f, 0.0f, 1.0f);
        p.quadraticTo (-0.1f, 0.1f, -1.0f, 0.0f);
        p.quadraticTo (-0.1f, -0.1f, 0.0f, -1.0f);
        p.closeSubPath();
        return p;
    }

    void drawSparkle (juce::Graphics& g, float x, float y, float size, juce::Colour c)
    {
        static const juce::Path sp = makeSparkle();
        g.setColour (c);
        g.fillPath (sp, juce::AffineTransform::scale (size).translated (x, y));
        g.setColour (juce::Colours::white.withAlpha (c.getFloatAlpha()));
        g.fillPath (sp, juce::AffineTransform::scale (size * 0.45f).translated (x, y));
    }

    void drawBubble (juce::Graphics& g, float x, float y, float r, float alpha)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.55f * alpha), x - r * 0.35f, y - r * 0.4f,
                                                 kCyan.withAlpha (0.18f * alpha), x + r * 0.6f, y + r * 0.7f, true));
        g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.7f * alpha));
        g.drawEllipse (x - r, y - r, r * 2.0f, r * 2.0f, 1.2f);
        g.setColour (juce::Colours::white.withAlpha (0.85f * alpha));
        g.fillEllipse (x - r * 0.55f, y - r * 0.62f, r * 0.45f, r * 0.25f);
    }

    // Botoncitos de ventana tipo Windows 98 (_ [] x)
    void drawWinButtons (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float s = area.getHeight();
        for (int i = 0; i < 3; ++i)
        {
            auto b = juce::Rectangle<float> (area.getRight() - (3 - i) * (s + 3.0f), area.getY(), s, s);
            g.setGradientFill (juce::ColourGradient (juce::Colours::white, b.getX(), b.getY(),
                                                     i == 2 ? juce::Colour (0xFFFF7AB8) : kSilver, b.getX(), b.getBottom(), false));
            g.fillRoundedRectangle (b, 3.0f);
            g.setColour (kPurple);
            g.drawRoundedRectangle (b, 3.0f, 1.2f);
            auto c = b.reduced (s * 0.28f);
            if (i == 0)      g.drawLine (c.getX(), c.getBottom(), c.getRight(), c.getBottom(), 1.6f);
            else if (i == 1) g.drawRect (c, 1.4f);
            else { g.drawLine (c.getX(), c.getY(), c.getRight(), c.getBottom(), 1.6f);
                   g.drawLine (c.getRight(), c.getY(), c.getX(), c.getBottom(), 1.6f); }
        }
    }
}

//==============================================================================
Y2KLookAndFeel::Y2KLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       kPurple);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentWhite);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentWhite);
    setColour (juce::Label::textColourId,               kPurple);
    setColour (juce::TooltipWindow::backgroundColourId, juce::Colour (0xFFFFF0FA));
    setColour (juce::TooltipWindow::textColourId,       kPurple);
    setColour (juce::TooltipWindow::outlineColourId,    kHotPink);
}

void Y2KLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float pos, float startAngle, float endAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto c = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float arcR = radius - 3.0f;

    // pista + valor (degradado cian -> rosa)
    juce::Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (kLilac.withAlpha (0.7f));
    g.strokePath (track, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setGradientFill (juce::ColourGradient (kCyan, c.x - radius, c.y, kHotPink, c.x + radius, c.y, false));
    g.strokePath (value, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // anillo cromado
    const float ringR = radius - 12.0f;
    g.setColour (juce::Colour (0x33401060));
    g.fillEllipse (c.x - ringR + 1.0f, c.y - ringR + 3.0f, ringR * 2.0f, ringR * 2.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, c.x - ringR * 0.5f, c.y - ringR,
                                             juce::Colour (0xFF7C84AE), c.x + ringR * 0.5f, c.y + ringR, false));
    g.fillEllipse (c.x - ringR, c.y - ringR, ringR * 2.0f, ringR * 2.0f);
    g.setColour (kPurple.withAlpha (0.6f));
    g.drawEllipse (c.x - ringR, c.y - ringR, ringR * 2.0f, ringR * 2.0f, 1.2f);

    // tapa de vidrio rosa brillante
    const float capR = ringR - 4.5f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xFFFF9AD6), c.x, c.y - capR,
                                             juce::Colour (0xFFB3107A), c.x, c.y + capR, false));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2.0f, capR * 2.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.8f), c.x, c.y - capR,
                                             juce::Colours::white.withAlpha (0.0f), c.x, c.y, false));
    g.fillEllipse (c.x - capR * 0.72f, c.y - capR * 0.95f, capR * 1.44f, capR * 0.95f);

    // indicador
    const float a = angle - juce::MathConstants<float>::halfPi;
    const auto dir = juce::Point<float> (std::cos (a), std::sin (a));
    const juce::Line<float> ln (c + dir * (capR * 0.25f), c + dir * (capR * 0.9f));
    g.setColour (kPurple);
    g.drawLine (ln, 6.0f);
    g.setColour (juce::Colours::white);
    g.drawLine (ln, 3.0f);
}

//==============================================================================
void DiabloAudioProcessorEditor::addKnob (Knob& k, const char* paramId,
                                          const juce::String& name, const juce::String& desc)
{
    k.name = name;
    k.description = desc;

    k.slider.setLookAndFeel (&laf);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 84, 18);
    k.slider.setTooltip (desc);
    addAndMakeVisible (k.slider);

    k.label.setText (name, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold)));
    k.label.setLookAndFeel (&laf);
    k.label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (k.label);

    k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, paramId, k.slider);
}

DiabloAudioProcessorEditor::DiabloAudioProcessorEditor (DiabloAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    using namespace ParamIDs;

    addKnob (kInput,  input,  es ("Entrada"), es ("Volumen de tu voz antes de entrar a los efectos. Si se escucha saturada sin querer, bájala."));
    addKnob (kOutput, output, es ("Salida"),  es ("Volumen final del plugin. Úsala para igualar el nivel con el beat."));

    addKnob (kPitch,    pitch,    es ("Tono"),        es ("Cambia la altura de tu voz en semitonos. Negativo = más grave (voz de monstruo), positivo = más aguda (voz de ardilla). 0 = normal."));
    addKnob (kPitchMix, pitchMix, es ("Mezcla Tono"), es ("Cuánta voz con el tono cambiado se mezcla con la original. Al 100 % solo escuchas la voz transformada."));
    addKnob (kDevil,    devil,    es ("Diablo"),      es ("Voz demoníaca: agrega una capa una octava más abajo, oscura y sucia. Súbela hasta que suene a villano."));

    addKnob (kStutter, stutter,     es ("Stutter"),         es ("Tartamudeo: de repente la voz se traba y repite un pedacito, como disco rayado. Más alto = pasa más seguido."));
    addKnob (kStSize,  stutterSize, es ("Tamaño Stutter"),  es ("Largo del pedacito que se repite, en milisegundos. Corto = trabadas rápidas tipo robot; largo = repeticiones lentas."));
    addKnob (kChop,    chop,        es ("Chop"),            es ("Corta la voz en pedacitos al azar, como si el audio se entrecortara. Muy usado en glitch."));

    addKnob (kRobot,   robot,     es ("Robot"),       es ("Voz de robot: mezcla tu voz con un tono metálico. Súbela para sonar a cyborg de los 2000."));
    addKnob (kRobotHz, robotFreq, es ("Frec. Robot"), es ("Frecuencia del robot. Baja = vibración rasposa y grave; alta = sonido metálico y brillante."));
    addKnob (kCrush,   crush,     es ("Crush"),       es ("Baja la calidad del audio como un MP3 viejo o un celular de los 2000. Da un sonido granulado y sucio."));

    addKnob (kEchoTime, echoTime,  es ("Eco Tiempo"), es ("Qué tan separadas suenan las repeticiones del eco, en milisegundos."));
    addKnob (kEchoMix,  echoMix,   es ("Eco Mezcla"), es ("Volumen del eco. Úsalo para que el final de las frases rebote."));
    addKnob (kRevMix,   reverbMix, es ("Reverb"),     es ("Cuánto espacio y profundidad le das a la voz. Alto = adlib lejano y flotando."));

    sections = {
        { es ("ENTRADA"),         {}, { &kInput } },
        { es ("DIABLO / PITCH"),  {}, { &kPitch, &kPitchMix, &kDevil } },
        { es ("GLITCH"),          {}, { &kStutter, &kStSize, &kChop } },
        { es ("ROBOT + LO-FI"),   {}, { &kRobot, &kRobotHz, &kCrush } },
        { es ("ESPACIO"),         {}, { &kEchoTime, &kEchoMix, &kRevMix } },
        { es ("SALIDA"),          {}, { &kOutput } },
    };

    currentTitle = es ("Diablo Glitch 2K");
    currentDesc  = es ("Pasa el mouse sobre una perilla para ver qué hace. Doble clic = valor original.");

    for (int i = 0; i < 16; ++i)
        bubbles.push_back ({ rng.nextFloat() * kW, rng.nextFloat() * kH, 8.0f + rng.nextFloat() * 22.0f,
                             0.2f + rng.nextFloat() * 0.5f, rng.nextFloat() * 6.28f, 6.0f + rng.nextFloat() * 14.0f });
    for (int i = 0; i < 22; ++i)
        sparkles.push_back ({ rng.nextFloat() * kW, rng.nextFloat() * kH, 5.0f + rng.nextFloat() * 9.0f,
                              rng.nextFloat() * 6.28f, 1.0f + rng.nextFloat() * 2.0f, rng.nextInt (3) });

    addMouseListener (this, true);
    setSize (kW, kH);
    startTimerHz (30);
}

DiabloAudioProcessorEditor::~DiabloAudioProcessorEditor()
{
    stopTimer();
    for (auto* k : { &kInput, &kOutput, &kPitch, &kPitchMix, &kDevil, &kStutter, &kStSize, &kChop,
                     &kRobot, &kRobotHz, &kCrush, &kEchoTime, &kEchoMix, &kRevMix })
    {
        k->slider.setLookAndFeel (nullptr);
        k->label.setLookAndFeel (nullptr);
    }
}

void DiabloAudioProcessorEditor::timerCallback()
{
    time += 1.0f / 30.0f;
    for (auto& b : bubbles)
    {
        b.y -= b.speed;
        b.x += std::sin (time * 1.3f + b.phase) * 0.35f;
        if (b.y < -40.0f) { b.y = (float) kH + 40.0f; b.x = rng.nextFloat() * kW; }
    }
    repaint();
}

//==============================================================================
void DiabloAudioProcessorEditor::buildBackground()
{
    background = juce::Image (juce::Image::ARGB, kW, kH, true);
    juce::Graphics g (background);

    // fondo: celeste -> lila -> rosa chicle
    juce::ColourGradient bg (juce::Colour (0xFFBFE6FF), 0.0f, 0.0f, juce::Colour (0xFFFFC2E6), 0.0f, (float) kH, false);
    bg.addColour (0.5, juce::Colour (0xFFE6CCFF));
    g.setGradientFill (bg);
    g.fillAll();

    // rayas holográficas diagonales
    for (int i = -kH; i < kW; i += 46)
    {
        g.setColour (juce::Colours::white.withAlpha (0.13f));
        g.drawLine ((float) i, (float) kH, (float) (i + kH), 0.0f, 14.0f);
    }

    // barra de título estilo ventana vieja
    {
        const auto bar = juce::Rectangle<float> (12.0f, 10.0f, kW - 24.0f, 30.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xFFFF7AC8), bar.getX(), 0.0f,
                                                 juce::Colour (0xFF7A5CFF), bar.getRight(), 0.0f, false));
        g.fillRoundedRectangle (bar, 8.0f);
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.fillRoundedRectangle (bar.withHeight (13.0f).reduced (2.0f, 1.0f), 6.0f);
        g.setColour (kPurple);
        g.drawRoundedRectangle (bar, 8.0f, 1.8f);

        g.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));
        g.setColour (kPurple.withAlpha (0.6f));
        g.drawText ("DIABLO_GLITCH_2K.exe", bar.toNearestInt().translated (13, 1).withTrimmedLeft (8), juce::Justification::centredLeft);
        g.setColour (juce::Colours::white);
        g.drawText ("DIABLO_GLITCH_2K.exe", bar.toNearestInt().translated (12, 0).withTrimmedLeft (8), juce::Justification::centredLeft);
        drawWinButtons (g, bar.reduced (6.0f, 5.0f));
    }

    // título cromado
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (juce::Font (juce::FontOptions (50.0f, juce::Font::bold | juce::Font::italic)),
                          "DIABLO GLITCH 2K", 26.0f, 90.0f);
        juce::Path tp;
        ga.createPath (tp);
        const auto tb = tp.getBounds();

        g.setColour (kHotPink.withAlpha (0.55f));
        g.fillPath (tp, juce::AffineTransform::translation (4.0f, 4.0f));
        g.setColour (kPurple);
        g.strokePath (tp, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::ColourGradient chrome (juce::Colours::white, 0.0f, tb.getY(), juce::Colour (0xFFFFB3E6), 0.0f, tb.getBottom(), false);
        chrome.addColour (0.45, juce::Colour (0xFFA9B6FF));
        chrome.addColour (0.55, juce::Colour (0xFF5E6AC9));
        g.setGradientFill (chrome);
        g.fillPath (tp);
        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.strokePath (tp, juce::PathStrokeType (1.0f));
    }

    // CD holográfico a la derecha
    {
        const juce::Point<float> c (836.0f, 80.0f);
        const float R = 34.0f;
        const juce::Colour cols[] = { juce::Colour (0xFFFF9AD6), juce::Colour (0xFFB9A4FF), juce::Colour (0xFF8FE3FF),
                                      juce::Colour (0xFFB8FFE0), juce::Colour (0xFFFFF3A8), juce::Colour (0xFFFFB38F) };
        for (int i = 0; i < 12; ++i)
        {
            juce::Path w;
            const float a0 = (float) i * juce::MathConstants<float>::twoPi / 12.0f;
            w.addPieSegment (c.x - R, c.y - R, R * 2.0f, R * 2.0f, a0, a0 + juce::MathConstants<float>::twoPi / 12.0f, 0.22f);
            g.setColour (cols[i % 6].withAlpha (0.9f));
            g.fillPath (w);
        }
        g.setColour (kPurple);
        g.drawEllipse (c.x - R, c.y - R, R * 2.0f, R * 2.0f, 2.0f);
        g.setColour (kPurple.withAlpha (0.7f));
        g.drawEllipse (c.x - R * 0.22f, c.y - R * 0.22f, R * 0.44f, R * 0.44f, 1.5f);
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.7f), c.x - R * 0.5f, c.y - R * 0.7f,
                                                 juce::Colours::transparentWhite, c.x, c.y, false));
        g.fillEllipse (c.x - R * 0.8f, c.y - R * 0.95f, R * 1.2f, R * 0.8f);
    }

    g.setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)));
    g.setColour (kPurple.withAlpha (0.8f));
    g.drawText (es ("~*~  glitch  ·  pitch diablo  ·  robot  ~*~"), 28, 92, 520, 20, juce::Justification::centredLeft);

    // paneles tipo ventana
    for (auto& s : sections)
    {
        const auto r = s.bounds.toFloat();

        g.setColour (juce::Colour (0x33401060));
        g.fillRoundedRectangle (r.translated (3.0f, 4.0f), 12.0f);

        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.82f), r.getX(), r.getY(),
                                                 juce::Colour (0xFFEFD9FF).withAlpha (0.62f), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, 12.0f);

        auto strip = r.withHeight (26.0f);
        g.setGradientFill (juce::ColourGradient (kHotPink, strip.getX(), 0.0f, kCyan, strip.getRight(), 0.0f, false));
        juce::Path sp;
        sp.addRoundedRectangle (strip.getX(), strip.getY(), strip.getWidth(), strip.getHeight(), 12.0f, 12.0f, true, true, false, false);
        g.fillPath (sp);
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.fillRect (strip.withHeight (10.0f).reduced (6.0f, 2.0f));

        g.setColour (kPurple);
        g.drawRoundedRectangle (r, 12.0f, 1.8f);

        g.setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)));
        g.setColour (kPurple.withAlpha (0.55f));
        g.drawText (s.title, strip.toNearestInt().withTrimmedLeft (11), juce::Justification::centredLeft);
        g.setColour (juce::Colours::white);
        g.drawText (s.title, strip.toNearestInt().withTrimmedLeft (10), juce::Justification::centredLeft);

        if (s.bounds.getWidth() > 200)
            drawWinButtons (g, strip.reduced (5.0f, 4.0f));
    }

    // barra de ayuda
    {
        const auto r = descBar.toFloat();
        g.setColour (juce::Colour (0x33401060));
        g.fillRoundedRectangle (r.translated (3.0f, 4.0f), 12.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.9f), r.getX(), r.getY(),
                                                 juce::Colour (0xFFD7F4FF).withAlpha (0.8f), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, 12.0f);
        g.setColour (kPurple);
        g.drawRoundedRectangle (r, 12.0f, 1.8f);

        // burbuja con "i"
        const float bx = r.getX() + 34.0f, by = r.getCentreY();
        drawBubble (g, bx, by, 20.0f, 1.0f);
        g.setColour (kHotPink);
        g.setFont (juce::Font (juce::FontOptions (26.0f, juce::Font::bold | juce::Font::italic)));
        g.drawText ("i", juce::Rectangle<float> (bx - 20.0f, by - 20.0f, 40.0f, 40.0f), juce::Justification::centred);
    }
}

void DiabloAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (background.isNull())
        buildBackground();

    g.drawImageAt (background, 0, 0);

    for (auto& b : bubbles)
        drawBubble (g, b.x, b.y, b.r, 0.8f);

    const juce::Colour cols[] = { juce::Colours::white, kCyan.brighter (0.4f), kHotPink.brighter (0.4f) };
    for (auto& s : sparkles)
    {
        const float tw = 0.5f + 0.5f * std::sin (time * s.speed + s.phase);
        drawSparkle (g, s.x, s.y, s.size * (0.5f + 0.5f * tw), cols[s.colour].withAlpha (0.25f + 0.65f * tw));
    }

    // texto de ayuda en español
    auto text = descBar.reduced (14, 8).withTrimmedLeft (58);
    g.setColour (kHotPink);
    g.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));
    g.drawText (currentTitle, text.removeFromTop (22), juce::Justification::centredLeft);
    g.setColour (kPurple);
    g.setFont (juce::Font (juce::FontOptions (14.5f)));
    g.drawFittedText (currentDesc, text, juce::Justification::topLeft, 2);
}

void DiabloAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    if (scanlines.isNull())
    {
        scanlines = juce::Image (juce::Image::ARGB, kW, kH, true);
        juce::Graphics sg (scanlines);
        sg.setColour (kPurple.withAlpha (0.05f));
        for (int y = 0; y < kH; y += 3)
            sg.fillRect (0, y, kW, 1);
    }
    g.drawImageAt (scanlines, 0, 0);
}

//==============================================================================
void DiabloAudioProcessorEditor::layoutSection (Section& s)
{
    auto inner = s.bounds.withTrimmedTop (30).reduced (8, 6);
    const int n = (int) s.knobs.size();
    const int cellW = inner.getWidth() / n;

    for (auto* k : s.knobs)
    {
        auto cell = inner.removeFromLeft (cellW);
        if (n == 1)
            cell = cell.withSizeKeepingCentre (cell.getWidth(), 150);

        k->label.setBounds (cell.removeFromTop (20));
        k->slider.setBounds (cell.reduced (2));
    }
}

void DiabloAudioProcessorEditor::resized()
{
    const int top = 120, rowH = 184, gap = 10;

    sections[0].bounds = { 18,  top, 110, rowH * 2 + gap };
    sections[1].bounds = { 138, top, 317, rowH };
    sections[2].bounds = { 465, top, 317, rowH };
    sections[3].bounds = { 138, top + rowH + gap, 317, rowH };
    sections[4].bounds = { 465, top + rowH + gap, 317, rowH };
    sections[5].bounds = { 792, top, 110, rowH * 2 + gap };

    descBar = { 18, top + rowH * 2 + gap + 10, kW - 36, 74 };

    for (auto& s : sections)
        layoutSection (s);

    buildBackground();
}

//==============================================================================
void DiabloAudioProcessorEditor::updateHover (const juce::MouseEvent& e)
{
    auto* c = e.originalComponent;
    for (auto& s : sections)
        for (auto* k : s.knobs)
            if (c == &k->slider || c == &k->label || k->slider.isParentOf (c))
            {
                currentTitle = s.title + es ("  ·  ") + k->name;
                currentDesc  = k->description;
                return;
            }
}

void DiabloAudioProcessorEditor::mouseEnter (const juce::MouseEvent& e) { updateHover (e); }
void DiabloAudioProcessorEditor::mouseMove  (const juce::MouseEvent& e) { updateHover (e); }

void DiabloAudioProcessorEditor::mouseExit (const juce::MouseEvent& e)
{
    if (e.originalComponent == this)
    {
        currentTitle = es ("Diablo Glitch 2K");
        currentDesc  = es ("Pasa el mouse sobre una perilla para ver qué hace. Doble clic = valor original.");
    }
}
