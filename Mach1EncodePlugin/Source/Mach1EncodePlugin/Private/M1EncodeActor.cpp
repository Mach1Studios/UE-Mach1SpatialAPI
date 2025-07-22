//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#include "M1EncodeActor.h"
#include "M1Common.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"

// Sets default values
AM1EncodeActor::AM1EncodeActor()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize with default values
    InputMode = M1Input_Stereo;
    OutputMode = M1Output_M1Spatial_8;
    AutoUpdateDecoder = true;
    
    // Initialize components
    InitComponents(16, 14); // Support up to 16 input channels and 14 output channels
}

// Called when the game starts or when spawned
void AM1EncodeActor::BeginPlay()
{
    Super::BeginPlay();
    
    Init();
    SetupEncoding();
    UpdateEncodeConfiguration();
}

// Called every frame
void AM1EncodeActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // TODO: Add any per-frame encoding logic here if needed
}

void AM1EncodeActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // Refresh configuration when properties change in editor
    if (PropertyChangedEvent.Property != nullptr)
    {
        FName PropertyName = PropertyChangedEvent.Property->GetFName();
        if (PropertyName == GET_MEMBER_NAME_CHECKED(AM1EncodeActor, InputMode) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AM1EncodeActor, OutputMode))
        {
            UpdateEncodeConfiguration();
        }
    }
}

void AM1EncodeActor::Init()
{
    // Initialize the Mach1 encoder
    M1Common::PrintDebug("Initializing Mach1 Encode Actor");
}

void AM1EncodeActor::SetupEncoding()
{
    // Setup the encoding parameters
    // TODO: Configure the M1EncodeCore with the selected input/output modes
}

void AM1EncodeActor::InitComponents(int32 MaxInputChannels, int32 MaxOutputChannels)
{
    // Initialize audio components for input and output channels
    // TODO: Create audio components for handling input and output audio
}

void AM1EncodeActor::UpdateEncodeConfiguration()
{
    // Update the encode configuration based on current settings
    int RequiredInputChannels = GetRequiredInputChannelCount();
    int RequiredOutputChannels = GetRequiredOutputChannelCount();
    
    M1Common::PrintDebug(TCHAR_TO_ANSI(*("Input channels required: " + FString::FromInt(RequiredInputChannels))));
    M1Common::PrintDebug(TCHAR_TO_ANSI(*("Output channels required: " + FString::FromInt(RequiredOutputChannels))));
    
    // If auto-update is enabled and we have a referenced decoder, update it
    if (AutoUpdateDecoder)
    {
        FeedToReferencedDecoder();
    }
}

int AM1EncodeActor::GetRequiredInputChannelCount()
{
    // Return the number of input channels required based on input mode
    switch (InputMode)
    {
        case M1Input_Mono:
            return 1;
        case M1Input_Stereo:
            return 2;
        case M1Input_LCR:
            return 3;
        case M1Input_Quad:
        case M1Input_AFormat:
            return 4;
        case M1Input_LCRS:
            return 5;
        case M1Input_FiveDotZero:
            return 5;
        case M1Input_FiveDotOneFilm:
        case M1Input_FiveDotOneDTS:
        case M1Input_FiveDotOneSMTPE:
            return 6;
        case M1Input_BFOAACN:
        case M1Input_BFOAFUMA:
            return 4;
        case M1Input_B2OAACN:
        case M1Input_B2OAFUMA:
            return 9;
        case M1Input_B3OAACN:
        case M1Input_B3OAFUMA:
            return 16;
        default:
            return 2;
    }
}

int AM1EncodeActor::GetRequiredOutputChannelCount()
{
    // Return the number of output channels based on output mode
    switch (OutputMode)
    {
        case M1Output_M1Spatial_4:
            return 4;
        case M1Output_M1Spatial_8:
            return 8;
        case M1Output_M1Spatial_14:
            return 14;
        default:
            return 8;
    }
}

void AM1EncodeActor::ClearAllSounds()
{
    // Clear all input channel sound assignments
    InputChannel1 = nullptr;
    InputChannel2 = nullptr;
    InputChannel3 = nullptr;
    InputChannel4 = nullptr;
    InputChannel5 = nullptr;
    InputChannel6 = nullptr;
    InputChannel7 = nullptr;
    InputChannel8 = nullptr;
    InputChannel9 = nullptr;
    InputChannel10 = nullptr;
    InputChannel11 = nullptr;
    InputChannel12 = nullptr;
    InputChannel13 = nullptr;
    InputChannel14 = nullptr;
    InputChannel15 = nullptr;
    InputChannel16 = nullptr;
}

