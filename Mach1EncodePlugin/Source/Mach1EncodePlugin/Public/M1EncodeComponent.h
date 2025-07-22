//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#pragma once

#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"

#include "M1Common.h"
#include "M1EncodeActor.h" // For enum definitions
#include "Mach1EncodeCore.h"
#include "Sound/SoundWave.h"
#include <memory>

#include "M1EncodeComponent.generated.h"

// Forward declarations for decode classes
class AM1DecodeActor;
class UM1DecodeComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MACH1ENCODEPLUGIN_API UM1EncodeComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	int MAX_INPUT_CHANNELS;
	int MAX_OUTPUT_CHANNELS;
	bool isInited;

	// Mach1 encoding core
	std::unique_ptr<M1EncodeCore> m1EncodeCore;

	void Init();
	void SetupEncoding();
	void UpdateEncodeConfiguration();
	int GetRequiredInputChannelCount();
	int GetRequiredOutputChannelCount();
	void ClearAllSounds();

	// Audio encoding helper functions
	void CollectInputSounds(TArray<USoundBase*>& InputSounds);
	USoundWave* CreateEncodedSoundWave(const TArray<USoundBase*>& InputSounds, const std::vector<std::vector<float>>& gains, int outputChannel, int inputChannelCount);

public:

	// Sets default values for this component's properties
	UM1EncodeComponent();

	void InitComponents(int32 MaxInputChannels, int32 MaxOutputChannels);

	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Called every frame
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== DECODE REFERENCE PROPERTIES ==========

	/** Reference to M1DecodeActor to automatically feed encoded output */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Decode Reference", DisplayName = "Reference Decode Actor")
		AM1DecodeActor* ReferenceDecodeActor = nullptr;

	/** Reference to M1DecodeComponent to automatically feed encoded output */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Decode Reference", DisplayName = "Reference Decode Component")
		UM1DecodeComponent* ReferenceDecodeComponent = nullptr;

	/** Automatically update referenced decoder when encoding changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Decode Reference", DisplayName = "Auto Update Decoder")
		bool AutoUpdateDecoder = true;

	// ========== ENCODE CONFIGURATION ==========

	/** Select the input format to encode from */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Encode Configuration", DisplayName = "Input Mode")
		TEnumAsByte<EMach1EncodeInputMode> InputMode = M1Input_Stereo;

	/** Select the Mach1 output format to encode to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Encode Configuration", DisplayName = "Output Mode")
		TEnumAsByte<EMach1EncodeOutputMode> OutputMode = M1Output_M1Spatial_8;

	// ========== INPUT CHANNEL ASSIGNMENTS ==========

	/** Input Channel 1 - Always required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 1")
		USoundBase* InputChannel1;

	/** Input Channel 2 - Used for Stereo and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 2", meta = (EditCondition = "InputMode >= M1Input_Stereo"))
		USoundBase* InputChannel2;

	/** Input Channel 3 - Used for LCR and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 3", meta = (EditCondition = "InputMode >= M1Input_LCR"))
		USoundBase* InputChannel3;

	/** Input Channel 4 - Used for Quad and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 4", meta = (EditCondition = "InputMode >= M1Input_Quad"))
		USoundBase* InputChannel4;

	/** Input Channel 5 - Used for LCRS and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 5", meta = (EditCondition = "InputMode >= M1Input_LCRS"))
		USoundBase* InputChannel5;

	/** Input Channel 6 - Used for 5.1 and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 6", meta = (EditCondition = "InputMode >= M1Input_FiveDotOneFilm"))
		USoundBase* InputChannel6;

	/** Input Channel 7 - Used for B-Format and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 7", meta = (EditCondition = "InputMode >= M1Input_BFOAACN"))
		USoundBase* InputChannel7;

	/** Input Channel 8 - Used for B-Format and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 8", meta = (EditCondition = "InputMode >= M1Input_BFOAACN"))
		USoundBase* InputChannel8;

	/** Input Channel 9 - Used for 2nd Order Ambisonic and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 9", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel9;

	/** Input Channel 10 - Used for 2nd Order Ambisonic and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 10", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel10;

	/** Input Channel 11 - Used for 2nd Order Ambisonic and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 11", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel11;

	/** Input Channel 12 - Used for 2nd Order Ambisonic and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 12", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel12;

	/** Input Channel 13 - Used for 2nd Order Ambisonic and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 13", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel13;

	/** Input Channel 14 - Used for 2nd Order Ambisonic and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 14", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel14;

	/** Input Channel 15 - Used for 2nd Order Ambisonic and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 15", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel15;

	/** Input Channel 16 - Used for 3rd Order Ambisonic and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 16", meta = (EditCondition = "InputMode >= M1Input_B3OAACN"))
		USoundBase* InputChannel16;

	// ========== OUTPUT STORAGE ==========

	/** Array to store the encoded output sounds */
	UPROPERTY(BlueprintReadOnly, Category = "Mach1 Output")
		TArray<USoundBase*> OutputSounds;

	// ========== BLUEPRINT CALLABLE FUNCTIONS ==========

	/** Perform the encoding operation */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void EncodeAudio();

	/** Get the encoded output as an array of sound assets */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		TArray<USoundBase*> GetEncodedOutput();

	/** Get the current number of input channels required */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		int GetCurrentInputChannelCount();

	/** Get the current number of output channels */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		int GetCurrentOutputChannelCount();

	/** Update the encoder configuration when settings change */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void RefreshEncodeConfiguration();

	/** Feed encoded output to the referenced decoder */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void FeedToReferencedDecoder();

	/** Set the reference decode actor */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void SetReferenceDecodeActor(AM1DecodeActor* DecodeActor);

	/** Set the reference decode component */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void SetReferenceDecodeComponent(UM1DecodeComponent* DecodeComponent);
}; 