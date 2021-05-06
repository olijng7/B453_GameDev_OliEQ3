#pragma once

#include "../OliEQ3Plugin.h"

class OliEQ3PluginGUI final
	: public AK::Wwise::Plugin::PluginMFCWindows<>
	, public AK::Wwise::Plugin::GUIWindows
{
public:
	OliEQ3PluginGUI();

};
