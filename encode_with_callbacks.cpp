//
// Created by bytedance on 2021/12/2.
//
#include "file_output_stream.h"
#include <AudioToolbox/ExtendedAudioFile.h>
#include <AudioToolbox/AudioFormat.h>
#include <iostream>
#include <vector>
using namespace std;

OSStatus readCallback(
        void *		inClientData,
        SInt64		inPosition,
        UInt32		requestCount,
        void *		buffer,
        UInt32 *	actualCount)
{
    std::cout << "readCallback: pos " << inPosition << "with " << requestCount;
    auto* stream = static_cast<FileOutputStream*>(inClientData);
    stream->seekp(inPosition, SEEK_SET);
    *actualCount = stream->read((uint8_t*)buffer, requestCount);
    std::cout << " actualCount " << *actualCount << std::endl;
    return 0;
}

OSStatus writeCallback(
        void * 		inClientData,
        SInt64		inPosition,
        UInt32		requestCount,
        const void *buffer,
        UInt32    * actualCount)
{
    std::cout << "writeCallback pos " << inPosition << "with "  << requestCount << std::endl;

    auto* stream = static_cast<FileOutputStream*>(inClientData);
    stream->seekp(inPosition, SEEK_SET);
    *actualCount = stream->write((uint8_t*)(buffer), requestCount);
    return 0;
}

// returns the file size now
SInt64 getSizeCallback(
        void * 		inClientData)
{
    std::cout << "getSizeCallback: ";

    auto* stream = static_cast<FileOutputStream*>(inClientData);
    auto ret = stream->length();
    std::cout << "getSizeCallback ret:" << ret << std::endl;
    return ret;
}

// set file to inSize
OSStatus setSizeProc(
        void *		inClientData,
        SInt64		inSize)
{
    std::cout << "setSizeProc: inSize " << inSize;

    auto* stream = static_cast<FileOutputStream*>(inClientData);

    auto current_pos = stream->tellp();
    stream->seekp(stream->tellp() + inSize, SEEK_SET);
    stream->seekp(current_pos, SEEK_SET);

    std::cout << " length after set size:" << stream->length() << std::endl;

    return 0;
}

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
        default:
            break;
    }

    return asbd;

}

int main(int argc, char* argv[])
{
    AudioFileTypeID file_type = kAudioFileWAVEType;
    int o_channels = 2;
    double o_sr = 44100;

    AudioStreamBasicDescription output_asbd_ = createASBD(file_type, o_sr, o_channels);

    AudioFileID audio_file_id;
    FileOutputStream file_stream("xxx.wav");

    OSStatus status = AudioFileInitializeWithCallbacks(&file_stream,readCallback,
                                                writeCallback,
                                                getSizeCallback,
                                                setSizeProc,
                                                file_type,
                                                &output_asbd_,0,&audio_file_id);

    assert(status == noErr);
    ExtAudioFileRef output_file;
    status = ExtAudioFileWrapAudioFileID(audio_file_id, true, &output_file);
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

        // write audio block
        status = ExtAudioFileWrite(output_file, num_frame_out_per_block, &outputData);

        assert(status == noErr);
    }

    ExtAudioFileDispose(output_file);

    return 0;
}