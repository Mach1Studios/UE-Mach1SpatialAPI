//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#pragma once

#include "Mach1Decode.h"
#include "Mach1DecodePositional.h"
#include <sstream>

#ifndef SMALL_NUMBER
    #define SMALL_NUMBER 0
#endif
#define MIN_SOUND_VOLUME (SMALL_NUMBER*2) // used if you want to make sure components never go silent and are restarted by UE

class M1Common {
public:
	template<typename T>
	static std::string toDebugString(const T& value)
	{
		std::ostringstream oss;
		oss.precision(2);
		oss << std::fixed << value;
		return oss.str();
	}

	static std::string toDebugString(const FVector& value)
	{
		std::ostringstream oss;
		oss.precision(2);
		oss << std::fixed << "(" << value.X << ", " << value.Y << ", " << value.Z << ")";
		return oss.str();
	}

	static void PrintDebug(const char* str) {
		UE_LOG(LogTemp, Warning, TEXT("%s\r\n"), *FString(str));
	}

	static Mach1Point3D ConvertToMach1Point3D(FVector vec)
	{
		return Mach1Point3D{ (float)vec.X, (float)vec.Y, (float)vec.Z };
	}

	static Mach1Point4D ConvertToMach1Point4D(FQuat quat)
	{
		return Mach1Point4D{ (float)quat.X, (float)quat.Y, (float)quat.Z, (float)quat.W };
	}
};
