// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrbMultiplayerAutomationReport.h"
#include "OrbMultiplayerAutomationController.generated.h"


/**
 * 
 */
UCLASS()
class ORBURIERTESTING_API AOrbMultiplayerAutomationController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void ServerRunTest(const TArray<FString>& Params, AOrbMultiplayerFunctionalTest* testToExecute);

	UFUNCTION(Client, Reliable)
	void ClientRunTest(const TArray<FString>& Params, AOrbMultiplayerFunctionalTest* testToExecute);
	
	// Ends the remote test
	void CancelTest(AOrbMultiplayerFunctionalTest* testToCancel);

	// Reports on the end of
	void ReportTestFinish(const FAutomationTestExecutionInfo& OutInfo, AOrbMultiplayerFunctionalTest* testToReportTo);

private:
	UFUNCTION(Server, Reliable)
	void ServerReportTestFinish(const TArray<FOrbMultiplayerAutomationReport>& Reports, AOrbMultiplayerFunctionalTest* testToReportTo);

	UFUNCTION(Client, Reliable)
	void ClientReportTestFinish(const TArray<FOrbMultiplayerAutomationReport>& Reports, AOrbMultiplayerFunctionalTest* testToReportTo);
	
	UFUNCTION(Server, Reliable)
	void ServerCancelTest(AOrbMultiplayerFunctionalTest* testToCancel);

	UFUNCTION(Client, Reliable)
	void ClientCancelTest(AOrbMultiplayerFunctionalTest* testToCancel);
};
