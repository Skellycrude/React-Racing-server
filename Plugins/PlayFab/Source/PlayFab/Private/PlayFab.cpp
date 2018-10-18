//////////////////////////////////////////////////////////////////////////////////////////////
// Automatically generated cpp file for the UE4 PlayFab plugin.
// This cpp file is the main cpp file for the plugin. We set up globals here.
//////////////////////////////////////////////////////////////////////////////////////////////

#include "CoreMinimal.h"
#include "PlayFabPrivate.h"
#include "PlayFabJsonValue.h"
#include "PlayFabJsonObject.h"

#include "PlayFabClientAPI.h" 
#include "PlayFabAuthenticationAPI.h" 
#include "PlayFabCloudScriptAPI.h" 
#include "PlayFabDataAPI.h" 
#include "PlayFabGroupsAPI.h" 
#include "PlayFabProfilesAPI.h" 

const FString IPlayFab::PlayFabURL(TEXT(".playfabapi.com"));

class FPlayFab : public IPlayFab
{
    /** IModuleInterface implementation */
    virtual void StartupModule() override
    {
        //Force classes to be compiled on shipping build
        UPlayFabJsonObject::StaticClass();
        UPlayFabJsonValue::StaticClass();

        UPlayFabClientAPI::StaticClass();
        UPlayFabAuthenticationAPI::StaticClass();
        UPlayFabCloudScriptAPI::StaticClass();
        UPlayFabDataAPI::StaticClass();
        UPlayFabGroupsAPI::StaticClass();
        UPlayFabProfilesAPI::StaticClass();
    }

    virtual void ShutdownModule() override
    {

    }

};

IMPLEMENT_MODULE(FPlayFab, PlayFab)

DEFINE_LOG_CATEGORY(LogPlayFab);
