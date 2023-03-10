// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),
	EIR_Max UMETA(DisplayName = "DefaultMax")

};


UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_PickUp UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "Pickedup"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),
	EIS_MAX UMETA(DisplayName = "DefaultMax"),

};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackGround;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;

};

UCLASS()
class SHOOTERGAME_PRO_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);


	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void SetActiveStarts();
	virtual void SetItemProperties(EItemState State);
	void FinishInterping();
	void ItemInterp(float DeltaTime);

	void EnableGlowMaterial();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	//?????? ???????? ????
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"));
	USkeletalMeshComponent* ItemMesh;

	//?????????????? ?????? ?????? ??????????, HUD ?????? ????????.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"));
	class UBoxComponent* CollisionBox;

	//?????? ?????? ?????? ???? ??????.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickUpWidget;

	//?????? ?????? ???????? ?????? 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	//?????? ?????? ?????? ????
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	//Curve Assets to use for the item's Z Location when Interping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	bool bInterping;



	FTimerHandle ItemInterpTimer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	float ZCurveTime;

	//pointer to the character 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* Character;

	float ItemInterpX;
	float ItemInterpY;

	float InterpInitialYawOffset; // ???????? ?????? ???? 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class USoundCue* PickupSound;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	//USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	bool bCanChangeCustomDepth;



	virtual void InitializeCustomDepth();
	virtual void OnConstruction(const FTransform& Transform)override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	class UDataTable* ItemRarityDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackGround;


public:
	FORCEINLINE UWidgetComponent* GetPickUpWidget() const { return PickUpWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	void SetItemState(EItemState State);
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	//FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	//FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 index) { SlotIndex = index; }

	void StartItemCurve(AShooterCharacter* Char);

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	void DisableGlowMaterial();
};
