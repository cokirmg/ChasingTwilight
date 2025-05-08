// Fill out your copyright notice in the Description page of Project Settings.


#include "AC_Pickable.h"

// Sets default values for this component's properties
UAC_Pickable::UAC_Pickable()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}




// Called when the game starts
void UAC_Pickable::BeginPlay()
{
	Super::BeginPlay();

	actualAmmount = totalAmmount;
	
}


// Called every frame
void UAC_Pickable::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UAC_Pickable::GetResource(float ammount)
{
	if (actualAmmount - ammount >= 0)
	{
		actualAmmount -= ammount;
		return true;
	}
	else
	{
		return false;
	}
}