// Blueprint callable functions
void AM1EncodeActor::EncodeAudio()
{
    // Perform the encoding operation
    M1Common::PrintDebug("Encoding audio with current configuration");
    
    // Initialize M1EncodeCore if not already done
    if (!m1EncodeCore)
    {
        m1EncodeCore = std::make_unique<M1EncodeCore>();
        
        // Configure the encoder based on current settings
        m1EncodeCore->setInputMode(static_cast<InputMode>(InputMode));
        m1EncodeCore->setOutputMode(static_cast<OutputMode>(OutputMode));
    }
    
    // Generate encoding coefficients
    m1EncodeCore->generatePointResults();
    
    // Get the gain coefficients for each output channel
    std::vector<std::vector<float>> gains = m1EncodeCore->resultingPoints.getGains();
    int outputChannelCount = GetRequiredOutputChannelCount();
    int inputChannelCount = GetRequiredInputChannelCount();
    
    M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Encoding: %d input -> %d output channels"), inputChannelCount, outputChannelCount)));
    
    // Clear existing output sounds
    OutputSounds.Empty();
    OutputSounds.SetNum(outputChannelCount);
    
    // Get input sounds array
    TArray<USoundBase*> InputSounds;
    CollectInputSounds(InputSounds);
    
    // For each output channel, create a sound wave with encoded audio
    for (int outputChannel = 0; outputChannel < outputChannelCount; outputChannel++)
    {
        // Create a new SoundWave for this output channel
        USoundWave* EncodedSoundWave = CreateEncodedSoundWave(InputSounds, gains, outputChannel, inputChannelCount);
        
        if (EncodedSoundWave)
        {
            OutputSounds[outputChannel] = EncodedSoundWave;
            M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Created encoded sound for output channel %d"), outputChannel)));
        }
    }
    
    // Auto-feed to referenced decoder if enabled
    if (AutoUpdateDecoder)
    {
        FeedToReferencedDecoder();
    }
}

TArray<USoundBase*> AM1EncodeActor::GetEncodedOutput()
{
    // Return the encoded output as an array of sound assets
    TArray<USoundBase*> LocalOutputSounds;
    
    // TODO: Implement actual encoded output retrieval
    // For now, return empty array
    
    return LocalOutputSounds;
}

int AM1EncodeActor::GetCurrentInputChannelCount()
{
    return GetRequiredInputChannelCount();
}

int AM1EncodeActor::GetCurrentOutputChannelCount()
{
    return GetRequiredOutputChannelCount();
}

void AM1EncodeActor::RefreshEncodeConfiguration()
{
    UpdateEncodeConfiguration();
}

void AM1EncodeActor::CollectInputSounds(TArray<USoundBase*>& InputSounds)
{
    // Collect all input channel sounds into an array
    InputSounds.Empty();
    
    TArray<USoundBase*> AllChannels = {
        InputChannel1, InputChannel2, InputChannel3, InputChannel4,
        InputChannel5, InputChannel6, InputChannel7, InputChannel8,
        InputChannel9, InputChannel10, InputChannel11, InputChannel12,
        InputChannel13, InputChannel14, InputChannel15, InputChannel16
    };
    
    int requiredChannels = GetRequiredInputChannelCount();
    for (int i = 0; i < requiredChannels && i < AllChannels.Num(); i++)
    {
        InputSounds.Add(AllChannels[i]);
    }
}

