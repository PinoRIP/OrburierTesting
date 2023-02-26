// Fill out your copyright notice in the Description page of Project Settings.


#include "General/OrbMultiplayerFunctionalTest.h"

#include "FunctionalTestBase.h"
#include "General/OrbMultiplayerAutomationController.h"
#include "Utils/OrburierTestingLogging.h"


// Sets default values
AOrbMultiplayerFunctionalTest::AOrbMultiplayerFunctionalTest()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AOrbMultiplayerFunctionalTest::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AOrbMultiplayerFunctionalTest::CommunicateRunTest(const TArray<FString>& Params)
{
	// If the current net mode is standalone we dont need to worry about communication between net clients
	if(!RunOnStandalone && CurrentNetMode == NM_Standalone)
		return false;
	
	if(CurrentNetMode == NM_ListenServer)
	{
		if(RunOnClient)
			CueRunTestInstructionToClient(Params);
		
		return RunOnListenServer;
	}

	if(CurrentNetMode == NM_DedicatedServer)
	{
		if(RunOnClient)
			CueRunTestInstructionToClient(Params);
		
		return RunOnDedicatedServer;
	}

	if(CurrentNetMode == NM_Client)
	{
		if(!RunTestSource)
			SendRunTestRpcToServer(Params);
		
		return RunOnClient;
	}

	UE_LOG(OrburierTestingLog, Error, TEXT("Invalid CurrentNetMode state in %s"), *GetName())
	return false;
}

void AOrbMultiplayerFunctionalTest::SendRunTestRpcToServer(const TArray<FString>& Params)
{
	if(!LocalMultiplayerAutomationController)
	{
		UE_LOG(OrburierTestingLog, Warning, TEXT("Can't communicate test start to server for text %s"), *GetName())
		return;
	}

	LocalMultiplayerAutomationController->ServerRunTest(Params, this);
	PendingTestResults.Add(LocalMultiplayerAutomationController);
}

void AOrbMultiplayerFunctionalTest::CueRunTestInstructionToClient(const TArray<FString>& Params)
{
	if(const UWorld* World = GetWorld())
	{
		for(FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if(Iterator->IsValid())
			{
				if(APlayerController* PlayerController = Iterator->Get())
				{
					if(AOrbMultiplayerAutomationController* MultiplayerAutomationController = Cast<AOrbMultiplayerAutomationController>(PlayerController))
					{
						if(MultiplayerAutomationController != RunTestSource)
						{
							MultiplayerAutomationController->ClientRunTest(Params, this);
							PendingTestResults.Add(MultiplayerAutomationController);
						}
					}
				}
			}
		}
	}
}

void AOrbMultiplayerFunctionalTest::ReceiveRunTest(const TArray<FString>& Params, AOrbMultiplayerAutomationController* source)
{
	RunTestSource = source;
	RunTest(Params);
}

// Called every frame
void AOrbMultiplayerFunctionalTest::Tick(float DeltaTime)
{
	if(!bIsFinishingNetwork)
	{
		if(IsSkipping)
		{
			FinishTest(EFunctionalTestResult::Succeeded, "Not run for this role");
			return;
		}
		
		Super::Tick(DeltaTime);
	}
	else if(bIsRunning || IsSkipping)
	{
		if(InitialEndTime == 0.f)
			InitialEndTime = TotalTime;
		
		TotalTime += DeltaTime;

		const int32 count = PendingTestResults.Num();
		if(count <= 0)
		{
			FinishTest(InitialTestResult, InitialMessage);
		}
		else if(TotalTime > InitialEndTime + (NetworkingFinishTimeLimit + (RunTestSource ? 0 : NetworkingFinishTimeLimit)))
		{
			CancelPendingTests();
			FinishTest(EFunctionalTestResult::Error, FString::FromInt(count) + " pending test result(s) did not come in time (" + FString::SanitizeFloat(NetworkingFinishTimeLimit) + "ms)! Original result: " + (InitialTestResult == EFunctionalTestResult::Succeeded ? "Success" : "Fail") + " - " + InitialMessage);
		}
	}
}

