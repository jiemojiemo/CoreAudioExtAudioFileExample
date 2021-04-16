
//
// Created by William.Hua on 2020/11/23.
//

#include "tool_function.h"
#include <iostream>
#include <AudioToolbox/ExtendedAudioFile.h>
#include <vector>
using namespace std;



AudioStreamBasicDescription createASBD(AudioFileTypeID file_type,
                                       double sample_rate,
                                       int num_channels){

    AudioStreamBasicDescription asbd;
    memset(&asbd, 0, sizeof(asbd));
    UInt32 size = sizeof(asbd);


    asbd.mSampleRate = sample_rate;
    asbd.mChannelsPerFrame = num_channels;

    switch (file_type) {
    case kAudioFileMPEG4Type:
        asbd.mFormatID = kAudioFormatMPEG4AAC;
        AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &asbd);
        break;
    case kAudioFileAIFFType:
        FillOutASBDForLPCM (asbd,sample_rate,num_channels,32,32,false,true,false);
        AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &asbd);
        break;
    case kAudioFileWAVEType:
        // we write float-interleave data as default
        FillOutASBDForLPCM (asbd,sample_rate,num_channels,32,32,true,false,false);
        break;
    case kAudioFileFLACType:
        asbd.mFormatID = kAudioFormatFLAC;
        AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &asbd);
        break;
    case kAudioFileMP3Type:
        asbd.mFormatID = kAudioFormatMPEGLayer3;
        AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &asbd);
        break;
    case kAudioFileAAC_ADTSType:
        asbd.mFormatID = kAudioFormatMPEG4AAC_HE_V2;
        AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &asbd);
        break;
    default:
        break;
    }

    return asbd;

}

int main(int argc, char* argv[])
{
    AudioFileTypeID file_type = kAudioFileMPEG4Type;
    int o_channels = 2;
    double o_sr = 44100;

    AudioStreamBasicDescription output_asbd = createASBD(file_type, o_sr, o_channels);

    // open output file
    CFURLRef output_url = createCFURLWithStdString("sin440_abc.aac");
    ExtAudioFileRef output_file;
    OSStatus status = ExtAudioFileCreateWithURL(output_url,file_type,
                                                &output_asbd, nullptr,
                                                kAudioFileFlags_EraseFile,
                                                &output_file);
    assert(status == noErr);
    double i_sr = 44100;
    double i_channels = 1;
    AudioStreamBasicDescription input_asbd;
    FillOutASBDForLPCM (input_asbd,i_sr,i_channels,32,32,true,false,true);
    status = ExtAudioFileSetProperty(output_file, kExtAudioFileProperty_ClientDataFormat,
                                     sizeof(input_asbd), &input_asbd);

//    Typically, when an ASBD is being used, the fields describe the complete layout
//    of the sample data in the buffers that are represented by this description -
//        where typically those buffers are represented by an AudioBuffer that is
//    contained in an AudioBufferList.
//
//        However, when an ASBD has the kAudioFormatFlagIsNonInterleaved flag, the
//    AudioBufferList has a different structure and semantic. In this case, the ASBD
//    fields will describe the format of ONE of the AudioBuffers that are contained in
//    the list, AND each AudioBuffer in the list is determined to have a single (mono)
//    channel of audio data. Then, the ASBD's mChannelsPerFrame will indicate the
//    total number of AudioBuffers that are contained within the AudioBufferList -
//        where each buffer contains one channel. This is used primarily with the
//    AudioUnit (and AudioConverter) representation of this list - and won't be found
//    in the AudioHardware usage of this structure.

    assert(status == noErr);
    const int num_frame_out_per_block = 1024;
    AudioBufferList *outputData = (AudioBufferList*)malloc(sizeof(AudioBufferList) + (sizeof(AudioBuffer) * (i_channels - 1)));

    // if input_asbd inIsNonInterleaved is true(planar data), mNumberBuffers set to number of channels
    outputData->mNumberBuffers = i_channels;
    auto* buffers = new AudioBuffer[int(i_channels)];
    for(auto i = 0; i < i_channels; ++i){
        buffers[i].mNumberChannels = 1;
        buffers[i].mDataByteSize = sizeof(float) * num_frame_out_per_block;
        buffers[i].mData = new float[num_frame_out_per_block];

        outputData->mBuffers[i] = buffers[i];
    }


    float t = 0;
    float tincr = 2 * M_PI * 440.0f / i_sr;
    for(int i = 0; i < 200; ++i){
        for(int j = 0; j < num_frame_out_per_block; ++j){
            float x = sin(t) * 0.8;
            for(int c = 0; c < i_channels; ++c){
                auto* in_channel = (float*)(buffers[c].mData);
                in_channel[j] = x;
            }

            t += tincr;
        }

        // write audio block
        status = ExtAudioFileWrite(output_file, num_frame_out_per_block, outputData);

        assert(status == noErr);
    }

    ExtAudioFileDispose(output_file);

    return 0;
}