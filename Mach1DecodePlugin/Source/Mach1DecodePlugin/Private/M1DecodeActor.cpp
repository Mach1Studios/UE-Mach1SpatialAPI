//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#include "M1DecodeActor.h"
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

// Sets default values for this actor's properties
AM1DecodeActor::AM1DecodeActor()
{
	// Call the InitComponents with default maximum channels (14)
	InitComponents(14);
}

void AM1DecodeActor::InitComponents(int32 MaxSpatialInputChannels)
{
	// Default to max channels but will be adjusted based on decode algorithm
	this->MAX_INPUT_CHANNELS = FMath::Max(MaxSpatialInputChannels, 14); // Support up to 14 channels

	// Set this actor to call Tick() every frame.
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

	LeftChannelsMain.SetNum(MAX_INPUT_CHANNELS);
	RightChannelsMain.SetNum(MAX_INPUT_CHANNELS);

	Volume = 1;
	for (int i = 0; i < MAX_INPUT_CHANNELS * 2; i++) GainCoeffs.Add(1);

	m1Positional.setPlatformType(Mach1PlatformType::Mach1PlatformUE);
	
	// Set default decode mode
	UpdateDecodeConfiguration();
}

void AM1DecodeActor::Init()
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

			for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
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

			isInited = true;
		}
	}
}

void AM1DecodeActor::SetSoundSet()
{
	if (isInited)
	{
		// First, stop all audio to prevent overlapping playback
		StopAllAudioComponents();
		
		SoundsMain.Empty();

		// Use the new configuration-based sound setup (multi-mono only)
		SetSoundsBasedOnConfiguration();

		if (Debug)
		{
		}

		// MULTI-MONO MODE: Use individual mono files (now the only mode)
		{
			// INDIVIDUAL MONO CHANNELS MODE: Use separate mono files
			SetupIndividualChannelPlayback();
		}
	}
}

void AM1DecodeActor::Play()
{
	if (isInited)
	{
		// Multi-mono playbook: Perfect synchronization for all channels
		// First, prepare all components
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			if (LeftChannelsMain[i]->GetSound())
			{
				LeftChannelsMain[i]->Stop();
				RightChannelsMain[i]->Stop();
			}
		}
		
		// Small delay to ensure all components are stopped
		FPlatformProcess::Sleep(0.01f);
		
		// Now start all components simultaneously
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			if (LeftChannelsMain[i]->GetSound())
			{
				LeftChannelsMain[i]->FadeIn(fadeInDuration);
				RightChannelsMain[i]->FadeIn(fadeInDuration);
			}
		}
		
		if (Debug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
				FString::Printf(TEXT("Playing Multi-Mono audio - Synchronized start (%d channels)"), GetRequiredChannelCount()));
		}
	}

	if (!isInited)
	{
		needToPlayAfterInit = true;
	}
}

void AM1DecodeActor::Pause()
{
	if (isInited)
	{
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			LeftChannelsMain[i]->SetPaused(true);
			RightChannelsMain[i]->SetPaused(true);
		}
	}
}

void AM1DecodeActor::Resume()
{
	if (isInited)
	{
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			LeftChannelsMain[i]->SetPaused(false);
			RightChannelsMain[i]->SetPaused(false);
		}
	}
}

void AM1DecodeActor::Seek(float timeInSeconds)
{
	if (isInited)
	{
		// For perfect synchronization, stop all first, then start all at the same time point
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			if (LeftChannelsMain[i]->GetSound())
			{
				LeftChannelsMain[i]->Stop();
				RightChannelsMain[i]->Stop();
			}
		}
		
		// Small delay to ensure synchronization
		FPlatformProcess::Sleep(0.005f);
		
		// Start all at the exact same time position
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			if (LeftChannelsMain[i]->GetSound())
			{
				LeftChannelsMain[i]->Play(timeInSeconds);
				RightChannelsMain[i]->Play(timeInSeconds);
			}
		}
		
		if (Debug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, 
				FString::Printf(TEXT("Synchronized seek to %.2fs"), timeInSeconds));
		}
	}
}

