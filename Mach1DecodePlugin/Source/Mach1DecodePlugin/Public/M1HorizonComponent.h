//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#pragma once

#include "M1BaseComponent.h"
#include "M1HorizonComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MACH1DECODEPLUGIN_API UM1HorizonComponent : public UM1BaseComponent
{
	GENERATED_BODY()

	void SetSoundsMain();

public:
	// Sets default values for this actor's properties
	UM1HorizonComponent();

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|Main")
		USoundBase* ChannelMain1;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|Main")
		USoundBase* ChannelMain2;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|Main")
		USoundBase* ChannelMain3;

	UPROPERTY(EditAnywhere, Category = "Mach1 Spatial Files|Main")
		USoundBase* ChannelMain4;

};
