#pragma once

#include "Processor.h"
#include "UserPresets.h"
#include "BinaryData.h"
#include <functional>

struct RadioThemeAssets {
    juce::Image base, lo, mid, hi, bandwidth, tuning, ring, filter, drive, tone, width, mix;
    juce::Image needle, needleShadow, pivot, am, fm, sw, power, dropout;
    juce::Image signal, overload, amber, bypass, headerPower, calibration;
};

inline juce::Image radioResource(const juce::String& name) {
    int size = 0;
    if (auto* data = BinaryData::getNamedResource(name.toRawUTF8(), size))
        return juce::ImageCache::getFromMemory(data, size);
    return {};
}

inline RadioThemeAssets loadRadioTheme(const juce::String& prefix) {
    const auto image = [&prefix](const char* suffix, const char* extension = "png") {
        return radioResource(prefix + "_" + suffix + "_" + extension);
    };
    return { image("base", "jpg"), image("lo"), image("mid"), image("hi"), image("bandwidth"),
             image("tuning"), image("ring"), image("filter"), image("drive"), image("tone"),
             image("width"), image("mix"), image("needle"), image("needle_shadow"), image("pivot"),
             image("am"), image("fm"), image("sw"), image("power"), image("dropout"),
             image("signal"), image("overload"), image("amber"), image("bypass"),
             image("headerpower"), image("calibration") };
}

class RadioKnob final : public juce::Slider {
public:
    float visualPosition = 0.0f;
    float sourceAngle = 0.0f;
    juce::Image artwork;
    juce::Rectangle<float> artworkBounds;
};

class RadioLook final : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics& g, int, int, int, int, float, float start, float end,
                          juce::Slider& slider) override {
        auto& knob = static_cast<RadioKnob&>(slider);
        if (!knob.artwork.isValid()) return;
        const float angle = start + knob.visualPosition * (end - start) - knob.sourceAngle;
        const auto centre = knob.artworkBounds.getCentre();
        juce::Graphics::ScopedSaveState save(g);
        g.addTransform(juce::AffineTransform::rotation(angle, centre.x, centre.y));
        g.drawImage(knob.artwork, knob.artworkBounds);
    }

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override {}

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                              bool highlighted, bool down) override {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(juce::Colour(down ? 0xff0c0b09 : highlighted ? 0xff393028 : 0xff211d18));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(juce::Colour(highlighted ? 0xffe2a859 : 0xff805d35));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    }
};

class RadioBoxEditor final : public juce::AudioProcessorEditor, private juce::Timer {
    static constexpr int designWidth = 1536;
    static constexpr int designHeight = 857;
    static constexpr int normalWidth = 1200;
    static constexpr int normalHeight = 670;
    static constexpr float rotaryStart = juce::MathConstants<float>::pi * 1.25f;
    static constexpr float rotaryEnd = juce::MathConstants<float>::pi * 2.75f;

    RadioBoxProcessor& processor;
    RadioLook look;
    std::array<RadioThemeAssets, 3> themes { loadRadioTheme("olive"), loadRadioTheme("burgundy"), loadRadioTheme("blue") };
    int themeIndex = -1;
    int zoomPercent = 100;
    float level = 0.0f;
    juce::File activeUserPreset;
    juce::Array<juce::File> userPresetFiles;
    bool rebuildingMenu = false;

    juce::ComboBox station, zoom, theme;
    juce::TextButton savePreset { "SAVE" }, openFolder { "FOLDER" }, reload { "R" };
    std::array<RadioKnob, 10> knobs;
    std::array<juce::ToggleButton, 6> toggles;
    juce::TooltipWindow tooltips { this, 350 };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::array<std::unique_ptr<SliderAttachment>, 10> sliderAttachments;
    std::unique_ptr<ButtonAttachment> dropoutAttachment, powerAttachment, lightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> themeAttachment;

