//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#include "M1DecodeComponent.h"
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

// Sets default values for this component's properties
UM1DecodeComponent::UM1DecodeComponent()
{
	// Call the InitComponents with default maximum channels (14)
	InitComponents(14);
}

void UM1DecodeComponent::InitComponents(int32 MaxSpatialInputChannels)
{
	this->MAX_INPUT_CHANNELS = MaxSpatialInputChannels;

	// Set this actor to call Tick() every frame.
	PrimaryComponentTick.bCanEverTick = true;

	LeftChannelsMain.SetNum(MAX_INPUT_CHANNELS);
	RightChannelsMain.SetNum(MAX_INPUT_CHANNELS);

	Volume = 1;
	for (int i = 0; i < MAX_INPUT_CHANNELS * 2; i++) GainCoeffs.Add(1);

	m1Positional.setPlatformType(Mach1PlatformType::Mach1PlatformUE);
	
	// Set default decode mode
	UpdateDecodeConfiguration();
}

void UM1DecodeComponent::Init()
{
	if (!isInited)
	{
		if (AttachToPlayerPawnCamera) {
			// Find the player pawn camera and attach to that as the listener reference for the Mach1Decode object
			if (APlayerController* player = GetWorld()->GetFirstPlayerController())//UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
			{
				TArray<UCameraComponent*> Cameras;
				if (APawn* playerPawn = player->GetPawn())
				{
					playerPawn->GetComponents<UCameraComponent>(Cameras);
					if (Cameras.Num() < 1)
					{
						APlayerCameraManager* GameCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
						listenerReferenceComponent = GameCameraManager->GetRootComponent();
					}
					else
					{
						// if there are many player pawn cameras grab the first index
						listenerReferenceComponent = Cameras[0];
					}
				}
			}
		} 
		else {
			// Use the parent component as the listener reference for the Mach1Decode object
			ParentComponent = GetAttachParent();

			if (ParentComponent) {
				listenerReferenceComponent = ParentComponent;
			} else {
				// grab the parent actor of components
				listenerReferenceComponent = GetAttachmentRoot();
			}
		}

		if (listenerReferenceComponent)
		{
			NullAttenuation = NewObject<USoundAttenuation>(this);
			NullAttenuation->Attenuation.bAttenuate = false;
			NullAttenuation->Attenuation.bSpatialize = true;

			for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
			{
				LeftChannelsMain[i] = NewObject <UAudioComponent>(listenerReferenceComponent, FName(*FString::Printf(TEXT("SoundCubeWalls %d_L_%d"), GetUniqueID(), i)));
				RightChannelsMain[i] = NewObject <UAudioComponent>(listenerReferenceComponent, FName(*FString::Printf(TEXT("SoundCubeWalls %d_R_%d"), GetUniqueID(), i)));

				LeftChannelsMain[i]->RegisterComponent(); // only for runtime
				RightChannelsMain[i]->RegisterComponent(); // only for runtime

				LeftChannelsMain[i]->SetRelativeLocation(FVector(0, -1, 0));
				RightChannelsMain[i]->SetRelativeLocation(FVector(0, 1, 0));

				LeftChannelsMain[i]->AttenuationSettings = NullAttenuation;
				RightChannelsMain[i]->AttenuationSettings = NullAttenuation;

				LeftChannelsMain[i]->AttachToComponent(listenerReferenceComponent, FAttachmentTransformRules::KeepRelativeTransform); // AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);//   AttachTo(sceneComponent);
				RightChannelsMain[i]->AttachToComponent(listenerReferenceComponent, FAttachmentTransformRules::KeepRelativeTransform);
			}

			isInited = true;
		}
	}
}

void UM1DecodeComponent::SetSoundSet()
{
	if (isInited)
	{
		// First, stop all audio to prevent overlapping playback
		StopAllAudioComponents();
		
		SoundsMain.Empty();

		// Use the new configuration-based sound setup
		SetSoundsBasedOnConfiguration();

        // INDIVIDUAL MONO CHANNELS MODE: Use separate mono files
        SetupIndividualChannelPlayback();
	}
}

void UM1DecodeComponent::Play()
{
	if (isInited)
	{
		// For perfect synchronization, stop all first
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
	}

	if (!isInited)
	{
		needToPlayAfterInit = true;
	}
}

void UM1DecodeComponent::Pause()
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

void UM1DecodeComponent::Resume()
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