USoundWave* AM1EncodeActor::CreateEncodedSoundWave(const TArray<USoundBase*>& InputSounds, const std::vector<std::vector<float>>& gains, int outputChannel, int inputChannelCount)
{
    if (outputChannel >= (int)gains.size())
    {
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Warning: Output channel %d exceeds gains array size %d"), outputChannel, (int)gains.size())));
        return nullptr;
    }
    
    // Get gain coefficients for this output channel
    const std::vector<float>& channelGains = gains[outputChannel];
    
    // Find the longest input sound to determine output length
    int32 MaxSampleCount = 0;
    int32 SampleRate = 44100; // Default sample rate
    TArray<const float*> InputAudioData;
    TArray<int32> InputSampleCounts;
    TArray<float> InputGains;
    
    M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Creating encoded sound for output channel %d with %d input channels"), outputChannel, inputChannelCount)));
    
    // Load audio data from each input sound and prepare for mixing
    for (int inputChannel = 0; inputChannel < inputChannelCount && inputChannel < InputSounds.Num(); inputChannel++)
    {
        USoundWave* InputSoundWave = Cast<USoundWave>(InputSounds[inputChannel]);
        if (InputSoundWave && inputChannel < channelGains.size())
        {
            float gain = channelGains[inputChannel];
            
            // Skip sounds with very low gain to optimize performance
            if (FMath::Abs(gain) < 0.001f)
            {
                InputAudioData.Add(nullptr);
                InputSampleCounts.Add(0);
                InputGains.Add(0.0f);
                continue;
            }
            
            // Load raw audio data from the SoundWave
            const uint8* RawData = nullptr;
            int32 RawDataSize = 0;
            
            // Get raw audio data
            if (InputSoundWave->RawData.GetBulkDataSize() > 0)
            {
                RawData = (const uint8*)InputSoundWave->RawData.LockReadOnly();
                RawDataSize = InputSoundWave->RawData.GetBulkDataSize();
                
                // Calculate sample count based on format
                int32 BytesPerSample = InputSoundWave->NumChannels * sizeof(int16); // Assuming 16-bit audio
                int32 SampleCount = RawDataSize / BytesPerSample;
                
                // Convert to float array (simplified - assumes 16-bit PCM)
                float* FloatData = new float[SampleCount];
                const int16* IntData = (const int16*)RawData;
                
                for (int32 i = 0; i < SampleCount; i++)
                {
                    FloatData[i] = (float)IntData[i] / 32768.0f; // Convert 16-bit to float
                }
                
                InputAudioData.Add(FloatData);
                InputSampleCounts.Add(SampleCount);
                InputGains.Add(gain);
                
                // Update max sample count and sample rate
                MaxSampleCount = FMath::Max(MaxSampleCount, SampleCount);
                SampleRate = InputSoundWave->GetSampleRateForCurrentPlatform();
                
                M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Input channel %d: %d samples, gain %.3f"), inputChannel, SampleCount, gain)));
                
                InputSoundWave->RawData.Unlock();
            }
            else
            {
                InputAudioData.Add(nullptr);
                InputSampleCounts.Add(0);
                InputGains.Add(0.0f);
                M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Input channel %d: No raw data available"), inputChannel)));
            }
        }
        else
        {
            InputAudioData.Add(nullptr);
            InputSampleCounts.Add(0);
            InputGains.Add(0.0f);
        }
    }
    
    // If no valid input data, return null
    if (MaxSampleCount == 0)
    {
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("No valid input audio data for output channel %d"), outputChannel)));
        
        // Clean up allocated memory
        for (int i = 0; i < InputAudioData.Num(); i++)
        {
            if (InputAudioData[i])
            {
                delete[] InputAudioData[i];
            }
        }
        return nullptr;
    }
    
    // Create mixed output buffer
    float* MixedAudioData = new float[MaxSampleCount];
    FMemory::Memzero(MixedAudioData, MaxSampleCount * sizeof(float));
    
    M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Mixing %d samples for output channel %d"), MaxSampleCount, outputChannel)));
    
    // Mix all input channels with their respective gains
    for (int inputChannel = 0; inputChannel < InputAudioData.Num(); inputChannel++)
    {
        if (InputAudioData[inputChannel] && InputGains[inputChannel] != 0.0f)
        {
            int32 inputSamples = InputSampleCounts[inputChannel];
            float gain = InputGains[inputChannel];
            
            for (int32 sampleIndex = 0; sampleIndex < FMath::Min(inputSamples, MaxSampleCount); sampleIndex++)
            {
                MixedAudioData[sampleIndex] += InputAudioData[inputChannel][sampleIndex] * gain;
            }
            
            M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Mixed input channel %d with gain %.3f"), inputChannel, gain)));
        }
    }
    
    // Apply output gain normalization to prevent clipping
    float MaxAmplitude = 0.0f;
    for (int32 i = 0; i < MaxSampleCount; i++)
    {
        MaxAmplitude = FMath::Max(MaxAmplitude, FMath::Abs(MixedAudioData[i]));
    }
    
    if (MaxAmplitude > 1.0f)
    {
        float NormalizationGain = 0.95f / MaxAmplitude; // Leave some headroom
        for (int32 i = 0; i < MaxSampleCount; i++)
        {
            MixedAudioData[i] *= NormalizationGain;
        }
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Applied normalization gain %.3f to prevent clipping"), NormalizationGain)));
    }
    
    // Create new SoundWave with mixed audio data
    USoundWave* EncodedSoundWave = NewObject<USoundWave>(this);
    if (EncodedSoundWave)
    {
        // Set audio properties
        EncodedSoundWave->SetSampleRate(SampleRate);
        EncodedSoundWave->NumChannels = 1; // Output is mono
        EncodedSoundWave->Duration = (float)MaxSampleCount / (float)SampleRate;
        EncodedSoundWave->bLooping = false;
        
        // Convert float data back to 16-bit PCM
        int32 OutputDataSize = MaxSampleCount * sizeof(int16);
        int16* OutputPCMData = new int16[MaxSampleCount];
        
        for (int32 i = 0; i < MaxSampleCount; i++)
        {
            // Clamp and convert float to 16-bit
            float ClampedSample = FMath::Clamp(MixedAudioData[i], -1.0f, 1.0f);
            OutputPCMData[i] = (int16)(ClampedSample * 32767.0f);
        }
        
        // Set raw audio data
        EncodedSoundWave->RawData.Lock(LOCK_READ_WRITE);
        void* LockedData = EncodedSoundWave->RawData.Realloc(OutputDataSize);
        FMemory::Memcpy(LockedData, OutputPCMData, OutputDataSize);
        EncodedSoundWave->RawData.Unlock();
        
        // Cleanup
        delete[] OutputPCMData;
        
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Created encoded SoundWave: %.2fs duration, %d Hz, %d samples"), 
            EncodedSoundWave->Duration, SampleRate, MaxSampleCount)));
    }
    
    // Clean up allocated memory
    delete[] MixedAudioData;
    for (int i = 0; i < InputAudioData.Num(); i++)
    {
        if (InputAudioData[i])
        {
            delete[] InputAudioData[i];
        }
    }
    
    return EncodedSoundWave;
}

