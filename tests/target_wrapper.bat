@echo off
SetLocal EnableDelayedExpansion
(set PATH=D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin;!PATH!)
if defined QT_PLUGIN_PATH (
    set QT_PLUGIN_PATH=D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\plugins;!QT_PLUGIN_PATH!
) else (
    set QT_PLUGIN_PATH=D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\plugins
)
%*
EndLocal
