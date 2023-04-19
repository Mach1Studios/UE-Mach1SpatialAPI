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

#include "M1BaseComponent.generated.h"

class ACameraActor;

//#define LEGACY_POSITIONAL

UCLASS(abstract)
class MACH1DECODEPLUGIN_API UM1BaseComponent : public USceneComponent
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

	Mach1DecodePositional m1Positional;

public:
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
