// Copyright (C) 2018 Gon�alo Marques - All Rights Reserved

#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavComponentBox.h"
#include "UINavComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/SlateBlueprintLibrary.h"

void UUINavWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitialSetup();
}

void UUINavWidget::InitialSetup()
{
	//If widget was already setup, apply only certain steps
	if (bCompletedSetup)
	{
		ReconfigureSetup();
		return;
	}

	bIsFocusable = true;
	WidgetClass = GetClass();
	if (CurrentPC == nullptr)
	{
		CurrentPC = Cast<AUINavController>(GetOwningPlayer());
		//check(CurrentPC != nullptr && "PlayerController isn't a UINavController");
		if (CurrentPC == nullptr)
		{
			DISPLAYERROR("PlayerController isn't a UINavController");
			return;
		}
	}

	if (bUseSelector && bUseMovementCurve)
	{
		//check(MoveCurve != nullptr && "UseMovementCurve is true but MoveCurve is null");
		if (MoveCurve == nullptr)
		{
			DISPLAYERROR("UseMovementCurve is true but MoveCurve is null");
			return;
		}
	}

	/*
	If this widget was added through a parent widget and should remove it from the viewport,
	remove that widget from viewport
	*/
	if (ParentWidget != nullptr && ParentWidget->IsInViewport() && bParentRemoved)
	{
		ParentWidget->RemoveFromParent();
	}

	FetchButtonsInHierarchy();
	ReadyForSetup();

	//check(UINavButtons.Num() == ButtonNavigations.Num() && "Dimension of UIUINavButtons and ButtonNavigations array is not the same.");
	if (UINavButtons.Num() != ButtonNavigations.Num())
	{
		DISPLAYERROR("Dimension of UINavButtons and ButtonNavigations array is not the same.");
		return;
	}

	if (UINavAnimations.Num() > 0 && UINavAnimations.Num() != UINavButtons.Num())
	{
		DISPLAYERROR("Number of animations doesn't match number of UINavButtons.");
		return;
	}

	if (bUseTextColor) ChangeTextColorToDefault();

	//If this widget doesn't need to create the selector, skip to setup
	if (!bUseSelector)
	{
		UINavSetup();
		bShouldTick = false;
		return;
	}
	else
	{
		SetupSelector();
	}

}

void UUINavWidget::ReconfigureSetup()
{
	if (bUseTextColor) ChangeTextColorToDefault();

	//If this widget doesn't need to create the selector, skip to setup
	if (!bUseSelector)
	{
		UINavSetup();
		return;
	}
	else
	{
		SetupSelector();
	}

	bShouldTick = true;
	WaitForTick = 0;
}

void UUINavWidget::CleanSetup()
{
	//Disable all buttons (bug fix)
	for (UUINavButton* button : UINavButtons)
	{
		button->SetIsEnabled(false);
	}

	PressedKeys.Empty();
}

void UUINavWidget::FetchButtonsInHierarchy()
{
	TraverseHierarquy();

	//check(FirstButtonIndex < UINavButtons.Num() && "Invalid FirstButton index, can't be greater than number of buttons.");
	//check(FirstButtonIndex > -1 && "Invalid FirstButton index, can't be less than 0.");

	if (FirstButtonIndex >= UINavButtons.Num())
	{
		DISPLAYERROR("Invalid FirstButton index, can't be greater than number of buttons.");
		return;
	}
	if (FirstButtonIndex < 0)
	{
		DISPLAYERROR("Invalid FirstButton index, can't be less than 0.");
		return;
	}

	ButtonIndex = FirstButtonIndex;

	while (UINavButtons[ButtonIndex]->Visibility == ESlateVisibility::Collapsed ||
		UINavButtons[ButtonIndex]->Visibility == ESlateVisibility::Hidden ||
		!UINavButtons[ButtonIndex]->bIsEnabled)
	{
		ButtonIndex++;
	}
}