void AM1DecodeActor::Stop()
{
	if (isInited)
	{
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			LeftChannelsMain[i]->FadeOut(fadeOutDuration, 0);
			RightChannelsMain[i]->FadeOut(fadeOutDuration, 0);
		}
	}
	needToPlayAfterInit = false;
}

// Called when the game starts or when spawned
void AM1DecodeActor::BeginPlay()
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

// Called every frame
void AM1DecodeActor::Tick(float DeltaTime)
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
				FQuat ObjRotation = FQuat::Identity;
				FVector ObjPosition = FVector(0, 0, 0);

				if (manualPawn != nullptr)
				{
					TArray<UCameraComponent*> CameraComponents;
					manualPawn->GetComponents<UCameraComponent>(CameraComponents);

					for (UCameraComponent* CameraComp : CameraComponents)
					{
						if (CameraComp->IsActive())
						{
							PlayerRotation = CameraComp->GetComponentRotation().Quaternion();
							PlayerPosition = CameraComp->GetComponentLocation();
						}
					}

					PlayerRotation = manualPawn->GetControlRotation().Quaternion();
					PlayerPosition = manualPawn->GetActorLocation();
				}
				else if (manualActor != nullptr)
				{
					PlayerRotation = manualActor->GetActorRotation().Quaternion();
					PlayerPosition = manualActor->GetActorLocation();
				}
				else if (manualCameraActor != nullptr)
				{
					PlayerRotation = manualCameraActor->GetCameraComponent()->GetComponentRotation().Quaternion();
					PlayerPosition = manualCameraActor->GetCameraComponent()->GetComponentLocation();
				}
				else if (ForceHMDRotation && UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
				{
					FRotator hmdRotator;
					FVector hmdPosition;
					UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(hmdRotator, hmdPosition);

					PlayerRotation = hmdRotator.Quaternion();
					PlayerPosition = hmdPosition;
				}
				else
				{
					APlayerCameraManager* playerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
					FRotator cameraRotator = playerCameraManager->GetCameraRotation();
					FVector cameraPosition = playerCameraManager->GetCameraLocation();

					PlayerRotation = cameraRotator.Quaternion();
					PlayerPosition = cameraPosition;
				}

				PlayerRotation = PlayerRotation * FQuat::MakeFromEuler(cameraManualAngleOffset);

				FVector scale = Collision->GetScaledBoxExtent(); 

				// M1 Reference object pos/rot
				if (useReferenceObjectPosition) ObjPosition = GetActorLocation();
				if (useDecodeRotationOffset) ObjRotation = GetActorRotation().Quaternion();

				if (Debug) {
					std::string info;

					info = "Camera Rotation:  " + M1Common::toDebugString(PlayerRotation.Euler());
					M1Common::PrintDebug(info.c_str());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());

					info = "Camera Position:  " + M1Common::toDebugString(PlayerPosition);
					M1Common::PrintDebug(info.c_str());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());

					info = "Actor Rotation:  " + M1Common::toDebugString(ObjRotation.Euler());
					M1Common::PrintDebug(info.c_str());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());

					info = "Actor Position:  " + M1Common::toDebugString(ObjPosition);
					M1Common::PrintDebug(info.c_str());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());
				}

				float masterGain = Volume;
 
				m1Positional.setMuteWhenOutsideObject(muteWhenOutsideObject);
				m1Positional.setMuteWhenInsideObject(muteWhenInsideObject);
				m1Positional.setUseAttenuation(useFalloff);
				m1Positional.setUsePlaneCalculation(usePlaneCalculation);
				m1Positional.setUseYawForRotation(useYawForRotation);
				m1Positional.setUsePitchForRotation(usePitchForRotation);
				m1Positional.setUseRollForRotation(useRollForRotation);

				// Listener
				m1Positional.setListenerPosition(M1Common::ConvertToMach1Point3D(PlayerPosition));
				FVector listenerAngle = (PlayerRotation.Euler());
				m1Positional.setListenerRotation(M1Common::ConvertToMach1Point3D(listenerAngle));
				//m1Positional.setListenerRotationQuat(M1Common::ConvertToMach1Point4D(PlayerRotation));

				// Decoder
				m1Positional.setDecoderAlgoPosition(M1Common::ConvertToMach1Point3D(ObjPosition));
				// Allow use of GameObject's transform.rotation as an additional offset rotator for the Decode API
	            if (useDecodeRotationOffset)
	            {
	            	FVector decoderAngle = (GetActorRotation().Euler());
					m1Positional.setDecoderAlgoRotation(M1Common::ConvertToMach1Point3D(decoderAngle));
	            } else
	            {
	                // This allows us to treat the GameObject as a point instead of using its shape as an additional rotator
                    m1Positional.setDecoderAlgoRotation({ 0, 0, 0 });
	            }
				m1Positional.setDecoderAlgoScale(M1Common::ConvertToMach1Point3D(scale));

				m1Positional.evaluatePositionResults();

				if (useFalloff)
				{
					m1Positional.setAttenuationCurve(attenuationCurve ? attenuationCurve->GetFloatValue(m1Positional.getDist()) : 1);
				}

				float coeffs[MAX_INPUT_CHANNELS * 2];
				m1Positional.getCoefficients(coeffs);
				for (int i = 0; i < MAX_INPUT_CHANNELS * 2; i++)
				{
					GainCoeffs[i] = coeffs[i];
				}
				SetVolumeMain(1.0);

				if (Debug) {
					Mach1Point3D points[] = {
					   {-1, 1, 1},
					   {1, 1, 1},
					   {-1, -1, 1},
					   {1, -1, 1},
					   {-1, 1, -1},
					   {1, 1, -1},
					   {-1, -1, -1}, 
					   {1, -1, -1},
					};

					FQuat quat = FQuat::MakeFromEuler(FVector(m1Positional.getPositionalRotation().x, m1Positional.getPositionalRotation().y, m1Positional.getPositionalRotation().z));
					DrawDebugBox(GetWorld(), PlayerPosition, FVector(scale), quat, FColor::Red);

					for (int i = 0; i < 8; i++)
					{
						Mach1Point3D p = points[i];
						(std::swap)(p.y, p.z); // convert to glm
						FVector point = FVector(p.z, p.x, p.y); // convertion to platfrom

						DrawDebugString(GetWorld(), PlayerPosition + quat * (point * scale), FString(std::to_string(i).c_str()), 0, FColor::White, 0);
						DrawDebugSphere(GetWorld(), PlayerPosition + quat * ((point + FVector(-0.1, 0, 0)) * scale), 10 * coeffs[i * 2 + 0], 16, FColor::Red, false, 0.5f);
						DrawDebugSphere(GetWorld(), PlayerPosition + quat * ((point + FVector(+0.1, 0, 0)) * scale), 10 * coeffs[i * 2 + 1], 16, FColor::Blue, false, 0.5f);
					}
				}

				if (Debug)
				{
					std::string info;
					info = "Lib Distance:  " + M1Common::toDebugString(m1Positional.getDist());
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Purple, info.c_str());

					std::string str = "Lib Euler Angles:    " + M1Common::toDebugString(m1Positional.getCurrentAngleInternal().x) + " , " + M1Common::toDebugString(m1Positional.getCurrentAngleInternal().y) + " , " + M1Common::toDebugString(m1Positional.getCurrentAngleInternal().z);
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Yellow, str.c_str());


					info = "Coeffs:  ";
					for (int i = 0; i < MAX_INPUT_CHANNELS * 2; i++)
					{
						info += M1Common::toDebugString(coeffs[i]) + ", ";
					}
					GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());
				}
			}
		}
	}
}

