#pragma once

#include "CoreMinimal.h"

class AActor;
class FScopedTransaction;

namespace OneSideScale
{
    bool GetLocalBounds(AActor* Actor, FBox& OutBounds);
    FVector FaceCenter(const FBox& Bounds, int32 Face);
    FTransform ScaleFromFace(const FTransform& Start, const FBox& Bounds, int32 Face,
        double OutwardDistance, double GridSize);
}

/** Owns exactly one actor transform transaction; never writes mesh or pivot data. */
class FOneSideScaleDrag
{
public:
    FOneSideScaleDrag();
    ~FOneSideScaleDrag();
    bool Begin(AActor* Actor, int32 Face);
    void Update(double OutwardDistance, double GridSize);
    void End(bool bCancel);
    bool IsActive() const { return Transaction.IsValid(); }
    const FTransform& GetStartTransform() const { return StartTransform; }
    const FBox& GetBounds() const { return Bounds; }

private:
    TWeakObjectPtr<AActor> Target;
    TUniquePtr<FScopedTransaction> Transaction;
    FTransform StartTransform;
    FBox Bounds = FBox(ForceInit);
    int32 ActiveFace = INDEX_NONE;
};