void UUINavWidget::TraverseHierarquy()
{
	//Find UINavButtons in the widget hierarchy
	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);
	for (UWidget* widget : Widgets)
	{
		UScrollBox* Scroll = Cast<UScrollBox>(widget);
		if (Scroll != nullptr)
		{
			ScrollBoxes.Add(Scroll);
		}

		UUINavButton* NewNavButton = Cast<UUINavButton>(widget);
		if (NewNavButton == nullptr)
		{
			UUINavComponent* UIComp = Cast<UUINavComponent>(widget);
			if (UIComp != nullptr)
			{
				NewNavButton = Cast<UUINavButton>(UIComp->NavButton);
				if (bOverrideButtonIndices) UIComp->OverrideButtonIndex();

				UINavComponentsIndices.Add(UINavButtons.Num());
				UINavComponents.Add(UIComp);

				if (Cast<UUINavComponentBox>(widget))
				{
					ComponentBoxIndices.Add(UINavButtons.Num());
					UINavComponentBoxes.Add(Cast<UUINavComponentBox>(widget));
				}
			}
		}

		if (NewNavButton == nullptr) continue;

		if (!bOverrideButtonIndices)
		{
			NewNavButton->ButtonIndex = UINavButtons.Num();
		}
		NewNavButton->CustomHover.AddDynamic(this, &UUINavWidget::HoverEvent);
		NewNavButton->CustomUnhover.AddDynamic(this, &UUINavWidget::UnhoverEvent);
		NewNavButton->CustomClick.AddDynamic(this, &UUINavWidget::ClickEvent);
		NewNavButton->CustomRelease.AddDynamic(this, &UUINavWidget::ReleaseEvent);
		bSwitchedStyle.Add(false);

		//Add button to array of UIUINavButtons
		UINavButtons.Add(NewNavButton);
	}

	if (bOverrideButtonIndices)
	{
		UINavButtons.HeapSort([](const UUINavButton& Wid1, const UUINavButton& Wid2)
		{
			return Wid1.ButtonIndex < Wid2.ButtonIndex;
		});
	}
}

void UUINavWidget::ChangeTextColorToDefault()
{
	UTextBlock* TextBlock = nullptr;
	for (int j = 0; j < UINavButtons.Num(); j++)
	{
		SwitchTextColorTo(j, TextDefaultColor);
	}
}

void UUINavWidget::SetupSelector()
{
	//check(TheSelector != nullptr && "Couldn't find TheSelector");
	if (TheSelector == nullptr)
	{
		DISPLAYERROR("Couldn't find TheSelector");
		return;
	}

	TheSelector->SetVisibility(ESlateVisibility::Hidden);

	UCanvasPanelSlot* SelectorSlot = Cast<UCanvasPanelSlot>(TheSelector->Slot);

	SelectorSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	SelectorSlot->SetPosition(FVector2D(0.f, 0.f));
}

