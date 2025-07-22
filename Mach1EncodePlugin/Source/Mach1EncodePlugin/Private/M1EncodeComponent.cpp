//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#include "M1EncodeComponent.h"
#include "M1Common.h"

// Forward declare decode classes (we can't include them due to circular dependency)
// The actual linking will happen at runtime through the references

// Sets default values for this component's properties
UM1EncodeComponent::UM1EncodeComponent()
{
    // Set this component to be ticked every frame
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize with default values
    InputMode = M1Input_Stereo;
    OutputMode = M1Output_M1Spatial_8;
    
    // Initialize components
    InitComponents(16, 14); // Support up to 16 input channels and 14 output channels
}

void UM1EncodeComponent::InitComponents(int32 MaxInputChannels, int32 MaxOutputChannels)
{
    MAX_INPUT_CHANNELS = MaxInputChannels;
    MAX_OUTPUT_CHANNELS = MaxOutputChannels;
    isInited = false;
    
    // Initialize output sounds array
    OutputSounds.Empty();
}

// Called when the game starts or when spawned
void UM1EncodeComponent::BeginPlay()
{
    Super::BeginPlay();
    
    Init();
    SetupEncoding();
    UpdateEncodeConfiguration();
}

// Called every frame
void UM1EncodeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // TODO: Add any per-frame encoding logic here if needed
}

void UM1EncodeComponent::Init()
{
    // Initialize the Mach1 encoder
    M1Common::PrintDebug("Initializing Mach1 Encode Component");
    isInited = true;
}

void UM1EncodeComponent::SetupEncoding()
{
    // Setup the encoding parameters
    // TODO: Configure the M1EncodeCore with the selected input/output modes
}

void UM1EncodeComponent::UpdateEncodeConfiguration()
{
    // Update the encode configuration based on current settings
    int RequiredInputChannels = GetRequiredInputChannelCount();
    int RequiredOutputChannels = GetRequiredOutputChannelCount();
    
    M1Common::PrintDebug(TCHAR_TO_ANSI(*("Component Input channels required: " + FString::FromInt(RequiredInputChannels))));
    M1Common::PrintDebug(TCHAR_TO_ANSI(*("Component Output channels required: " + FString::FromInt(RequiredOutputChannels))));
    
    // If auto-update is enabled and we have a referenced decoder, update it
    if (AutoUpdateDecoder)
    {
        FeedToReferencedDecoder();
    }
}

int UM1EncodeComponent::GetRequiredInputChannelCount()
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

int UM1EncodeComponent::GetRequiredOutputChannelCount()
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

void UM1EncodeComponent::ClearAllSounds()
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
void UM1EncodeComponent::EncodeAudio()
{
    // Perform the encoding operation
    M1Common::PrintDebug("Component: Encoding audio with current configuration");
    
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
    
    M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component Encoding: %d input -> %d output channels"), inputChannelCount, outputChannelCount)));
    
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
            M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component: Created encoded sound for output channel %d"), outputChannel)));
        }
    }
    
    // Auto-feed to referenced decoder if enabled
    if (AutoUpdateDecoder)
    {
        FeedToReferencedDecoder();
    }
}

TArray<USoundBase*> UM1EncodeComponent::GetEncodedOutput()
{
    // Return the encoded output as an array of sound assets
    return OutputSounds;
}

int UM1EncodeComponent::GetCurrentInputChannelCount()
{
    return GetRequiredInputChannelCount();
}

int UM1EncodeComponent::GetCurrentOutputChannelCount()
{
    return GetRequiredOutputChannelCount();
}

void UM1EncodeComponent::RefreshEncodeConfiguration()
{
    UpdateEncodeConfiguration();
}

void UM1EncodeComponent::FeedToReferencedDecoder()
{
    // Feed encoded output to the referenced decoder
    if (ReferenceDecodeActor)
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
        
        // TODO: Set the decode mode and assign our output sounds to the decoder's input channels
        // This will require accessing the decode actor's properties through reflection or direct access
        M1Common::PrintDebug(TCHAR_TO_ANSI(*("Setting decode mode to: " + FString::FromInt(DecodeMode))));
    }
    
    if (ReferenceDecodeComponent)
    {
        M1Common::PrintDebug("Feeding encoded output to referenced M1DecodeComponent");
        
        // Similar logic for component reference
        // TODO: Implement component-to-component data transfer
    }
}

void UM1EncodeComponent::SetReferenceDecodeActor(AM1DecodeActor* DecodeActor)
{
    ReferenceDecodeActor = DecodeActor;
    M1Common::PrintDebug("Reference decode actor set");
    
    // Auto-feed if enabled
    if (AutoUpdateDecoder)
    {
        FeedToReferencedDecoder();
    }
}

void UM1EncodeComponent::SetReferenceDecodeComponent(UM1DecodeComponent* DecodeComponent)
{
    ReferenceDecodeComponent = DecodeComponent;
    M1Common::PrintDebug("Reference decode component set");
    
    // Auto-feed if enabled
    if (AutoUpdateDecoder)
    {
        FeedToReferencedDecoder();
    }
}

void UM1EncodeComponent::CollectInputSounds(TArray<USoundBase*>& InputSounds)
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

USoundWave* UM1EncodeComponent::CreateEncodedSoundWave(const TArray<USoundBase*>& InputSounds, const std::vector<std::vector<float>>& gains, int outputChannel, int inputChannelCount)
{
    if (outputChannel >= (int)gains.size())
    {
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component Warning: Output channel %d exceeds gains array size %d"), outputChannel, (int)gains.size())));
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
    
    M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component: Creating encoded sound for output channel %d with %d input channels"), outputChannel, inputChannelCount)));
    
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
                
                M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component Input channel %d: %d samples, gain %.3f"), inputChannel, SampleCount, gain)));
                
                InputSoundWave->RawData.Unlock();
            }
            else
            {
                InputAudioData.Add(nullptr);
                InputSampleCounts.Add(0);
                InputGains.Add(0.0f);
                M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component Input channel %d: No raw data available"), inputChannel)));
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
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component: No valid input audio data for output channel %d"), outputChannel)));
        
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
    
    M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component: Mixing %d samples for output channel %d"), MaxSampleCount, outputChannel)));
    
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
            
            M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component: Mixed input channel %d with gain %.3f"), inputChannel, gain)));
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
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component: Applied normalization gain %.3f to prevent clipping"), NormalizationGain)));
    }
    
    // Create new SoundWave with mixed audio data
    USoundWave* EncodedSoundWave = NewObject<USoundWave>(GetOwner());
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
        
        M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component: Created encoded SoundWave: %.2fs duration, %d Hz, %d samples"), 
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