void AM1EncodeActor::FeedToReferencedDecoder()
{
    // Feed encoded output to the referenced decoder
    if (ReferenceDecodeActor && OutputSounds.Num() > 0)
    {
        M1Common::PrintDebug("Feeding encoded output to referenced M1DecodeActor");
        
        // Map our output mode to corresponding decode mode
        int DecodeMode = 1; // Default to 8-channel
        switch (OutputMode)
        {
            case M1Output_M1Spatial_4:
                DecodeMode = 0; // Mach1DecodeMode_Spatial_4
                break;
            case M1Output_M1Spatial_8:
                DecodeMode = 1; // Mach1DecodeMode_Spatial_8
                break;
            case M1Output_M1Spatial_14:
                DecodeMode = 2; // Mach1DecodeMode_Spatial_14
                break;
        }
        
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Setting decode mode to: %d, feeding %d output sounds"), DecodeMode, OutputSounds.Num())));
        
        // Set the decode mode using reflection to avoid circular dependency
        UProperty* DecodeModeProperty = ReferenceDecodeActor->GetClass()->FindPropertyByName(TEXT("DecodeMode"));
        if (DecodeModeProperty)
        {
            DecodeModeProperty->SetValue_InContainer(ReferenceDecodeActor, DecodeMode);
        }
        
        // Set input mode to individual mono channels
        UProperty* InputModeProperty = ReferenceDecodeActor->GetClass()->FindPropertyByName(TEXT("InputMode"));
        if (InputModeProperty)
        {
            InputModeProperty->SetValue_InContainer(ReferenceDecodeActor, 0); // IndividualMonoChannels
        }
        
        // Assign our output sounds to the decoder's input channels
        TArray<FString> ChannelNames = {
            TEXT("Channel1"), TEXT("Channel2"), TEXT("Channel3"), TEXT("Channel4"),
            TEXT("Channel5"), TEXT("Channel6"), TEXT("Channel7"), TEXT("Channel8"),
            TEXT("Channel9"), TEXT("Channel10"), TEXT("Channel11"), TEXT("Channel12"),
            TEXT("Channel13"), TEXT("Channel14")
        };
        
        for (int i = 0; i < OutputSounds.Num() && i < ChannelNames.Num(); i++)
        {
            UProperty* ChannelProperty = ReferenceDecodeActor->GetClass()->FindPropertyByName(*ChannelNames[i]);
            if (ChannelProperty && OutputSounds[i])
            {
                ChannelProperty->SetValue_InContainer(ReferenceDecodeActor, OutputSounds[i]);
                M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Assigned output sound %d to decoder %s"), i, *ChannelNames[i])));
            }
        }
        
        // Trigger decoder refresh
        if (ReferenceDecodeActor->GetClass()->FindFunction(TEXT("RefreshDecodeConfiguration")))
        {
            ReferenceDecodeActor->ProcessEvent(ReferenceDecodeActor->GetClass()->FindFunction(TEXT("RefreshDecodeConfiguration")), nullptr);
            M1Common::PrintDebug("Triggered decoder configuration refresh");
        }
    }
    
    if (ReferenceDecodeComponent && OutputSounds.Num() > 0)
    {
        M1Common::PrintDebug("Feeding encoded output to referenced M1DecodeComponent");
        
        // Similar logic for component reference using reflection
        // Map output mode to decode mode
        int DecodeMode = 1; // Default to 8-channel
        switch (OutputMode)
        {
            case M1Output_M1Spatial_4:
                DecodeMode = 0; // Mach1DecodeMode_Spatial_4_Component
                break;
            case M1Output_M1Spatial_8:
                DecodeMode = 1; // Mach1DecodeMode_Spatial_8_Component
                break;
            case M1Output_M1Spatial_12:
            case M1Output_M1Spatial_14:
                DecodeMode = 2; // Mach1DecodeMode_Spatial_14_Component
                break;
        }
        
        // Set properties using reflection
        UProperty* DecodeModeProperty = ReferenceDecodeComponent->GetClass()->FindPropertyByName(TEXT("DecodeMode"));
        if (DecodeModeProperty)
        {
            DecodeModeProperty->SetValue_InContainer(ReferenceDecodeComponent, DecodeMode);
        }
        
        UProperty* InputModeProperty = ReferenceDecodeComponent->GetClass()->FindPropertyByName(TEXT("InputMode"));
        if (InputModeProperty)
        {
            InputModeProperty->SetValue_InContainer(ReferenceDecodeComponent, 0); // IndividualMonoChannels_Component
        }
        
        // Assign output sounds to component channels
        TArray<FString> ChannelNames = {
            TEXT("Channel1"), TEXT("Channel2"), TEXT("Channel3"), TEXT("Channel4"),
            TEXT("Channel5"), TEXT("Channel6"), TEXT("Channel7"), TEXT("Channel8"),
            TEXT("Channel9"), TEXT("Channel10"), TEXT("Channel11"), TEXT("Channel12"),
            TEXT("Channel13"), TEXT("Channel14")
        };
        
        for (int i = 0; i < OutputSounds.Num() && i < ChannelNames.Num(); i++)
        {
            UProperty* ChannelProperty = ReferenceDecodeComponent->GetClass()->FindPropertyByName(*ChannelNames[i]);
            if (ChannelProperty && OutputSounds[i])
            {
                ChannelProperty->SetValue_InContainer(ReferenceDecodeComponent, OutputSounds[i]);
            }
        }
        
        // Trigger component refresh
        if (ReferenceDecodeComponent->GetClass()->FindFunction(TEXT("RefreshDecodeConfiguration")))
        {
            ReferenceDecodeComponent->ProcessEvent(ReferenceDecodeComponent->GetClass()->FindFunction(TEXT("RefreshDecodeConfiguration")), nullptr);
            M1Common::PrintDebug("Triggered decode component configuration refresh");
        }
    }
}

