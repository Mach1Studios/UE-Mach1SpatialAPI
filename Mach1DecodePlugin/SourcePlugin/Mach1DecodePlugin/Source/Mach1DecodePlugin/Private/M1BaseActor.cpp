//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#include "M1BaseActor.h"
#include "Camera/CameraActor.h"
#include "Runtime/Launch/Resources/Version.h"

#include "Mach1DecodePluginPrivatePCH.h"

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 18) || ENGINE_MAJOR_VERSION == 5
#include "HeadMountedDisplay.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#elif ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 17
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#endif

#include <sstream>

#define MIN_SOUND_VOLUME (KINDA_SMALL_NUMBER*2)

template<typename T>
std::string toDebugString(const T& value)
{
	std::ostringstream oss;
	oss.precision(2);
	oss << std::fixed << value;
	return oss.str();
}

template < >
std::string toDebugString<FVector>(const FVector& value)
{
	std::ostringstream oss;
	oss.precision(2);
	oss << std::fixed << "(" << value.X << ", " << value.Y << ", " << value.Z << ")";
	return oss.str();
}

FVector AM1BaseActor::GetEuler(FQuat q1)
{
	float sq = q1.X * q1.Y + q1.Z * q1.W;
	float sqx = q1.X * q1.X;
	float sqy = q1.Y * q1.Y;
	float sqz = q1.Z * q1.Z;

	return FMath::RadiansToDegrees(FVector(
		atan2(2.0f * q1.X * q1.W - 2 * q1.Y * q1.Z, 1.0f - 2.0f * sqx - 2.0f * sqz),
		atan2(2.0f * q1.Y * q1.W - 2 * q1.X * q1.Z, 1.0f - 2.0f * sqy - 2.0f * sqz),
		sin(2.0f * sq)
	));

}

Mach1Point3D AM1BaseActor::ConvertToMach1Point3D(FVector vec)
{
	return Mach1Point3D{ (float)vec.X, (float)vec.Y, (float)vec.Z };
}

Mach1Point4D AM1BaseActor::ConvertToMach1Point4D(FQuat quat)
{
	return Mach1Point4D{ (float)quat.X, (float)quat.Y, (float)quat.Z, (float)quat.W };
}

// Sets default values
void AM1BaseActor::InitComponents(int32 InMaxSoundsPerChannel)
{
	this->MAX_SOUNDS_PER_CHANNEL = InMaxSoundsPerChannel;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Root->bAutoActivate = true;
	Root->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	Collision->InitBoxExtent(FVector(100, 100, 100));
	Collision->bEditableWhenInherited = false;
	Collision->SetHiddenInGame(false);
	Collision->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	Billboard = CreateDefaultSubobject< UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetHiddenInGame(false);
	Billboard->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	LeftChannelsMain.SetNum(MAX_SOUNDS_PER_CHANNEL);
	RightChannelsMain.SetNum(MAX_SOUNDS_PER_CHANNEL);

	LeftChannelsBlend.SetNum(MAX_SOUNDS_PER_CHANNEL);
	RightChannelsBlend.SetNum(MAX_SOUNDS_PER_CHANNEL);

	Volume = 1;
	for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL * 2; i++) GainCoeffs.Add(1);

	m1Positional.setPlatformType(Mach1PlatformType::Mach1PlatformUE);
}

