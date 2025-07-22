//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h" 
#include "Components/BillboardComponent.h"
#include "Components/SceneCaptureComponent.h"
#include "Camera/CameraComponent.h"

#include "M1Common.h"
#include "Mach1Encode.h"
#include "Mach1EncodeCAPI.h"
#include "Mach1EncodeCore.h"
#include "Sound/SoundWave.h"

#include <vector>
#include <memory>

#include "M1EncodeActor.generated.h"

class ACameraActor;

// Mach1 Encode Input Modes (maps to Mach1EncodeInputMode enum from API)
UENUM(BlueprintType)
enum EMach1EncodeInputMode
{
	M1Input_Mono = 0 			UMETA(DisplayName = "Mono"),
	M1Input_Stereo = 1 			UMETA(DisplayName = "Stereo"),
	M1Input_LCR = 2 			UMETA(DisplayName = "LCR"),
	M1Input_Quad = 3 			UMETA(DisplayName = "Quad"),
	M1Input_LCRS = 4 			UMETA(DisplayName = "LCRS"),
	M1Input_AFormat = 5 		UMETA(DisplayName = "A-Format"),
	M1Input_FiveDotZero = 6 	UMETA(DisplayName = "5.0"),
	M1Input_FiveDotOneFilm = 7 	UMETA(DisplayName = "5.1 Film"),
	M1Input_FiveDotOneDTS = 8 	UMETA(DisplayName = "5.1 DTS"),
	M1Input_FiveDotOneSMTPE = 9 UMETA(DisplayName = "5.1 SMTPE"),
	M1Input_BFOAACN = 10 		UMETA(DisplayName = "B-Format ACN"),
	M1Input_BFOAFUMA = 11 		UMETA(DisplayName = "B-Format FUMA"),
	M1Input_B2OAACN = 12 		UMETA(DisplayName = "2nd Order ACN"),
	M1Input_B2OAFUMA = 13 		UMETA(DisplayName = "2nd Order FUMA"),
	M1Input_B3OAACN = 14 		UMETA(DisplayName = "3rd Order ACN"),
	M1Input_B3OAFUMA = 15 		UMETA(DisplayName = "3rd Order FUMA")
};