void UUINavWidget::UINavSetup()
{
	//Re-enable all buttons (bug fix)
	for (UUINavButton* button : UINavButtons)
	{
		button->SetIsEnabled(true);
	}

	//If this widget was added through a child widget, destroy it
	if (ReturnedFromWidget != nullptr)
	{
		if (ReturnedFromWidget != nullptr)
		{
			ReturnedFromWidget->Destruct();
		}
		ReturnedFromWidget = nullptr;
	}

	CurrentPC->SetActiveWidget(this);

	if (bUseSelector)
	{
		if (TheSelector == nullptr) return;
		TheSelector->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	bCompletedSetup = true;

	NavigateTo(ButtonIndex);
	OnNavigate(-1, ButtonIndex);
	if (UINavAnimations.Num() > 0) ExecuteAnimations(-1, ButtonIndex);

	OnSetupCompleted();
}

void UUINavWidget::ReadyForSetup_Implementation()
{

}

void UUINavWidget::NativeTick(const FGeometry & MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (!bUseSelector) return;

	if (bMovingSelector)
	{
		HandleSelectorMovement(DeltaTime);
	}
	else
	{
		if (!bShouldTick) return;

		if (WaitForTick == 1)
		{
			UINavSetup();
			bShouldTick = false;
			return;
		}

		WaitForTick++;
	}
}

FReply UUINavWidget::NativeOnMouseMove(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	if (CurrentPC->GetCurrentInputType() != EInputType::Mouse && InMouseEvent.GetCursorDelta().Size() > 0.000f)
	{
		CurrentPC->NotifyMouseInputType();
	}
	return FReply::Handled();
}

FReply UUINavWidget::NativeOnKeyDown(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent)
{
	Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	if (PressedKeys.Find(InKeyEvent.GetKey()) == INDEX_NONE)
	{
		PressedKeys.Add(InKeyEvent.GetKey());
		CurrentPC->NotifyKeyPressed(InKeyEvent.GetKey());
	}

	return FReply::Handled();
}

FReply UUINavWidget::NativeOnKeyUp(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent)
{
	Super::NativeOnKeyUp(InGeometry, InKeyEvent);

	PressedKeys.Remove(InKeyEvent.GetKey());
	CurrentPC->NotifyKeyReleased(InKeyEvent.GetKey());

	return FReply::Handled();
}

void UUINavWidget::HandleSelectorMovement(float DeltaTime)
{
	MovementCounter += DeltaTime;

	//Movement is finished
	if (MovementCounter >= MovementTime)
	{
		MovementCounter = 0.f;
		bMovingSelector = false;
		bAllowNavigation = true;
		TheSelector->SetRenderTranslation(SelectorDestination);
		if (HaltedIndex != -1)
		{
			if (HaltedIndex == SELECT_INDEX)
			{
				OnSelect(ButtonIndex);
			}
			else if (HaltedIndex == RETURN_INDEX)
			{
				OnReturn();
			}
			else
			{
				NavigateTo(HaltedIndex);
			}
			HaltedIndex = -1;
		}
		return;
	}

	TheSelector->SetRenderTranslation(SelectorOrigin + Distance*MoveCurve->GetFloatValue(MovementCounter));
}

void UUINavWidget::AppendVerticalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (Dimension == -1) Dimension = UINavButtons.Num();
	//check(Dimension > 0 && "Append Navigation Dimension should be greater than 0");
	if (Dimension <= 0)
	{
		DISPLAYERROR("Append Navigation Dimension should be greater than 0");
		return;
	}

	TArray<FButtonNavigation> ButtonsNav = TArray<FButtonNavigation>();
	FButtonNavigation NewNav;

	int StartingIndex = ButtonNavigations.Num();

	for (int i = 0; i < Dimension; i++)
	{
		if (i == 0)
		{
			NewNav.UpButton = EdgeNavigation.UpButton == -1 ? (bWrap ? StartingIndex + Dimension - 1 : NewNav.UpButton) : EdgeNavigation.UpButton;
		}
		else
		{
			NewNav.UpButton = StartingIndex + i - 1;
		}

		if (i == Dimension - 1)
		{
			NewNav.DownButton = EdgeNavigation.DownButton == -1 ? (bWrap ? StartingIndex : NewNav.DownButton) : EdgeNavigation.DownButton;
		}
		else
		{
			NewNav.DownButton = StartingIndex + i + 1;
		}

		if (EdgeNavigation.LeftButton != -1)
		{
			NewNav.LeftButton = EdgeNavigation.LeftButton;
		}
		if (EdgeNavigation.RightButton != -1)
		{
			NewNav.RightButton = EdgeNavigation.RightButton;
		}

		ButtonsNav.Add(NewNav);
	}

	ButtonNavigations.Append(ButtonsNav);
}

void UUINavWidget::AppendHorizontalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (Dimension == -1) Dimension = UINavButtons.Num();
	//check(Dimension > 0 && "Append Navigation Dimension should be greater than 0");
	if (Dimension <= 0)
	{
		DISPLAYERROR("Append Navigation Dimension should be greater than 0");
		return;
	}

	TArray<FButtonNavigation> ButtonsNav = TArray<FButtonNavigation>();
	FButtonNavigation NewNav;

	int StartingIndex = ButtonNavigations.Num();

	for (int i = 0; i < Dimension; i++)
	{
		if (i == 0)
		{
			NewNav.LeftButton = EdgeNavigation.LeftButton == -1 ? (bWrap ? StartingIndex + Dimension - 1 : NewNav.LeftButton) : EdgeNavigation.LeftButton;
		}
		else
		{
			NewNav.LeftButton = StartingIndex + i - 1;
		}

		if (i == Dimension - 1)
		{
			NewNav.RightButton = EdgeNavigation.RightButton == -1 ? (bWrap ? StartingIndex : NewNav.RightButton) : EdgeNavigation.RightButton;
		}
		else
		{
			NewNav.RightButton = StartingIndex + i + 1;
		}

		if (EdgeNavigation.UpButton != -1)
		{
			NewNav.UpButton = EdgeNavigation.UpButton;
		}
		if (EdgeNavigation.DownButton != -1)
		{
			NewNav.DownButton = EdgeNavigation.DownButton;
		}

		ButtonsNav.Add(NewNav);
	}

	ButtonNavigations.Append(ButtonsNav);
}

