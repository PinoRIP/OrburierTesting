// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OrbMultiplayerAutomationReport.generated.h"

USTRUCT()
struct FOrbMultiplayerAutomationReport
{
	GENERATED_BODY()

	UPROPERTY()
	bool IsError;
	
	UPROPERTY()
	FString Message;
	
	UPROPERTY()
	FString Context;

	UPROPERTY()
	FGuid Artifact;
};
