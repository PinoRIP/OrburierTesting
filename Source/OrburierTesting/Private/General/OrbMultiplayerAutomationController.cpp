// Fill out your copyright notice in the Description page of Project Settings.


#include "General/OrbMultiplayerAutomationController.h"
#include "General/OrbMultiplayerFunctionalTest.h"

void AOrbMultiplayerAutomationController::ServerRunTest_Implementation(const TArray<FString>& Params, AOrbMultiplayerFunctionalTest* testToExecute)
{
	testToExecute->ReceiveRunTest(Params, this);
}

void AOrbMultiplayerAutomationController::ClientRunTest_Implementation(const TArray<FString>& Params, AOrbMultiplayerFunctionalTest* testToExecute)
{
	testToExecute->ReceiveRunTest(Params, this);
}

void AOrbMultiplayerAutomationController::CancelTest(AOrbMultiplayerFunctionalTest* testToCancel)
{
	if(GetNetMode() == ENetMode::NM_Client)
	{
		ServerCancelTest(testToCancel);
	}
	else
	{
		ClientCancelTest(testToCancel);
	}
}

void AOrbMultiplayerAutomationController::ReportTestFinish(const FAutomationTestExecutionInfo& OutInfo, AOrbMultiplayerFunctionalTest* testToReportTo)
{
	TArray<FOrbMultiplayerAutomationReport> Reports;
	for (const FAutomationExecutionEntry& Entry : OutInfo.GetEntries())
	{
		if(Entry.Event.Type == EAutomationEventType::Error || Entry.Event.Type == EAutomationEventType::Warning)
		{
			FOrbMultiplayerAutomationReport& item = Reports.Add_GetRef(FOrbMultiplayerAutomationReport());
			item.IsError = Entry.Event.Type == EAutomationEventType::Error;
			item.Message = Entry.Event.Message;
			item.Context = Entry.Event.Context;
			item.Artifact = Entry.Event.Artifact;
		}
	}
	
	if(GetNetMode() == ENetMode::NM_Client)
	{
		ServerReportTestFinish(Reports, testToReportTo);
	}
	else
	{
		ClientReportTestFinish(Reports, testToReportTo);
	}
}

void AOrbMultiplayerAutomationController::ServerReportTestFinish_Implementation(const TArray<FOrbMultiplayerAutomationReport>& Reports, AOrbMultiplayerFunctionalTest* testToReportTo)
{
	testToReportTo->ReceiveTestFinishReport(Reports, this);
}

void AOrbMultiplayerAutomationController::ClientReportTestFinish_Implementation(const TArray<FOrbMultiplayerAutomationReport>& Reports, AOrbMultiplayerFunctionalTest* testToReportTo)
{
	testToReportTo->ReceiveTestFinishReport(Reports, this);
}

void AOrbMultiplayerAutomationController::ServerCancelTest_Implementation(AOrbMultiplayerFunctionalTest* testToCancel)
{
	testToCancel->ReceiveCancelTest();
}

void AOrbMultiplayerAutomationController::ClientCancelTest_Implementation(AOrbMultiplayerFunctionalTest* testToCancel)
{
	testToCancel->ReceiveCancelTest();
}
