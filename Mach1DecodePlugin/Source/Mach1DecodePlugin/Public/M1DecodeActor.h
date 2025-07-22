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
#include "Mach1Decode.h"
#include "Mach1DecodePositional.h"
#include "Mach1DecodeCAPI.h"

#include <vector>

#include "M1DecodeActor.generated.h"

class ACameraActor;

//#define LEGACY_POSITIONAL

// Mach1 Decode Modes (maps to Mach1DecodeMode enum from API)
UENUM(BlueprintType)
enum EMach1DecodeMode
{
	Mach1DecodeMode_Spatial_4 = 0 	UMETA(DisplayName = "Spatial 4-Channel"),
	Mach1DecodeMode_Spatial_8 = 1 	UMETA(DisplayName = "Spatial 8-Channel"),
	Mach1DecodeMode_Spatial_14 = 2 UMETA(DisplayName = "Spatial 14-Channel")
};

// Input mode for Mach1 decode
UENUM(BlueprintType)
enum EMach1InputMode
{
	IndividualMonoChannels = 0 	UMETA(DisplayName = "Individual Mono Channels"),
	MultichannelFile = 1 		UMETA(DisplayName = "Single Multichannel File")
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MACH1DECODEPLUGIN_API AM1DecodeActor : public AActor
{
	GENERATED_BODY()

protected:

	static FVector GetEuler(FQuat q1);
	USoundAttenuation* NullAttenuation;

	// Calculated gain coefficients to apply to the spatial mixer's gain-volume per channel
	TArray<float> GainCoeffs;

	TArray<USoundBase*> SoundsMain;
	TArray<UAudioComponent*> LeftChannelsMain;
	TArray<UAudioComponent*> RightChannelsMain;

	USceneComponent* Root;
	UBoxComponent* Collision;
	UBillboardComponent* Billboard;

	int MAX_INPUT_CHANNELS;
	bool isInited;
	bool needToPlayAfterInit; 

	void Init();
	void SetSoundSet();
	void SetVolumeMain(float volume);

	virtual void SetSoundsMain();

	// New flexible methods
	void UpdateDecodeConfiguration();
	void SetSoundsBasedOnConfiguration();
	int GetRequiredChannelCount();
	void ClearAllSounds();
	USoundBase* GetChannelByIndex(int index);

	Mach1DecodePositional m1Positional;

public:

	// Sets default values for this actor's properties
	AM1DecodeActor();

	void InitComponents(int32 MaxSpatialInputChannels);

	// Called when the game starts or when spawned
	void BeginPlay(); // overriden

	// Called every frame
	void Tick(float DeltaSeconds); // overriden

	// always tick
	bool ShouldTickIfViewportsOnly() const override { return true; }
	#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif

	/** When true automatically get the orientation from the first indexed camera's rotations, tends to not include rotations applied to parent/pawn. When false automatically use PlayerCameraManager which tends to capture all rotations applied to an HMD or camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Force HMD rotation instead of Player Controller")
		bool ForceHMDRotation = false;

	/** Use this to apply an additional rotation offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Manual Camera Rotation Offset")
		FVector cameraManualAngleOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Use Reference Object Position")
		bool useReferenceObjectPosition = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Add Rotation offset from Reference Object Rotation")
		bool useDecodeRotationOffset = false;

	/** Reference a manual Pawn instead of the automatically found first indexed camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Manual Reference Pawn")
		APawn* manualPawn = nullptr;

	/** Reference a manual Actor/Object instead of this Actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Manual Reference Actor")
		AActor* manualActor = nullptr;

	/** Reference a manual Camera instead of the automatically found first index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Manual Reference Camera Actor")
		ACameraActor* manualCameraActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Display Debug")
		bool Debug = false;

	// ========== FLEXIBLE DECODE CONFIGURATION ==========

	/** Select the Mach1 decode mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Decode Configuration", DisplayName = "Decode Mode")
		TEnumAsByte<EMach1DecodeMode> DecodeMode = Mach1DecodeMode_Spatial_8;

	/** Choose input mode: Individual mono channels or single multichannel file */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Decode Configuration", DisplayName = "Input Mode")
		TEnumAsByte<EMach1InputMode> InputMode = IndividualMonoChannels;

	// ========== INDIVIDUAL MONO CHANNEL INPUTS ==========
	
	/** Channel 1 - Used for all decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 1", meta = (EditCondition = "InputMode == IndividualMonoChannels"))
		USoundBase* Channel1;

	/** Channel 2 - Used for all decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 2", meta = (EditCondition = "InputMode == IndividualMonoChannels"))
		USoundBase* Channel2;

	/** Channel 3 - Used for all decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 3", meta = (EditCondition = "InputMode == IndividualMonoChannels"))
		USoundBase* Channel3;

	/** Channel 4 - Used for all decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 4", meta = (EditCondition = "InputMode == IndividualMonoChannels"))
		USoundBase* Channel4;

	/** Channel 5 - Used for 8+, 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 5", meta = (EditCondition = "InputMode == IndividualMonoChannels && (DecodeMode == Mach1DecodeMode_Spatial_8 || DecodeMode == Mach1DecodeMode_Spatial_14)"))
		USoundBase* Channel5;

	/** Channel 6 - Used for 8+, 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 6", meta = (EditCondition = "InputMode == IndividualMonoChannels && (DecodeMode == Mach1DecodeMode_Spatial_8 || DecodeMode == Mach1DecodeMode_Spatial_14)"))
		USoundBase* Channel6;

	/** Channel 7 - Used for 8+, 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 7", meta = (EditCondition = "InputMode == IndividualMonoChannels && (DecodeMode == Mach1DecodeMode_Spatial_8 || DecodeMode == Mach1DecodeMode_Spatial_14)"))
		USoundBase* Channel7;

	/** Channel 8 - Used for 8+, 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 8", meta = (EditCondition = "InputMode == IndividualMonoChannels && (DecodeMode == Mach1DecodeMode_Spatial_8 || DecodeMode == Mach1DecodeMode_Spatial_14)"))
		USoundBase* Channel8;

	/** Channel 9 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 9", meta = (EditCondition = "InputMode == IndividualMonoChannels && DecodeMode == Mach1DecodeMode_Spatial_14"))
		USoundBase* Channel9;

	/** Channel 10 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 10", meta = (EditCondition = "InputMode == IndividualMonoChannels && DecodeMode == Mach1DecodeMode_Spatial_14"))
		USoundBase* Channel10;

	/** Channel 11 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 11", meta = (EditCondition = "InputMode == IndividualMonoChannels && DecodeMode == Mach1DecodeMode_Spatial_14"))
		USoundBase* Channel11;

	/** Channel 12 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 12", meta = (EditCondition = "InputMode == IndividualMonoChannels && DecodeMode == Mach1DecodeMode_Spatial_14"))
		USoundBase* Channel12;

	/** Channel 13 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 13", meta = (EditCondition = "InputMode == IndividualMonoChannels && DecodeMode == Mach1DecodeMode_Spatial_14"))
		USoundBase* Channel13;

	/** Channel 14 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 14", meta = (EditCondition = "InputMode == IndividualMonoChannels && DecodeMode == Mach1DecodeMode_Spatial_14"))
		USoundBase* Channel14;

	// ========== MULTICHANNEL FILE INPUT ==========

	/** Single multichannel audio file containing all Mach1 channels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Multichannel File", DisplayName = "Multichannel Audio File", meta = (EditCondition = "InputMode == MultichannelFile"))
		USoundBase* MultichannelAudioFile;

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void Play();

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void Pause();

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void Resume();

	/** Plays mix at a specific time in seconds. Use Pause/Resume to control play/stop flow if needed. */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void Seek(float timeInSeconds);

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void Stop();

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		TArray<USoundBase*> GetSoundsMain();

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		TArray<UAudioComponent*> GetAudioComponentsMain();

	/** Get the current number of channels based on decode algorithm */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		int GetCurrentChannelCount();

	/** Update the decoder configuration when settings change */
	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void RefreshDecodeConfiguration();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Options", DisplayName = "Autoplay")
		bool autoplay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Options", DisplayName = "Volume")
		float Volume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Options", DisplayName = "Fade In Duration")
		float fadeInDuration = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Options", DisplayName = "Fade Out Duration")
		float fadeOutDuration = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attenuation & Rotation Settings", DisplayName = "Use Falloff")
		bool useFalloff = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attenuation & Rotation Settings", DisplayName = "Attenuation Curve")
		UCurveFloat* attenuationCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attenuation & Rotation Settings", DisplayName = "Mute When Inside Object")
		bool muteWhenInsideObject = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attenuation & Rotation Settings", DisplayName = "Mute When Outside Object")
		bool muteWhenOutsideObject = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attenuation & Rotation Settings", DisplayName = "Use Closest Point Rotation & Mute When Inside")
		bool usePlaneCalculation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attenuation & Rotation Settings", DisplayName = "Use Yaw for Positional Rotation")
		bool useYawForRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attenuation & Rotation Settings", DisplayName = "Use Pitch for Positional Rotation")
		bool usePitchForRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attenuation & Rotation Settings", DisplayName = "Use Roll for Positional Rotation")
		bool useRollForRotation = true;
};
