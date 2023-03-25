//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#pragma once

#include "GameFramework/Actor.h"
#include "M1BaseActor.h"
#include "M1HorizonActor.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MACH1DECODEPLUGIN_API AM1HorizonActor : public AM1BaseActor
{
	GENERATED_BODY()

	void SetSoundsMain();
	void SetSoundsBlendMode();

public:
	// Sets default values for this actor's properties
	AM1HorizonActor();

	// Audio positions for the 4channel quad
	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|Main")
		USoundBase* ChannelMain1;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|Main")
		USoundBase* ChannelMain2;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|Main")
		USoundBase* ChannelMain3;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|Main")
		USoundBase* ChannelMain4;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|BlendMode")
		USoundBase* ChannelBlend1;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|BlendMode")
		USoundBase* ChannelBlend2;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|BlendMode")
		USoundBase* ChannelBlend3;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|BlendMode")
		USoundBase* ChannelBlend4;

};
