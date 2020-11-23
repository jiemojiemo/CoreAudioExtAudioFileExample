
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
        FillOutASBDForLPCM (asbd,sample_rate,num_channels,32,32,true,false,false);
        asbd.mFormatFlags = kAudioFormatFlagIsBigEndian |
            kAudioFormatFlagIsSignedInteger |
            kAudioFormatFlagIsPacked;
        break;
    case kAudioFileWAVEType:
        // we write float-interleave data as default
        FillOutASBDForLPCM (asbd,sample_rate,num_channels,32,32,true,false,false);
        break;
    case kAudioFileFLACType:
        asbd.mFormatID = kAudioFormatFLAC;
        AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &asbd);
    default:
        break;
    }

    return asbd;

}

int main(int argc, char* argv[])
{
    AudioFileTypeID file_type = kAudioFileFLACType; // or kAudioFileMPEG4Type, kAudioFileAIFFType, kAudioFileWAVEType
    int o_channels = 2;
    double o_sr = 44100;

    AudioStreamBasicDescription output_asbd = createASBD(file_type, o_sr, o_channels);

    // open output file
    CFURLRef output_url = createCFURLWithStdString("sin440.flac");
    ExtAudioFileRef output_file;
    OSStatus status = ExtAudioFileCreateWithURL(output_url,file_type,
                                                &output_asbd, nullptr,
                                                kAudioFileFlags_EraseFile,
                                                &output_file);
    assert(status == noErr);
    double i_sr = 44100;
    double i_channels = 2;
    AudioStreamBasicDescription input_asbd;
    FillOutASBDForLPCM (input_asbd,i_sr,i_channels,32,32,true,false,false);
    status = ExtAudioFileSetProperty(output_file, kExtAudioFileProperty_ClientDataFormat,
                                     sizeof(input_asbd), &input_asbd);

    assert(status == noErr);

    const int num_frame_out_per_block = 1024;
    AudioBufferList outputData;
    outputData.mNumberBuffers = 1;
    outputData.mBuffers[0].mNumberChannels = i_channels;
    outputData.mBuffers[0].mDataByteSize = sizeof(float)*num_frame_out_per_block*i_channels;
    std::vector<float> buffer(num_frame_out_per_block * i_channels);
    outputData.mBuffers[0].mData = buffer.data();

    float t = 0;
    float tincr = 2 * M_PI * 440.0f / i_sr;
    for(int i = 0; i < 200; ++i){
        for(int j = 0; j < num_frame_out_per_block; ++j){
            buffer[j * i_channels] = sin(t);
            buffer[j * i_channels + 1] = buffer[j * i_channels];

            t += tincr;
        }

        // write block
        status = ExtAudioFileWrite(output_file, num_frame_out_per_block, &outputData);

        assert(status == noErr);
    }

    ExtAudioFileDispose(output_file);


    return 0;
}