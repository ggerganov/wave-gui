/*! \file data.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "data.h"

#include <cmath>

namespace Data {

    const char * StateInput::configNames[] = {
        "11B/s, Low Freq",
        "11B/s, Med Freq",
        "11B/s, High Freq",
        "16B/s, Stable",
        "22B/s, Med Freq",
        "43B/s, Protocol 1",
        "43B/s, Protocol 2",
        "64B/s, Protocol 1",
        "64B/s, Protocol 2",
        "86B/s, Protocol 1",
        "86B/s, Protocol 2",
        "172B/s, Protocol 1",
        "258B/s, Protocol 1",
    };

    StateInput StateInput::getDefaultConfig(ConfigId cid) {
        StateInput cfg;

        switch (cid) {
            case BW11_LowFreq:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 4*Constants::kFactor;
                cfg.subFramesPerTx = 32*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*1;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   8*cfg.getHzPerFrame();
                cfg.freqStart_hz =  60*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 130*cfg.getHzPerFrame();

                break;
            case BW11_MedFreq:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 4*Constants::kFactor;
                cfg.subFramesPerTx = 32*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*1;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =  12*cfg.getHzPerFrame();
                cfg.freqStart_hz = 144*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 250*cfg.getHzPerFrame();

                break;
            case BW11_HighFreq:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 4*Constants::kFactor;
                cfg.subFramesPerTx = 32*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*1;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   8*cfg.getHzPerFrame();
                cfg.freqStart_hz = 280*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 350*cfg.getHzPerFrame();
                break;
            case BW16_Stable:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 8*Constants::kFactor;
                cfg.nRampFramesEnd = 8*Constants::kFactor;
                cfg.nRampFramesBlend = 8*Constants::kFactor;
                cfg.nConfirmFrames = 16*Constants::kFactor;
                cfg.subFramesPerTx = 64*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*2;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   2*cfg.getHzPerFrame();
                cfg.freqStart_hz =  60*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 126*cfg.getHzPerFrame();

                break;
            case BW22_MedFreq:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 12*Constants::kFactor;
                cfg.subFramesPerTx = 64*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*4;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   6*cfg.getHzPerFrame();
                cfg.freqStart_hz = 140*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 342*cfg.getHzPerFrame();

                break;
            case BW43_Protocol1:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 8*Constants::kFactor;
                cfg.nRampFramesEnd = 8*Constants::kFactor;
                cfg.nRampFramesBlend = 8*Constants::kFactor;
                cfg.nConfirmFrames = 4*Constants::kFactor;
                cfg.subFramesPerTx = 32*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*4;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   6*cfg.getHzPerFrame();
                cfg.freqStart_hz = 140*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 342*cfg.getHzPerFrame();

                break;
            case BW43_Protocol2:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 8*Constants::kFactor;
                cfg.subFramesPerTx = 48*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*6;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   4*cfg.getHzPerFrame();
                cfg.freqStart_hz = 140*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 342*cfg.getHzPerFrame();

                break;
            case BW64_Protocol1:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 12*Constants::kFactor;
                cfg.subFramesPerTx = 48*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*9;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   4*cfg.getHzPerFrame();
                cfg.freqStart_hz =  92*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 386*cfg.getHzPerFrame();

                break;
            case BW64_Protocol2:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 12*Constants::kFactor;
                cfg.subFramesPerTx = 32*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*6;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   5*cfg.getHzPerFrame();
                cfg.freqStart_hz =  93*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 342*cfg.getHzPerFrame();

                break;
            case BW166_Protocol1:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 4*Constants::kFactor;
                cfg.subFramesPerTx = 32*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*8;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   3*cfg.getHzPerFrame();
                cfg.freqStart_hz = 140*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 342*cfg.getHzPerFrame();

                break;
            case BW166_Protocol2:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 8*Constants::kFactor;
                cfg.subFramesPerTx = 48*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*12;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   3*cfg.getHzPerFrame();
                cfg.freqStart_hz = 105*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 400*cfg.getHzPerFrame();

                break;
            case BW172_Protocol1:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 4*Constants::kFactor;
                cfg.subFramesPerTx = 32*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*16;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   2*cfg.getHzPerFrame();
                cfg.freqStart_hz = 140*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 400*cfg.getHzPerFrame();

                break;
            case BW258_Protocol1:
                cfg.sampleRate = Constants::kDefaultSamplingRate;
                cfg.samplesPerFrame = Constants::kMaxSamplesPerFrame;
                cfg.samplesPerSubFrame = cfg.samplesPerFrame/Constants::kSubFrames;
                cfg.nRampFramesBegin = 16*Constants::kFactor;
                cfg.nRampFramesEnd = 16*Constants::kFactor;
                cfg.nRampFramesBlend = 16*Constants::kFactor;
                cfg.nConfirmFrames = 4*Constants::kFactor;
                cfg.subFramesPerTx = 32*Constants::kFactor;
                cfg.nDataBitsPerTx = 8*24;

                cfg.encodeIdParity = true;

                cfg.sendVolume = 0.1f;
                cfg.sendDuration_ms = 100.0f;

                cfg.freqDelta_hz =   2*cfg.getHzPerFrame();
                cfg.freqStart_hz = 52*cfg.getHzPerFrame();
                cfg.freqCheck_hz = 440*cfg.getHzPerFrame();

                break;
            default:
                break;
        };

        cfg.nConfirmFrames = std::max(1, cfg.nConfirmFrames);

        cfg.nRampFramesBegin = std::round(cfg.nRampFramesBegin);
        cfg.nRampFramesEnd = std::round(cfg.nRampFramesEnd);
        cfg.nRampFramesBlend = std::round(cfg.nRampFramesBlend);
        cfg.nConfirmFrames = std::round(cfg.nConfirmFrames);
        cfg.subFramesPerTx = std::round(cfg.subFramesPerTx);

        return cfg;
    }

}
