//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#pragma once

#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h" 
#include "Components/SceneCaptureComponent.h"
#include "Camera/CameraComponent.h"

#include "M1Common.h"
#include "Mach1Decode.h"
#include "Mach1DecodePositional.h"

#include <vector>

#include "M1DecodeComponent.generated.h"

class ACameraActor;

//#define LEGACY_POSITIONAL

// Mach1 Decode Modes (maps to Mach1DecodeMode enum from API)
UENUM(BlueprintType)
enum EMach1DecodeModeComponent
{
	Mach1DecodeMode_Spatial_4_Component = 0 	UMETA(DisplayName = "Spatial 4-Channel"),
	Mach1DecodeMode_Spatial_8_Component = 1 	UMETA(DisplayName = "Spatial 8-Channel"),
	Mach1DecodeMode_Spatial_14_Component = 2 UMETA(DisplayName = "Spatial 14-Channel")
};

// Input mode for Mach1 decode
UENUM(BlueprintType)
enum EMach1InputModeComponent
{
	IndividualMonoChannels_Component = 0 	UMETA(DisplayName = "Individual Mono Channels"),
	MultichannelFile_Component = 1 		UMETA(DisplayName = "Single Multichannel File")
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MACH1DECODEPLUGIN_API UM1DecodeComponent : public USceneComponent
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

	USceneComponent* ParentComponent = nullptr;
	USceneComponent* listenerReferenceComponent = nullptr;
	FVector PlayerPosition;
	FQuat PlayerRotation;

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
	// Sets default values for this component's properties
	UM1DecodeComponent();

	void InitComponents(int32 MaxSpatialInputChannels);

	// Called when the game starts or when spawned
	void BeginPlay(); // overriden

	// Called every frame
	void TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction); // overriden

	/** When true automatically get the orientation from the first indexed camera's rotations. When false automatically use the parent component's Position and Rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Find PlayerPawn Camera and Attach Automatically")
		bool AttachToPlayerPawnCamera = true;

	/** Use this to apply an additional rotation offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Manual Camera Offset")
		FVector cameraManualAngleOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Override Reference Object Position")
		bool useReferenceObjectPosition = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Class Settings", DisplayName = "Override Reference Object Rotation")
		bool useReferenceObjectRotation = true;

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
		TEnumAsByte<EMach1DecodeModeComponent> DecodeMode = Mach1DecodeMode_Spatial_8_Component;

	/** Choose input mode: Individual mono channels or single multichannel file */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Decode Configuration", DisplayName = "Input Mode")
		TEnumAsByte<EMach1InputModeComponent> InputMode = IndividualMonoChannels_Component;

	// ========== INDIVIDUAL MONO CHANNEL INPUTS ==========
	
	/** Channel 1 - Used for all decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 1", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component"))
		USoundBase* Channel1;

	/** Channel 2 - Used for all decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 2", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component"))
		USoundBase* Channel2;

	/** Channel 3 - Used for all decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 3", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component"))
		USoundBase* Channel3;

	/** Channel 4 - Used for all decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 4", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component"))
		USoundBase* Channel4;

	/** Channel 5 - Used for 8+, 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 5", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && (DecodeMode == Mach1DecodeMode_Spatial_8_Component || DecodeMode == Mach1DecodeMode_Spatial_14_Component)"))
		USoundBase* Channel5;

	/** Channel 6 - Used for 8+, 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 6", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && (DecodeMode == Mach1DecodeMode_Spatial_8_Component || DecodeMode == Mach1DecodeMode_Spatial_14_Component)"))
		USoundBase* Channel6;

	/** Channel 7 - Used for 8+, 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 7", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && (DecodeMode == Mach1DecodeMode_Spatial_8_Component || DecodeMode == Mach1DecodeMode_Spatial_14_Component)"))
		USoundBase* Channel7;

	/** Channel 8 - Used for 8+, 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 8", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && (DecodeMode == Mach1DecodeMode_Spatial_8_Component || DecodeMode == Mach1DecodeMode_Spatial_14_Component)"))
		USoundBase* Channel8;

	/** Channel 9 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 9", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && DecodeMode == Mach1DecodeMode_Spatial_14_Component"))
		USoundBase* Channel9;

	/** Channel 10 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 10", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && DecodeMode == Mach1DecodeMode_Spatial_14_Component"))
		USoundBase* Channel10;

	/** Channel 11 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 11", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && DecodeMode == Mach1DecodeMode_Spatial_14_Component"))
		USoundBase* Channel11;

	/** Channel 12 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 12", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && DecodeMode == Mach1DecodeMode_Spatial_14_Component"))
		USoundBase* Channel12;

	/** Channel 13 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 13", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && DecodeMode == Mach1DecodeMode_Spatial_14_Component"))
		USoundBase* Channel13;

	/** Channel 14 - Used for 14+ channel decode types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Mono Channels", DisplayName = "Channel 14", meta = (EditCondition = "InputMode == IndividualMonoChannels_Component && DecodeMode == Mach1DecodeMode_Spatial_14_Component"))
		USoundBase* Channel14;

	// ========== MULTICHANNEL FILE INPUT ==========

	/** Single multichannel audio file containing all Mach1 channels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mach1 Multichannel File", DisplayName = "Multichannel Audio File", meta = (EditCondition = "InputMode == MultichannelFile_Component"))
		USoundBase* MultichannelAudioFile;

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void Play();

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void Pause();

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void Resume();

	UFUNCTION(BlueprintCallable, Category = "Mach1Spatial Functions")
		void Seek(float time);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attenuation & Rotation Settings", DisplayName = "Use Attenuation")
		bool useAttenuation = false;

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
