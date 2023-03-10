// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() : Speed(0.f), bIsInAir(false), bIsAccelerating(false),
MovementOffsetYaw(0.f), LastMovementOffsetYaw(0.f), bAiming(false), CharacterYaw(0.f),
CharacterYawLastFrame(0.f), RootYawOffset(0.f)
{

}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (ShooterCharacter)
	{
		bCrouching = ShooterCharacter->GetCrouching();
		
		//Get the Speed of the Character from velocity
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;

		//is the character falling
		Speed = Velocity.Size();
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}

		else {
			bIsAccelerating = false;
		}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		if(ShooterCharacter->GetVelocity().Size() > 0.f)
			LastMovementOffsetYaw = MovementOffsetYaw;

		bAiming = ShooterCharacter->GetAiming();

		//FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation : %f"), AimRotation.Yaw);
		//FString MovementRotationMessage = FString::Printf(TEXT("Movement Rot: %f"), MovementRotation.Yaw);
		//FString OffsetMessage = FString::Printf(TEXT("Movement offset Yaw : %f"), MovementOffsetYaw);

		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, OffsetMessage);
		//}
	}

	TurnInPlace();
}

void UShooterAnimInstance::NativeInitializeAnimation() //애니메이션 인스턴스가 생성될때 한번 실행
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	// trygetpawnOwner : 어떤 캐릭터가 실제로 사용되는가?!
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;
	if (Speed > 0)
	{
		// 움직이고 있을때는 캐릭터 rotate 해주지 않는다.
	}
	else
	{
		CharacterYawLastFrame = CharacterYaw;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float YawDelta{ CharacterYaw - CharacterYawLastFrame };

		RootYawOffset -= YawDelta;

		if (GEngine) GEngine->AddOnScreenDebugMessage(
			1, 
			-1, 
			FColor::Blue, 
			FString::Printf(TEXT("CharacterYaw : %f"),
				CharacterYaw));

		if (GEngine) GEngine->AddOnScreenDebugMessage(
			2,
			-1,
			FColor::Red,
			FString::Printf(TEXT("RootYawOffset : %f"),
				CharacterYaw));
	}
	
}
