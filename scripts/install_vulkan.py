#!/usr/bin/env python3

import platform
import os

vulkan_version = "1.4.341.1"
vulkan_end_point = "https://sdk.lunarg.com/sdk/download"

dict = {
  "Windows": ("windows/vulkansdk-windows-X64-", ".exe"),
  "Darwin" : ("mac/vulkansdk-macos-", ".zip"),
  "Linux"  : ("linux/vulkansdk-linux-x86_64-", ".tar.xz"),
}

os_name = platform.system()

def check_vulkan_downloaded():
  vulkan_sdk = os.environ.get("VULKAN_SDK")
  # we want to download as long as it mismatches
  return vulkan_sdk is not None and vulkan_version in vulkan_sdk

def fetch_vulkan_package():
  full_url = vulkan_end_point+"/"+vulkan_version+dict[os_name][0]+vulkan_version+dict[os_name][1]
  print(f"Full url: {full_url}")


if __name__ == "__main__":
  if not check_vulkan_downloaded():
    # do prechecking.
    if os_name != "Windows":
      print("Other platforms not supposed other than Windows right now")
      exit(-1)

    fetch_vulkan_package()

