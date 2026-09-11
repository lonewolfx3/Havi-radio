#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../Source/Editor.h"
#include <cassert>
#include <cmath>
#include <iostream>

static juce::Slider& slider(RadioBoxEditor& editor, const char* id) {
    auto* result = dynamic_cast<juce::Slider*>(editor.findChildWithID(juce::String("knob.") + id));
    assert(result != nullptr);
    return *result;
}

int main() {
    juce::ScopedJuceInitialiser_GUI init;
    const int previousZoom = UserPresets::zoom();
    UserPresets::saveZoom(100);
    RadioBoxProcessor processor;
    RadioBoxEditor editor(processor);
    juce::AudioBuffer<float> audio(2, 512);
    juce::MidiBuffer midi;

    for (int preset = 0; preset < 10; ++preset) {
        processor.selectPreset(preset);
        editor.refresh(0.0f);
        for (size_t i = 0; i < havi::controlIDs.size(); ++i) {
            const float expected = havi::factoryControls[static_cast<size_t>(preset)][14] > 0.5f && i >= 11 && i <= 13
                ? 0.0f : havi::factoryControls[static_cast<size_t>(preset)][i];
            const float actual = processor.state.getRawParameterValue(havi::controlIDs[i])->load();
            assert(std::abs(actual - expected) < 0.005f);
            if (i < 10) assert(std::abs(slider(editor, havi::controlIDs[i]).getValue() - actual) < 0.005);
        }
    }

    auto* menu = dynamic_cast<juce::ComboBox*>(editor.findChildWithID("preset.selector"));
    assert(menu != nullptr);
    menu->setSelectedId(3, juce::sendNotificationSync);
    assert(processor.getCurrentProgram() == 2);

    for (int i = 0; i < 90; ++i) editor.refresh(1.0f / 60.0f);
    auto& drive = static_cast<RadioKnob&>(slider(editor, "drive"));
    const float before = drive.visualPosition;
    processor.selectPreset(6);
    editor.refresh(0.0f);
    const float target = processor.state.getParameter("drive")->convertTo0to1(16.0f);
    assert(drive.visualPosition == before);
    editor.refresh(1.0f / 60.0f);
    assert(drive.visualPosition < before && drive.visualPosition > target);
    slider(editor, "drive").setValue(73.4, juce::sendNotificationSync);
    assert(std::abs(processor.controls()[6] - 73.4f) < 0.01f && processor.modified());

    processor.selectBand(2);
    assert(processor.controls()[11] == 0 && processor.controls()[12] == 0 && processor.controls()[13] == 1);
    processor.setDropout(true);
    assert(processor.controls()[11] == 0 && processor.controls()[12] == 0 && processor.controls()[13] == 0);
    processor.setDropout(false);
    assert(processor.controls()[11] == 0 && processor.controls()[12] == 0 && processor.controls()[13] == 1);
    processor.selectBand(0);
    assert(processor.controls()[11] == 1 && processor.controls()[12] == 0 && processor.controls()[13] == 0);

    const auto unique = juce::String("RadioBox-CI-") + juce::String(juce::Time::currentTimeMillis());
    auto result = UserPresets::save(unique, processor.state.copyState());
    assert(result.wasOk());
    auto presetFile = UserPresets::users().getChildFile(unique + ".radiobox");
    assert(presetFile.existsAsFile());
    slider(editor, "drive").setValue(11.0, juce::sendNotificationSync);
    juce::ValueTree restored;
    assert(UserPresets::load(presetFile, restored).wasOk());
    processor.loadUserState(restored);
    assert(std::abs(processor.controls()[6] - 73.4f) < 0.01f);
    const auto renamed = unique + "-Renamed";
    assert(UserPresets::rename(presetFile, renamed).wasOk());
    presetFile = UserPresets::users().getChildFile(renamed + ".radiobox");
    assert(presetFile.existsAsFile());
    assert(presetFile.deleteFile());

    editor.setZoomPercent(50);
    assert(editor.getWidth() == 600 && editor.getHeight() == 335);
    editor.setZoomPercent(150);
    assert(editor.getWidth() == 1800 && editor.getHeight() == 1005);
    editor.setZoomPercent(100);

    processor.prepareToPlay(48000, 512);
    for (int block = 0; block < 100; ++block) {
        for (int channel = 0; channel < 2; ++channel)
            for (int sample = 0; sample < 512; ++sample)
                audio.setSample(channel, sample, 0.8f * std::sin(static_cast<float>(sample) * 0.13f));
        processor.processBlock(audio, midi);
        editor.refresh(1.0f / 60.0f);
    }
    assert(editor.meterValue() > 0.8f);
    for (int block = 0; block < 180; ++block) {
        audio.clear();
        processor.processBlock(audio, midi);
        editor.refresh(1.0f / 60.0f);
    }
    assert(editor.meterValue() < 0.001f);

    auto output = juce::File::getCurrentWorkingDirectory().getChildFile("ui-previews");
    output.createDirectory();
    auto* theme = dynamic_cast<juce::ComboBox*>(editor.findChildWithID("theme.selector"));
    assert(theme != nullptr);
    const char* names[] { "olive", "burgundy", "blue" };
    for (int index = 0; index < 3; ++index) {
        theme->setSelectedId(index + 1, juce::sendNotificationSync);
        editor.refresh(0.0f);
        auto picture = editor.createComponentSnapshot(editor.getLocalBounds());
        auto stream = output.getChildFile(juce::String(names[index]) + ".png").createOutputStream();
        assert(stream != nullptr);
        juce::PNGImageFormat format;
        assert(format.writeImageToStream(picture, *stream));
    }

    UserPresets::saveZoom(previousZoom);
    std::cout << "PASS: layered themes, preset CRUD, zoom, live controls, dropout restore and metering.\n";
}
