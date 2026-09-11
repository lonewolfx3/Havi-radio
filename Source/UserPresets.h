#pragma once
#include <JuceHeader.h>
#include <algorithm>

// UI-thread file operations. Audio processing never touches the filesystem.
class UserPresets {
public:
    static juce::File root() {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Radio Box Presets");
    }
    static juce::File users() { return root().getChildFile("User Presets"); }
    static juce::File factories() { return root().getChildFile("Factory Presets"); }
    static juce::Result initialise() {
        auto r = users().createDirectory();
        if (r.failed()) return r;
        r = factories().createDirectory();
        if (r.failed()) return r;
        const auto note = factories().getChildFile("Factory Presets.txt");
        if (!note.existsAsFile())
            note.replaceWithText("RadioBox factory presets are built into the plug-in and are protected from editing or deletion.\n");
        note.setReadOnly(true);
        factories().setReadOnly(true);
        return juce::Result::ok();
    }
    static juce::Array<juce::File> list() {
        auto files = users().findChildFiles(juce::File::findFiles, false, "*.radiobox");
        std::sort(files.begin(), files.end(), [](const juce::File& a, const juce::File& b) {
            return a.getFileNameWithoutExtension().compareNatural(b.getFileNameWithoutExtension()) < 0;
        });
        return files;
    }
    static juce::Result validateName(const juce::String& name) {
        if (name.trim().isEmpty() || name != name.trim() || name.length() > 80
            || name.containsAnyOf("/\\:*?\"<>|") || name.endsWithChar('.')
            || name == "." || name == "..")
            return juce::Result::fail("Use 1–80 characters without filesystem punctuation or trailing spaces.");
        return juce::Result::ok();
    }
    static juce::Result save(const juce::String& name, const juce::ValueTree& state) {
        auto valid = validateName(name);
        if (valid.failed()) return valid;
        auto ready = initialise();
        if (ready.failed()) return ready;
        auto target = users().getChildFile(name + ".radiobox");
        if (target.exists()) return juce::Result::fail("A preset with this name already exists.");
        juce::XmlElement document("RadioBoxUserPreset");
        document.setAttribute("version", 1);
        document.setAttribute("name", name);
        document.addChildElement(state.createXml().release());
        juce::TemporaryFile temporary(target);
        if (!document.writeTo(temporary.getFile()) || !temporary.overwriteTargetFileWithTemporary())
            return juce::Result::fail("Could not save preset. Check folder permissions.");
        return juce::Result::ok();
    }
    static juce::Result load(const juce::File& file, juce::ValueTree& state) {
        if (file.getParentDirectory() != users() || file.getSize() > 1024 * 1024)
            return juce::Result::fail("Invalid preset file.");
        auto xml = juce::XmlDocument::parse(file);
        if (!xml || !xml->hasTagName("RadioBoxUserPreset") || xml->getIntAttribute("version") != 1)
            return juce::Result::fail("This file is not a supported RadioBox preset.");
        auto* child = xml->getChildByName("HaviRadio");
        if (!child) return juce::Result::fail("Preset settings are missing.");
        state = juce::ValueTree::fromXml(*child);
        return state.isValid() ? juce::Result::ok() : juce::Result::fail("Invalid preset settings.");
    }
    static juce::Result rename(const juce::File& file, const juce::String& name) {
        auto valid = validateName(name);
        if (valid.failed()) return valid;
        if (file.getParentDirectory() != users()) return juce::Result::fail("Factory presets cannot be renamed.");
        auto target = users().getChildFile(name + ".radiobox");
        if (target == file) return juce::Result::ok();
        if (target.exists()) return juce::Result::fail("That name is already in use.");
        return file.moveFileTo(target) ? juce::Result::ok() : juce::Result::fail("Could not rename preset.");
    }
    static juce::Result remove(const juce::File& file) {
        if (file.getParentDirectory() != users()) return juce::Result::fail("Factory presets cannot be deleted.");
        return file.moveToTrash() ? juce::Result::ok() : juce::Result::fail("Could not move preset to trash.");
    }
    static constexpr std::array<int, 9> zooms{50,60,70,80,90,100,110,125,150};
    static int zoom() {
        auto xml = juce::XmlDocument::parse(root().getChildFile("Preferences.xml"));
        const int value = xml ? xml->getIntAttribute("zoom", 100) : 100;
        return std::find(zooms.begin(), zooms.end(), value) != zooms.end() ? value : 100;
    }
    static juce::Result saveZoom(int value) {
        if (std::find(zooms.begin(), zooms.end(), value) == zooms.end()) return juce::Result::fail("Unsupported zoom.");
        auto ready = initialise();
        if (ready.failed()) return ready;
        juce::XmlElement xml("RadioBoxPreferences"); xml.setAttribute("zoom", value);
        juce::TemporaryFile tmp(root().getChildFile("Preferences.xml"));
        return xml.writeTo(tmp.getFile()) && tmp.overwriteTargetFileWithTemporary()
            ? juce::Result::ok() : juce::Result::fail("Could not save UI size preference.");
    }
};
