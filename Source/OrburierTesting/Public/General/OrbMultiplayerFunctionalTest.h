// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "OrbMultiplayerAutomationReport.h"
#include "OrbMultiplayerFunctionalTest.generated.h"

class AOrbMultiplayerAutomationController;

UCLASS()
class ORBURIERTESTING_API AOrbMultiplayerFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AOrbMultiplayerFunctionalTest();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Orburier|Testing|Setup")
	bool RunOnStandalone = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Orburier|Testing|Setup")
	bool RunOnDedicatedServer = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Orburier|Testing|Setup")
	bool RunOnListenServer = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Orburier|Testing|Setup")
	bool RunOnClient = true;
	
	ENetMode CurrentNetMode = ENetMode::NM_MAX;

	virtual bool CommunicateRunTest(const TArray<FString>& Params);
	void SendRunTestRpcToServer(const TArray<FString>& Params);
	void CueRunTestInstructionToClient(const TArray<FString>& Params);
	virtual bool RunMultiplayerTest(const TArray<FString>& Params);

private:
	void ReceiveRunTest(const TArray<FString>& Params, AOrbMultiplayerAutomationController* source);
	UPROPERTY()
	AOrbMultiplayerAutomationController* RunTestSource = nullptr;

	UPROPERTY()
	AOrbMultiplayerAutomationController* LocalMultiplayerAutomationController = nullptr;
	AOrbMultiplayerAutomationController* FindLocalMultiplayerAutomationController() const;

	bool bIsFinishingNetwork = false;

	void ReceiveTestFinishReport(const TArray<FOrbMultiplayerAutomationReport>& Reports, AOrbMultiplayerAutomationController* Origin);
	void ReceiveCancelTest();
	void CancelPendingTests();
	
	TArray<AOrbMultiplayerAutomationController*> PendingTestResults;
	EFunctionalTestResult InitialTestResult;
	FString InitialMessage;
	float InitialEndTime = 0.f;
	bool IsSkipping = false;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual bool RunTest(const TArray<FString>& Params) override sealed;
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message)override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Orburier|Testing|Setup")
	float NetworkingStartupTimeLimit = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Orburier|Testing|Setup")
	float NetworkingFinishTimeLimit = 20.f;

	virtual void PrepareTestAfterNetwork();
	/** Called when the test is ready to prepare */
	UPROPERTY(BlueprintAssignable)
	FFunctionalTestEventSignature OnTestPrepareAfterNetwork;

private:
	float ConfiguredPreparationTimeLimit = 0.f;

	friend AOrbMultiplayerAutomationController;
};
