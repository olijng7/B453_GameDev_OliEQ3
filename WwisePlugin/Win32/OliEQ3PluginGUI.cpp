
#include "OliEQ3PluginGUI.h"

OliEQ3PluginGUI::OliEQ3PluginGUI()
{
}

ADD_AUDIOPLUGIN_CLASS_TO_CONTAINER(
    OliEQ3,            // Name of the plug-in container for this shared library
    OliEQ3PluginGUI,   // Authoring plug-in class to add to the plug-in container
    OliEQ3FX           // Corresponding Sound Engine plug-in class
);
