// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DG_PlayerController.generated.h"

/**
 * 구조:
 * - Character는 PlayerState에서 ASC / AttributeSet을 찾아서 사용
 * - PlayerController는 PlayerState의 ASC / AttributeSet을 사용하여 입력에 따른 능력 실행
 * 
 * 목적:
 * - 
 */
UCLASS()
class PROJECTDG_API ADG_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
	
	
	
};