    inline static constexpr std::array<const char*, 10> knobIDs {
        "loRoll", "midEq", "hiRoll", "bandwidth", "tuning", "filter", "drive", "tone", "width", "mix"
    };
    inline static const std::array<juce::Rectangle<float>, 10> knobArtBounds {{
        { 138.0f, 208.5f, 75.0f, 78.0f }, { 258.75f, 208.5f, 76.5f, 78.0f },
        { 379.5f, 209.25f, 78.0f, 78.0f }, { 232.5f, 365.25f, 120.0f, 124.5f },
        { 664.5f, 549.0f, 204.0f, 195.75f }, { 1097.25f, 175.5f, 115.5f, 121.5f },
        { 1281.75f, 175.5f, 117.0f, 123.0f }, { 1097.25f, 380.25f, 115.5f, 121.5f },
        { 988.5f, 597.75f, 117.0f, 123.0f }, { 1182.0f, 597.0f, 120.0f, 124.5f }
    }};

    float raw(const char* id) const { return processor.state.getRawParameterValue(id)->load(); }
    float scale() const { return static_cast<float>(getWidth()) / static_cast<float>(designWidth); }
    juce::Rectangle<int> scaled(juce::Rectangle<float> bounds) const {
        return bounds.transformedBy(juce::AffineTransform::scale(scale())).getSmallestIntegerContainer();
    }

    static void drawImage(juce::Graphics& g, const juce::Image& image, juce::Rectangle<float> bounds,
                          float opacity = 1.0f) {
        if (!image.isValid()) return;
        juce::Graphics::ScopedSaveState save(g);
        g.setOpacity(opacity);
        g.drawImage(image, bounds, juce::RectanglePlacement::stretchToFit, false);
    }

    static void drawRotated(juce::Graphics& g, const juce::Image& image, juce::Rectangle<float> bounds,
                            float angle, juce::Point<float> pivot) {
        if (!image.isValid()) return;
        juce::Graphics::ScopedSaveState save(g);
        g.addTransform(juce::AffineTransform::rotation(angle, pivot.x, pivot.y));
        drawImage(g, image, bounds);
    }

