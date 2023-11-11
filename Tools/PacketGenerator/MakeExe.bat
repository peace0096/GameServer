pushd %~dp0
pyinstaller --onefile PacketGenerator.py
MOVE .\dist\PacketGenerator.exe .\GenPackets.exe
@RD /S /Q .\build
@RD /s /Q .\dist
DEL /s /F /Q .\PacketGenerator.spec
PAUSE