void UM1DecodeComponent::Seek(float time)
{
	if (isInited)
	{
		// For perfect synchronization, stop all first
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
				LeftChannelsMain[i]->Play(time);
				RightChannelsMain[i]->Play(time);
			}
		}
		
		if (Debug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, 
				FString::Printf(TEXT("Component Synchronized seek to %.2fs"), time));
		}
	}
}

void UM1DecodeComponent::Stop()
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
void UM1DecodeComponent::BeginPlay()
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
void UM1DecodeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (manualPawn != nullptr)
	{
		TArray<UCameraComponent*> CameraComponents;
		manualPawn->GetComponents<UCameraComponent>(CameraComponents);

		for (UCameraComponent* CameraComp : CameraComponents)
		{
			if (CameraComp->IsActive())
			{
				if (useReferenceObjectRotation) PlayerRotation = CameraComp->GetComponentRotation().Quaternion();
				if (useReferenceObjectPosition) PlayerPosition = CameraComp->GetComponentLocation();
			}
		}

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
		if (useReferenceObjectRotation) PlayerRotation = manualCameraActor->GetCameraComponent()->GetComponentRotation().Quaternion();
		if (useReferenceObjectPosition) PlayerPosition = manualCameraActor->GetCameraComponent()->GetComponentLocation();
	}
	else if (AttachToPlayerPawnCamera) 
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			FRotator hmdRotator;
			FVector hmdPosition;
			UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(hmdRotator, hmdPosition);

			PlayerRotation = hmdRotator.Quaternion();
			PlayerPosition = hmdPosition;
		}
		else if (APlayerController* player = GetWorld()->GetFirstPlayerController()) 
		{
			if (APawn* playerPawn = player->GetPawn()) 
			{
				PlayerRotation = FQuat::Identity;
				PlayerPosition = FVector(0, 0, 0);
			}
			else
			{
				APlayerCameraManager* playerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
				FRotator cameraRotator = playerCameraManager->GetCameraRotation();
				FVector cameraPosition = playerCameraManager->GetCameraLocation();

				PlayerRotation = cameraRotator.Quaternion();
				PlayerPosition = cameraPosition;
			}
		}
		else 
		{
			// did not find a player camera or HMD, reverting to parent component
		}
	}
	else 
	{
		// Use the parent component as the listener orientation reference
		PlayerRotation = listenerReferenceComponent->GetComponentQuat();
		PlayerPosition = listenerReferenceComponent->GetComponentLocation();
	}

	PlayerRotation = PlayerRotation * FQuat::MakeFromEuler(cameraManualAngleOffset);

	if (Debug) {
		std::string info;

		info = "Listener Rotation:  " + M1Common::toDebugString(PlayerRotation.Euler());
		GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());

		info = "Listener Position:  " + M1Common::toDebugString(PlayerPosition);
		GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());

		info = "Reference Rotation:  " + M1Common::toDebugString(GetComponentRotation().Quaternion().Euler());
		GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());

		info = "Reference Position:  " + M1Common::toDebugString(GetComponentLocation());
		GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, info.c_str());
	}

	float masterGain = Volume;

	m1Positional.setMuteWhenOutsideObject(muteWhenOutsideObject);
	m1Positional.setMuteWhenInsideObject(muteWhenInsideObject);
	m1Positional.setUseAttenuation(useAttenuation);
	m1Positional.setUsePlaneCalculation(usePlaneCalculation);
	m1Positional.setUseYawForRotation(useYawForRotation);
	m1Positional.setUsePitchForRotation(usePitchForRotation);
	m1Positional.setUseRollForRotation(useRollForRotation);

	FVector listenerAngle = (PlayerRotation.Euler());
	m1Positional.setListenerPosition(M1Common::ConvertToMach1Point3D(PlayerPosition));
	m1Positional.setListenerRotation(M1Common::ConvertToMach1Point3D(listenerAngle));
	//m1Positional.setListenerRotationQuat(M1Common::ConvertToMach1Point4D(PlayerRotation));

	FVector decoderAngle = (GetComponentRotation().Euler());
	m1Positional.setDecoderAlgoPosition(M1Common::ConvertToMach1Point3D(GetComponentLocation()));
	m1Positional.setDecoderAlgoRotation(M1Common::ConvertToMach1Point3D(decoderAngle));
	//m1Positional.setDecoderAlgoRotationQuat(M1Common::ConvertToMach1Point4D(GetComponentRotation().Quaternion()));

	m1Positional.setDecoderAlgoScale(M1Common::ConvertToMach1Point3D(GetComponentScale()));

	m1Positional.evaluatePositionResults();

	if (useAttenuation)
	{
		m1Positional.setAttenuationCurve(attenuationCurve ? attenuationCurve->GetFloatValue(m1Positional.getDist()) : 1);
	}

	float coeffs[18];
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

		for (int i = 0; i < 8; i++)
		{
			Mach1Point3D p = points[i];
			(std::swap)(p.y, p.z); // convert to glm
			FVector point = FVector(p.z, p.x, p.y); // convertion to platfrom

			DrawDebugString(GetWorld(), PlayerPosition + quat * (point * GetComponentScale()), FString(std::to_string(i).c_str()), 0, FColor::White, 0);
			DrawDebugSphere(GetWorld(), PlayerPosition + quat * ((point + FVector(-0.1, 0, 0)) * GetComponentScale()), 10 * coeffs[i * 2 + 0], 16, FColor::Red, false, 0.5f);
			DrawDebugSphere(GetWorld(), PlayerPosition + quat * ((point + FVector(+0.1, 0, 0)) * GetComponentScale()), 10 * coeffs[i * 2 + 1], 16, FColor::Blue, false, 0.5f);
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

void UM1DecodeComponent::SetVolumeMain(float volume)
{
	if (isInited)
	{
		float masterGain = FMath::Max(MIN_SOUND_VOLUME, this->Volume * volume);
		
        // INDIVIDUAL MONO CHANNELS MODE: Apply per-channel spatial processing
        SetVolumeIndividualChannels(masterGain);
	}
}

void UM1DecodeComponent::SetVolumeIndividualChannels(float masterGain)
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
		FString CoeffInfo = TEXT("Component Individual Channels: ");
		for (int i = 0; i < FMath::Min(4, MAX_INPUT_CHANNELS); i++) // Show first 4 channels
		{
			CoeffInfo += FString::Printf(TEXT("Ch%d[L=%.2f,R=%.2f] "), i, GainCoeffs[i * 2], GainCoeffs[i * 2 + 1]);
		}
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Orange, CoeffInfo);
	}
}

