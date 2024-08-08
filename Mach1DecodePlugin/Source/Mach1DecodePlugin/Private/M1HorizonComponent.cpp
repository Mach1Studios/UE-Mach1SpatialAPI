//  Mach1 SDK
//  Copyright © 2017 Mach1. All rights reserved.
//

#include "M1HorizonComponent.h"
#include "Mach1DecodePluginPrivatePCH.h"

void UM1HorizonComponent::SetSoundsMain()
{
	SoundsMain.Add(ChannelMain1);
	SoundsMain.Add(ChannelMain2);
	SoundsMain.Add(ChannelMain3);
	SoundsMain.Add(ChannelMain4);
}

UM1HorizonComponent::UM1HorizonComponent()
{
	InitComponents(4);
	m1Positional.setDecodeAlgoType(Mach1DecodeAlgoType::Mach1DecodeAlgoSpatial_4);
}
 
