// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"

#include "Components/SkeletalMeshComponent.h"
#include "ShooterCharacter.h"
#include "kismet/GameplayStatics.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "DrawDebugHelpers.h"
// Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MESH"));
	Mesh->SetupAttachment(Root);
}

void AGun::PullTrigger() 
{
	//UE_LOG(LogTemp, Warning, TEXT("You shot!"));
	if(CurrentAmmo > 0){
		IsReloading = false;
		CurrentAmmo--;
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Mesh, TEXT("MuzzleFlashSocket"));
		UGameplayStatics::SpawnSoundAttached(MuzzleSound, Mesh, TEXT("MuzzleFlashSocket"));
		FHitResult Hit;
		FVector ShotDirection;
		bool bSuccess = GunTrace(Hit, ShotDirection);
		if(bSuccess){
			//DrawDebugPoint(GetWorld(), Hit.Location, 20, FColor::Red, true);
			
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleFlash ,Hit.Location, ShotDirection.Rotation());
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, Hit.Location);
			AActor* HitActor = Hit.GetActor();
			if(HitActor != nullptr){
				FPointDamageEvent DamageEvent(Damage, Hit, ShotDirection, nullptr);
				AController *OwnerController = GetOwnerController();
				HitActor ->TakeDamage(Damage, DamageEvent, OwnerController, this);
			}
		}
		/* APawn* OwnerPawn = Cast<APawn>(GetOwner());
		MakeNoise(1, OwnerPawn, GetActorLocation()); */
	}
	else{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), OutOfAmmo, GetActorLocation());
		IsReloading = false;
		UE_LOG(LogTemp, Warning, TEXT("No Ammo!"));
		
	}
}

// Called when the game starts or when spawned
void AGun::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo	= MaxAmmo;
}

// Called every frame
void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AGun::ReturnAmmo() 
{
	return CurrentAmmo;
}

void AGun::Reload() 
{	
	//AController *OwnerController = GetOwnerController();
	AShooterCharacter* OwnerCharacter = Cast<AShooterCharacter>(GetOwner());
	
	//OwnerCharacter -> IsReloading = false;
	CurrentAmmo	= MaxAmmo;
	OwnerCharacter -> IsReloading = false;
	//OwnerController -> IsReloading = false;
}


bool AGun::GunTrace(FHitResult& Hit, FVector& ShotDirection) 
{	
	AController *OwnerController = GetOwnerController();
	if(OwnerController == nullptr) return false;
	//GetPlayerViewPoint( FVector& Location, FRotator& Rotation )
	FVector Location;
	FRotator Rotation;
	OwnerController->GetPlayerViewPoint(Location, Rotation);
	ShotDirection = -Rotation.Vector();//where is the shot comming from
	//DrawDebugCamera(GetWorld(), Location, Rotation, 90, 2, FColor::Red, true);

	//ECC_GameTraceChannel1
	//start point and end point
	FVector End = Location + Rotation.Vector() * MaxRange;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	return GetWorld()->LineTraceSingleByChannel(Hit, Location, End, ECollisionChannel::ECC_GameTraceChannel1, Params);
}

AController* AGun::GetOwnerController() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return nullptr;
	return OwnerPawn->GetController();
}

