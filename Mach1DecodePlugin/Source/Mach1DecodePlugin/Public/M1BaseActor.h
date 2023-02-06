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

#include "Mach1Decode.h"
#include "Mach1DecodePositional.h"

#include <vector>

#include "M1BaseActor.generated.h"

class ACameraActor;

//#define LEGACY_POSITIONAL

UCLASS(abstract)
class MACH1DECODEPLUGIN_API AM1BaseActor : public AActor
{
	GENERATED_BODY()

protected:

	static FVector GetEuler(FQuat q1);
	USoundAttenuation* NullAttenuation;

	// Calculated gain coefficients to apply to the spatial mixer's gain-volume per channel
	TArray<float> GainCoeffs;

	TArray<USoundWave*> SoundsMain;
	TArray<UAudioComponent*> LeftChannelsMain;
	TArray<UAudioComponent*> RightChannelsMain;

	TArray<USoundWave*> SoundsBlendMode;
	TArray<UAudioComponent*> LeftChannelsBlend;
	TArray<UAudioComponent*> RightChannelsBlend;

	USceneComponent* Root;
	UBoxComponent* Collision;
	UBillboardComponent* Billboard;

	int MAX_SOUNDS_PER_CHANNEL;
	bool isInited;
	bool needToPlayAfterInit; 

	void Init();
	void SetSoundSet();
	void SetVolumeMain(float volume);
	void SetVolumeBlend(float volume);

	virtual void SetSoundsMain();
	virtual void SetSoundsBlendMode();

	Mach1DecodePositional m1Positional;

	Mach1Point3D ConvertToMach1Point3D(FVector vec);
	Mach1Point4D ConvertToMach1Point4D(FQuat quat);

public:

	void InitComponents(int32 InMaxSoundsPerChannel);

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

	UFUNCTION(BlueprintCallable, Category = "Trigger Options")
		void Play();

	UFUNCTION(BlueprintCallable, Category = "Trigger Options")
		void Pause();

	UFUNCTION(BlueprintCallable, Category = "Trigger Options")
		void Resume();

	UFUNCTION(BlueprintCallable, Category = "Trigger Options")
		void Seek(float time);

	UFUNCTION(BlueprintCallable, Category = "Trigger Options")
		void Stop();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Mode (beta)", DisplayName = "Use Blend Mode")
		bool useBlendMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Mode (beta)", DisplayName = "Attenuation BlendMode Curve")
		UCurveFloat* attenuationBlendModeCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Mode (beta)", DisplayName = "Ignore Top Bottom Planes in BlendMode")
		bool ignoreTopBottom = true;
};
