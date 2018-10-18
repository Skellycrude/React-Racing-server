// Copyright (C) 2018 Gon�alo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UINavComponent.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavComponent : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
		void OverrideButtonIndex();
	virtual void OverrideButtonIndex_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
		void OnNavigatedTo();
	virtual void OnNavigatedTo_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
		void OnNavigatedFrom();
	virtual void OnNavigatedFrom_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent)
		int ComponentIndex = -1;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = UINavComponent)
		class UUINavButton* NavButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, OptionalWidget = true), Category = UINavComponent)
		class UTextBlock* NavText;

};
