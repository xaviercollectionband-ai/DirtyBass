name: Build DirtyBass VST3

on:
  workflow_dispatch:
  push:
    branches: [ main ]

jobs:
  build-windows:
    runs-on: windows-latest

    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Configure
        run: cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

      - name: Build VST3
        run: cmake --build build --config Release --parallel

      - name: Find VST3
        shell: pwsh
        run: |
          Write-Host "Searching for VST3 files..."
          $vsts = Get-ChildItem -Path build -Recurse -Filter "*.vst3"

          if (-not $vsts) {
            Write-Host "NO VST3 FILE FOUND."
            Write-Host "Build directory contents:"
            Get-ChildItem build -Recurse | Select-Object FullName
            exit 1
          }

          Write-Host "Found VST3:"
          $vsts | ForEach-Object {
            Write-Host $_.FullName
          }

          $vst = $vsts | Select-Object -First 1
          Compress-Archive -Path $vst.FullName -DestinationPath "DirtyBass-Windows-VST3.zip"

      - name: Upload VST3 artifact
        uses: actions/upload-artifact@v4
        with:
          name: DirtyBass-Windows-VST3
          path: DirtyBass-Windows-VST3.zip
