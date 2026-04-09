// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CMLogChannels.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Engine/Engine.h"
#include "Misc/MessageDialog.h"


namespace CMDebug
{
	static void Print(const FString& Message, const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogCameraManager, Warning, TEXT("%s"), *Message);
	}


	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowMsgAsWarning = true)
	{
		if (bShowMsgAsWarning)
		{
			const FText MsgTitle = FText::FromString(TEXT("Warning"));
			return FMessageDialog::Open(MsgType, FText::FromString(Message), MsgTitle);
		}
		else
		{

			return FMessageDialog::Open(MsgType, FText::FromString(Message));
		}

	}

	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 6.f;
		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}

	static void ShowNotifyError(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.FadeOutDuration = 6.f;
		NotifyInfo.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Error"));

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
}

