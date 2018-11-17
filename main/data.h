/*! \file data.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <vector>
#include <array>
#include <cstring>

namespace Data {

namespace Constants {
constexpr auto kDefaultSamplingRate = 48000;
constexpr auto kSubFrames = 1;
constexpr auto kFactor = ((float)(kSubFrames))/8;
constexpr auto kMaxSamplesPerFrame = 1024;
constexpr auto kMaxDataBits = 256;
constexpr auto kMaxBitsPerChecksum = 10;
constexpr auto kMaxSpectrumHistory = 2*kSubFrames;
constexpr auto ikMaxSpectrumHistory = 1.0/kMaxSpectrumHistory;
constexpr auto kMaxDataSize = 1024;
}

using AmplitudeData = std::array<float, 2*Constants::kMaxSamplesPerFrame>;
using SpectrumData  = std::array<float,   Constants::kMaxSamplesPerFrame>;

struct StateInput {
    enum ConfigId {
        BW11_LowFreq,
        BW11_MedFreq,
        BW11_HighFreq,
        BW16_Stable,
        BW22_MedFreq,
        BW43_Protocol1,
        BW43_Protocol2,
        BW64_Protocol1,
        BW64_Protocol2,
        BW166_Protocol1,
        BW166_Protocol2,
        BW172_Protocol1,
        BW258_Protocol1,
        COUNT,
    };

    StateInput() {
        dataBits.fill(0);
        sendData.fill(0);

        strcpy(sendData.data(), "Sample Data");
    }

    inline float getHzPerFrame() const { return ((double)(sampleRate))/samplesPerFrame; }

    static const char * configNames[];
    static StateInput getDefaultConfig(ConfigId cid);

    int sampleRate = Constants::kDefaultSamplingRate;
    int samplesPerFrame = Constants::kMaxSamplesPerFrame;
    int samplesPerSubFrame = samplesPerFrame/Constants::kSubFrames;
    int nRampFramesBegin = 16*Constants::kFactor;
    int nRampFramesEnd = 16*Constants::kFactor;
    int nRampFramesBlend = 16*Constants::kFactor;
    int nConfirmFrames = 1;
    int subFramesPerTx = 64*Constants::kFactor;
    int nDataBitsPerTx = 64*Constants::kFactor;
    int nECCBytesPerTx = 4;

    bool encodeIdParity = true;
    bool useChecksum = false;

    float sendVolume = 0.1f;
    float sendDuration_ms = 100.0f;

    float freqDelta_hz =  2*getHzPerFrame();
    float freqStart_hz = 100*getHzPerFrame();
    float freqCheck_hz = 360*getHzPerFrame();

    std::array<bool, Constants::kMaxDataBits> dataBits;
    std::array<char, Constants::kMaxDataSize> sendData;
};

struct StateData {
    int nIterations = 0;

    int samplesPerFrame = 0;
    int samplesPerSubFrame = 0;

    bool sendingData = false;
    bool sendingDataBuffer = false;
    bool receivingData = false;

    AmplitudeData * sampleAmplitude = nullptr;
    SpectrumData * sampleSpectrum = nullptr;
    SpectrumData * historySpectrumAverage = nullptr;
    std::array<AmplitudeData, Constants::kMaxDataBits> * bitAmplitude = nullptr;

    std::array<char, Constants::kMaxDataSize> * receivedData = nullptr;
};
}