void AM1EncodeActor::SetReferenceDecodeActor(AM1DecodeActor* DecodeActor)
{
    ReferenceDecodeActor = DecodeActor;
    M1Common::PrintDebug("Reference decode actor set");
    
    // Auto-feed if enabled
    if (AutoUpdateDecoder)
    {
        FeedToReferencedDecoder();
    }
}

void AM1EncodeActor::SetReferenceDecodeComponent(UM1DecodeComponent* DecodeComponent)
{
    ReferenceDecodeComponent = DecodeComponent;
    M1Common::PrintDebug("Reference decode component set");
    
    // Auto-feed if enabled
    if (AutoUpdateDecoder)
    {
        FeedToReferencedDecoder();
    }
}

void AM1EncodeActor::EnableStreamingMode(bool bEnable)
{
    bStreamingMode = bEnable;
    
    if (bStreamingMode)
    {
        // Initialize streaming buffers
        int inputChannelCount = GetRequiredInputChannelCount();
        int outputChannelCount = GetRequiredOutputChannelCount();
        
        InputAudioBuffers.SetNum(inputChannelCount);
        for (int i = 0; i < inputChannelCount; i++)
        {
            InputAudioBuffers[i].SetNum(1024); // Default buffer size
            InputAudioBuffers[i].Init(0.0f, 1024);
        }
        
        MixedOutputBuffer.SetNum(1024);
        MixedOutputBuffer.Init(0.0f, 1024);
        
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Streaming mode enabled: %d input channels, %d output channels"), inputChannelCount, outputChannelCount)));
    }
    else
    {
        InputAudioBuffers.Empty();
        MixedOutputBuffer.Empty();
        M1Common::PrintDebug("Streaming mode disabled");
    }
}

