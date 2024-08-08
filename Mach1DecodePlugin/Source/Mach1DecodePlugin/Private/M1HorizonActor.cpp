//  Mach1 SDK
//  Copyright � 2017 Mach1. All rights reserved.
//

#include "M1HorizonActor.h"
#include "Mach1DecodePluginPrivatePCH.h"

void AM1HorizonActor::SetSoundsMain()
{
	SoundsMain.Add(ChannelMain1);
	SoundsMain.Add(ChannelMain2);
	SoundsMain.Add(ChannelMain3);
	SoundsMain.Add(ChannelMain4);
}

AM1HorizonActor::AM1HorizonActor()
{
	InitComponents(4);
	m1Positional.setDecodeAlgoType(Mach1DecodeAlgoType::Mach1DecodeAlgoSpatial_4);
}
 
