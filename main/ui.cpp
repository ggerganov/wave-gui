/*! \file ui.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "ui.h"

#include "data.h"

#include "cg_logger.h"
#include "cg_window2d.h"
#include "cg_glfw3.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl.h"

#include <GLFW/glfw3.h>

#include <cmath>
#include <chrono>
#include <cinttypes>

#include "imgui/imgui_style.h"

namespace {
    constexpr auto programId = "audio2";

    const ::Data::StateInput * g_inp = nullptr;
}

struct UI::Data {
    Data() : stateInput(new ::Data::StateInput()) {
        *stateInput = ::Data::StateInput::getDefaultConfig(::Data::StateInput::BW16_Stable);
    }

    int frametime_ms = 0;

    CG::Window2D * window = nullptr;

    //ImGuiWindowFlags windowFlags = ImGuiWindowFlags_ShowBorders;
    ImGuiWindowFlags windowFlags = 0;

    std::shared_ptr<::Data::StateInput> stateInput;
    std::weak_ptr<::Data::StateData> stateData;
    std::shared_ptr<::Data::StateData> stateDataLocked;

    std::map<Event, std::function<void()>> callbacks;
};

UI::UI() : _data(new Data) {
    CG_INFO(0, "Creating UI object\n");

    if (CG::GLFW3::getInstance().init() == false) {
        CG_FATAL(0, "Error initializing GLFW!\n");
    }
}

UI::~UI() {
    CG_INFO(0, "Destroying UI object\n");

    CG::GLFW3::getInstance().terminate();
}

bool UI::init(std::weak_ptr<CG::Window2D> window) {
    if (window.expired()) {
        CG_FATAL(0, "Cannot initialize UI with null window\n");
        return false;
    }

    if (auto w = window.lock()) {
        _data->window = w.get();
    } else {
        return false;
    }

    ImGui::CreateContext();
    ImGui_Init(_data->window->getGLFWWindow(), true);

    ImGui::applyStyle<ImGui::IMGUI_STYLE_TEAL_AND_ORANGE>();

    if (auto & c = _data->callbacks[BUTTON_INIT]) c();

    return true;
}

void UI::pollEvents() {
    CG::GLFW3::getInstance().poll();
}

void UI::processKeyboard() {
    if (ImGui::GetIO().KeysDown[GLFW_KEY_ESCAPE]) {
        if (auto & c = _data->callbacks[KEY_ESCAPE]) c();
    }

    //if (ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
    //    if (auto & c = _data->callbacks[BUTTON_INIT]) c();
    //}

    if (ImGui::IsKeyPressed(GLFW_KEY_F1)) {
        if (_data->stateDataLocked->sendingData) {
            if (auto & c = _data->callbacks[BUTTON_DATA_OFF]) c();
        } else {
            if (auto & c = _data->callbacks[BUTTON_DATA_ON]) c();
        }
    }

    if (ImGui::IsKeyPressed(GLFW_KEY_F2)) {
        if (auto & c = _data->callbacks[BUTTON_DATA_CLEAR]) c();
        if (auto & c = _data->callbacks[BUTTON_DATA_ON]) c();
        if (auto & c = _data->callbacks[BUTTON_DATA_SEND]) c();
    }
}

void UI::update() {
    pollEvents();
    processKeyboard();
}

void UI::render() const {
    ImGui_NewFrame();

    auto tStart = std::chrono::high_resolution_clock::now();

    if (!(_data->stateDataLocked = _data->stateData.lock())) {
        CG_WARN(0, "Missing state data\n");
    }

    renderMainMenuBar();
    renderWindowControls();
    renderWindowInput();
    //renderWindowOutput();

    auto tEnd = std::chrono::high_resolution_clock::now();
    _data->frametime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();

    ImGui::Render();
    ImGui_RenderDrawData(ImGui::GetDrawData());
}

void UI::terminate() {
    CG_INFO(0, "Terminating UI\n");

    ImGui_Shutdown();
}

void UI::setStateData(std::weak_ptr<::Data::StateData> stateData) {
    _data->stateData = stateData;
}

std::weak_ptr<::Data::StateInput> UI::getStateInput() const {
    return _data->stateInput;
}

void UI::setEventCallback(Event event, std::function<void()> && callback) {
    _data->callbacks[event] = std::move(callback);
}

void UI::renderMainMenuBar() const {
    static bool showStyleEditorWindow = false;
	static bool showDemoWindow = false;
	static bool showMetricsWindow = true;
    static bool showDebugWindow = true;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", "Escape")) {
				if (auto & c = _data->callbacks[KEY_ESCAPE]) c();
			}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Style")) {
            ImGui::renderStyleSelectorMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows")) {
            ImGui::MenuItem("Debug") && (showDebugWindow = !showDebugWindow);
            ImGui::MenuItem("Metrics") && (showMetricsWindow = !showMetricsWindow);
            ImGui::MenuItem("Style Editor") && (showStyleEditorWindow = !showStyleEditorWindow);
            ImGui::MenuItem("ImGui Demo") && (showDemoWindow = !showDemoWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showDemoWindow) {
        ImGui::ShowTestWindow();
    }

    if (showMetricsWindow) {
        ImGui::ShowMetricsWindow(&showMetricsWindow);
    }

    if (showStyleEditorWindow) {
        ImGui::Begin((std::string("Style Editor##") + ::programId).c_str(), &showStyleEditorWindow, _data->windowFlags);
        ImGui::ShowStyleEditor();
        ImGui::End();
    }

    if (showDebugWindow) {
        ImVec4 clearColor;
        ImGui::SetNextWindowPos(ImVec2(8, 521), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(382, 276), ImGuiSetCond_FirstUseEver);
        ImGui::Begin((std::string("Debug##") + ::programId).c_str(), &showDebugWindow, _data->windowFlags);
        ImGui::renderDebugWindow<120>(_data->frametime_ms, clearColor);
        ImGui::End();
    }
}

void UI::renderWindowControls() const {
    const auto & inp = _data->stateInput;
    const auto & data = _data->stateDataLocked;

    ImGui::SetNextWindowPos(ImVec2(0, 22), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(388, 493), ImGuiSetCond_FirstUseEver);
    ImGui::Begin((std::string("Controls##") + ::programId).c_str(), nullptr, _data->windowFlags);

    static bool updateStats = true;
    static double frameLength_ms = 0;
    static double subFrameLength_ms = 0;

    //if (ImGui::DragInt("Sample Rate", &inp->sampleRate, 22050, 128, 4*44100)) updateStats = true;
    //if (ImGui::DragInt("Frames Per Buffer", &inp->samplesPerFrame, 512, 128, ::Data::Constants::kMaxFramesPerBuffer)) updateStats = true;

    if (updateStats) {
        frameLength_ms = (1000.0/inp->sampleRate)*inp->samplesPerFrame;
        subFrameLength_ms = frameLength_ms/::Data::Constants::kSubFrames;
        updateStats = false;
    }

    ImGui::Columns(2, "", false);
    ImGui::SetColumnOffset(1, 80);
    if (ImGui::Button("Init\nAudio", ImVec2(80, 80))) {
        if (auto & c = _data->callbacks[BUTTON_INIT]) c();
    }
    ImGui::NextColumn();
    ImGui::Text("Sample Rate:           %d\n", inp->sampleRate);
    ImGui::Text("Samples per Frame:     %d\n", inp->samplesPerFrame);
    ImGui::Text("Sub-Frames:            %d\n", ::Data::Constants::kSubFrames);
    ImGui::Text("Samples per Sub-Frame: %d\n", inp->samplesPerSubFrame);
    ImGui::Text("Frame duration:        %g [ms]\n", frameLength_ms);
    ImGui::NextColumn();
    ImGui::Columns(1);

    if (ImGui::CollapsingHeader("Rx. / Tx. Options", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool updateSendParameters = false;

        {
            static int cid = 3;
            if (ImGui::Combo("Tx. Protocol", &cid, ::Data::StateInput::configNames, ::Data::StateInput::ConfigId::COUNT)) {
                auto oldVol = inp->sendVolume;
                auto oldSendData = inp->sendData;
                *inp = ::Data::StateInput::getDefaultConfig((::Data::StateInput::ConfigId)cid);
                inp->sendVolume = oldVol;
                inp->sendData = oldSendData;

                if (auto & c = _data->callbacks[BUTTON_DATA_ON]) c();
                if (auto & c = _data->callbacks[BUTTON_DATA_OFF]) c();
            }
        }

        {
            int idx = std::round(inp->freqDelta_hz/inp->getHzPerFrame());
            ImGui::PushItemWidth(80);
            if (ImGui::SliderInt("##freqDelta", &idx, 2, 32)) {
                inp->freqDelta_hz = idx*inp->getHzPerFrame();
                updateSendParameters = true;
            }
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("Freq. Delta [Hz] = %4.4f", inp->freqDelta_hz);
        }
        {
            int idx = std::round(inp->freqStart_hz/inp->getHzPerFrame());
            ImGui::PushItemWidth(80);
            //ImGui::IsKeyPressed(GLFW_KEY_LEFT)  && (--idx, updateSendParameters = true);
            //ImGui::IsKeyPressed(GLFW_KEY_RIGHT) && (++idx, updateSendParameters = true);
            ImGui::SliderInt("##freqStart", &idx, 0, inp->samplesPerFrame) && (updateSendParameters = true);
            inp->freqStart_hz = idx*inp->getHzPerFrame();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("Freq. Start [Hz] = %4.4f", inp->freqStart_hz);

            if ((inp->freqCheck_hz <= inp->freqStart_hz + (inp->nDataBitsPerTx - 1)*inp->freqDelta_hz &&
                 inp->freqCheck_hz >= inp->freqStart_hz) ||
                (inp->freqCheck_hz + (::Data::Constants::kMaxBitsPerChecksum - 1)*inp->freqDelta_hz >= inp->freqStart_hz &&
                 inp->freqCheck_hz <= inp->freqStart_hz)) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(!)");
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Invalid frequency parameters\n");
                    ImGui::EndTooltip();
                }
            }
        }

        {
            int idx = std::round(inp->freqCheck_hz/inp->getHzPerFrame());
            ImGui::PushItemWidth(80);
            //ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(GLFW_KEY_LEFT)  && (--idx, updateSendParameters = true);
            //ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(GLFW_KEY_RIGHT) && (++idx, updateSendParameters = true);
            ImGui::SliderInt("##freqCheck", &idx, 0, inp->samplesPerFrame) && (updateSendParameters = true);
            inp->freqCheck_hz = idx*inp->getHzPerFrame();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("Freq. Check [Hz] = %4.4f", inp->freqCheck_hz);
        }

        ImGui::Checkbox("Encode DataId Parity", &inp->encodeIdParity);
        ImGui::Checkbox("Use checksum", &inp->useChecksum);
        ImGui::SliderFloat("Volume", &inp->sendVolume, 0.0f, 1.0f);
        ImGui::SliderInt("Ramp Begin/End", &inp->nRampFramesBegin, 1, 256); inp->nRampFramesEnd = inp->nRampFramesBegin;
        ImGui::SliderInt("Ramp Blend", &inp->nRampFramesBlend, 1, 256);
        ImGui::SliderInt("nSF Confirm", &inp->nConfirmFrames, 3, 64);
        ImGui::SliderInt("nSF per Data", &inp->subFramesPerTx, 1, 256);
        {
            int idx = inp->nDataBitsPerTx/8;
            if (ImGui::SliderInt("Data bytes per Tx", &idx, 1, ::Data::Constants::kMaxDataBits/8)) {
                inp->nDataBitsPerTx = 8*idx;
            }
            ImGui::SliderInt("EEC Bytes", &inp->nECCBytesPerTx, 0, 31);
        }

        ImGui::Checkbox("1##dataBit0", &inp->dataBits[0]) && (updateSendParameters = true); ImGui::SameLine();
        ImGui::Checkbox("2##dataBit1", &inp->dataBits[1]) && (updateSendParameters = true); ImGui::SameLine();
        ImGui::Checkbox("3##dataBit2", &inp->dataBits[2]) && (updateSendParameters = true); ImGui::SameLine();
        ImGui::Checkbox("4##dataBit3", &inp->dataBits[3]) && (updateSendParameters = true); ImGui::SameLine();
        ImGui::Checkbox("5##dataBit4", &inp->dataBits[4]) && (updateSendParameters = true); ImGui::SameLine();
        ImGui::Checkbox("6##dataBit5", &inp->dataBits[5]) && (updateSendParameters = true); ImGui::SameLine();
        ImGui::Checkbox("7##dataBit6", &inp->dataBits[6]) && (updateSendParameters = true); ImGui::SameLine();
        ImGui::Checkbox("8##dataBit7", &inp->dataBits[7]) && (updateSendParameters = true);

        ImGui::IsKeyPressed(GLFW_KEY_1) && (inp->dataBits[0] = !inp->dataBits[0], updateSendParameters = true);
        ImGui::IsKeyPressed(GLFW_KEY_2) && (inp->dataBits[1] = !inp->dataBits[1], updateSendParameters = true);
        ImGui::IsKeyPressed(GLFW_KEY_3) && (inp->dataBits[2] = !inp->dataBits[2], updateSendParameters = true);
        ImGui::IsKeyPressed(GLFW_KEY_4) && (inp->dataBits[3] = !inp->dataBits[3], updateSendParameters = true);
        ImGui::IsKeyPressed(GLFW_KEY_5) && (inp->dataBits[4] = !inp->dataBits[4], updateSendParameters = true);
        ImGui::IsKeyPressed(GLFW_KEY_6) && (inp->dataBits[5] = !inp->dataBits[5], updateSendParameters = true);
        ImGui::IsKeyPressed(GLFW_KEY_7) && (inp->dataBits[6] = !inp->dataBits[6], updateSendParameters = true);
        ImGui::IsKeyPressed(GLFW_KEY_8) && (inp->dataBits[7] = !inp->dataBits[7], updateSendParameters = true);

        if (updateSendParameters && data->sendingData) {
            if (auto & c = _data->callbacks[BUTTON_DATA_ON]) c();
        }

        ImGui::SameLine();
        if (ImGui::Button("On")) {
            if (auto & c = _data->callbacks[BUTTON_DATA_ON]) c();
        }
        ImGui::SameLine();

        if (ImGui::Button("Off")) {
            if (auto & c = _data->callbacks[BUTTON_DATA_OFF]) c();
        }

        ImGui::Text("Tx duration: %4.4f ms", inp->subFramesPerTx*subFrameLength_ms);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Bandwidth:   %4.2f B/s",
                           (inp->nDataBitsPerTx/8 > inp->nECCBytesPerTx) ?
                           1000.0/(inp->subFramesPerTx*subFrameLength_ms)*(inp->nDataBitsPerTx/8.0 - inp->nECCBytesPerTx) :
                           1000.0/(inp->subFramesPerTx*subFrameLength_ms)*inp->nDataBitsPerTx/8.0);
    }

    ImGui::End();
}

void UI::renderWindowInput() const {
    const auto & inp = _data->stateInput;
    const auto & data = _data->stateDataLocked;

    ::g_inp = inp.get(); // dirty hack used in the PlotHistogram lambda functions

    ImGui::SetNextWindowPos(ImVec2(391, 22), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(807, 776), ImGuiSetCond_FirstUseEver);
    ImGui::Begin((std::string("Input##") + ::programId).c_str(), nullptr, _data->windowFlags);

    auto histSize = ImGui::GetContentRegionAvail();
    histSize.y *= 0.60;
    ImGui::BeginChild("##histograms", histSize);

    if (data->sampleAmplitude != nullptr) {
        auto wSize = ImGui::GetContentRegionAvail();
        wSize.y *= 0.333;
        ImGui::PlotLines("##plotWaveform", data->sampleAmplitude->data(), data->samplesPerFrame, 0, "Waveform", -0.2f, 0.2f, wSize);
    } else {
        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "Audio not initialized yet!");
    }

    if (data->sampleSpectrum != nullptr) {
        auto wSize = ImGui::GetContentRegionAvail();
        wSize.y *= 0.5;
        static float yScale = 1.0f;
        auto posSave = ImGui::GetCursorScreenPos();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
        ImGui::PlotHistogram("##plotDataRange",
                             [](void *data, int i) -> float {
                                 if (data == nullptr ||
                                     i*::g_inp->getHzPerFrame() < ::g_inp->freqStart_hz ||
                                     i*::g_inp->getHzPerFrame() > ::g_inp->freqStart_hz + ::g_inp->nDataBitsPerTx*::g_inp->freqDelta_hz) return 0.0f;
                                 return 1.0f;
                             },
                             data->sampleSpectrum->data(), data->samplesPerFrame/2, 0, "\nGreen: Data, Red: Checksum", 0.5f, 1.0f, wSize);
        ImGui::PopStyleColor(2);

        ImGui::SetCursorScreenPos(posSave);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
        ImGui::PlotHistogram("##plotDataRange",
                             [](void *data, int i) -> float {
                                 if (data == nullptr ||
                                     i*::g_inp->getHzPerFrame() < ::g_inp->freqCheck_hz ||
                                     i*::g_inp->getHzPerFrame() > ::g_inp->freqCheck_hz + ::Data::Constants::kMaxBitsPerChecksum*::g_inp->freqDelta_hz) return 0.0f;
                                 return 1.0f;
                             },
                             data->sampleSpectrum->data(), data->samplesPerFrame/2, 0, NULL, 0.5f, 1.0f, wSize);
        ImGui::PopStyleColor(2);

        ImGui::SetCursorScreenPos(posSave);
        ImGui::PlotHistogram("##plotSpectrumCurrent", data->sampleSpectrum->data(), data->samplesPerFrame/2, 0,
                (std::string("Current Spectrum, Y max = ") + std::to_string(yScale)).c_str(), 0.0f, yScale, wSize);

        ImGui::IsItemHovered() && (yScale *= (1.0 + 0.01*ImGui::GetIO().MouseWheel));
    }

    if (data->historySpectrumAverage != nullptr) {
        auto wSize = ImGui::GetContentRegionAvail();
        wSize.y *= 1.0;
        static float yScale = 1.0f;
        auto posSave = ImGui::GetCursorScreenPos();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
        ImGui::PlotHistogram("##plotDataRange",
                             [](void *data, int i) -> float {
                                 if (data == nullptr ||
                                     i*::g_inp->getHzPerFrame() < ::g_inp->freqStart_hz ||
                                     i*::g_inp->getHzPerFrame() > ::g_inp->freqStart_hz + ::g_inp->nDataBitsPerTx*::g_inp->freqDelta_hz) return 0.0f;
                                 return 1.0f;
                             },
                             data->sampleSpectrum->data(), data->samplesPerFrame/2, 0, "\nGreen: Data, Red: Checksum", 0.5f, 1.0f, wSize);
        ImGui::PopStyleColor(2);

        ImGui::SetCursorScreenPos(posSave);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
        ImGui::PlotHistogram("##plotDataRange",
                             [](void *data, int i) -> float {
                                 if (data == nullptr ||
                                     i*::g_inp->getHzPerFrame() < ::g_inp->freqCheck_hz ||
                                     i*::g_inp->getHzPerFrame() > ::g_inp->freqCheck_hz + ::Data::Constants::kMaxBitsPerChecksum*::g_inp->freqDelta_hz) return 0.0f;
                                 return 1.0f;
                             },
                             data->sampleSpectrum->data(), data->samplesPerFrame/2, 0, NULL, 0.5f, 1.0f, wSize);
        ImGui::PopStyleColor(2);

        ImGui::SetCursorScreenPos(posSave);
        ImGui::PlotHistogram("##plotSpectrumAverage", data->historySpectrumAverage->data(), data->samplesPerFrame/2, 0,
                (std::string("Average Spectrum, Y max = ") + std::to_string(yScale)).c_str(), 0.0f, yScale, wSize);

        ImGui::IsItemHovered() && (yScale *= (1.0 + 0.01*ImGui::GetIO().MouseWheel));
    }

    //if (data->historySpectrumAverage != nullptr) {
    //    auto wSize = ImGui::GetContentRegionAvail();
    //    static float yScale = 20.0f;
    //    static ::Data::SpectrumData snrSpectrum;
    //    for (int i = 0; i < (int) snrSpectrum.size(); ++i) {
    //        snrSpectrum[i] = data->sampleSpectrum->at(i)/data->historySpectrumAverage->at(i);
    //    }
    //    ImGui::PlotHistogram("SNR", snrSpectrum.data(), data->samplesPerFrame/2, 0,
    //            (std::string("Y scale = ") + std::to_string(yScale)).c_str(), 0.0f, yScale, wSize);
    //    ImGui::IsItemHovered() && (yScale *= (1.0 + 0.1*ImGui::GetIO().MouseWheel));
    //}
    ImGui::EndChild();

    {
        auto wSize = ImGui::GetContentRegionAvail();
        wSize.y *= 0.5;

        ImGui::Columns(2, "", false);
        ImGui::SetColumnOffset(1, wSize.y);
        if (data->receivingData) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Receiving --->");
        } else {
            ImGui::Text("Received:");
        }
        if (ImGui::Button("Clear", ImVec2(wSize.y, wSize.y - 20))) {
            if (auto & c = _data->callbacks[BUTTON_DATA_CLEAR]) c();
        }
        ImGui::NextColumn();
        if (data->receivedData) {
            wSize.x = ImGui::GetContentRegionAvailWidth();
            ImGui::PushTextWrapPos(0.0f);
            ImGui::InputTextMultiline("##dataReceived", data->receivedData->data(), ::Data::Constants::kMaxDataSize, wSize);
            ImGui::PopTextWrapPos();
        }
        ImGui::NextColumn();
        ImGui::Columns(1);
    }

    {
        auto wSize = ImGui::GetContentRegionAvail();

        ImGui::Columns(2, "", false);
        ImGui::SetColumnOffset(1, wSize.y);
        if (data->sendingData) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Sending   --->:");
        } else {
            ImGui::Text("To send:");
        }
        static int nBytes = 32;
        ImGui::DragInt("Bytes", &nBytes, 1, 1, 512);
        if (ImGui::Button("Random", ImVec2(wSize.y, 0.5*(wSize.y - 44)))) {
            inp->sendData.fill(0);
            for (int i = 0; i < nBytes; ++i) {
                if (i > 0 && (i%32 == 0)) {
                    inp->sendData[i] = '\n';
                } else {
                    if (rand()%8 == 0) {
                        inp->sendData[i] = ' ';
                    } else {
                        inp->sendData[i] = 48 + rand()%42;
                    }
                }
            }
        }
        if (ImGui::Button("Send", ImVec2(wSize.y, 0.5*(wSize.y - 44)))) {
            if (auto & c = _data->callbacks[BUTTON_DATA_CLEAR]) c();
            if (auto & c = _data->callbacks[BUTTON_DATA_ON]) c();
            if (auto & c = _data->callbacks[BUTTON_DATA_SEND]) c();
        }
        ImGui::NextColumn();
        if (data->receivedData) {
            ImGui::PushTextWrapPos(0.0f);
            ImGui::InputTextMultiline("##dataSend", inp->sendData.data(), ::Data::Constants::kMaxDataSize, ImGui::GetContentRegionAvail());
            ImGui::PopTextWrapPos();
        }
        ImGui::NextColumn();
        ImGui::Columns(1);
    }

    ImGui::End();
}

void UI::renderWindowOutput() const {
    const auto & data = _data->stateDataLocked;

    ImGui::SetNextWindowPos(ImVec2(385, 557), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(813, 237), ImGuiSetCond_FirstUseEver);
    ImGui::Begin((std::string("Output##") + ::programId).c_str(), nullptr, _data->windowFlags);

    if (data->bitAmplitude != nullptr) {
        for (const auto & bAmpl : *data->bitAmplitude) {
            auto wSize = ImGui::GetContentRegionAvail();
            wSize.y = 20;
            ImGui::PlotLines("", bAmpl.data(), data->samplesPerFrame, 0, NULL, -1.0f, 1.0f, wSize);
        }
    }

    ImGui::End();
}
