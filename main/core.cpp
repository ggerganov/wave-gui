/*! \file core.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "core.h"

#include "data.h"

#include "cg_logger.h"
#include "cg_ring_buffer.h"

#include "fftw3.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include "reed-solomon/rs.hpp"

#include <cmath>
#include <thread>
#include <mutex>
#include <array>
#include <atomic>
#include <cstdlib>
#include <cinttypes>
#include <functional>

#ifdef __EMSCRIPTEN__
static bool g_isInitialized = false;
#else
static bool g_isInitialized = true;
#endif

namespace {
    constexpr float IRAND_MAX = 1.0f/RAND_MAX;
    inline float frand() { return ((float)(rand()%RAND_MAX)*IRAND_MAX); }

    inline void updateStateData(const ::Data::StateData * bsrc, ::Data::StateData * bdst) {
        bdst->nIterations = bsrc->nIterations;
        bdst->samplesPerFrame = bsrc->samplesPerFrame;
        bdst->samplesPerSubFrame = bsrc->samplesPerSubFrame;
        bdst->sendingData = bsrc->sendingData;
        bdst->receivingData = bsrc->receivingData;
        bdst->sampleAmplitude = bsrc->sampleAmplitude;
        bdst->sampleSpectrum = bsrc->sampleSpectrum;
        bdst->historySpectrumAverage = bsrc->historySpectrumAverage;
        bdst->bitAmplitude = bsrc->bitAmplitude;
        bdst->receivedData = bsrc->receivedData;
    }

    inline void addAmplitude(const ::Data::AmplitudeData & src, ::Data::AmplitudeData & dst, float scalar, int startId, int finalId) {
        for (int i = startId; i < finalId; i++) {
            dst[i] += scalar*src[i];
        }
    }

    bool initAudio(SDL_AudioDeviceID & devid_in, SDL_AudioDeviceID & devid_out) {
        CG_INFO(0, "Initializing audio I/O ...\n");

        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
            return (1);
        }

        {
            int devcount = SDL_GetNumAudioDevices(SDL_FALSE);
            for (int i = 0; i < devcount; i++) {
                CG_INFO(0, "Output  device #%d: '%s'\n", i, SDL_GetAudioDeviceName(i, SDL_FALSE));
            }
        }
        {
            int devcount = SDL_GetNumAudioDevices(SDL_TRUE);
            for (int i = 0; i < devcount; i++) {
                CG_INFO(0, "Capture device #%d: '%s'\n", i, SDL_GetAudioDeviceName(i, SDL_TRUE));
            }
        }

        SDL_AudioSpec desiredSpec;
        SDL_zero(desiredSpec);

        desiredSpec.freq = ::Data::Constants::kDefaultSamplingRate;
        desiredSpec.format = AUDIO_F32SYS;
        desiredSpec.channels = 1;
        desiredSpec.samples = 1024;
        desiredSpec.callback = NULL;

        SDL_AudioSpec obtainedSpec;
        SDL_zero(obtainedSpec);

        devid_out = SDL_OpenAudioDevice(nullptr, SDL_FALSE, &desiredSpec, &obtainedSpec, 0);
        if (!devid_out) {
            CG_FATAL(0, "Couldn't open an audio device for playback: %s!\n", SDL_GetError());
            devid_out = 0;
        } else {
            CG_INFO(0, "Obtained spec for output device (SDL Id = %d):\n", devid_out);
            CG_INFO(0, "    - Sample rate:       %d (required: %d)\n", obtainedSpec.freq, desiredSpec.freq);
            CG_INFO(0, "    - Format:            %d (required: %d)\n", obtainedSpec.format, desiredSpec.format);
            CG_INFO(0, "    - Channels:          %d (required: %d)\n", obtainedSpec.channels, desiredSpec.channels);
            CG_INFO(0, "    - Samples per frame: %d (required: %d)\n", obtainedSpec.samples, desiredSpec.samples);

            if (obtainedSpec.format != desiredSpec.format ||
                obtainedSpec.channels != desiredSpec.channels ||
                obtainedSpec.samples != desiredSpec.samples) {
                SDL_CloseAudio();
                return false;
            }
        }

        SDL_AudioSpec captureSpec;
        captureSpec = obtainedSpec;
        captureSpec.freq = ::Data::Constants::kDefaultSamplingRate;
        captureSpec.format = AUDIO_F32SYS;
        captureSpec.samples = 1024;

        devid_in = SDL_OpenAudioDevice(nullptr, SDL_TRUE, &captureSpec, &captureSpec, 0);
        if (!devid_in) {
            CG_FATAL(0, "Couldn't open an audio device for capture: %s!\n", SDL_GetError());
            devid_in = 0;
        } else {
            CG_INFO(0, "Obtained spec for input device (SDL Id = %d):\n", devid_in);
            CG_INFO(0, "    - Sample rate:       %d\n", captureSpec.freq);
            CG_INFO(0, "    - Format:            %d (required: %d)\n", captureSpec.format, desiredSpec.format);
            CG_INFO(0, "    - Channels:          %d (required: %d)\n", captureSpec.channels, desiredSpec.channels);
            CG_INFO(0, "    - Samples per frame: %d\n", captureSpec.samples);
        }

        return true;
    }
}

static std::function<bool()> g_init;

static SDL_AudioDeviceID g_devid_in = 0;
static SDL_AudioDeviceID g_devid_out = 0;

int init() {
    if (g_isInitialized) return 0;

    if (::initAudio(g_devid_in, g_devid_out) == false) return 22;

    g_isInitialized = true;
    g_init();

    return 0;
}

extern "C" {
    int doInit() { return init(); }
}

struct Core::Data {
    Data() {
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

        for (auto & s : historySpectrum) {
            s.fill(0);
        }
    }

    ~Data() {
        free();

        SDL_CloseAudio();
        SDL_Quit();
    }

    bool init() {
        if (g_isInitialized == false) {
            return false;
        }

#ifdef __EMSCRIPTEN__
        devid_in = g_devid_in;
        devid_out = g_devid_out;
#else
        if (::initAudio(devid_in, devid_out) == false) return false;
#endif

        int numChannels = 1;

        SDL_PauseAudioDevice(devid_in, SDL_FALSE);
        SDL_PauseAudioDevice(devid_out, SDL_FALSE);

        fftIn = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*samplesPerFrame);
        fftOut = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*samplesPerFrame);
        fftPlan = fftwf_plan_dft_r2c_1d(numChannels*samplesPerFrame, sampleAmplitude.data(), fftOut, FFTW_ESTIMATE);

        for (int i = 0; i < samplesPerFrame; ++i) {
            fftOut[i][0] = 0.0f;
            fftOut[i][1] = 0.0f;
        }

        CG_INFO(0, "Data successfully initialized\n");

        return true;
    }

    void free() {
        if( devid_in || devid_out ) {
            SDL_PauseAudioDevice(devid_in, SDL_TRUE);
            SDL_CloseAudioDevice(devid_in);
            SDL_PauseAudioDevice(devid_out, SDL_TRUE);
            SDL_CloseAudioDevice(devid_out);
            devid_in = 0;
            devid_out = 0;
        }

        if (fftPlan) fftwf_destroy_plan(fftPlan);
        if (fftIn) fftwf_free(fftIn);
        if (fftOut) fftwf_free(fftOut);

        fftPlan = 0;
        fftIn = 0;
        fftOut = 0;
    }

    enum BufferId {
        BUFFER_UI,
        BUFFER_CACHED,
        BUFFER_ACTIVE,
    };

    std::thread workerMain;

    bool needRecache = false;
    bool cacheUpdated = false;
    bool encodeIdParity = true;
    bool useChecksum = false;
    std::atomic<bool> isRunning;

    mutable std::mutex mutexStateData;

    std::weak_ptr<::Data::StateInput> stateInput;
    std::array<std::shared_ptr<::Data::StateData>, 3> stateData;

    CG::RingBuffer<std::function<void()>, 256> inputQueue;

    // App-specific data
    bool isInitialized = false;

    int sampleRate = 0;
    int samplesPerFrame = 1;
    int samplesPerSubFrame = 1;
    float isamplesPerFrame = isamplesPerFrame = 1.0f/samplesPerFrame;

    SDL_AudioDeviceID devid_in = 0;
    SDL_AudioDeviceID devid_out = 0;

    ::Data::AmplitudeData sampleAmplitude;
    ::Data::AmplitudeData outputBlock;
    ::Data::AmplitudeData outputBlockTmp;
    std::array<::Data::AmplitudeData, ::Data::Constants::kMaxDataBits> bitAmplitude;
    std::array<::Data::AmplitudeData, ::Data::Constants::kMaxDataBits> bit0Amplitude;
    std::array<::Data::AmplitudeData, ::Data::Constants::kMaxBitsPerChecksum> checksumAmplitude;
    std::array<::Data::AmplitudeData, ::Data::Constants::kMaxBitsPerChecksum> checksum0Amplitude;

    fftwf_plan fftPlan;
    fftwf_complex *fftIn;
    fftwf_complex *fftOut;

    float sendVolume;
    float hzPerFrame;
    float ihzPerFrame;
    ::Data::SpectrumData sampleSpectrum;
    ::Data::SpectrumData sampleSpectrumTmp;

    float freqStart_hz;
    float freqDelta_hz;
    float freqCheck_hz;

    int frameId = 0;
    int nRampFrames = 0;
    int nRampFramesBegin = 0;
    int nRampFramesEnd = 0;
    int nRampFramesBlend = 0;
    int dataId = 0;
    bool waitForNewFrame = false;
    std::array<bool, ::Data::Constants::kMaxDataBits> dataBits;
    std::array<float, ::Data::Constants::kMaxDataBits> dataFreqs_hz;
    std::array<float, ::Data::Constants::kMaxBitsPerChecksum> checksumFreqs_hz;

    int historyId = 0;
    ::Data::SpectrumData historySpectrumAverage;
    std::array<::Data::SpectrumData, ::Data::Constants::kMaxSpectrumHistory> historySpectrum;

    int sendId = 0;
    int receivedId = 0;
    int nConfirmFrames = 0;
    int subFramesPerTx;
    int curTxSubFrameId = 0;
    int nDataBitsPerTx = 0;
    int nECCBytesPerTx = 0;
    std::array<char, ::Data::Constants::kMaxDataSize> sendData;
    std::array<char, ::Data::Constants::kMaxDataSize> receivedData;

    std::shared_ptr<RS::ReedSolomon> rs = nullptr;
};

Core::Core() : _data(new Data()) {
    CG_INFO(0, "Creating Core object\n");

    _data->stateData[Data::BUFFER_UI]     = std::make_shared<::Data::StateData>();
    _data->stateData[Data::BUFFER_CACHED] = std::make_shared<::Data::StateData>();
    _data->stateData[Data::BUFFER_ACTIVE] = std::make_shared<::Data::StateData>();

    g_init = [this]() {
        this->addEvent(Core::Init);
        return true;
    };
}

Core::~Core() {
    CG_INFO(0, "Destroying Core object\n");

    if (_data->workerMain.joinable()) _data->workerMain.join();
}

void Core::init() {
    _data->isRunning = true;

    _data->workerMain = std::thread(&Core::main, this);
}

void Core::update() {
    if (_data->cacheUpdated == false) return;

    std::lock_guard<std::mutex> lock(_data->mutexStateData);

    auto & bsrc = _data->stateData[Data::BUFFER_CACHED];
    auto & bdst = _data->stateData[Data::BUFFER_UI];

    ::updateStateData(bsrc.get(), bdst.get());

    _data->cacheUpdated = false;
}

void Core::terminate() {
    _data->isRunning = false;
}

std::weak_ptr<::Data::StateData> Core::getStateData() const {
    return _data->stateData[Data::BUFFER_UI];
}

void Core::setStateInput(std::weak_ptr<::Data::StateInput> stateInput) {
    _data->stateInput = stateInput;
}

void Core::addEvent(Event event) {
    auto inp = _data->stateInput.lock();

    if (inp == nullptr) return;

    switch(event) {
        case Init:
            {
                auto sampleRate = inp->sampleRate;
                auto samplesPerFrame = inp->samplesPerFrame;
                auto samplesPerSubFrame = inp->samplesPerSubFrame;
                auto hzPerFrame = inp->getHzPerFrame();

                _data->inputQueue.push([this, sampleRate, samplesPerFrame, samplesPerSubFrame, hzPerFrame]() {
                    if (_data->isInitialized) return;

                    _data->isInitialized = false;
                    _data->needRecache = true;

                    _data->stateData[Data::BUFFER_ACTIVE]->sampleAmplitude = nullptr;
                    _data->stateData[Data::BUFFER_ACTIVE]->sampleSpectrum = nullptr;
                    _data->stateData[Data::BUFFER_ACTIVE]->historySpectrumAverage = nullptr;
                    _data->stateData[Data::BUFFER_ACTIVE]->bitAmplitude = nullptr;
                    _data->stateData[Data::BUFFER_ACTIVE]->receivedData = nullptr;

                    _data->sampleRate = sampleRate;
                    _data->samplesPerFrame = samplesPerFrame;
                    _data->samplesPerSubFrame = samplesPerSubFrame;
                    _data->isamplesPerFrame = 1.0f/samplesPerFrame;
                    _data->hzPerFrame = hzPerFrame;
                    _data->ihzPerFrame = 1.0f/hzPerFrame;

                    _data->free();
                    if (_data->init() == false) return;

                    CG_INFO(0, "Hz per frame = %4.4f\n", _data->hzPerFrame);

                    _data->isInitialized = true;
                    _data->stateData[Data::BUFFER_ACTIVE]->samplesPerFrame = _data->samplesPerFrame;
                    _data->stateData[Data::BUFFER_ACTIVE]->samplesPerSubFrame = _data->samplesPerSubFrame;
                    _data->stateData[Data::BUFFER_ACTIVE]->sampleAmplitude = &_data->sampleAmplitude;
                    _data->stateData[Data::BUFFER_ACTIVE]->sampleSpectrum = &_data->sampleSpectrum;
                    _data->stateData[Data::BUFFER_ACTIVE]->historySpectrumAverage = &_data->historySpectrumAverage;
                    _data->stateData[Data::BUFFER_ACTIVE]->bitAmplitude = &_data->bitAmplitude;
                    _data->stateData[Data::BUFFER_ACTIVE]->receivedData = &_data->receivedData;
                });
                break;
            }
        case DataOn:
            {
                auto freqStart_hz = inp->freqStart_hz;
                auto freqDelta_hz = inp->freqDelta_hz;
                auto freqCheck_hz = inp->freqCheck_hz;
                auto dataBits = inp->dataBits;
                auto nDataBitsPerTx = inp->nDataBitsPerTx;
                auto nECCBytesPerTx = inp->nECCBytesPerTx;
                auto encodeIdParity = inp->encodeIdParity;
                auto useChecksum = inp->useChecksum;

                _data->inputQueue.push([this, freqStart_hz, freqDelta_hz, freqCheck_hz, dataBits, nDataBitsPerTx,
                                       nECCBytesPerTx, encodeIdParity, useChecksum]() {
                    _data->needRecache = true;

                    _data->freqStart_hz = freqStart_hz;
                    _data->freqDelta_hz = freqDelta_hz;
                    _data->freqCheck_hz = freqCheck_hz;
                    _data->encodeIdParity = encodeIdParity;
                    _data->useChecksum = useChecksum;

                    _data->dataBits = dataBits;
                    _data->nDataBitsPerTx = nDataBitsPerTx;
                    _data->nECCBytesPerTx = nECCBytesPerTx;

                    if (nDataBitsPerTx/8 > nECCBytesPerTx && nECCBytesPerTx > 0) {
                        _data->rs = std::make_shared<RS::ReedSolomon>(nDataBitsPerTx/8 - nECCBytesPerTx, nECCBytesPerTx);
                    } else {
                        CG_WARN(0, "Not using ECC because the specified number of ECC bytes is too big for this protocol\n");
                        _data->rs.reset();
                        _data->nECCBytesPerTx = 0;
                    }

                    for (int k = 0; k < (int) _data->dataBits.size(); ++k) {
                        auto freq = freqStart_hz + freqDelta_hz*k;
                        _data->dataFreqs_hz[k] = freq;

                        float phaseOffset = 2*M_PI*::frand();
                        for (int i = 0; i < _data->samplesPerFrame; i++) {
                            _data->bitAmplitude[k][i] = std::sin((2.0*M_PI*i)*freq*_data->isamplesPerFrame*_data->ihzPerFrame + phaseOffset);
                        }
                        for (int i = 0; i < _data->samplesPerFrame; i++) {
                            _data->bit0Amplitude[k][i] = std::sin((2.0*M_PI*i)*(freq + _data->hzPerFrame)*_data->isamplesPerFrame*_data->ihzPerFrame + phaseOffset);
                        }

                        if (_data->dataBits[k] == false) continue;

                        CG_INFO(0, "\tBit %d -> %4.2f Hz\n", k, freq);
                    }

                    for (int k = 0; k < ::Data::Constants::kMaxBitsPerChecksum; ++k) {
                        _data->checksumAmplitude[k].fill(0);
                        _data->checksum0Amplitude[k].fill(0);
                    }

                    _data->frameId = 0;
                    _data->nRampFrames = _data->nRampFramesBegin;
                    _data->subFramesPerTx = 0;
                    _data->waitForNewFrame = true;

                    _data->stateData[Data::BUFFER_ACTIVE]->sendingData = true;
                    _data->stateData[Data::BUFFER_ACTIVE]->sendingDataBuffer = false;
                    ++_data->dataId;
                });
                break;
            }
        case DataOff:
            {
                CG_INFO(0, "Data OFF\n");

                _data->inputQueue.push([this]() {
                    _data->needRecache = true;

                    _data->frameId = 0;
                    _data->nRampFrames = _data->nRampFramesEnd;
                    _data->subFramesPerTx = _data->nRampFramesEnd;

                    _data->stateData[Data::BUFFER_ACTIVE]->sendingData = false;
                    _data->stateData[Data::BUFFER_ACTIVE]->sendingDataBuffer = false;
                });
                break;
            }
        case DataSend:
            {
                CG_INFO(0, "Data Send, size = %d\n", (int) strlen(inp->sendData.data()));

                auto sendData = inp->sendData;
                auto subFramesPerTx = inp->subFramesPerTx;

                _data->inputQueue.push([this, sendData, subFramesPerTx]() {
                    _data->needRecache = true;

                    _data->stateData[Data::BUFFER_ACTIVE]->sendingData = true;
                    _data->stateData[Data::BUFFER_ACTIVE]->sendingDataBuffer = true;

                    for (int k = 0; k < ::Data::Constants::kMaxBitsPerChecksum; ++k) {
                        auto freq = _data->freqCheck_hz + _data->freqDelta_hz*k;
                        _data->checksumFreqs_hz[k] = freq;

                        float phaseOffset = 2*M_PI*::frand();
                        for (int i = 0; i < _data->samplesPerFrame; i++) {
                            _data->checksumAmplitude[k][i] = std::sin((2.0*M_PI*i)*freq*_data->isamplesPerFrame*_data->ihzPerFrame + phaseOffset);
                        }
                        for (int i = 0; i < _data->samplesPerFrame; i++) {
                            _data->checksum0Amplitude[k][i] = std::sin((2.0*M_PI*i)*(freq + _data->hzPerFrame)*_data->isamplesPerFrame*_data->ihzPerFrame + phaseOffset);
                        }
                    }

                    _data->sendId = 0;
                    _data->frameId = 0;
                    _data->curTxSubFrameId = 0;
                    _data->nRampFrames = _data->nRampFramesBegin;
                    _data->waitForNewFrame = true;

                    _data->subFramesPerTx = subFramesPerTx;
                    _data->sendData = sendData;

                });
                break;
            }
        case DataClear:
            {
                CG_INFO(0, "Data Clear\n");

                _data->inputQueue.push([this]() {
                    _data->needRecache = true;

                    _data->receivedId = 0;
                    _data->receivedData.fill(0);
                });
                break;
            }
        default:
            {
                CG_FATAL(0, "Unknown event %d\n", event);
                break;
            }
    };
}

void Core::input() {
    {
        auto inp = _data->stateInput.lock();

        if (inp == nullptr) return;

        _data->sendVolume = inp->sendVolume;
        _data->nRampFramesBegin = inp->nRampFramesBegin;
        _data->nRampFramesEnd = inp->nRampFramesEnd;
        _data->nRampFramesBlend = inp->nRampFramesBlend;
        _data->nConfirmFrames = inp->nConfirmFrames;
    }

    while (_data->inputQueue.size() > 0) {
        _data->inputQueue.pop()();
    }
}

void Core::main() {
    auto data = _data->stateData[Data::BUFFER_ACTIVE];

    while (_data->isRunning) {
        input();

        // main stuff
        if (_data->isInitialized) {
            auto subFrame = data->nIterations % ::Data::Constants::kSubFrames;

            auto sampleStartId = (subFrame*_data->samplesPerSubFrame);
            auto sampleFinalId = sampleStartId + _data->samplesPerSubFrame;

            // check if receiving data
            {
                std::array<std::uint8_t, ::Data::Constants::kMaxDataBits/8> receivedData;
                static std::array<std::uint8_t, ::Data::Constants::kMaxDataBits/8> receivedDataLast;
                std::uint16_t requiredChecksum = 0;
                std::uint16_t curChecksum = 0;
                std::uint8_t curParity = 0;

                static std::uint8_t lastParity = 2;
                static std::uint16_t lastChecksum = -1;
                static std::uint16_t lastReceivedChecksum = -1;
                static std::uint16_t nTimesReceived = 0;

                if (_data->receivedId == 0) {
                    receivedDataLast.fill(0);
                    lastReceivedChecksum = 0;
                }

                receivedData.fill(0);
                requiredChecksum += 1;

                bool isValid = true;
                {
                    int bin = std::round(_data->checksumFreqs_hz[0]*_data->ihzPerFrame);
                    if (_data->historySpectrumAverage[bin] < 10*_data->historySpectrumAverage[bin - 1] &&
						_data->historySpectrumAverage[bin] < 10*_data->historySpectrumAverage[bin + 1]) {
                        if (data->receivingData == true) {
                            _data->needRecache = true;
                            data->receivingData = false;
                        }
                    } else {
                        curChecksum += 1;
                        if (data->receivingData == false) {
                            _data->needRecache = true;
                            data->receivingData = true;
                        }
                    }
                }

                for (int k = 0; k < _data->nDataBitsPerTx; ++k) {
                    int bin = std::round(_data->dataFreqs_hz[k]*_data->ihzPerFrame);
                    if (_data->historySpectrumAverage[bin] > 1.0*_data->historySpectrumAverage[bin + 1]) {
                        receivedData[k/8] += (1 << (k%8));
                    } else {
                        if (_data->useChecksum) {
                            requiredChecksum += (1 << ((k%8)+2));
                        }
                    }
                }

                for (int k = 1; k < ::Data::Constants::kMaxBitsPerChecksum; ++k) {
                    int bin = std::round(_data->checksumFreqs_hz[k]*_data->ihzPerFrame);
                    if (_data->historySpectrumAverage[bin] > 1.0*_data->historySpectrumAverage[bin + 1]) {
                        curChecksum += (1 << k);
                        if (k == 1) curParity = 1;
                    }
                }

                requiredChecksum = (requiredChecksum & ((1 << ::Data::Constants::kMaxBitsPerChecksum) - 1));

                isValid = _data->useChecksum ? (curChecksum == requiredChecksum) || (curChecksum == (requiredChecksum ^ (1 << 1))) : data->receivingData;
                bool checksumMatch = (lastChecksum == curChecksum);

                if (_data->rs) {
                    static std::array<std::uint8_t, ::Data::Constants::kMaxDataBits/8> repaired;
                    bool decoded = true;
                    if (_data->rs->Decode(receivedData.data(), repaired.data()) != 0) {
                        decoded = false;
                    } else {
                        for (int i = 0; i < _data->nDataBitsPerTx/8 - _data->nECCBytesPerTx; ++i) {
                            receivedData[i] = repaired[i];
                        }
                        receivedData[_data->nDataBitsPerTx/8 - _data->nECCBytesPerTx] = 0;
                    }
                    checksumMatch = true;
                    isValid &= decoded;
                }

                if (isValid && checksumMatch) {
                    for (int i = 0; i < _data->nDataBitsPerTx/8 - _data->nECCBytesPerTx; ++i) {
                        if (receivedData[i] == 0) receivedData[i] = ' ';
                    }
                    if (++nTimesReceived == _data->nConfirmFrames && receivedData != receivedDataLast) {
                        receivedDataLast = receivedData;
                        lastReceivedChecksum = curChecksum;

                        CG_WARN(0, "Receiving data: %s\n", receivedData.data());
                        static auto tLast = std::chrono::steady_clock::now();
                        auto tNow = std::chrono::steady_clock::now();
                        if (std::chrono::duration_cast<std::chrono::milliseconds>(tNow - tLast).count() > 500) {
                            _data->receivedId = 0;
                            _data->receivedData.fill(0);
                        } else {
                            if (curParity == lastParity && _data->receivedId > 0 && _data->encodeIdParity) {
                                _data->receivedId -= _data->nDataBitsPerTx/8 - _data->nECCBytesPerTx;
                            }
                        }
                        lastParity = curParity;
                        tLast = tNow;

                        for (int i = 0; i < _data->nDataBitsPerTx/8 - _data->nECCBytesPerTx; ++i) {
                            _data->receivedData[_data->receivedId++] = receivedData[i];
                        }
                    }
                } else if (isValid && (checksumMatch == false)) {
                    lastChecksum = curChecksum;
                    nTimesReceived = 0;
                } else if (isValid == false) {
                    lastChecksum = -1;
                    nTimesReceived = 0;
                }
            }

            // store spectrum in history
            static int nNotReceiving = 0;
            if (data->receivingData == false) ++nNotReceiving; else nNotReceiving = 0;
            if (nNotReceiving == 8*::Data::Constants::kSubFrames) {
                for (auto & s : _data->historySpectrum) {
                    s.fill(0);
                }
                _data->historySpectrumAverage.fill(0);
            }

            if (true) {
                for (int i = 0; i < _data->samplesPerFrame/2; ++i) {
                    _data->historySpectrumAverage[i] *= ::Data::Constants::kMaxSpectrumHistory;
                    _data->historySpectrumAverage[i] -= _data->historySpectrum[_data->historyId][i];
                    _data->historySpectrumAverage[i] += _data->sampleSpectrum[i];
                    _data->historySpectrumAverage[i] *= ::Data::Constants::ikMaxSpectrumHistory;
                }
                _data->historySpectrum[_data->historyId] = _data->sampleSpectrum;
                if (++_data->historyId >= ::Data::Constants::kMaxSpectrumHistory) _data->historyId = 0;
            }

            if (subFrame == 0) {
                _data->waitForNewFrame = false;
            }

            // prepare data to send
            if (data->sendingDataBuffer && !_data->waitForNewFrame) {
                if (_data->curTxSubFrameId >= _data->subFramesPerTx) {
                    _data->curTxSubFrameId = 0;
                    _data->frameId = 0;
                    _data->sendId += _data->nDataBitsPerTx/8 - _data->nECCBytesPerTx;
                } else if (_data->curTxSubFrameId >= _data->nRampFrames) {
                    _data->nRampFrames = _data->nRampFramesBlend;
                }

                if (_data->sendData[_data->sendId] == 0) {
                    data->sendingData = false;
                    data->sendingDataBuffer = false;
                    _data->nRampFrames = _data->nRampFramesEnd;
                } else {
                    _data->curTxSubFrameId = _data->frameId;

                    static std::array<std::uint8_t, ::Data::Constants::kMaxDataBits/8> encoded;
                    if (_data->rs) {
                        _data->rs->Encode(_data->sendData.data() + _data->sendId, encoded.data());
                    } else {
                        for (int j = 0; j < _data->nDataBitsPerTx/8; ++j) {
                            encoded[j] = _data->sendData[_data->sendId + j];
                        }
                    }

                    for (int j = 0; j < _data->nDataBitsPerTx/8; ++j) {
                        for (int i = 0; i < 8; ++i) {
                            _data->dataBits[j*8 + i] = encoded[j] & (1 << i);
                        }
                    }
                }
            }

            // send data
            if (data->sendingData && !_data->waitForNewFrame) {
                std::uint16_t nFreq = 0;
                std::uint16_t checksum = 0;

                checksum += (1 << 0);

                if (_data->encodeIdParity) {
                    if ((_data->dataId + _data->sendId/(_data->nDataBitsPerTx/8 - _data->nECCBytesPerTx)) & 1) {
                        checksum += (1 << 1);
                    }
                }

                for (int i = sampleStartId; i < sampleFinalId; ++i) {
                    _data->outputBlockTmp[i] = 0.0f;
                }

                for (int k = 0; k < _data->nDataBitsPerTx; ++k) {
                    ++nFreq;
                    if (_data->dataBits[k] == false) {
                        checksum += (1 << ((k%8)+2));
                        ::addAmplitude(_data->bit0Amplitude[k], _data->outputBlockTmp, _data->sendVolume, sampleStartId, sampleFinalId);
                        continue;
                    }
                    ::addAmplitude(_data->bitAmplitude[k], _data->outputBlockTmp, _data->sendVolume, sampleStartId, sampleFinalId);
                }

                if (_data->rs == nullptr) {
                    for (int k = 0; k < ::Data::Constants::kMaxBitsPerChecksum; ++k) {
                        ++nFreq;
                        if ((checksum & (1 << k)) || (k == 0)) {
                            ::addAmplitude(_data->checksumAmplitude[k], _data->outputBlockTmp, _data->sendVolume, sampleStartId, sampleFinalId);
                            continue;
                        }
                        ::addAmplitude(_data->checksum0Amplitude[k], _data->outputBlockTmp, _data->sendVolume, sampleStartId, sampleFinalId);
                    }
                } else {
                    for (int k = 0; k < 2; ++k) {
                        ++nFreq;
                        if ((checksum & (1 << k)) || (k == 0)) {
                            ::addAmplitude(_data->checksumAmplitude[k], _data->outputBlockTmp, _data->sendVolume, sampleStartId, sampleFinalId);
                            continue;
                        }
                        ::addAmplitude(_data->checksum0Amplitude[k], _data->outputBlockTmp, _data->sendVolume, sampleStartId, sampleFinalId);
                    }
                }

                if (nFreq == 0) nFreq = 1;
                float scale = 1.0f/nFreq;
                for (int i = sampleStartId; i < sampleFinalId; ++i) {
                    _data->outputBlockTmp[i] *= scale;
                }
            } else {
                for (int i = sampleStartId; i < sampleFinalId; ++i) {
                    _data->outputBlockTmp[i] = 0.0f;
                }
            }

            static double interp = 0.0;
            if (_data->frameId == 0 && _data->sendId == 0) {
                interp = 0.0f;
            }
            double dinterp = 1.0/(_data->nRampFrames*_data->samplesPerSubFrame);
            if (_data->frameId < _data->nRampFrames) {
                for (int i = sampleStartId; i < sampleFinalId; ++i) {
                    interp = std::min(1.0, interp + dinterp);
                    _data->outputBlock[i] = interp*_data->outputBlockTmp[i];
                }
            } else if (_data->subFramesPerTx > 0 && _data->frameId >= _data->subFramesPerTx - _data->nRampFrames) {
                for (int i = sampleStartId; i < sampleFinalId; ++i) {
                    interp = std::max(0.0, interp - dinterp);
                    _data->outputBlock[i] = interp*_data->outputBlockTmp[i];
                }
            } else {
                interp = 1.0;
                for (int i = sampleStartId; i < sampleFinalId; ++i) {
                    _data->outputBlock[i] = _data->outputBlockTmp[i];
                }
            }

            // read data
            int nBytesRecorded = 0;
            while (true) {
                nBytesRecorded = SDL_DequeueAudio(_data->devid_in, _data->sampleAmplitude.data() + sampleStartId, sizeof(float)*_data->samplesPerSubFrame);
                if (nBytesRecorded != 0) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            if (nBytesRecorded < 0) {
                CG_FATAL(0, "Unable to capture data\n");
                continue;
            }

            // calculate spectrum
            for (int i = sampleStartId; i < sampleFinalId; ++i) {
                _data->fftIn[i][0] = _data->sampleAmplitude[i];
                _data->fftIn[i][1] = 0;
            }

            fftwf_execute(_data->fftPlan);

            for (int i = 0; i < _data->samplesPerFrame; ++i) {
                _data->sampleSpectrumTmp[i] = (_data->fftOut[i][0]*_data->fftOut[i][0] + _data->fftOut[i][1]*_data->fftOut[i][1]);
            }
            for (int i = 1; i < _data->samplesPerFrame/2; ++i) {
                _data->sampleSpectrumTmp[i] += _data->sampleSpectrumTmp[_data->samplesPerFrame - i];
                _data->sampleSpectrumTmp[_data->samplesPerFrame - i] = 0.0f;
            }

            _data->sampleSpectrum = _data->sampleSpectrumTmp;

            if (data->sendingData) {
                if (_data->sendId < 4) {
                    SDL_PauseAudioDevice(_data->devid_out, SDL_TRUE);
                } else {
                    SDL_PauseAudioDevice(_data->devid_out, SDL_FALSE);
                }

                // write data
                int err = SDL_QueueAudio(_data->devid_out, _data->outputBlock.data() + sampleStartId, sizeof(float)*_data->samplesPerSubFrame);
                if (err) {
                    CG_FATAL(0, "Unable to write audio data\n");
                    continue;
                }
            } else {
                SDL_PauseAudioDevice(_data->devid_out, SDL_FALSE);
            }

            ++data->nIterations;
            if (!_data->waitForNewFrame) ++_data->frameId;
        }

        if ((int) SDL_GetQueuedAudioSize(_data->devid_in) > 32*sizeof(float)*_data->samplesPerFrame) {
            printf("nIter = %d, Queue size: %d\n", data->nIterations, SDL_GetQueuedAudioSize(_data->devid_in));
            SDL_ClearQueuedAudio(_data->devid_in);
        }

        cache();
    }
}

void Core::cache() {
    if (_data->needRecache == false) return;

    std::lock_guard<std::mutex> lock(_data->mutexStateData);

    auto & bsrc = _data->stateData[Data::BUFFER_ACTIVE];
    auto & bdst = _data->stateData[Data::BUFFER_CACHED];

    ::updateStateData(bsrc.get(), bdst.get());

    _data->cacheUpdated = true;
    _data->needRecache = false;
}