// Mach1 Encode Output Modes (maps to Mach1EncodeOutputMode enum from API)
UENUM(BlueprintType)
enum EMach1EncodeOutputMode
{
	M1Output_M1Spatial_4 = 0 	UMETA(DisplayName = "Mach1 Spatial 4-Channel"),
    M1Output_M1Spatial_8 = 1 	UMETA(DisplayName = "Mach1 Spatial 8-Channel"),
    M1Output_M1Spatial_12 = 2 	UMETA(DisplayName = "Mach1 Spatial 12-Channel"),
    M1Output_M1Spatial_14 = 3 	UMETA(DisplayName = "Mach1 Spatial 14-Channel")
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MACH1ENCODEPLUGIN_API AM1EncodeActor : public AActor
{
	GENERATED_BODY()

protected:

	static FVector GetEuler(FQuat q1);

	// Calculated gain coefficients from the encode algorithm
	TArray<TArray<float>> GainCoeffs;

	TArray<USoundBase*> InputSounds;
	TArray<USoundBase*> OutputSounds;

	USceneComponent* Root;
	UBoxComponent* Collision;
	UBillboardComponent* Billboard;

	int MAX_INPUT_CHANNELS;
	int MAX_OUTPUT_CHANNELS;
	bool isInited;

	// Mach1 encoding core
	std::unique_ptr<M1EncodeCore> m1EncodeCore;

	// Real-time audio processing buffers
	TArray<TArray<float>> InputAudioBuffers;
	TArray<float> MixedOutputBuffer;
	bool bStreamingMode = false;

	void Init();
	void SetupEncoding();

	// New flexible methods
	void UpdateEncodeConfiguration();
	int GetRequiredInputChannelCount();
	int GetRequiredOutputChannelCount();
	void ClearAllSounds();

	// Audio encoding helper functions
	void CollectInputSounds(TArray<USoundBase*>& InputSounds);
	USoundWave* CreateEncodedSoundWave(const TArray<USoundBase*>& InputSounds, const std::vector<std::vector<float>>& gains, int outputChannel, int inputChannelCount);
	USoundBase* GetInputChannelByIndex(int index);

	void* M1obj; // Mach1Encode object

public:

	// Sets default values for this actor's properties
	AM1EncodeActor();

	void InitComponents(int32 MaxInputChannels, int32 MaxOutputChannels);

	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Called every frame
	void Tick(float DeltaSeconds) override;

	// always tick
	bool ShouldTickIfViewportsOnly() const override { return true; }
	#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Display Debug")
		bool Debug = false;

	// ========== DECODE REFERENCE PROPERTIES ==========

	/** Reference to M1DecodeActor to automatically feed encoded output */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Decode Reference", DisplayName = "Reference Decode Actor")
		class AM1DecodeActor* ReferenceDecodeActor = nullptr;

	/** Reference to M1DecodeComponent to automatically feed encoded output */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Decode Reference", DisplayName = "Reference Decode Component")
		class UM1DecodeComponent* ReferenceDecodeComponent = nullptr;

	/** Automatically update referenced decoder when encoding changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Decode Reference", DisplayName = "Auto Update Decoder")
		bool AutoUpdateDecoder = true;

	// ========== FLEXIBLE ENCODE CONFIGURATION ==========

	/** Select the input format to encode from */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Encode Configuration", DisplayName = "Input Mode")
		TEnumAsByte<EMach1EncodeInputMode> InputMode = M1Input_Stereo;

	/** Select the Mach1 output format to encode to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Encode Configuration", DisplayName = "Output Mode")
		TEnumAsByte<EMach1EncodeOutputMode> OutputMode = M1Output_M1Spatial_8;

	// ========== INPUT CHANNEL ASSIGNMENTS ==========
	
	/** Input Channel 1 */
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

	/** Input Channel 7 - Used for higher order ambisonics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 7", meta = (EditCondition = "InputMode >= M1Input_BFOAACN"))
		USoundBase* InputChannel7;

	/** Input Channel 8 - Used for higher order ambisonics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 8", meta = (EditCondition = "InputMode >= M1Input_BFOAACN"))
		USoundBase* InputChannel8;

	/** Input Channel 9 - Used for 2nd order and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 9", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel9;

	/** Input Channel 10 - Used for 2nd order and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 10", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel10;

	/** Input Channel 11 - Used for 2nd order and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 11", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel11;

	/** Input Channel 12 - Used for 2nd order and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 12", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel12;

	/** Input Channel 13 - Used for 2nd order and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 13", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel13;

	/** Input Channel 14 - Used for 2nd order and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 14", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel14;

	/** Input Channel 15 - Used for 2nd order and above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 15", meta = (EditCondition = "InputMode >= M1Input_B2OAACN"))
		USoundBase* InputChannel15;

	/** Input Channel 16 - Used for 3rd order */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Input Channels", DisplayName = "Input Channel 16", meta = (EditCondition = "InputMode >= M1Input_B3OAACN"))
		USoundBase* InputChannel16;

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void EncodeAudio();

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		TArray<USoundBase*> GetEncodedOutput();

	/** Get the current number of input channels based on input mode */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		int GetCurrentInputChannelCount();

	/** Get the current number of output channels based on output mode */
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
		void SetReferenceDecodeActor(class AM1DecodeActor* DecodeActor);

	/** Set the reference decode component */
		UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
	void SetReferenceDecodeComponent(class UM1DecodeComponent* DecodeComponent);

	// Real-time audio processing
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
	void EnableStreamingMode(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
	void ProcessAudioBuffer(const TArray<float>& InputBuffer, int InputChannelIndex, TArray<float>& OutputBuffer);

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
	void SetSpatialPosition(float Azimuth, float Elevation, float Diverge);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Options", DisplayName = "Auto Encode")
		bool autoEncode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Options", DisplayName = "Volume")
		float Volume = 1.0f;
}; 