void AM1BaseActor::Init()
{
	if (!isInited)
	{
		USceneComponent* cameraComponent = nullptr;

		if (APlayerController* player = GetWorld()->GetFirstPlayerController())//UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
		{
			TArray<UCameraComponent*> Cameras;
			if (APawn* playerPawn = player->GetPawn())
			{
				playerPawn->GetComponents<UCameraComponent>(Cameras);
				if (Cameras.Num() < 1)
				{
					APlayerCameraManager* GameCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
					cameraComponent = GameCameraManager->GetRootComponent();
				}
				else
				{
					cameraComponent = Cameras[0];
				}
			}
		}

		if (cameraComponent)
		{
			NullAttenuation = NewObject<USoundAttenuation>(this);
			NullAttenuation->Attenuation.bAttenuate = false;
			NullAttenuation->Attenuation.bSpatialize = true;

			for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
			{
				LeftChannelsMain[i] = NewObject <UAudioComponent>(cameraComponent, FName(*FString::Printf(TEXT("SoundCubeWalls %d_L_%d"), GetUniqueID(), i)));
				RightChannelsMain[i] = NewObject <UAudioComponent>(cameraComponent, FName(*FString::Printf(TEXT("SoundCubeWalls %d_R_%d"), GetUniqueID(), i)));

				LeftChannelsMain[i]->RegisterComponent(); // only for runtime
				RightChannelsMain[i]->RegisterComponent(); // only for runtime

				LeftChannelsMain[i]->SetRelativeLocation(FVector(0, -1, 0));
				RightChannelsMain[i]->SetRelativeLocation(FVector(0, 1, 0));

				LeftChannelsMain[i]->AttenuationSettings = NullAttenuation;
				RightChannelsMain[i]->AttenuationSettings = NullAttenuation;

				LeftChannelsMain[i]->AttachToComponent(cameraComponent, FAttachmentTransformRules::KeepRelativeTransform); // AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);//   AttachTo(sceneComponent);
				RightChannelsMain[i]->AttachToComponent(cameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
			}

			if (useBlendMode)
			{
				for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
				{
					LeftChannelsBlend[i] = NewObject <UAudioComponent>(cameraComponent, FName(*FString::Printf(TEXT("SoundCubeCenter %d_L_%d"), GetUniqueID(), i)));
					RightChannelsBlend[i] = NewObject <UAudioComponent>(cameraComponent, FName(*FString::Printf(TEXT("SoundCubeCenter %d_R_%d"), GetUniqueID(), i)));

					LeftChannelsBlend[i]->RegisterComponent(); // only for runtime
					RightChannelsBlend[i]->RegisterComponent(); // only for runtime

					LeftChannelsBlend[i]->SetRelativeLocation(FVector(0, -1, 0));
					RightChannelsBlend[i]->SetRelativeLocation(FVector(0, 1, 0));

					LeftChannelsBlend[i]->AttenuationSettings = NullAttenuation;
					RightChannelsBlend[i]->AttenuationSettings = NullAttenuation;

					LeftChannelsBlend[i]->AttachToComponent(cameraComponent, FAttachmentTransformRules::KeepRelativeTransform); // AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);//   AttachTo(sceneComponent);
					RightChannelsBlend[i]->AttachToComponent(cameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
				}
			}
			isInited = true;
		}
	}
}

void AM1BaseActor::SetSoundSet()
{
	if (isInited)
	{
		SoundsMain.Empty();

		SetSoundsMain();

		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			if (SoundsMain[i])
			{
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 23) || ENGINE_MAJOR_VERSION == 5
				SoundsMain[i]->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
#else
				SoundsMain[i]->bVirtualizeWhenSilent = true;
#endif
			}
		}

		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			if (SoundsMain[i])
			{
				LeftChannelsMain[i]->SetSound(SoundsMain[i]);
				RightChannelsMain[i]->SetSound(SoundsMain[i]);
			}
		}

		if (useBlendMode)
		{
			SoundsBlendMode.Empty();

			SetSoundsBlendMode();

			for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
			{
				if (SoundsBlendMode[i])
				{
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 23) || ENGINE_MAJOR_VERSION == 5
					SoundsBlendMode[i]->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
#else
					SoundsBlendMode[i]->bVirtualizeWhenSilent = true;
#endif
				}
			}

			for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
			{
				if (SoundsBlendMode[i])
				{
					LeftChannelsBlend[i]->SetSound(SoundsBlendMode[i]);
					RightChannelsBlend[i]->SetSound(SoundsBlendMode[i]);
				}
			}
		}
	}
}

void AM1BaseActor::Play()
{
	if (isInited)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsMain[i]->FadeIn(fadeInDuration);
			RightChannelsMain[i]->FadeIn(fadeInDuration);
		}
	}

	if (isInited && useBlendMode)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsBlend[i]->FadeIn(fadeInDuration);
			RightChannelsBlend[i]->FadeIn(fadeInDuration);
		}
	}

	if (!isInited)
	{
		needToPlayAfterInit = true;
	}
}

void AM1BaseActor::Pause()
{
	if (isInited)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsMain[i]->SetPaused(true);
			RightChannelsMain[i]->SetPaused(true);
		}
	}

	if (isInited && useBlendMode)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsBlend[i]->SetPaused(true);
			RightChannelsBlend[i]->SetPaused(true);
		}
	}
}

void AM1BaseActor::Resume()
{
	if (isInited)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsMain[i]->SetPaused(false);
			RightChannelsMain[i]->SetPaused(false);
		}
	}

	if (isInited && useBlendMode)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsBlend[i]->SetPaused(false);
			RightChannelsBlend[i]->SetPaused(false);
		}
	}
}

void AM1BaseActor::Seek(float time)
{
	if (isInited)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsMain[i]->Play(time);
			RightChannelsMain[i]->Play(time);
		}
	}

	if (isInited && useBlendMode)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsBlend[i]->Play(time);
			RightChannelsBlend[i]->Play(time);
		}
	}
}

