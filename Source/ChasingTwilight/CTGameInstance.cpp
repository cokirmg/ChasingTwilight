#include "CTGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UCTGameInstance::UCTGameInstance()
{
}

void UCTGameInstance::Init()
{
	Super::Init();

	if(IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCTGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCTGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UCTGameInstance::OnJoinSessionComplete);
		}
	}
}

void UCTGameInstance::OnCreateSessionComplete(FName SessionName, bool Succeeded)
{
	if (Succeeded)
	{
		GetWorld()->ServerTravel("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap?listen");
	}
}

void UCTGameInstance::OnFindSessionComplete(bool Succeeded)
{
	if (Succeeded)
	{
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;

		if (SearchResults.Num())
		{
			SessionInterface->JoinSession(0, FName("Crete Session"), SearchResults[0]);
		}
	}
}

void UCTGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if(APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FString JoinAddress = "";
		SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
		if(JoinAddress != "")
		{
			PController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UCTGameInstance::CreateServer()
{
	UE_LOG(LogTemp, Warning, TEXT("CreateServer"));
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bIsLANMatch = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = 5;

	SessionInterface->CreateSession(0, FName("Crete Session"), SessionSettings);
}

void UCTGameInstance::JoinServer()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->QuerySettings.Set("SEARCH_PRESENCE", true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}