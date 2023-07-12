#!/usr/bin/env python3

import os

Import("env")

def main() -> None:
    tidbyt_wifi_ssid = os.environ.get("TIDBYT_WIFI_SSID")
    tidbyt_wifi_password = os.environ.get("TIDBYT_WIFI_PASSWORD")
    tidbyt_remote_url = os.environ.get("TIDBYT_REMOTE_URL")

    env.Append(
        CCFLAGS=[
            f"-DTIDBYT_WIFI_SSID='\"{tidbyt_wifi_ssid}\"'",
            f"-DTIDBYT_WIFI_PASSWORD='\"{tidbyt_wifi_password}\"'",
            f"-DTIDBYT_REMOTE_URL='\"{tidbyt_remote_url}\"'",
        ],
        CPPDEFINES=[
            "-DNO_GFX",
            "-DNO_FAST_FUNCTIONS",
        ],
    )

main()