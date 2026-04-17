#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"


/**
 * Debug 유틸 네임스페이스
 *
 * 목적:
 * - OnScreen Debug
 * - Log 출력
 * - 프로젝트 전체에서 공통 사용
 *
 * 사용 예:
 * Debug::Print(TEXT("Hello"));
 * Debug::PrintFloat(TEXT("Speed"), SpeedValue);
 */

namespace Debug
{
    /**
     * 문자열 출력
     *
     * Msg      : 출력 메시지
     * Color    : 화면 표시 색상
     * InKey    : 메시지 갱신용 Key (-1이면 새 메시지)
     * Duration : 화면 표시 시간
     */
    inline void Print(
        const FString& Msg,
        const FColor& Color = FColor::MakeRandomColor(),
        int32 InKey = -1,
        float Duration = 7.f)
    {
        // 화면 출력
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                InKey,
                Duration,
                Color,
                Msg
            );
        }

        // 로그 출력
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
    }

    /**
     * float 값 출력
     *
     * Title : 값 설명
     * Value : 출력할 float
     */
    inline void PrintFloat(
        const FString& Title,
        float Value,
        int32 InKey = -1,
        const FColor& Color = FColor::MakeRandomColor(),
        float Duration = 7.f)
    {
        const FString Msg =
            Title + TEXT(": ") + FString::SanitizeFloat(Value);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                InKey,
                Duration,
                Color,
                Msg
            );
        }

        UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
    }

    /**
     * bool 값 출력
     */
    inline void PrintBool(
        const FString& Title,
        bool bValue,
        int32 InKey = -1,
        const FColor& Color = FColor::MakeRandomColor(),
        float Duration = 7.f)
    {
        const FString Msg =
            Title + TEXT(": ") + (bValue ? TEXT("True") : TEXT("False"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                InKey,
                Duration,
                Color,
                Msg
            );
        }

        UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
    }
}