AOrbMultiplayerAutomationController* AOrbMultiplayerFunctionalTest::FindLocalMultiplayerAutomationController() const
{
	if(const UWorld* World = GetWorld())
	{
		for(FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if(Iterator->IsValid())
			{
				if(APlayerController* PlayerController = Iterator->Get(); PlayerController->IsLocalController())
				{
					if(AOrbMultiplayerAutomationController* MultiplayerAutomationController = Cast<AOrbMultiplayerAutomationController>(PlayerController))
					{
						return MultiplayerAutomationController;
					}
				}
			}
		}
	}

	return nullptr;
}

void AOrbMultiplayerFunctionalTest::ReceiveTestFinishReport(const TArray<FOrbMultiplayerAutomationReport>& Reports, AOrbMultiplayerAutomationController* Origin)
{
	const int32 index = PendingTestResults.Find(Origin);
	if(index >= 0)
	{
		bool hasError = false;
		FFunctionalTestBase* CurrentFunctionalTest = static_cast<FFunctionalTestBase*>(FAutomationTestFramework::Get().GetCurrentTest());
		for (const FOrbMultiplayerAutomationReport& Report : Reports) 
		{
			CurrentFunctionalTest->AddEvent(FAutomationEvent(Report.IsError ? EAutomationEventType::Error : EAutomationEventType::Warning, "RECEIVED: " + Report.Message, Report.Context));
			hasError = hasError || Report.IsError;
		}

		PendingTestResults.RemoveAt(index);
	}
}

void AOrbMultiplayerFunctionalTest::ReceiveCancelTest()
{
	Super::FinishTest(EFunctionalTestResult::Succeeded, "Test was canceled by remote");
}

void AOrbMultiplayerFunctionalTest::CancelPendingTests()
{
	for (AOrbMultiplayerAutomationController* PendingTestResult : PendingTestResults)
	{
		PendingTestResult->CancelTest(this);
	}

	PendingTestResults.Empty(PendingTestResults.Num());
}

bool AOrbMultiplayerFunctionalTest::RunTest(const TArray<FString>& Params)
{
	LocalMultiplayerAutomationController = FindLocalMultiplayerAutomationController();
	CurrentNetMode = GetNetMode();
	bIsFinishingNetwork = false;
	InitialMessage.Empty();
	InitialTestResult = EFunctionalTestResult::Default;
	InitialEndTime = 0.f;
	PendingTestResults.Empty(PendingTestResults.Num());
	IsSkipping = false;
	
	if(ConfiguredPreparationTimeLimit == 0.f)
		ConfiguredPreparationTimeLimit = PreparationTimeLimit;
	PreparationTimeLimit = NetworkingStartupTimeLimit + ConfiguredPreparationTimeLimit;

	if(!CommunicateRunTest(Params))
	{
		IsSkipping = true;
		return true;
	}
	
	return RunMultiplayerTest(Params);
}

void AOrbMultiplayerFunctionalTest::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	if (PendingTestResults.Num() > 0)
	{
		InitialTestResult = TestResult;
		InitialMessage = Message;
		bIsFinishingNetwork = true;
		return;
	}
	
	AOrbMultiplayerAutomationController* reportTarget = nullptr;
	if(RunTestSource)
	{
		reportTarget = RunTestSource;
	}
	
	RunTestSource = nullptr;
	Super::FinishTest(TestResult, Message);

	if(reportTarget)
	{
		FAutomationTestExecutionInfo OutInfo;
		if (const FFunctionalTestBase* FunctionalTest = static_cast<FFunctionalTestBase*>(FAutomationTestFramework::Get().GetCurrentTest()))
		{
			FunctionalTest->GetExecutionInfo(OutInfo);
		}
		else
		{
			OutInfo.AddError("Could not fetch current functional test!");
		}
		reportTarget->ReportTestFinish(OutInfo, this);
	}
		
}

bool AOrbMultiplayerFunctionalTest::RunMultiplayerTest(const TArray<FString>& Params)
{
	return Super::RunTest(Params);
}

void AOrbMultiplayerFunctionalTest::PrepareTestAfterNetwork()
{
}
