#include "M1Scheduler.h"

#include "M1DecodeActor.h"

void UM1Scheduler::PlaySync(const TArray<AM1DecodeActor*>& AM1Actors)
{
	for (AM1DecodeActor* Actor : AM1Actors)
	{
		if (IsValid(Actor))
		{
			Actor->Play();
		}
	}
}