void AM1BaseActor::Stop()
{
	if (isInited)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsMain[i]->FadeOut(fadeOutDuration, 0);
			RightChannelsMain[i]->FadeOut(fadeOutDuration, 0);
		}
	}

	if (isInited && useBlendMode)
	{
		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			LeftChannelsBlend[i]->FadeOut(fadeOutDuration, 0);
			RightChannelsBlend[i]->FadeOut(fadeOutDuration, 0);
		}
	}
	needToPlayAfterInit = false;
}

// Called when the game starts or when spawned
void AM1BaseActor::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine)
	{
		if (APlayerController* player = GetWorld()->GetFirstPlayerController())
		{
			Init();
			SetSoundSet();
			if (autoplay || needToPlayAfterInit) Play();
		}
	}
}

void PrintDebug(const char* str) {
	UE_LOG(LogTemp, Warning, TEXT("%s\r\n"), *FString(str));
}

// Called every frame
void AM1BaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GEngine && Root->IsActive())
	{
		if (APlayerController* player = GetWorld()->GetFirstPlayerController())
		{
			if (APawn* playerPawn = player->GetPawn())
			{
				Collision->SetHiddenInGame(!Debug);
				Billboard->SetHiddenInGame(!Debug);

				FQuat PlayerRotation = FQuat::Identity;
				FVector PlayerPosition = FVector(0, 0, 0);

				if (manualPawn != nullptr)
				{
					if (useReferenceObjectRotation) PlayerRotation = manualPawn->GetControlRotation().Quaternion();
					if (useReferenceObjectPosition) PlayerPosition = manualPawn->GetActorLocation();
				}
				else if (manualActor != nullptr)
				{
					if (useReferenceObjectRotation) PlayerRotation = manualActor->GetActorRotation().Quaternion();
					if (useReferenceObjectPosition) PlayerPosition = manualActor->GetActorLocation();
				}
				else if (manualCameraActor != nullptr)
				{
					if (useReferenceObjectRotation) PlayerRotation = manualCameraActor->GetActorRotation().Quaternion();
					if (useReferenceObjectPosition) PlayerPosition = manualCameraActor->GetActorLocation();
				}

				if (ForceHMDRotation && UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
				{
					FRotator hmdRotator;
					FVector hmdPosition;
					UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(hmdRotator, hmdPosition);

					FQuat hmdQuat = FQuat::MakeFromEuler(FVector(-hmdRotator.Quaternion().Euler().X, -hmdRotator.Quaternion().Euler().Y, hmdRotator.Quaternion().Euler().Z));

					PlayerRotation = PlayerRotation * hmdQuat;// rotator.Quaternion() * player->GetControlRotation().Quaternion();
					PlayerPosition = PlayerPosition + hmdPosition;
				}
				else
				{
					APlayerCameraManager* playerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
					FRotator hmdRotator = playerCameraManager->GetCameraRotation();
					FVector hmdPosition = playerCameraManager->GetCameraLocation();
					
					// Some leftover calculations for exploring multi-cam or custom VR cam setups
					//FQuat hmdQuat = FQuat::MakeFromEuler(FVector(-hmdRotator.Quaternion().Euler().X, -hmdRotator.Quaternion().Euler().Y, hmdRotator.Quaternion().Euler().Z));
					//PlayerRotation = PlayerRotation * player->GetControlRotation().Quaternion() * hmdQuat;
					
					PlayerRotation = player->GetControlRotation().Quaternion();
					PlayerPosition = PlayerPosition + playerPawn->GetActorLocation(); // + hmdPosition
				}

				PlayerRotation = PlayerRotation * FQuat::MakeFromEuler(cameraManualAngleOffset);

				if (Debug) {
					std::string info;

					info = "Camera Rotation:  " + toDebugString(PlayerRotation.Euler());
					PrintDebug(info.c_str());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());

					info = "Camera Position:  " + toDebugString(PlayerPosition);
					PrintDebug(info.c_str());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());

					info = "Actor Rotation:  " + toDebugString(GetActorRotation().Quaternion().Euler());
					PrintDebug(info.c_str());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());

					info = "Actor Position:  " + toDebugString(GetActorLocation());
					PrintDebug(info.c_str());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());
				}

				FVector point = GetActorLocation();

				FVector scale = Collision->GetScaledBoxExtent(); // GetActorScale() / 2 *
																 //scale = FVector(scale.Y, scale.Z, scale.X);

				float masterGain = Volume;

				m1Positional.setUseBlendMode(useBlendMode);
				m1Positional.setIgnoreTopBottom(ignoreTopBottom);
				m1Positional.setMuteWhenOutsideObject(muteWhenOutsideObject);
				m1Positional.setMuteWhenInsideObject(muteWhenInsideObject);
				m1Positional.setUseAttenuation(useFalloff);
				m1Positional.setUsePlaneCalculation(useClosestPointRotationMuteInside);
				m1Positional.setUseYawForRotation(useYawForRotation);
				m1Positional.setUsePitchForRotation(usePitchForRotation);
				m1Positional.setUseRollForRotation(useRollForRotation);

				m1Positional.setListenerPosition(ConvertToMach1Point3D(PlayerPosition));
				FVector listenerAngle = (PlayerRotation.Euler());
				m1Positional.setListenerRotation(ConvertToMach1Point3D(listenerAngle));
				m1Positional.setDecoderAlgoPosition(ConvertToMach1Point3D(GetActorLocation()));
				FVector decoderAngle = (GetActorRotation().Euler());
				m1Positional.setDecoderAlgoRotation(ConvertToMach1Point3D(decoderAngle));
				m1Positional.setDecoderAlgoScale(ConvertToMach1Point3D(scale));

				m1Positional.evaluatePositionResults();

				if (useFalloff)
				{
					m1Positional.setAttenuationCurve(attenuationCurve ? attenuationCurve->GetFloatValue(m1Positional.getDist()) : 1);
					m1Positional.setAttenuationCurveBlendMode(attenuationBlendModeCurve ? attenuationBlendModeCurve->GetFloatValue(m1Positional.getDist()) : 1);
				}

				float coeffs[18];
				m1Positional.getCoefficients(coeffs);
				for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL * 2; i++)
				{
					GainCoeffs[i] = coeffs[i];
				}
				SetVolumeMain(1.0);

				float coeffsInterior[18];
				m1Positional.getCoefficientsInterior(coeffsInterior);
				if (useBlendMode)
				{
					for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL * 2; i++)
					{
						GainCoeffs[i] = coeffsInterior[i];
					}
					SetVolumeBlend(1.0);
				}

				if (Debug)
				{
					std::string info;
					info = "Lib Distance:  " + toDebugString(m1Positional.getDist());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Purple, info.c_str());

					std::string str = "Lib Euler Angles:    " + toDebugString(m1Positional.getCurrentAngle().x) + " , " + toDebugString(m1Positional.getCurrentAngle().y) + " , " + toDebugString(m1Positional.getCurrentAngle().z);
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Yellow, str.c_str());


					info = "Coeffs:  ";
					for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL * 2; i++)
					{
						info += toDebugString(coeffs[i]) + ", ";
					}
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());
				}
			}
		}
	}
}

