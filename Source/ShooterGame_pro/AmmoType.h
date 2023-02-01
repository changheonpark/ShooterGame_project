// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR UMETA(DisplayName = "Assault Rifle"),
	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};


class SHOOTERGAME_PRO_API AmmoType
{
public:
	AmmoType();
	~AmmoType();
};
