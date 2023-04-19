//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

#include "M1BaseActor.h"

#include <vector>
#include <thread>

#include "M1Scheduler.generated.h"

UCLASS()
class MACH1DECODEPLUGIN_API UM1Scheduler : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

/*
	UFUNCTION(BlueprintCallable, Category = "M1 Utility")
	static inline void PlayArrayAsync(TArray<AM1BaseActor*> AM1Actors) {
		for (auto& actor : AM1Actors)
		{
			auto t = std::thread([&] {
				actor->Play();
			});
			//t.detach();
			t.join();
		}
	}
*/

	UFUNCTION(BlueprintCallable, Category = "M1 Utility")
	static inline void PlaySync(TArray<AM1BaseActor*> AM1Actors) {
		for (auto& actor : AM1Actors)
		{
			actor->Play();
		}
	}

};