void AM1EncodeActor::ProcessAudioBuffer(const TArray<float>& InputBuffer, int InputChannelIndex, TArray<float>& OutputBuffer)
{
    if (!bStreamingMode || !m1EncodeCore)
    {
        OutputBuffer.Init(0.0f, InputBuffer.Num());
        return;
    }
    
    // Update the input buffer for this channel
    if (InputChannelIndex >= 0 && InputChannelIndex < InputAudioBuffers.Num())
    {
        int BufferSize = FMath::Min(InputBuffer.Num(), InputAudioBuffers[InputChannelIndex].Num());
        for (int i = 0; i < BufferSize; i++)
        {
            InputAudioBuffers[InputChannelIndex][i] = InputBuffer[i];
        }
    }
    
    // Generate encoding coefficients
    m1EncodeCore->generatePointResults();
    std::vector<std::vector<float>> gains = m1EncodeCore->resultingPoints.getGains();
    
    // Ensure output buffer is properly sized
    int outputChannelCount = GetRequiredOutputChannelCount();
    OutputBuffer.SetNum(InputBuffer.Num());
    OutputBuffer.Init(0.0f, InputBuffer.Num());
    
    // For real-time processing, mix all input channels for each output channel
    for (int outputChannel = 0; outputChannel < outputChannelCount; outputChannel++)
    {
        if (outputChannel < gains.size())
        {
            const std::vector<float>& channelGains = gains[outputChannel];
            
            // Mix input channels for this output channel
            for (int sampleIndex = 0; sampleIndex < InputBuffer.Num(); sampleIndex++)
            {
                float mixedSample = 0.0f;
                
                for (int inputChannel = 0; inputChannel < InputAudioBuffers.Num() && inputChannel < channelGains.size(); inputChannel++)
                {
                    if (sampleIndex < InputAudioBuffers[inputChannel].Num())
                    {
                        mixedSample += InputAudioBuffers[inputChannel][sampleIndex] * channelGains[inputChannel];
                    }
                }
                
                // For demonstration, return the first output channel
                if (outputChannel == 0 && sampleIndex < OutputBuffer.Num())
                {
                    OutputBuffer[sampleIndex] = FMath::Clamp(mixedSample, -1.0f, 1.0f);
                }
            }
        }
    }
}

void AM1EncodeActor::SetSpatialPosition(float Azimuth, float Elevation, float Diverge)
{
    if (m1EncodeCore)
    {
        m1EncodeCore->setAzimuthDegrees(Azimuth);
        m1EncodeCore->setElevationDegrees(Elevation);
        m1EncodeCore->setDiverge(Diverge);
        
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Spatial position updated: Az=%.1f°, El=%.1f°, Div=%.2f"), Azimuth, Elevation, Diverge)));
        
        // Auto-feed to decoder if enabled
        if (AutoUpdateDecoder && bStreamingMode)
        {
            FeedToReferencedDecoder();
        }
    }
} 