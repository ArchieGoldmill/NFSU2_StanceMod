// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "stdio.h"
#include <windows.h>
#include "includes/injector/injector.hpp"
#include <cstdint>
#include "includes/IniReader.h"
#include <d3d9.h>

DWORD RearCamberFn1 = 0x004012B0;
DWORD RearCamberFn2 = 0x005BACF0;
DWORD RearCamberCaveExit = 0x0062797C;
float RearCamberF1 = 1.0;
void __declspec(naked) ApplyCamber() 
{
	__asm 
	{
			mov[esp + 0x000000D0 + 4], esi
			lea ecx, [esp + 0x00000990 + 4]
			push esi
			push esi
			call RearCamberFn1
			mov edx, [esp + 0x000009C8 + 4]
			mov eax, [esp + 0x000009CC + 4]
			mov ecx, [esp + 0x000009D0 + 4]
			mov[esp + 0x00000168 + 4], edx
			mov edx, [esp + 0x000000D0 + 4]
			mov[esp + 0x0000016C + 4], eax

			mov eax, 0x8654A4
			mov eax, [eax]
			cmp eax, 3
			jne nodiv
			sar edx, 1
			nodiv:

			push edx
			lea eax, [esp + 0x0000099C + 4]
			mov[esp + 0x00000174 + 4], ecx
			push esi
			lea ecx, [esp + 0x000009A0 + 4]
			push esi
			mov dword ptr [esp + 0x000009D4 + 4], 0x00000000
			mov dword ptr [esp + 0x000009D8 + 4], 0x00000000
			mov dword ptr [esp + 0x000009DC + 4], 0x00000000
			mov dword ptr [esp + 0x000009E0 + 4], 0x3F800000
			call RearCamberFn2
			mov edx, [esp + 0x00000174 + 4]
			mov eax, [esp + 0x00000178 + 4]
			mov ecx, [esp + 0x0000017C + 4]
			mov[esp + 0x000009D4 + 4], edx
			lea edx, [esp + 0x000009A4 + 4]
			add esp, 0x14
			mov[esp + 0x000009C4 + 4], eax
			mov[esp + 0x000009C8 + 4], ecx
			mov[esp + 0x000000D0 + 4], edx
			ret
	}
}

void __declspec(naked) RearCamberCave()
{
	__asm
	{
		neg dword ptr[esp+0xc8]
		add esi, 0x80
		call ApplyCamber
		sub esi, 0x80

		/*add esi, 0x180
		call ApplyCamber
		sub esi, 0x180*/

		neg dword ptr[esp + 0xc8]
		add esi, 0xC0
		call ApplyCamber
		sub esi, 0xC0

		/*add esi, 0x1C0
		call ApplyCamber
		sub esi, 0x1C0*/

		mov[esp + 0x000000D0], edx
		jmp RearCamberCaveExit
	}
}

DWORD TrackWidthCaveExit = 0x00627183;
float* CamberPtr = (float*)0x007A6754;
float TrackWidthMult = 150.0;
float TrackWidthF1 = 0.0;
float TrackWidthF2 = 0.5;
void __declspec(naked) TrackWidthCave()
{
	__asm
	{
			mov edx, [esp + 0x58]
			mov edx, [edx + 0x04]
			lea edx, [edx + 0x10]

			push eax

			fld dword ptr[TrackWidthF1]
			mov al, [edx + 0x00000354]
			test al, al
			je check1
			fadd dword ptr[TrackWidthF2]
			check1:
			mov al, [edx + 0x00000353]
			test al, al
			je check2
			fadd dword ptr[TrackWidthF2]
			check2 :

			mov eax, CamberPtr
			fld dword ptr [eax]
			fld TrackWidthMult
			fdiv st(1), st(0)
			fstp st(0)
			fmul st(1), st(0)
			fstp st(0)

			pop eax
			fld dword ptr[eax - 0x38]
			push eax

			fcom TrackWidthF1
			fnstsw ax
			test ah, 0x21
			jnp left
			fxch st(1)
			fchs
			fxch st(1)
			left:
			fadd st(1), st(0)
			fstp st(0)

			pop eax
			mov eax, [eax]
			jmp TrackWidthCaveExit
	}
}

float TireWidthMult = 1.5;
DWORD TireWidthCaveExit = 0x0062718C;
void __declspec(naked) TireWidthCave()
{
	__asm
	{
		fld dword ptr[esp + 0x68]
		fmul TireWidthMult
		fst dword ptr[esp + 0x68]
		mov edx, 0x3F800000

		jmp TireWidthCaveExit;
	}
}

void Init()
{
	CIniReader iniReader("NFSU2_StanceMod.ini");

	if (iniReader.ReadInteger((char*)"MAIN", (char*)"WindowDamageEnabled", 0) == 1) 
	{
		char* wndDamage = (char*)0x0059FC13;
		injector::MakeNOP(0x0059FC13, 1, true);
		*wndDamage = 0xEB;
	}

	if (iniReader.ReadInteger((char*)"MAIN", (char*)"StanceModEnabled", 0) == 1)
	{
		injector::MakeJMP(0x00627975, RearCamberCave, true);
		injector::MakeJMP(0x0062717E, TrackWidthCave, true);

		float* mainMenuSteerAngle = (float*)0x007F6090;
		injector::MakeNOP(0x007F6090, 4, true);
		*mainMenuSteerAngle = 0;
	}

	if (iniReader.ReadInteger((char*)"MAIN", (char*)"TireWidthEnabled", 0) == 1)
	{
		injector::MakeJMP(0x00627183, TireWidthCave, true);
	}

	float camber = iniReader.ReadFloat((char*)"CAMBER", (char*)"CamberAngle", 7.0);
	if (camber < 0)
	{
		camber = 0;
	}
	else if (camber > 15) {
		camber = 15;
	}

	*CamberPtr = camber;

	float trackm = iniReader.ReadFloat((char*)"CAMBER", (char*)"TrackWidthOffset", 1.5);
	if (trackm <= 0)
	{
		trackm = 0.00000000001;
	}
	else if (trackm > 2) 
	{
		trackm = 2;
	}

	TrackWidthMult = 225.0 / trackm;

	float tirem = iniReader.ReadFloat((char*)"CAMBER", (char*)"TireWidthMultiplier", 1.15);
	if (tirem < 1) 
	{
		tirem = 1;
	}else if(tirem > 1.5)
	{
		tirem = 1.5;
	}

	TireWidthMult = tirem;
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
		IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)(base);
		IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

		if ((base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base)) == 0x75BCC7) // Check if .exe file is compatible - Thanks to thelink2012 and MWisBest
		{
			Init();
		}

		else
		{
			MessageBoxA(NULL, "This .exe is not supported.\nPlease use v1.2 NTSC speed2.exe (4,57 MB (4.800.512 bytes)).", "NFSU2 Stance Mod", MB_ICONERROR);
			return FALSE;
		}
	}
	return TRUE;

}
