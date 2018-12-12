// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSProjectile.h"
#include "FPSCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

AFPSProjectile::AFPSProjectile() 
{
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(10.0f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->SetSimulatePhysics(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPSProjectile::OnHit);	// set up a notification for when this component hits something blocking
	

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f * 0.1;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 100000.0f;
}

void AFPSProjectile::Tick(float DeltaTime)
{
	
	UWorld* world = GetWorld();

	FVector ave_pos = FVector(0, 0, 0);
	FVector ave_vel = FVector(0, 0, 0);

	int n_bullets = 0;

	FVector currentlocation = CollisionComp->GetRelativeTransform().GetLocation();

	Super::Tick(DeltaTime);

	for (TActorIterator<AFPSProjectile> i(world, AFPSProjectile::StaticClass()); i; ++i)
	{
		AFPSProjectile* bullet = *i;

		if (bullet == this) continue;

		//FVector itlocation = bullet->CollisionComp->GetRelativeTransform().GetLocation();//get location of other bullet

		float distance = this->GetDistanceTo(bullet);


		if (distance < 300)//cohesion 200
		{
		
			FVector bullet_pos = bullet->GetActorLocation();//get iterator bullet location
			FVector bullet_vel = bullet->GetVelocity();// get iterator bullet velocity

			n_bullets++;

			ave_pos += bullet_pos;
			ave_vel += bullet_vel;

		}

		if (distance < 120)//50
		{
			//UE_LOG(LogTemp, Warning, TEXT("seperation"));
			
			FVector pos = AActor::GetActorLocation();//get main bullet location
			FVector bullet_pos = bullet->GetActorLocation();//get iterator bullet location
			FVector dif_pos = pos - bullet_pos;// difference from iterator bullet
			dif_pos.Normalize();
			CollisionComp->AddImpulse(dif_pos*20);//5
		}


		

	}

	if (n_bullets != 0)//separation
	{
		ave_pos /= n_bullets;
		ave_vel /= n_bullets;

		FVector pos = AActor::GetActorLocation();//get main bullet location
		FVector dif_pos = ave_pos - pos;// difference from iterator bullet
		FVector dif_vel = ave_vel;// difference from iterator bullet
		dif_vel.Normalize();
		dif_pos.Normalize();
		CollisionComp->AddImpulse(dif_pos * 50);//4
		CollisionComp->AddImpulse(dif_vel * 2);//0.1
	}


}


void AFPSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{

		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		FVector Scale = OtherComp->GetComponentScale();
		Scale *= 0.7;

		if (Scale.GetMin() < 0.5f)
		{
			OtherActor->Destroy();
		}
		else
		{
			OtherComp->SetWorldScale3D(Scale);
		}

		UMaterialInstanceDynamic* MatInst = OtherComp->CreateAndSetMaterialInstanceDynamic(0);
		if (MatInst)
		{
			MatInst->SetVectorParameterValue("Color", FLinearColor::MakeRandomColor());
		}
		//Destroy();
	}
}