void UM1DecodeComponent::StopAllAudioComponents()
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

void UM1DecodeComponent::SetupIndividualChannelPlayback()
{
	M1Common::PrintDebug("Component: Setting up individual mono channel playback");
	
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
			FString::Printf(TEXT("Component Individual Channels: %d/%d assigned"), validChannels, GetRequiredChannelCount()));
	}
	
	M1Common::PrintDebug(TCHAR_TO_ANSI(*FString::Printf(TEXT("Component individual channels: %d/%d assigned"), validChannels, GetRequiredChannelCount())));
}

void UM1DecodeComponent::SetSoundsMain()
{
}

TArray<USoundBase*> UM1DecodeComponent::GetSoundsMain()
{
	return SoundsMain;
}

TArray<UAudioComponent*> UM1DecodeComponent::GetAudioComponentsMain()
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

// ========== FLEXIBLE DECODE CONFIGURATION METHODS ==========

void UM1DecodeComponent::UpdateDecodeConfiguration()
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

void UM1DecodeComponent::SetSoundsBasedOnConfiguration()
{
	// Clear existing sound assignments
	ClearAllSounds();
	
	int requiredChannels = GetRequiredChannelCount();
	
    // Use individual mono channels
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

int UM1DecodeComponent::GetRequiredChannelCount()
{
	switch (DecodeMode.GetValue())
	{
		case Mach1DecodeMode_Spatial_4_Component:
			return 4;
		case Mach1DecodeMode_Spatial_8_Component:
			return 8;
		case Mach1DecodeMode_Spatial_14_Component:
			return 14;
		default:
			return 8; // Default to 8-channel
	}
}

void UM1DecodeComponent::ClearAllSounds()
{
	SoundsMain.Empty();
}

USoundBase* UM1DecodeComponent::GetChannelByIndex(int index)
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

int UM1DecodeComponent::GetCurrentChannelCount()
{
	return GetRequiredChannelCount();
}

void UM1DecodeComponent::RefreshDecodeConfiguration()
{
	UpdateDecodeConfiguration();
	
	// If already initialized, update the audio components
	if (isInited)
	{
		// Re-initialize with new configuration
		SetSoundSet();
	}
}
