// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "SubmachineGun"),
	EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};

UCLASS()
class SHOOTERGAME_PRO_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
		AWeapon();
		virtual void Tick(float DeltaTime) override;
		void ThrowWeapon();
		FORCEINLINE int32 GetAmmo() const { return Ammo; }
		FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

		void DecrementAmmo();
		
		FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
		FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
		FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
		FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }

		void ReloadAmmo(int32 Amount);

		void SetMovingClip(bool Move) { bMovingClip = Move; }

protected:
	void StopFalling();

private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Propertiese", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	//maximum ammo that weapon can hold
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Propertiese", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Propertiese", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Propertiese", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Propertiese", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;


	bool bMovingClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Propertiese", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;
};