    void styleToolbarControl(juce::ComboBox& box) {
        box.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xf0181613));
        box.setColour(juce::ComboBox::textColourId, juce::Colour(0xffffd596));
        box.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff93652f));
        box.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffffc673));
        addAndMakeVisible(box);
    }

    void styleToolbarButton(juce::TextButton& button) {
        button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffd596));
        button.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffd596));
        addAndMakeVisible(button);
    }

    void showError(const juce::String& message) {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "RadioBox", message, "OK", this);
    }

    void askForName(const juce::String& title, const juce::String& initial,
                    std::function<void(juce::String)> completion) {
        auto* dialog = new juce::AlertWindow(title, "Preset name:", juce::MessageBoxIconType::NoIcon);
        dialog->addTextEditor("name", initial, "Name");
        dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        dialog->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
        juce::Component::SafePointer<RadioBoxEditor> safe(this);
        dialog->enterModalState(true, juce::ModalCallbackFunction::create(
            [safe, dialog, completion = std::move(completion)](int result) mutable {
                if (result == 1 && safe != nullptr)
                    completion(dialog->getTextEditorContents("name"));
            }), true);
    }

    void saveCurrentPreset(const juce::String& name) {
        const auto result = UserPresets::save(name, processor.state.copyState());
        if (result.failed()) { showError(result.getErrorMessage()); return; }
        activeUserPreset = UserPresets::users().getChildFile(name + ".radiobox");
        rebuildPresetMenu(activeUserPreset);
    }

    void renameCurrentPreset(const juce::String& name) {
        if (!activeUserPreset.existsAsFile()) { showError("Select a user preset before renaming it."); return; }
        const auto result = UserPresets::rename(activeUserPreset, name);
        if (result.failed()) { showError(result.getErrorMessage()); return; }
        activeUserPreset = UserPresets::users().getChildFile(name + ".radiobox");
        rebuildPresetMenu(activeUserPreset);
    }

    void deleteCurrentPreset() {
        if (!activeUserPreset.existsAsFile()) { showError("Select a user preset before deleting it."); return; }
        const auto file = activeUserPreset;
        juce::Component::SafePointer<RadioBoxEditor> safe(this);
        juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::WarningIcon, "Delete Preset",
            "Move ‘" + file.getFileNameWithoutExtension() + "’ to the Trash?", "Delete", "Cancel", this,
            juce::ModalCallbackFunction::create([safe, file](int result) {
                if (result != 1 || safe == nullptr) return;
                const auto removed = UserPresets::remove(file);
                if (removed.failed()) safe->showError(removed.getErrorMessage());
                else { safe->activeUserPreset = {}; safe->rebuildPresetMenu({}); }
            }));
    }

    void rebuildPresetMenu(const juce::File& selectFile) {
        const juce::ScopedValueSetter<bool> guard(rebuildingMenu, true);
        station.clear(juce::dontSendNotification);
        for (int i = 0; i < static_cast<int>(havi::presetNames.size()); ++i)
            station.addItem(havi::presetNames[static_cast<size_t>(i)], i + 1);
        userPresetFiles = UserPresets::list();
        if (!userPresetFiles.isEmpty()) {
            station.addSeparator();
            for (int i = 0; i < userPresetFiles.size(); ++i)
                station.addItem(userPresetFiles.getReference(i).getFileNameWithoutExtension(), 1000 + i);
        }
        station.addSeparator();
        station.addItem("Rename User Preset…", 9001);
        station.addItem("Delete User Preset…", 9002);
        if (selectFile.existsAsFile()) {
            for (int i = 0; i < userPresetFiles.size(); ++i)
                if (userPresetFiles.getReference(i) == selectFile) { station.setSelectedId(1000 + i); return; }
        }
        station.setSelectedId(processor.getCurrentProgram() + 1);
    }

    void presetMenuChanged() {
        if (rebuildingMenu) return;
        const int id = station.getSelectedId();
        if (id >= 1 && id <= 10) {
            activeUserPreset = {};
            processor.selectPreset(id - 1);
        } else if (id >= 1000 && id < 1000 + userPresetFiles.size()) {
            juce::ValueTree loaded;
            const auto file = userPresetFiles.getReference(id - 1000);
            const auto result = UserPresets::load(file, loaded);
            if (result.failed()) showError(result.getErrorMessage());
            else { processor.loadUserState(loaded); activeUserPreset = file; }
        } else if (id == 9001) {
            const auto initial = activeUserPreset.getFileNameWithoutExtension();
            askForName("Rename Preset", initial, [this](juce::String name) { renameCurrentPreset(name); });
        } else if (id == 9002) {
            deleteCurrentPreset();
        }
        refresh(0.0f);
    }

    void applyTheme(int index) {
        themeIndex = juce::jlimit(0, 2, index);
        auto& assets = themes[static_cast<size_t>(themeIndex)];
        const std::array<juce::Image, 10> knobImages {
            assets.lo, assets.mid, assets.hi, assets.bandwidth, assets.tuning,
            assets.filter, assets.drive, assets.tone, assets.width, assets.mix
        };
        for (size_t i = 0; i < knobs.size(); ++i) knobs[i].artwork = knobImages[i];
        repaint();
    }

    void timerCallback() override { refresh(1.0f / 60.0f); }

