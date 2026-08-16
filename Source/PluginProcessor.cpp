#include "PluginProcessor.h"
#include "PluginEditor.h"

DirtyBassAudioProcessor::DirtyBassAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout DirtyBassAudioProcessor::createParameterLayout()
{
    using P = juce::AudioParameterFloat;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<P>("DRIVE", "Drive", juce::NormalisableRange<float>(0.0f, 1.0f), 0.45f));
    layout.add(std::make_unique<P>("GRIT", "Grit", juce::NormalisableRange<float>(0.0f, 1.0f), 0.25f));
    layout.add(std::make_unique<P>("BODY", "Body", juce::NormalisableRange<float>(0.0f, 1.0f), 0.55f));
    layout.add(std::make_unique<P>("BITE", "Bite", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    layout.add(std::make_unique<P>("MIX", "Mix", juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    layout.add(std::make_unique<P>("OUTPUT", "Output", juce::NormalisableRange<float>(0.0f, 1.5f), 0.8f));
    return layout;
}

void DirtyBassAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    preFilterL.prepare(spec); preFilterR.prepare(spec);
    postFilterL.prepare(spec); postFilterR.prepare(spec);
    preFilterL.reset(); preFilterR.reset(); postFilterL.reset(); postFilterR.reset();

    drive.reset(sampleRate, 0.03); grit.reset(sampleRate, 0.03);
    body.reset(sampleRate, 0.03); bite.reset(sampleRate, 0.03);
    mix.reset(sampleRate, 0.03); output.reset(sampleRate, 0.03);
    envelope = 0.0f;
}

void DirtyBassAudioProcessor::releaseResources() {}

bool DirtyBassAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto in = layouts.getChannelSet(true, 0);
    auto out = layouts.getChannelSet(false, 0);
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && in == out;
}

void DirtyBassAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto n = buffer.getNumSamples();
    if (n == 0) return;

    auto* d = buffer.getArrayOfReadPointers();
    const auto driveTarget = apvts.getRawParameterValue("DRIVE")->load();
    const auto gritTarget = apvts.getRawParameterValue("GRIT")->load();
    const auto bodyTarget = apvts.getRawParameterValue("BODY")->load();
    const auto biteTarget = apvts.getRawParameterValue("BITE")->load();
    const auto mixTarget = apvts.getRawParameterValue("MIX")->load();
    const auto outTarget = apvts.getRawParameterValue("OUTPUT")->load();

    drive.setTargetValue(driveTarget);
    grit.setTargetValue(gritTarget);
    body.setTargetValue(bodyTarget);
    bite.setTargetValue(biteTarget);
    mix.setTargetValue(mixTarget);
    output.setTargetValue(outTarget);

    for (int i = 0; i < n; ++i)
    {
        const float drv = drive.getNextValue();
        const float grt = grit.getNextValue();
        const float bod = body.getNextValue();
        const float bit = bite.getNextValue();
        const float wet = mix.getNextValue();
        const float out = output.getNextValue();

        float xL = buffer.getSample(0, i);
        float xR = buffer.getNumChannels() > 1 ? buffer.getSample(1, i) : xL;
        const float cleanL = xL, cleanR = xR;

        const float level = 0.5f * (std::abs(xL) + std::abs(xR));
        envelope += (level - envelope) * (level > envelope ? 0.0025f : 0.00035f);
        const float dynamic = 1.0f + drv * (1.5f + 3.0f * envelope);

        // Pre-emphasis keeps the low end from turning into indistinct fuzz.
        xL *= (0.85f + 0.25f * bod);
        xR *= (0.85f + 0.25f * bod);

        auto saturate = [dynamic, grt](float x)
        {
            const float a = std::tanh(x * dynamic * (1.0f + 2.5f * grt));
            const float b = std::tanh(x * dynamic * 0.35f);
            return a * (0.82f - 0.22f * grt) + b * (0.18f + 0.22f * grt);
        };

        xL = saturate(xL);
        xR = saturate(xR);

        // A subtle asymmetric harmonic component controlled by Grit.
        xL += grt * 0.08f * std::tanh(xL * xL * (xL >= 0.0f ? 1.0f : -1.0f));
        xR += grt * 0.08f * std::tanh(xR * xR * (xR >= 0.0f ? 1.0f : -1.0f));

        // Simple post EQ: body low shelf-ish compensation and bite presence.
        const float lowGain = 0.75f + 0.55f * bod;
        const float highTilt = 1.0f + 0.55f * bit;
        xL *= lowGain;
        xR *= lowGain;
        xL = xL * (1.0f - 0.10f * bit) + (xL - 0.15f * cleanL) * 0.10f * highTilt;
        xR = xR * (1.0f - 0.10f * bit) + (xR - 0.15f * cleanR) * 0.10f * highTilt;

        buffer.setSample(0, i, (cleanL * (1.0f - wet) + xL * wet) * out);
        if (buffer.getNumChannels() > 1)
            buffer.setSample(1, i, (cleanR * (1.0f - wet) + xR * wet) * out);
    }
}

juce::AudioProcessorEditor* DirtyBassAudioProcessor::createEditor()
{
    return new DirtyBassAudioProcessorEditor(*this);
}

void DirtyBassAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void DirtyBassAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