void UUINavWidget::AppendGridNavigation(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap)
{
	//check(DimensionX > 0 && "Append Navigation Dimension should be greater than 0");
	//check(DimensionY > 0 && "Append Navigation Dimension should be greater than 0");
	if (DimensionX <= 0)
	{
		DISPLAYERROR("Append Navigation Dimension should be greater than 0");
		return;
	}
	if (DimensionY <= 0)
	{
		DISPLAYERROR("Append Navigation Dimension should be greater than 0");
		return;
	}

	TArray<FButtonNavigation> ButtonsNav = TArray<FButtonNavigation>();
	FButtonNavigation NewNav;

	int StartingIndex = ButtonNavigations.Num();

	int Iterations = DimensionX * DimensionY;
	int i;
	for (i = 0; i < Iterations; i++)
	{

		//Top edge
		if (i < DimensionX)
		{
			NewNav.UpButton = bWrap ? i + DimensionX * (DimensionY - 1) : EdgeNavigation.UpButton;
		}
		//Rest of the grid
		else
		{
			NewNav.UpButton = StartingIndex + i - DimensionX;
		}

		//Bottom edge
		if ((i / DimensionX) == (DimensionY - 1))
		{
			NewNav.DownButton = bWrap ? i % DimensionX : EdgeNavigation.DownButton;
		}
		//Rest of the grid
		else
		{
			NewNav.DownButton = StartingIndex + i + DimensionX;
		}

		//Left edge
		if (((i + DimensionX) % DimensionX) == 0)
		{
			NewNav.LeftButton = bWrap ? i - 1 + DimensionX : EdgeNavigation.LeftButton;
		}
		//Rest of the grid
		else
		{
			NewNav.LeftButton = StartingIndex + i - 1;
		}

		//Right edge
		if (((i + 1 + DimensionX) % DimensionX) == 0)
		{
			NewNav.RightButton = bWrap ? i + 1 - DimensionX : EdgeNavigation.RightButton;
		}
		//Rest of the grid
		else
		{
			NewNav.RightButton = StartingIndex + i + 1;
		}

		ButtonsNav.Add(NewNav);
	}

	ButtonNavigations.Append(ButtonsNav);
}

void UUINavWidget::UpdateSelectorLocation(int Index)
{
	if (TheSelector == nullptr) return;
	TheSelector->SetRenderTranslation(GetButtonLocation(Index));
}

FVector2D UUINavWidget::GetButtonLocation(int Index)
{
	FGeometry Geom = UINavButtons[Index]->GetCachedGeometry();
	FVector2D LocalSize = Geom.GetLocalSize();
	FVector2D LocalPosition;
	switch (SelectorPositioning)
	{
		case ESelectorPosition::Position_Center:
			LocalPosition = LocalSize / 2;
			break;
		case ESelectorPosition::Position_Top:
			LocalPosition = FVector2D(LocalSize.X / 2, 0.f);
			break;
		case ESelectorPosition::Position_Bottom:
			LocalPosition = FVector2D(LocalSize.X / 2, LocalSize.Y);
			break;
		case ESelectorPosition::Position_Left:
			LocalPosition = FVector2D(0.f, LocalSize.Y / 2);
			break;
		case ESelectorPosition::Position_Right:
			LocalPosition = FVector2D(LocalSize.X, LocalSize.Y / 2);
			break;
		case ESelectorPosition::Position_Top_Right:
			LocalPosition = FVector2D(LocalSize.X, 0.f);
			break;
		case ESelectorPosition::Position_Top_Left:
			LocalPosition = FVector2D(0.f, 0.f);
			break;
		case ESelectorPosition::Position_Bottom_Right:
			LocalPosition = FVector2D(LocalSize.X, LocalSize.Y);
			break;
		case ESelectorPosition::Position_Bottom_Left:
			LocalPosition = FVector2D(0.f, LocalSize.Y);
			break;
	}
	
	FVector2D PixelPos, ViewportPos;
	USlateBlueprintLibrary::LocalToViewport(GetWorld(), Geom, LocalPosition, PixelPos, ViewportPos);
	ViewportPos += SelectorOffset;
	return ViewportPos;
}

