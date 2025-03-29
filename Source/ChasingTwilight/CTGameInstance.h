#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/GameInstance.h"
#include "CTGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHASINGTWILIGHT_API UCTGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UCTGameInstance();

protected:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	IOnlineSessionPtr SessionInterface;

	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool Succeeded);
	virtual void OnFindSessionComplete(bool Succeeded);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(BlueprintCallable)
	void CreateServer();

	UFUNCTION(BlueprintCallable)
	void JoinServer();
};