#if WITH_EDITOR
void AM1BaseActor::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != NULL)
	{
		if (PropertyChangedEvent.Property->GetFName() == FName(TEXT("Debug")))
		{
			Collision->SetHiddenInGame(!Debug);
			Billboard->SetHiddenInGame(!Debug);
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AM1BaseActor::SetVolumeMain(float volume)
{
	if (isInited)
	{
		float masterGain = FMath::Max(MIN_SOUND_VOLUME, this->Volume * volume);
		float newVolume = 0;

		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			newVolume = GainCoeffs[i * 2] * masterGain;
			newVolume = FMath::Max(MIN_SOUND_VOLUME, newVolume);
			LeftChannelsMain[i]->SetVolumeMultiplier(newVolume);

			newVolume = GainCoeffs[i * 2 + 1] * masterGain;
			newVolume = FMath::Max(MIN_SOUND_VOLUME, newVolume);
			RightChannelsMain[i]->SetVolumeMultiplier(newVolume);
		}
	}
}

void AM1BaseActor::SetVolumeBlend(float volume)
{
	if (isInited && useBlendMode)
	{
		float masterGain = FMath::Max(MIN_SOUND_VOLUME, this->Volume * volume);
		float newVolume = 0;

		for (int i = 0; i < MAX_SOUNDS_PER_CHANNEL; i++)
		{
			newVolume = GainCoeffs[i * 2] * masterGain;
			newVolume = FMath::Max(MIN_SOUND_VOLUME, newVolume);
			LeftChannelsBlend[i]->SetVolumeMultiplier(newVolume);

			newVolume = GainCoeffs[i * 2 + 1] * masterGain;
			newVolume = FMath::Max(MIN_SOUND_VOLUME, newVolume);
			RightChannelsBlend[i]->SetVolumeMultiplier(newVolume);
		}
	}
}

void AM1BaseActor::SetSoundsMain()
{
}

void AM1BaseActor::SetSoundsBlendMode()
{
}
