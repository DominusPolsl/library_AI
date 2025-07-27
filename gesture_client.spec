# -*- mode: python ; coding: utf-8 -*-
import os

datas = [
    ("C:/Users/kosti/AppData/Local/Programs/Python/Python310/Lib/site-packages/mediapipe/modules/hand_landmark", "mediapipe/modules/hand_landmark"),
    ("C:/Users/kosti/AppData/Local/Programs/Python/Python310/Lib/site-packages/mediapipe/modules/pose_landmark", "mediapipe/modules/pose_landmark"),
    ("C:/Users/kosti/AppData/Local/Programs/Python/Python310/Lib/site-packages/mediapipe/modules/palm_detection", "mediapipe/modules/palm_detection"),
]

a = Analysis(
    ['gesture_client.py'],
    pathex=[],
    binaries=[],
    datas=datas,
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)

pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='gesture_client',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,  # <-- консоль залиш для діагностики
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