void UUINavWidget::ExecuteAnimations(int From, int To)
{
		if (From != -1)
	{
		if (IsAnimationPlaying(UINavAnimations[From]))
		{
			ReverseAnimation(UINavAnimations[From]);
		}
		else
		{
			PlayAnimation(UINavAnimations[From], 0.0f, 1, EUMGSequencePlayMode::Reverse, AnimationPlaybackSpeed);
		}
	}
	if (IsAnimationPlaying(UINavAnimations[To]))
	{
		ReverseAnimation(UINavAnimations[To]);
	}
	else
	{
		PlayAnimation(UINavAnimations[To], 0.0f, 1, EUMGSequencePlayMode::Forward, AnimationPlaybackSpeed);
	}
}

void UUINavWidget::UpdateTextColor(int Index)
{
	SwitchTextColorTo(ButtonIndex, TextDefaultColor);
	SwitchTextColorTo(Index, TextNavigatedColor);
}

void UUINavWidget::SwitchTextColorTo(int Index, FLinearColor Color)
{
	UTextBlock* NewText = nullptr;
	int NewComponentIndex = UINavComponentsIndices.Find(Index);
	if (NewComponentIndex != INDEX_NONE)
	{
		NewText = UINavComponents[NewComponentIndex]->NavText;
		//check(NewText != nullptr && "When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
		if (NewText == nullptr)
		{
			DISPLAYERROR("When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
			return;
		}
	}
	else
	{
		NewText = Cast<UTextBlock>(UINavButtons[Index]->GetChildAt(0));
		//check(NewText != nullptr && "When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
		if (NewText == nullptr)
		{
			DISPLAYERROR("When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
			return;
		}
	}
	NewText->SetColorAndOpacity(Color);
}

void UUINavWidget::UpdateButtonsStates(int Index, bool bHovered)
{
	//Update new button state
	if (!(bHovered ^ bSwitchedStyle[Index]))
	{
		SwitchButtonStyle(Index);
		bSwitchedStyle[Index] = !bHovered;
	}

	if (ButtonIndex == Index) return;

	//Update previous button state
	if (!(bHovered ^ !bSwitchedStyle[Index]))
	{
		SwitchButtonStyle(ButtonIndex);
		bSwitchedStyle[ButtonIndex] = false;
	}
}

void UUINavWidget::SwitchButtonStyle(int Index)
{
	UUINavButton* TheButton = UINavButtons[Index];
	FButtonStyle NewStile = TheButton->WidgetStyle;
	FSlateBrush TempState;
	TempState = NewStile.Hovered;
	NewStile.Hovered = NewStile.Normal;
	NewStile.Normal = TempState;
	TheButton->SetStyle(NewStile);
}

void UUINavWidget::SetAllowNavigation(bool bAllow)
{
	bAllowNavigation = bAllow;
}

void UUINavWidget::SetSelector(UUserWidget * NewSelector)
{
	//check(TheSelector != nullptr && "Received invalid Selector");
	if (NewSelector == nullptr)
	{
		DISPLAYERROR("Received invalid Selector");
	}
	TheSelector = NewSelector;
}

void UUINavWidget::SetSelectorScale(FVector2D NewScale)
{
	if (TheSelector == nullptr) return;
	TheSelector->SetRenderScale(NewScale);
}

void UUINavWidget::SetSelectorVisibility(bool bVisible)
{
	ESlateVisibility Vis = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	TheSelector->SetVisibility(Vis);
}

void UUINavWidget::NavigateTo(int Index, bool bHoverEvent)
{
	bool bShouldNotify = Index != ButtonIndex ? true : false;
		
	if (bUseButtonStates)
	{
		UpdateButtonsStates(Index, bHoverEvent);
	}

	if (bUseSelector)
	{
		if (bUseMovementCurve)
		{
			BeginSelectorMovement(Index);
		}
		else
		{
			UpdateSelectorLocation(Index);
		}
	}

	if (bUseTextColor)
	{
		UpdateTextColor(Index);
	}

	if (bShouldNotify)
	{
		OnNavigate(ButtonIndex, Index);

		UUINavComponent* FromComponent = GetUINavComponentAtIndex(ButtonIndex);
		UUINavComponent* ToComponent = GetUINavComponentAtIndex(Index);
		if (FromComponent != nullptr) FromComponent->OnNavigatedFrom();
		if (ToComponent != nullptr) ToComponent->OnNavigatedTo();

		if (UINavAnimations.Num() > 0) ExecuteAnimations(ButtonIndex, Index);
	}
	//Update all the possible scroll boxes in the widget
	for (int i = 0; i < ScrollBoxes.Num(); ++i)
	{
		ScrollBoxes[i]->ScrollWidgetIntoView(UINavButtons[Index]);
	}
	ButtonIndex = Index;

}

void UUINavWidget::BeginSelectorMovement(int Index)
{
	SelectorOrigin = GetButtonLocation(ButtonIndex);
	SelectorDestination = GetButtonLocation(Index);
	Distance = SelectorDestination - SelectorOrigin;

	float MinTime, MaxTime;
	MoveCurve->GetTimeRange(MinTime, MaxTime);
	MovementTime = MaxTime - MinTime;

	bMovingSelector = true;
	bAllowNavigation = false;
}

void UUINavWidget::OnNavigate_Implementation(int From, int To)
{

}

void UUINavWidget::OnSelect_Implementation(int Index)
{

}

void UUINavWidget::OnReturn_Implementation()
{
	ReturnToParent();
}

void UUINavWidget::OnInputChanged_Implementation(EInputType From, EInputType To)
{

}

void UUINavWidget::OnSetupCompleted_Implementation()
{

}

UWidget* UUINavWidget::GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, bool bRemoveParent)
{
	//check(NewWidgetClass != nullptr && "GoToWidget: No Widget Class found");
	if (NewWidgetClass == nullptr)
	{
		DISPLAYERROR("GoToWidget: No Widget Class found");
		return nullptr;
	}

	UUINavWidget* NewWidget = CreateWidget<UUINavWidget>(CurrentPC, NewWidgetClass);
	NewWidget->ParentWidget = this;
	NewWidget->ParentWidgetClass = WidgetClass;
	NewWidget->bParentRemoved = bRemoveParent;
	if (HasUserFocus(CurrentPC))
	{
		NewWidget->SetUserFocus(CurrentPC);
	}
	NewWidget->AddToViewport();
	CleanSetup();
	return NewWidget;
}

void UUINavWidget::ReturnToParent()
{
	if (ParentWidget == nullptr)
	{
		if (bAllowRemoveIfRoot)
		{
			CurrentPC->OnRootWidgetRemoved();
			RemoveFromParent();
		}
		return;
	}

	if (HasUserFocus(CurrentPC))
	{
		ParentWidget->SetUserFocus(CurrentPC);
	}

	//If parent was removed, add it to viewport
	if (bParentRemoved)
	{
		ParentWidget->ReturnedFromWidget = this;
		ParentWidget->AddToViewport();
	}
	else
	{
		CurrentPC->SetActiveWidget(ParentWidget);
		ParentWidget->ReconfigureSetup();
	}
	RemoveFromParent();
}

void UUINavWidget::MenuNavigate(ENavigationDirection Direction)
{
	int NewIndex = FindNextIndex(Direction);
	if (NewIndex == -1) return;
	NavigateTo(NewIndex);
}

int UUINavWidget::FindNextIndex(ENavigationDirection Direction)
{
	int NewIndex = FetchIndexByDirection(Direction, ButtonIndex);
	if (NewIndex == -1) return -1;

	//Check if the button is visible, if not, skip to next button
	while (UINavButtons[NewIndex]->Visibility == ESlateVisibility::Collapsed ||
		UINavButtons[NewIndex]->Visibility == ESlateVisibility::Hidden ||
		!UINavButtons[NewIndex]->bIsEnabled)
	{
		NewIndex = FetchIndexByDirection(Direction, NewIndex);
	}
	return NewIndex;
}

int UUINavWidget::FetchIndexByDirection(ENavigationDirection Direction, int Index)
{
	int LocalIndex = -1;

	switch (Direction)
	{
		case ENavigationDirection::Up:
			LocalIndex = ButtonNavigations[Index].UpButton;
			break;
		case ENavigationDirection::Down:
			LocalIndex = ButtonNavigations[Index].DownButton;
			break;
		case ENavigationDirection::Left:
			LocalIndex = ButtonNavigations[Index].LeftButton;
			break;
		case ENavigationDirection::Right:
			LocalIndex = ButtonNavigations[Index].RightButton;
			break;
		default:
			break;
	}

	return LocalIndex;
}

UUINavButton * UUINavWidget::GetUINavButtonAtIndex(int Index)
{
	if (UINavButtons.Num() <= Index)
	{
		DISPLAYERROR("GetUINavButtonAtIndex was given an invalid index!");
		return nullptr;
	}
	return UINavButtons[Index];
}

UUINavComponent * UUINavWidget::GetUINavComponentAtIndex(int Index)
{
	int ValidIndex = UINavComponentsIndices.Find(Index);
	if (ValidIndex == INDEX_NONE) return nullptr;
	
	return UINavComponents[ValidIndex];
}

UUINavComponentBox * UUINavWidget::GetUINavComponentBoxAtIndex(int Index)
{
	int ValidIndex = ComponentBoxIndices.Find(Index);
	if (ValidIndex == INDEX_NONE) return nullptr;
	
	return UINavComponentBoxes[ValidIndex];
}

void UUINavWidget::HoverEvent(int Index)
{
	if (CurrentPC->GetCurrentInputType() != EInputType::Mouse)
	{
		SwitchButtonStyle(Index);
		return;
	}

	CurrentPC->NotifyMouseInputType();

	if (!bAllowNavigation)
	{
		HaltedIndex = Index;
		return;
	}

	NavigateTo(Index, true);
}

void UUINavWidget::UnhoverEvent(int Index)
{
	if (bUseButtonStates)
	{
		/*
		If the button didn't switch style, switch style to make sure it's still selected
		Otherwise, if the button is the selected button, also switch style to make sure it's still selected
		*/
		if (!bSwitchedStyle[Index] || (bSwitchedStyle[Index] && ButtonIndex == Index))
		{
			SwitchButtonStyle(Index);
			bSwitchedStyle[Index] = ButtonIndex == Index;
		}

		ButtonIndex = bSwitchedStyle[Index] ? Index : ButtonIndex;
	}
}

void UUINavWidget::ClickEvent(int Index)
{
	CurrentPC->NotifyMouseInputType();

	if (!bAllowNavigation) return;

	OnSelect(Index);
}

void UUINavWidget::ReleaseEvent(int Index)
{
	//Handle button style switching when mouse is released
	if (!UINavButtons[Index]->IsHovered())
	{
		SwitchButtonStyle(Index);
		for (int i = 0; i < UINavButtons.Num(); i++)
		{
			if (i == Index) continue;
			if (UINavButtons[i]->IsHovered())
			{
				SwitchButtonStyle(i);
				return;
			}
		}
	}
}

void UUINavWidget::NavigateInDirection(ENavigationDirection Direction)
{
	if (Direction == ENavigationDirection::None) return;

	if (!bAllowNavigation)
	{
		HaltedIndex = FindNextIndex(Direction);
		return;
	}

	UUINavComponentBox* ComponentBox = GetUINavComponentBoxAtIndex(ButtonIndex);
	if (ComponentBox != nullptr && (Direction == ENavigationDirection::Left || Direction == ENavigationDirection::Right))
	{
		if (Direction == ENavigationDirection::Left) ComponentBox->NavigateLeft();
		else if (Direction == ENavigationDirection::Right) ComponentBox->NavigateRight();
	}
	else MenuNavigate(Direction);
	
}

void UUINavWidget::MenuSelect()
{
	if (!bAllowNavigation)
	{
		HaltedIndex = SELECT_INDEX;
		return;
	}
	OnSelect(ButtonIndex);
}

void UUINavWidget::MenuReturn()
{
	if (!bAllowNavigation)
	{
		HaltedIndex = RETURN_INDEX;
		return;
	}
	OnReturn();
}


