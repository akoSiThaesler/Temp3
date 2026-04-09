// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "Framework/Application/IInputProcessor.h"
#include "Input/Events.h"

class FCMKeyInputPreProcessor : public IInputProcessor
{
public:
    FCMKeyInputPreProcessor() {}

    virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { }

    virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
    {
        const bool KeyReturn = HandleKey(InKeyEvent, false);
        return KeyReturn;
    }

    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
    {
        const bool KeyReturn = HandleKey(InKeyEvent, true);
        return KeyReturn;
    }

    virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
    {
        // HandleKey(MouseEvent.GetEffectingButton());
        return false;
    }

    virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
    {
        const FKeyEvent KeyEvent = CreateKeyEventFromPointerEvent(MouseEvent, true);
        const bool KeyReturn = HandleKey(KeyEvent, true);
        return KeyReturn;
    }

    virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
    {
        const FKeyEvent KeyEvent = CreateKeyEventFromPointerEvent(MouseEvent, false);
        const bool KeyReturn = HandleKey(KeyEvent, false);
        return KeyReturn;
    }

    virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override
    {
        if (InWheelEvent.GetWheelDelta() != 0)
        {
            const FKey Key = InWheelEvent.GetWheelDelta() < 0 ? EKeys::MouseScrollDown : EKeys::MouseScrollUp;
            const FModifierKeysState ModifierKeysState = InWheelEvent.GetModifierKeys();
            const FKeyEvent KeyEvent(Key, ModifierKeysState, 0, false, 0, 0);  // Create FKeyEvent
            const bool KeyReturn = HandleKey(KeyEvent, true);
            return KeyReturn;
        }
        return false;
    }

    DECLARE_DELEGATE_RetVal_OneParam(bool, FSettingsPressAnyKeyInputPreProcessorKeySelected, const FKeyEvent&);
    FSettingsPressAnyKeyInputPreProcessorKeySelected OnKeySelected;

    /*DECLARE_DELEGATE_RetVal_OneParam(bool, FSettingsPressAnyKeyInputPreProcessorCanceled, const FKeyEvent&);
    FSettingsPressAnyKeyInputPreProcessorCanceled OnKeyCanceled;*/

private:
    bool HandleKey(const FKeyEvent& KeyEvent, const bool bIsPressed) const
    {
        if(bIsPressed)
        {
            return OnKeySelected.Execute(KeyEvent);
        }
        //return OnKeyCanceled.Execute(KeyEvent);
        return false;
    }

    static FKeyEvent CreateKeyEventFromPointerEvent(const FPointerEvent& MouseEvent, bool bIsPressed)
    {
        const FModifierKeysState ModifierKeysState = MouseEvent.GetModifierKeys();
        return FKeyEvent(MouseEvent.GetEffectingButton(), ModifierKeysState, 0, bIsPressed, 0, 0);
    }
};
