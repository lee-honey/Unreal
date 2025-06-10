// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterNonPlayer.h"
#include "Engine/AssetManager.h"
#include "AI/ABAIController.h"
#include "CharacterStat/ABCharacterStatComponent.h"

AABCharacterNonPlayer::AABCharacterNonPlayer()
{
	GetMesh()->SetHiddenInGame(true); // 메쉬가로딩될때까지는숨겨놓도록

	AIControllerClass = AABAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AABCharacterNonPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ensure(NPCMeshes.Num() > 0);
	int32 RandIndex = FMath::RandRange(0, NPCMeshes.Num() - 1);
	NPCMeshHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(NPCMeshes[RandIndex], FStreamableDelegate::CreateUObject(this, &AABCharacterNonPlayer::NPCMeshLoadCompleted));
} // 비동기방식으로 메시지정

void AABCharacterNonPlayer::SetDead()
{
	Super::SetDead();

	FTimerHandle DeadTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DeadTimerHandle, FTimerDelegate::CreateLambda(
		[&]()
		{
			Destroy();
		}
	), DeadEventDelayTime, false);
}

void AABCharacterNonPlayer::NPCMeshLoadCompleted()
{
	if (NPCMeshHandle.IsValid()) // 로딩이완료되면핸들값보고 메시가져오고메시지정
	{
		USkeletalMesh* NPCMesh = Cast<USkeletalMesh>(NPCMeshHandle->GetLoadedAsset());
		if (NPCMesh)
		{
			GetMesh()->SetSkeletalMesh(NPCMesh);
			GetMesh()->SetHiddenInGame(false); // 안보이게했다가 로딩되면 그때 보여지도록
		}
	}

	NPCMeshHandle->ReleaseHandle();
}

float AABCharacterNonPlayer::GetAIPatrolRadius()
{
	return 800.0f;
}

float AABCharacterNonPlayer::GetAIDetectRange()
{
	return 400.0f;
}

float AABCharacterNonPlayer::GetAIAttackRange()
{
	return Stat->GetTotalStat().AttackRange + Stat->GetAttackRadius() * 2;
}

float AABCharacterNonPlayer::GetAITurnSpeed()
{
	return 2.0f;
}

void AABCharacterNonPlayer::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void AABCharacterNonPlayer::AttackByAI()
{
	ProcessComboCommand();
}

void AABCharacterNonPlayer::NotifyComboActionEnd()
{
	Super::NotifyComboActionEnd();
	OnAttackFinished.ExecuteIfBound();
}
