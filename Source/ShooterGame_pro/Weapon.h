// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_PRO_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
		AWeapon();
		virtual void Tick(float DeltaTime) override;
		void ThrowWeapon();
		FORCEINLINE int32 GetAmmo() const { return Ammo; }

		void DecrementAmmo();
protected:
	void StopFalling();

private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Propertiese", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;


};
