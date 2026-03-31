//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "M1Scheduler.generated.h"

UCLASS()
class MACH1DECODEPLUGIN_API UM1Scheduler : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "M1 Utility")
	static void PlaySync(const TArray<class AM1DecodeActor*>& AM1Actors);

};