public:
    explicit RadioBoxEditor(RadioBoxProcessor& p) : juce::AudioProcessorEditor(p), processor(p) {
        setLookAndFeel(&look);
        UserPresets::initialise();

        styleToolbarControl(station);
        styleToolbarControl(zoom);
        styleToolbarControl(theme);
        station.setComponentID("preset.selector");
        zoom.setComponentID("zoom.selector");
        theme.setComponentID("theme.selector");
        station.setTooltip("Factory and user presets. User preset actions are at the bottom.");
        zoom.setTooltip("UI SIZE");
        theme.setTooltip("UI COLOR");
        station.onChange = [this] { presetMenuChanged(); };

        for (int z : UserPresets::zooms) zoom.addItem(juce::String(z) + "%", z);
        zoom.onChange = [this] { setZoomPercent(zoom.getSelectedId()); };
        theme.addItemList({ "Olive Green", "Burgundy", "Blue" }, 1);
        themeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state, "theme", theme);

        styleToolbarButton(savePreset);
        styleToolbarButton(openFolder);
        styleToolbarButton(reload);
        savePreset.setComponentID("preset.save");
        openFolder.setComponentID("preset.folder");
        reload.setComponentID("preset.reload");
        savePreset.setTooltip("Save the complete current RadioBox state as a user preset");
        openFolder.setTooltip("Open Radio Box Presets in Finder or Explorer");
        reload.setTooltip("Reload the user preset list");
        savePreset.onClick = [this] { askForName("Save Preset", {}, [this](juce::String name) { saveCurrentPreset(name); }); };
        openFolder.onClick = [this] {
            const auto result = UserPresets::initialise();
            if (result.failed()) showError(result.getErrorMessage());
            else if (!UserPresets::root().startAsProcess()) showError("Could not open the preset folder.");
        };
        reload.onClick = [this] { rebuildPresetMenu(activeUserPreset); };

        for (size_t i = 0; i < knobs.size(); ++i) {
            auto& knob = knobs[i];
            knob.setComponentID(juce::String("knob.") + knobIDs[i]);
            knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            knob.setRotaryParameters(rotaryStart, rotaryEnd, true);
            knob.setPopupDisplayEnabled(true, true, this);
            knob.setMouseDragSensitivity(i == 4 ? 360 : 220);
            knob.setTooltip(processor.state.getParameter(knobIDs[i])->getName(80));
            addAndMakeVisible(knob);
            sliderAttachments[i] = std::make_unique<SliderAttachment>(p.state, knobIDs[i], knob);
            const float normalized = p.state.getParameter(knobIDs[i])->convertTo0to1(raw(knobIDs[i]));
            knob.visualPosition = normalized;
            knob.sourceAngle = rotaryStart + normalized * (rotaryEnd - rotaryStart);
        }

        const std::array<const char*, 3> bandIDs { "am", "fm", "sw" };
        for (int i = 0; i < 3; ++i) {
            auto& toggle = toggles[static_cast<size_t>(i)];
            toggle.setComponentID(juce::String("switch.") + bandIDs[static_cast<size_t>(i)]);
            toggle.setTooltip(juce::String(bandIDs[static_cast<size_t>(i)]).toUpperCase() + " radio band");
            toggle.onClick = [this, i] { if (raw("dropout") < 0.5f) processor.selectBand(i); };
            addAndMakeVisible(toggle);
        }
        toggles[3].setComponentID("switch.dropout");
        toggles[4].setComponentID("switch.power");
        toggles[5].setComponentID("switch.vuLight");
        toggles[3].setTooltip("Signal Dropout");
        toggles[4].setTooltip("Power / audio bypass");
        toggles[5].setTooltip("VU meter backlight");
        for (int i = 3; i < 6; ++i) addAndMakeVisible(toggles[static_cast<size_t>(i)]);
        dropoutAttachment = std::make_unique<ButtonAttachment>(p.state, "dropout", toggles[3]);
        powerAttachment = std::make_unique<ButtonAttachment>(p.state, "power", toggles[4]);
        lightAttachment = std::make_unique<ButtonAttachment>(p.state, "vuLight", toggles[5]);

        rebuildPresetMenu({});
        zoomPercent = UserPresets::zoom();
        zoom.setSelectedId(zoomPercent, juce::dontSendNotification);
        setResizable(false, false);
        setSize(juce::roundToInt(normalWidth * zoomPercent / 100.0f),
                juce::roundToInt(normalHeight * zoomPercent / 100.0f));
        applyTheme(juce::jlimit(0, 2, static_cast<int>(raw("theme"))));
        refresh(0.0f);
        startTimerHz(60);
    }

    ~RadioBoxEditor() override {
        stopTimer();
        setLookAndFeel(nullptr);
    }

    void setZoomPercent(int value) {
        if (std::find(UserPresets::zooms.begin(), UserPresets::zooms.end(), value) == UserPresets::zooms.end()) return;
        zoomPercent = value;
        zoom.setSelectedId(value, juce::dontSendNotification);
        setSize(juce::roundToInt(normalWidth * value / 100.0f),
                juce::roundToInt(normalHeight * value / 100.0f));
        const auto result = UserPresets::saveZoom(value);
        if (result.failed()) showError(result.getErrorMessage());
    }

    int getZoomPercent() const { return zoomPercent; }

    void refresh(float seconds) {
        const int requestedTheme = juce::jlimit(0, 2, static_cast<int>(std::lround(raw("theme"))));
        if (requestedTheme != themeIndex) applyTheme(requestedTheme);
        theme.setSelectedId(themeIndex + 1, juce::dontSendNotification);
        if (!activeUserPreset.existsAsFile() && !rebuildingMenu)
            station.setSelectedId(processor.getCurrentProgram() + 1, juce::dontSendNotification);
        for (size_t i = 0; i < knobs.size(); ++i) {
            const float target = processor.state.getParameter(knobIDs[i])->convertTo0to1(raw(knobIDs[i]));
            knobs[i].setValue(raw(knobIDs[i]), juce::dontSendNotification);
            knobs[i].visualPosition = knobs[i].isMouseButtonDown()
                ? target : havi::visualStep(knobs[i].visualPosition, target, seconds);
            knobs[i].repaint();
        }
        const bool dropout = raw("dropout") > 0.5f;
        for (int i = 0; i < 3; ++i) {
            toggles[static_cast<size_t>(i)].setEnabled(!dropout);
            toggles[static_cast<size_t>(i)].setToggleState(raw(havi::controlIDs[static_cast<size_t>(11 + i)]) > 0.5f,
                                                            juce::dontSendNotification);
        }
        if (seconds > 0.0f) level = havi::Meter::animate(level, processor.meter.take(), seconds);
        repaint();
    }

    float meterValue() const { return level; }

    void paint(juce::Graphics& g) override {
        juce::Graphics::ScopedSaveState save(g);
        g.addTransform(juce::AffineTransform::scale(scale()));
        auto& assets = themes[static_cast<size_t>(juce::jlimit(0, 2, themeIndex))];
        drawImage(g, assets.base, { 0.0f, 0.0f, static_cast<float>(designWidth), static_cast<float>(designHeight) });

        drawImage(g, assets.ring, { 615.75f, 492.75f, 306.0f, 310.5f });
        drawImage(g, assets.calibration, { 922.5f, 371.25f, 36.0f, 37.5f });

        const bool dropout = raw("dropout") > 0.5f;
        const std::array<juce::Image, 3> bandImages { assets.am, assets.fm, assets.sw };
        const std::array<juce::Rectangle<float>, 3> bandBounds {{
            { 162.0f, 599.25f, 79.5f, 118.5f }, { 252.0f, 599.25f, 80.25f, 118.5f },
            { 343.5f, 599.25f, 80.25f, 118.5f }
        }};
        for (int i = 0; i < 3; ++i) {
            const bool active = !dropout && raw(havi::controlIDs[static_cast<size_t>(11 + i)]) > 0.5f;
            drawImage(g, bandImages[static_cast<size_t>(i)], bandBounds[static_cast<size_t>(i)], active ? 1.0f : 0.48f);
        }

        const bool powerOn = raw("power") > 0.5f;
        const auto powerBounds = juce::Rectangle<float>(1323.0f, 429.0f, 65.25f, 54.75f);
        drawRotated(g, assets.power, powerBounds, powerOn ? juce::MathConstants<float>::pi : 0.0f,
                    powerBounds.getCentre());
        const auto dropoutBounds = juce::Rectangle<float>(504.0f, 648.0f, 37.5f, 63.75f);
        drawRotated(g, assets.dropout, dropoutBounds, dropout ? juce::MathConstants<float>::pi : 0.0f,
                    dropoutBounds.getCentre());
        drawImage(g, assets.headerPower, { 1332.75f, 3.75f, 36.0f, 36.0f }, powerOn ? 1.0f : 0.45f);

        const bool meterLight = raw("vuLight") > 0.5f;
        if (!meterLight) {
            g.setColour(juce::Colour(0x76000000));
            g.fillRoundedRectangle(544.0f, 171.0f, 467.0f, 291.0f, 30.0f);
        } else {
            drawImage(g, assets.amber, { 1029.75f, 289.5f, 51.0f, 54.0f });
        }
        if (level > 0.025f) drawImage(g, assets.signal, { 660.75f, 392.25f, 33.0f, 33.0f });
        if (processor.overload.load()) drawImage(g, assets.overload, { 843.0f, 392.25f, 33.0f, 33.0f });
        if (!powerOn) drawImage(g, assets.bypass, { 1364.25f, 629.25f, 52.5f, 57.0f });

        const auto pivot = juce::Point<float>(768.0f, 414.0f);
        const float desiredNeedleAngle = juce::degreesToRadians(-58.0f + 116.0f * juce::jlimit(0.0f, 1.0f, level));
        const float sourceNeedleAngle = juce::degreesToRadians(31.0f);
        const float needleRotation = desiredNeedleAngle - sourceNeedleAngle;
        drawRotated(g, assets.needleShadow, { 774.0f, 272.25f, 99.0f, 143.25f }, needleRotation, pivot);
        drawRotated(g, assets.needle, { 774.75f, 272.25f, 91.5f, 144.0f }, needleRotation, pivot);
        drawImage(g, assets.pivot, { 751.5f, 402.0f, 33.0f, 30.0f });
    }

    void resized() override {
        station.setBounds(scaled({ 342.0f, 5.0f, 140.0f, 34.0f }));
        savePreset.setBounds(scaled({ 486.0f, 5.0f, 42.0f, 34.0f }));
        openFolder.setBounds(scaled({ 532.0f, 5.0f, 52.0f, 34.0f }));
        reload.setBounds(scaled({ 588.0f, 5.0f, 30.0f, 34.0f }));
        zoom.setBounds(scaled({ 622.0f, 5.0f, 55.0f, 34.0f }));
        theme.setBounds(scaled({ 1012.0f, 5.0f, 165.0f, 34.0f }));

        for (size_t i = 0; i < knobs.size(); ++i) {
            const auto art = scaled(knobArtBounds[i]);
            const int padding = juce::jmax(8, juce::roundToInt(22.0f * scale()));
            const auto hit = art.expanded(padding);
            knobs[i].setBounds(hit);
            knobs[i].artworkBounds = art.toFloat().translated(static_cast<float>(-hit.getX()),
                                                               static_cast<float>(-hit.getY()));
        }
        toggles[0].setBounds(scaled({ 151.0f, 581.0f, 102.0f, 150.0f }));
        toggles[1].setBounds(scaled({ 241.0f, 581.0f, 102.0f, 150.0f }));
        toggles[2].setBounds(scaled({ 332.0f, 581.0f, 102.0f, 150.0f }));
        toggles[3].setBounds(scaled({ 480.0f, 594.0f, 82.0f, 145.0f }));
        toggles[4].setBounds(scaled({ 1302.0f, 374.0f, 105.0f, 145.0f }));
        toggles[5].setBounds(scaled({ 912.0f, 357.0f, 58.0f, 60.0f }));
    }
};
