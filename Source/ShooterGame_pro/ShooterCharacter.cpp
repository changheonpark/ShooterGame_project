// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "BulletHitInterface.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :BaseTurnRate(45.f), BaseLookUpRate(45.f), bAiming(false),
CameraDefaultFOV(0.f), CameraZoomedFOV(35.f), CameraCurrentFOV(0.f), ZoomInterSpeed(20.f),
HipTurnRate(90.f), HipLookUpRate(90.f), AimingTurnRate(20.f), AimingLookUpRate(20.f),
MouseHipTurnRate(1.0f), MouseHipLookUpRate(1.0f), MouseAimingTurnRate(0.2f), MouseAimingLookUpRate(0.2f),
CrosshairSpreadMultiplier(0.f), CrosshairVelocityFactor(0.f), CrosshairInAirFactor(0.f), CrosshairAimFactor(0.f), CrosshairShootingFactor(0.f),
ShootTimeDuration(0.05f), bFiringBullet(false),
AutomaticFireRate(0.1f), bShouldFire(true), bFireButtonPressed(false),
bShouldTraceForItems(false),
CameraInterpDistance(250.f), CameraInterpElevation(65.f),
Starting9mmAmmo(85), StartingARAmmo(120),
CombatState(ECombatState::ECS_Unoccupied),
bCrouching(false),
health(100.f), maxHealth(100.f), stunChance(0.25f)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.0f;
	CameraBoom->bUsePawnControlRotation = true; //���� ĳ���Ͱ� ����, ī�޶� ���� �װ� ����.
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//Dont rotate when the controller rotates => Let the controller only affect the camera
	bUseControllerRotationPitch = false;   //��Ʈ�ѷ��� ĳ���Ͱ� ���� ����.
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // ĳ���� ��ǲ�� �������� ���� => false�� ����
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); //yaw�� ������ ���� && �� ������
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;


}

float AShooterCharacter::TakeDamage(float damageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	if (health - damageAmount < 0)
	{
		health = 0.f;
		Die();

		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(
				FName(TEXT("IsCharacterDead")), true
			);
		}
	}

	else {
		UGameplayStatics::PlaySoundAtLocation(this, DamagedSound, GetActorLocation());
		health -= damageAmount;
	}

	return damageAmount;
}

void AShooterCharacter::Die()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && DeathtMontages)
	{
		animInstance->Montage_Play(DeathtMontages);
	}
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	if (PC)
	{
		DisableInput(PC);
	}
}


// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();



	InitializeAmmoMap();
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	//check to see if AmmoMap contains Ammo's AmmoType
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		//check to see if the gun is empty
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);

}

void AShooterCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if ((CurrentItemIndex == NewItemIndex) || (NewItemIndex >= Inventory.Num()) || (CombatState != ECombatState::ECS_Unoccupied)) return;
	auto OldEquippedWeapon = EquippedWeapon;
	auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
	EquipWeapon(NewWeapon);

	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	NewWeapon->SetItemState(EItemState::EIS_Equipped);
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;
}


// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CameraInterpZoom(DeltaTime);
	SetLookRates();

	CalculateCrosshairSpread(DeltaTime);
	TraceForItems();
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	//PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);

}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::FinishReloading()
{
	if (CombatState == ECombatState::ECS_Stunned) return;

	CombatState = ECombatState::ECS_Unoccupied;

	if (EquippedWeapon == nullptr) return;
	const auto AmmoType{ EquippedWeapon->GetAmmoType() };

	if (AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();
		
		if (MagEmptySpace > CarriedAmmo)
		{
			// Reload the magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}

		else
		{
			//fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}

}

void AShooterCharacter::IncreamentOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else {
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };
	//��ǥ�ϴ� ��ġ = ī�޶����(location) + Forward * A + Up * B

	return CameraLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	/*if (Item->GetEquipSound())
	{
		UGameplayStatics::PlaySound2D(this, Item->GetEquipSound());
	}*/
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else
		{
			SwapWeapon(Weapon);
		}

		
	}

	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}

void AShooterCharacter::Stun()
{
	if (health <= 0.f) return;
	CombatState = ECombatState::ECS_Stunned;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HitReactMontages)
	{
		AnimInstance->Montage_Play(HitReactMontages);
	}

	
}


void AShooterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix { YawRotation }.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator Rotation { Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{ YawRotation }.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor{};
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else 
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(TurnScaleFactor * Value);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(LookUpScaleFactor * Value);
}


void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();
		EquippedWeapon->DecrementAmmo();
		StartFireTimer();
	}

	//StartCrosshairBulletFire();

}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	if (bAiming)
	{
		//interpolate to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV, 
			CameraZoomedFOV, 
			DeltaTime, 
			ZoomInterSpeed); //������������ ��ǥ�������� �ε巴�� ����� �ִ� �Լ��̴�.

	}

	else
	{
		//interpolate to default FOV
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV, 
			CameraDefaultFOV, 
			DeltaTime, 
			ZoomInterSpeed); //������������ ��ǥ�������� �ε巴�� ����� �ִ� �Լ��̴�.
	}

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange,
		VelocityMultiplierRange,
		Velocity.Size()
	);


	if (GetCharacterMovement()->IsFalling()) //���߿� ������ ������ �������ٰ� ���� �پ���
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}

	else //���� ������ ������ ����
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);
	}
	

	//���ؽ� 
	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.2f, DeltaTime, 30.0f);
	}

	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, 30.0f);
	}




	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.0f);
	}

	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 60.0f);
	}
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

}



void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
	
}

void AShooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
			// FTimerHandle������ �����Ͽ�, �ְ�, ����� Ŭ����, �Լ��� �ּ�, �ݺ��� �ð�, Loop���� ...
			//SetTimer�� Tick�� �ٸ� Ÿ�Ӷ������� ������ �ð����� ȣ���� �����ϴ�.
	
}

void AShooterCharacter::AutoFireReset()
{
	if (CombatState == ECombatState::ECS_Stunned) return;

	CombatState = ECombatState::ECS_Unoccupied;
	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed)
			FireWeapon();
	}
	else
	{
		//Reload Weapon
		ReloadWeapon();
	}
}



void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrossHairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}




void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

bool AShooterCharacter::GetBeamEndLocation( const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	//check crosshair trace hit 
	FVector OutBeamLocation;

	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}

	else //no trace hair hit
	{
		//OutBeamLocation is the End Location for the line trace location
	}

	//gun barrel trace 

	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(
		OutHitResult,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility
	);

	if (!OutHitResult.bBlockingHit)//�ѱ� <-> �� end ����Ʈ ���̿� ��ü�� ������
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	return true;



	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}


	// GetScreenSpce location of crosshair
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	//CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//3������ 2�������� �����ϴ� ���� projection�̶�� �ϸ�, 2������ ������ 3�������� �����ϴ°��� Deprojection�̶�� �Ѵ�.
	//GetWorld position and direction of crosshair
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld) // was deprojection successful?
	{
		FHitResult ScreenTraceHit;
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

		//set beam end to line trace End
		OutBeamLocation = End;

		//Trace outward from crosshair world location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		if (ScreenTraceHit.bBlockingHit) //was trace hit?
		{
			OutBeamLocation = ScreenTraceHit.Location;

		}
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	//Get ViewportSize
	FVector2D ViewportSize;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2, ViewportSize.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector Start{ CrosshairWorldPosition };
		FVector End{ Start + CrosshairWorldDirection * 50'000.f};
		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(
			OutHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
		}

		if (OutHitResult.bBlockingHit)
		{

			return true;
		}


	}

	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);

		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}

			if (TraceHitItem && TraceHitItem->GetPickUpWidget())
			{
				// Show Item PickUpWidget
				TraceHitItem->GetPickUpWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();
			}

			//we hit an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)  //�츮�� ���������� hit trace�� �����۰� ���� ����Ű�°��� �ٸ��ٸ�
				{//Ȥ�� �ƹ��͵� ����Ű�� ���� �ʰų�!
					TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}

			//Store Reference to HitItem for next frame
			TraceHitItemLastFrame = TraceHitItem;
		}
	}

	else if (TraceHitItemLastFrame)
	{
		//No longer overlapping any Items
		TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}



void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{ 
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		

		if (HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped); //���⼭ �ѱ� �� ���¸� ���� -> item
	}

	if (EquippedWeapon == nullptr)
	{
		EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
	}

	else
	{
		EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon) 
	{
		FDetachmentTransformRules DetachmentTransfromRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransfromRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (TraceHitItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected after TraceHit"));
		TraceHitItem->StartItemCurve(this);
		TraceHitItem = nullptr;
		
		
	}
}

void AShooterCharacter::SelectButtonReleased()
{
	
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}

	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;   //�������� 
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if(EquippedWeapon == nullptr)
		return false;
	
	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SendBullet()
{
	//(�ѹ߻� ȭ��)  ��ƼŬ �ý����� ���� ������ ã�� ����
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		//ī�޶󿡼� Crosshair�� ����Ű�� �������� ����� �ǳ�, 
		//�츮�� �ٶ�� ���� �ѽ�~crosshair�� Ÿ������ ���� ���̿� ���� �ִ��� �ñ��� ���̴�.
		FHitResult BeamHitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);
		if (bBeamEnd)
		{
			// Hit Actor�� BulletHitInterface�� �����ߴ°�?
			if (BeamHitResult.GetActor())
			{
				//Enemy�� IBulletHitInterface�� ��ӹ޴´�. �׷��Ƿ�, enemy�� �θ�Ŭ������ IBulletHitInterFace�� ĳ���� �Ǵ°��� ������ ����.
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.GetActor());
				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult);
				}

				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.GetActor());
				if (HitEnemy)
				{
					int32 Damage{};
					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetEnemyHead())
					{
						//Head Shot
						Damage = EquippedWeapon->GetHeadShotDamage();
						UGameplayStatics::ApplyDamage(BeamHitResult.GetActor(), Damage, GetController(),
							this, UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, true);
					}

					else 
					{
						//Body Shot
						Damage = EquippedWeapon->GetDamage();
						UGameplayStatics::ApplyDamage(BeamHitResult.GetActor(), Damage,
							GetController(),
							this, UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, false);
					}
					
					
				}
			}

			//���� �´°� ������ �⺻ ��ƼŬ�� �����Ѵ�.
			else {
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						BeamHitResult.Location);
				}
			}


			

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
			}
		}

	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon == nullptr) return;

	if (CarryingAmmo())
	{
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}


	return false;
}



//void AShooterCharacter::GrabClip()
//{
//	if (EquippedWeapon == nullptr) return;
//
//	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
//	ClipTransform =  EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);
//	
//	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
//	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
//	HandSceneComponent->SetWorldTransform(ClipTransform);
//
//	EquippedWeapon->SetMovingClip(true);
//
//}
//
//void AShooterCharacter::ReleaseClip()
//{
//	EquippedWeapon->SetMovingClip(false);
//}