#if WITH_EDITOR
void AM1DecodeActor::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != NULL)
	{
		FString PropertyName = PropertyChangedEvent.Property->GetName();
		
		if (PropertyName == TEXT("Debug"))
		{
			Collision->SetHiddenInGame(!Debug);
			Billboard->SetHiddenInGame(!Debug);
		}
		
		// Refresh configuration if decode-related properties changed
		if (PropertyName.Contains("DecodeMode") || 
			PropertyName.Contains("InputMode") || 
			PropertyName.Contains("Channel"))
		{
			RefreshDecodeConfiguration();
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


void AM1DecodeActor::SetVolumeMain(float volume)
{
	if (isInited)
	{
		float masterGain = FMath::Max(MIN_SOUND_VOLUME, this->Volume * volume);
		
		// MULTI-MONO MODE: Apply per-channel spatial processing (now the only mode)
		SetVolumeIndividualChannels(masterGain);
	}
}

void AM1DecodeActor::SetVolumeIndividualChannels(float masterGain)
{
	// Individual channel processing - each channel gets its own spatial coefficient
	float newVolume = 0;
	
	for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
	{
		newVolume = GainCoeffs[i * 2] * masterGain;
		newVolume = FMath::Max(MIN_SOUND_VOLUME, newVolume);
		LeftChannelsMain[i]->SetVolumeMultiplier(newVolume);

		newVolume = GainCoeffs[i * 2 + 1] * masterGain;
		newVolume = FMath::Max(MIN_SOUND_VOLUME, newVolume);
		RightChannelsMain[i]->SetVolumeMultiplier(newVolume);
	}
	
	if (Debug)
	{
		FString CoeffInfo = TEXT("Individual Channels: ");
		for (int i = 0; i < FMath::Min(4, MAX_INPUT_CHANNELS); i++) // Show first 4 channels
		{
			CoeffInfo += FString::Printf(TEXT("Ch%d[L=%.2f,R=%.2f] "), i, GainCoeffs[i * 2], GainCoeffs[i * 2 + 1]);
		}
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Orange, CoeffInfo);
	}
}

void AM1DecodeActor::SetSoundsMain()
{
}

TArray<USoundBase*> AM1DecodeActor::GetSoundsMain()
{
	return SoundsMain;
}

TArray<UAudioComponent*> AM1DecodeActor::GetAudioComponentsMain()
{
	TArray<UAudioComponent*> arrayOfAllPlayerComponents;
	for (UAudioComponent* componentL : LeftChannelsMain) {
		arrayOfAllPlayerComponents.Add(componentL);
	}

	for (UAudioComponent* componentR : RightChannelsMain) {
		arrayOfAllPlayerComponents.Add(componentR);
	}
	return arrayOfAllPlayerComponents;
}

// ========== NEW FLEXIBLE DECODE METHODS ==========

void AM1DecodeActor::UpdateDecodeConfiguration()
{
	// Set the decode mode based on the selected enum
	Mach1DecodeMode decodeMode = static_cast<Mach1DecodeMode>(DecodeMode.GetValue());
	m1Positional.setDecodeMode(decodeMode);
	
	// Update the required channel count
	int requiredChannels = GetRequiredChannelCount();
	this->MAX_INPUT_CHANNELS = requiredChannels;
	
	// Resize audio component arrays if needed
	if (LeftChannelsMain.Num() < requiredChannels)
	{
		LeftChannelsMain.SetNum(requiredChannels);
		RightChannelsMain.SetNum(requiredChannels);
	}
	
	// Set up sounds based on current configuration
	SetSoundsBasedOnConfiguration();
}

void AM1DecodeActor::SetSoundsBasedOnConfiguration()
{
	// Clear existing sound assignments
	ClearAllSounds();
	
	int requiredChannels = GetRequiredChannelCount();
	
	// Use individual mono channels (now the only mode)
	for (int i = 0; i < requiredChannels; i++)
	{
		USoundBase* channelSound = GetChannelByIndex(i);
		SoundsMain.Add(channelSound); // Add even if null to maintain proper indexing
	}
	
	// Ensure we have the exact number of channels required
	if (SoundsMain.Num() < requiredChannels)
	{
		// Pad with nulls if we don't have enough sounds
		while (SoundsMain.Num() < requiredChannels)
		{
			SoundsMain.Add(nullptr);
		}
	}
	else if (SoundsMain.Num() > requiredChannels)
	{
		// Trim excess sounds
		SoundsMain.SetNum(requiredChannels);
	}
}

int AM1DecodeActor::GetRequiredChannelCount()
{
	switch (DecodeMode.GetValue())
	{
		case Mach1DecodeMode_Spatial_4:
			return 4;
		case Mach1DecodeMode_Spatial_8:
			return 8;
		case Mach1DecodeMode_Spatial_14:
			return 14;
		default:
			return 8; // Default to 8-channel
	}
}

void AM1DecodeActor::ClearAllSounds()
{
	SoundsMain.Empty();
}

USoundBase* AM1DecodeActor::GetChannelByIndex(int index)
{
	// Return main channels
	switch (index)
	{
		case 0: return Channel1;
		case 1: return Channel2;
		case 2: return Channel3;
		case 3: return Channel4;
		case 4: return Channel5;
		case 5: return Channel6;
		case 6: return Channel7;
		case 7: return Channel8;
		case 8: return Channel9;
		case 9: return Channel10;
		case 10: return Channel11;
		case 11: return Channel12;
		case 12: return Channel13;
		case 13: return Channel14;
		default: return nullptr;
	}
}

int AM1DecodeActor::GetCurrentChannelCount()
{
	return GetRequiredChannelCount();
}

void AM1DecodeActor::RefreshDecodeConfiguration()
{
	UpdateDecodeConfiguration();
	
	// If already initialized, update the audio components
	if (isInited)
	{
		// Re-initialize with new configuration
		SetSoundSet();
	}
}

void AM1DecodeActor::StopAllAudioComponents()
{
	// Stop all audio components to prevent overlapping playback
	for (int i = 0; i < LeftChannelsMain.Num(); i++)
	{
		if (LeftChannelsMain[i])
		{
			LeftChannelsMain[i]->Stop();
			LeftChannelsMain[i]->SetSound(nullptr);
		}
	}
	
	for (int i = 0; i < RightChannelsMain.Num(); i++)
	{
		if (RightChannelsMain[i])
		{
			RightChannelsMain[i]->Stop();
			RightChannelsMain[i]->SetSound(nullptr);
		}
	}
}

void AM1DecodeActor::SetupIndividualChannelPlayback()
{
	// Individual mono channel setup with proper spatial processing
	M1Common::PrintDebug("Setting up individual mono channel playback");
	
	int validChannels = 0;
	
	for (int i = 0; i < FMath::Min(SoundsMain.Num(), MAX_INPUT_CHANNELS); i++)
	{
		if (SoundsMain[i])
		{
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 23) || ENGINE_MAJOR_VERSION == 5
			SoundsMain[i]->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
#else
			SoundsMain[i]->bVirtualizeWhenSilent = true;
#endif
			
			LeftChannelsMain[i]->SetSound(SoundsMain[i]);
			RightChannelsMain[i]->SetSound(SoundsMain[i]);
			validChannels++;
		}
		else
		{
			// Clear unused channels
			LeftChannelsMain[i]->SetSound(nullptr);
			RightChannelsMain[i]->SetSound(nullptr);
		}
	}
	
	if (Debug)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
			FString::Printf(TEXT("Individual Channels: %d/%d assigned"), validChannels, GetRequiredChannelCount()));
	}
	
	M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Individual channels: %d/%d assigned"), validChannels, GetRequiredChannelCount())));
}

// ========== MULTICHANNEL PROCESSING REMOVED ==========
// All multichannel complexity has been removed for cleaner, more reliable multi-mono processing
