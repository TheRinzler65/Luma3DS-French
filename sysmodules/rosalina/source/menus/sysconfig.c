/*
*   This file is part of Luma3DS
*   Copyright (C) 2016-2021 Aurora Wright, TuxSH
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

#include <3ds.h>
#include "luma_config.h"
#include "menus/sysconfig.h"
#include "memory.h"
#include "draw.h"
#include "fmt.h"
#include "utils.h"
#include "ifile.h"

Menu sysconfigMenu = {
    "Menu configuration du syst\x8A""me",
    {
        { "Controle du volume", METHOD, .method=&SysConfigMenu_AdjustVolume},
        { "Controle Connexion Wifi", METHOD, .method = &SysConfigMenu_ControlWifi },
        { "Alterner les LEDs", METHOD, .method = &SysConfigMenu_ToggleLEDs },
        { "Alterner l'adaptateur Wifi", METHOD, .method = &SysConfigMenu_ToggleWireless },
        { "Alterner Bouton Power", METHOD, .method=&SysConfigMenu_TogglePowerButton },
        { "Alterner l'alim du slot de la cartouche", METHOD, .method=&SysConfigMenu_ToggleCardIfPower},
        {},
    }
};

bool isConnectionForced = false;
s8 currVolumeSliderOverride = -1;

void SysConfigMenu_ToggleLEDs(void)
{
    Draw_Lock();
    Draw_ClearFramebuffer();
    Draw_FlushFramebuffer();
    Draw_Unlock();

    do
    {
        Draw_Lock();
        Draw_DrawString(10, 10, COLOR_TITLE, "Menu configuration du syst\x8A""me");
        Draw_DrawString(10, 30, COLOR_WHITE, "Appuyer sur A pour basculer,\nAppuyer sur B pour revenir en arri\x8A""re.");
        Draw_DrawString(10, 60, COLOR_RED, "ATTENTION:");
        Draw_DrawString(10, 80, COLOR_WHITE, "  * Le passage en mode veille r\x82""initialise\nl'\x82""tat de la DEL !");
        Draw_DrawString(10, 100, COLOR_WHITE, "  * Il n'est pas possible de faire basculer les\nLED lorsque la batterie est faible !");

        Draw_FlushFramebuffer();
        Draw_Unlock();

        u32 pressed = waitInputWithTimeout(1000);

        if(pressed & KEY_A)
        {
            mcuHwcInit();
            u8 result;
            MCUHWC_ReadRegister(0x28, &result, 1);
            result = ~result;
            MCUHWC_WriteRegister(0x28, &result, 1);
            mcuHwcExit();
        }
        else if(pressed & KEY_B)
            return;
    }
    while(!menuShouldExit);
}

void SysConfigMenu_ToggleWireless(void)
{
    Draw_Lock();
    Draw_ClearFramebuffer();
    Draw_FlushFramebuffer();
    Draw_Unlock();

    bool nwmRunning = isServiceUsable("nwm::EXT");

    do
    {
        Draw_Lock();
        Draw_DrawString(10, 10, COLOR_TITLE, "Menu configuration du syst\x8A""me");
        Draw_DrawString(10, 30, COLOR_WHITE, "Appuyez sur A pour basculer,\nAppuyez sur B pour revenir en arri\x8A""re.");

        u8 wireless = (*(vu8 *)((0x10140000 | (1u << 31)) + 0x180));

        if(nwmRunning)
        {
            Draw_DrawString(10, 60, COLOR_WHITE, "Statut actuel:");
            Draw_DrawString(100, 60, (wireless ? COLOR_GREEN : COLOR_RED), (wireless ? " ON " : " OFF"));
        }
        else
        {
            Draw_DrawString(10, 50, COLOR_RED, "NWM ne fonctionne pas.");
            Draw_DrawString(10, 60, COLOR_RED, "Si vous \x88""tes actuellement sur le Menu Test,");
            Draw_DrawString(10, 70, COLOR_RED, "appuyez sur R+DROITE pour faire basculer l'option WiFi.");
            Draw_DrawString(10, 80, COLOR_RED, "Sinon, il suffit de quitter et d'attendre quelques secondes.");
        }

        Draw_FlushFramebuffer();
        Draw_Unlock();

        u32 pressed = waitInputWithTimeout(1000);

        if(pressed & KEY_A && nwmRunning)
        {
            nwmExtInit();
            NWMEXT_ControlWirelessEnabled(!wireless);
            nwmExtExit();
        }
        else if(pressed & KEY_B)
            return;
    }
    while(!menuShouldExit);
}

void SysConfigMenu_UpdateStatus(bool control)
{
    MenuItem *item = &sysconfigMenu.items[1];

    if(control)
    {
        item->title = "Controle Connexion sans fil";
        item->method = &SysConfigMenu_ControlWifi;
    }
    else
    {
        item->title = "D\x82""sactiver la connexion sans fil forc\x82""e";
        item->method = &SysConfigMenu_DisableForcedWifiConnection;
    }
}

static bool SysConfigMenu_ForceWifiConnection(u32 slot)
{
    char ssid[0x20 + 1] = {0};
    isConnectionForced = false;

    if(R_FAILED(acInit()))
        return false;

    acuConfig config = {0};
    ACU_CreateDefaultConfig(&config);
    ACU_SetNetworkArea(&config, 2);
    ACU_SetAllowApType(&config, 1 << slot);
    ACU_SetRequestEulaVersion(&config);

    Handle connectEvent = 0;
    svcCreateEvent(&connectEvent, RESET_ONESHOT);

    bool forcedConnection = false;
    if(R_SUCCEEDED(ACU_ConnectAsync(&config, connectEvent)))
    {
        if(R_SUCCEEDED(svcWaitSynchronization(connectEvent, -1)) && R_SUCCEEDED(ACU_GetSSID(ssid)))
            forcedConnection = true;
    }
    svcCloseHandle(connectEvent);

    if(forcedConnection)
    {
        isConnectionForced = true;
        SysConfigMenu_UpdateStatus(false);
    }
    else
        acExit();

    char infoString[80] = {0};
    u32 infoStringColor = forcedConnection ? COLOR_GREEN : COLOR_RED;
    if(forcedConnection)
        sprintf(infoString, "Une connexion a \x82""t\x82"" forc\x82""e avec succ\x8A""s vers: %s", ssid);
    else
       sprintf(infoString, "Echec de la connexion au slot %d", (int)slot + 1);

    Draw_Lock();
    Draw_ClearFramebuffer();
    Draw_FlushFramebuffer();
    Draw_Unlock();

    do
    {
        Draw_Lock();
        Draw_DrawString(10, 10, COLOR_TITLE, "Menu configuration du syst\x8A""me");
        Draw_DrawString(10, 30, infoStringColor, infoString);
        Draw_DrawString(10, 40, COLOR_WHITE, "Appuyez sur B pour revenir en arri\x8A""re.");

        Draw_FlushFramebuffer();
        Draw_Unlock();

        u32 pressed = waitInputWithTimeout(1000);

        if(pressed & KEY_B)
            break;
    }
    while(!menuShouldExit);

    return forcedConnection;
}

void SysConfigMenu_TogglePowerButton(void)
{
    u32 mcuIRQMask;

    Draw_Lock();
    Draw_ClearFramebuffer();
    Draw_FlushFramebuffer();
    Draw_Unlock();

    mcuHwcInit();
    MCUHWC_ReadRegister(0x18, (u8*)&mcuIRQMask, 4);
    mcuHwcExit();

    do
    {
        Draw_Lock();
        Draw_DrawString(10, 10, COLOR_TITLE, "Menu configuration du syst\x8A""me");
        Draw_DrawString(10, 30, COLOR_WHITE, "Appuyez sur A pour basculer,\nAppuyez sur B pour revenir en arri\x8A""re.");

        Draw_DrawString(10, 60, COLOR_WHITE, "Statut actuel:");
        Draw_DrawString(100, 60, (((mcuIRQMask & 0x00000001) == 0x00000001) ? COLOR_RED : COLOR_GREEN), (((mcuIRQMask & 0x00000001) == 0x00000001) ? " DESACTIVER" : "   ACTIVER "));

        Draw_FlushFramebuffer();
        Draw_Unlock();

        u32 pressed = waitInputWithTimeout(1000);

        if(pressed & KEY_A)
        {
            mcuHwcInit();
            MCUHWC_ReadRegister(0x18, (u8*)&mcuIRQMask, 4);
            mcuIRQMask ^= 0x00000001;
            MCUHWC_WriteRegister(0x18, (u8*)&mcuIRQMask, 4);
            mcuHwcExit();
        }
        else if(pressed & KEY_B)
            return;
    }
    while(!menuShouldExit);
}

void SysConfigMenu_ControlWifi(void)
{
    Draw_Lock();
    Draw_ClearFramebuffer();
    Draw_FlushFramebuffer();
    Draw_Unlock();

    u32 slot = 0;
    char ssids[3][32] = {{0}};

    Result resInit = acInit();
    for (u32 i = 0; i < 3; i++)
    {
        if (R_SUCCEEDED(ACI_LoadNetworkSetting(i)))
            ACI_GetNetworkWirelessEssidSecuritySsid(ssids[i]);
        else
            strcpy(ssids[i], "(non configure)");
    }
    if (R_SUCCEEDED(resInit))
        acExit();

    do
    {
        Draw_Lock();
        Draw_DrawString(10, 10, COLOR_TITLE, "Menu configuration du syst\x8A""me");
        u32 posY = Draw_DrawString(10, 30, COLOR_WHITE, "Appuyez sur A pour forcer une connexion \x85"" un slot,\nAppuyer sur B pour revenir en arri\x8A""re.\n\n");

        for (u32 i = 0; i < 3; i++)
        {
            Draw_DrawString(10, posY + SPACING_Y * i, COLOR_TITLE, slot == i ? ">" : " ");
            Draw_DrawFormattedString(30, posY + SPACING_Y * i, COLOR_WHITE, "[%d] %s", (int)i + 1, ssids[i]);
        }

        Draw_FlushFramebuffer();
        Draw_Unlock();

        u32 pressed = waitInputWithTimeout(1000);

        if(pressed & KEY_A)
        {
            if(SysConfigMenu_ForceWifiConnection(slot))
            {
                // Connection successfully forced, return from this menu to prevent ac handle refcount leakage.
                break;
            }

            Draw_Lock();
            Draw_ClearFramebuffer();
            Draw_FlushFramebuffer();
            Draw_Unlock();
        }
        else if(pressed & KEY_DOWN)
        {
            slot = (slot + 1) % 3;
        }
        else if(pressed & KEY_UP)
        {
            slot = (3 + slot - 1) % 3;
        }
        else if(pressed & KEY_B)
            return;
    }
    while(!menuShouldExit);
}

void SysConfigMenu_DisableForcedWifiConnection(void)
{
    Draw_Lock();
    Draw_ClearFramebuffer();
    Draw_FlushFramebuffer();
    Draw_Unlock();

    acExit();
    SysConfigMenu_UpdateStatus(true);

    do
    {
        Draw_Lock();
        Draw_DrawString(10, 10, COLOR_TITLE, "Menu configuration du syst\x8A""me");
        Draw_DrawString(10, 30, COLOR_WHITE, "La connexion forcee a \x82""t\x82"" d\x82""sactiv\x82""e avec succ\x8A""s.\nRemarque : la connexion automatique peut rester interrompue..");

        u32 pressed = waitInputWithTimeout(1000);
        if(pressed & KEY_B)
            return;
    }
    while(!menuShouldExit);
}

void SysConfigMenu_ToggleCardIfPower(void)
{
    Draw_Lock();
    Draw_ClearFramebuffer();
    Draw_FlushFramebuffer();
    Draw_Unlock();

    bool cardIfStatus = false;
    bool updatedCardIfStatus = false;

    do
    {
        Result res = FSUSER_CardSlotGetCardIFPowerStatus(&cardIfStatus);
        if (R_FAILED(res)) cardIfStatus = false;

        Draw_Lock();
        Draw_DrawString(10, 10, COLOR_TITLE, "Menu configuration du syst\x8A""me");
        u32 posY = Draw_DrawString(10, 30, COLOR_WHITE, "Appuyez sur A pour basculer,\nAppuyez sur B pour revenir en arri\x8A""re.\n\n");
        posY = Draw_DrawString(10, posY, COLOR_WHITE, "L'insertion ou retrait d'une cartouche\nr\x82""initialise son \x82""tat,\nEt vous devrez r\x82""inserer la carte si vous\nsouhaitez y jouer..\n\n");
        Draw_DrawString(10, posY, COLOR_WHITE, "Statut actuel:");
        Draw_DrawString(100, posY, !cardIfStatus ? COLOR_RED : COLOR_GREEN, !cardIfStatus ? " DESACTIVER" : "   ACTIVER ");

        Draw_FlushFramebuffer();
        Draw_Unlock();

        u32 pressed = waitInputWithTimeout(1000);

        if(pressed & KEY_A)
        {
            if (!cardIfStatus)
                res = FSUSER_CardSlotPowerOn(&updatedCardIfStatus);
            else
                res = FSUSER_CardSlotPowerOff(&updatedCardIfStatus);

            if (R_SUCCEEDED(res))
                cardIfStatus = !updatedCardIfStatus;
        }
        else if(pressed & KEY_B)
            return;
    }
    while(!menuShouldExit);
}

static Result SysConfigMenu_ApplyVolumeOverride(void)
{
    // Credit: profi200
    u8 tmp;
    Result res = cdcChkInit();

    if (R_SUCCEEDED(res)) res = CDCCHK_ReadRegisters2(0, 116, &tmp, 1); // CDC_REG_VOL_MICDET_PIN_SAR_ADC
    if (currVolumeSliderOverride >= 0)
        tmp &= ~0x80;
    else
        tmp |= 0x80;
    if (R_SUCCEEDED(res)) res = CDCCHK_WriteRegisters2(0, 116, &tmp, 1);

    if (currVolumeSliderOverride >= 0) {
        s8 calculated = -128 + (((float)currVolumeSliderOverride/100.f) * 108);
        if (calculated > -20)
            res = -1; // Just in case
        s8 volumes[2] = {calculated, calculated}; // Volume in 0.5 dB steps. -128 (muted) to 48. Do not go above -20 (100%).
        if (R_SUCCEEDED(res)) res = CDCCHK_WriteRegisters2(0, 65, volumes, 2); // CDC_REG_DAC_L_VOLUME_CTRL, CDC_REG_DAC_R_VOLUME_CTRL
    }

    cdcChkExit();
    return res;
}

void SysConfigMenu_LoadConfig(void)
{
    s64 out = -1;
    svcGetSystemInfo(&out, 0x10000, 7);
    currVolumeSliderOverride = (s8)out;
    if (currVolumeSliderOverride >= 0)
        SysConfigMenu_ApplyVolumeOverride();
}

void SysConfigMenu_AdjustVolume(void)
{
    Draw_Lock();
    Draw_ClearFramebuffer();
    Draw_FlushFramebuffer();
    Draw_Unlock();

    s8 tempVolumeOverride = currVolumeSliderOverride;
    static s8 backupVolumeOverride = -1;
    if (backupVolumeOverride == -1)
        backupVolumeOverride = tempVolumeOverride >= 0 ? tempVolumeOverride : 85;

    do
    {
        Draw_Lock();
        Draw_DrawString(10, 10, COLOR_TITLE, "Menu configuration du syst\x8A""me");
        u32 posY = Draw_DrawString(10, 30, COLOR_WHITE, "Y: Bascule sur le curseur du volume.\nDPAD: R\x82""gler le niveau de volume.\nA: Appliquer\nB: Revenir en arri\x8A""re\n\n");
        Draw_DrawString(10, posY, COLOR_WHITE, "Statut actuel:");
        posY = Draw_DrawString(100, posY, (tempVolumeOverride == -1) ? COLOR_RED : COLOR_GREEN, (tempVolumeOverride == -1) ? " DESACTIVER" : "   ACTIVER ");
        if (tempVolumeOverride != -1) {
            posY = Draw_DrawFormattedString(30, posY, COLOR_WHITE, "\nValeur: [%d%%]", tempVolumeOverride);
        } else {
            posY = Draw_DrawString(30, posY, COLOR_WHITE, "\n                 ");
        }

        Draw_FlushFramebuffer();
        u32 pressed = waitInputWithTimeout(1000);

        if(pressed & KEY_A)
        {
            currVolumeSliderOverride = tempVolumeOverride;
            Result res = SysConfigMenu_ApplyVolumeOverride();
            LumaConfig_SaveSettings();
            if (R_SUCCEEDED(res))
                Draw_DrawString(10, posY, COLOR_GREEN, "\nSucc\x8A""s!");
            else
                Draw_DrawFormattedString(10, posY, COLOR_RED, "\nEchec: 0x%08lX", res);
        }
        else if(pressed & KEY_B)
            return;
        else if(pressed & KEY_Y)
        {
            Draw_DrawString(10, posY, COLOR_WHITE, "\n                 ");
            if (tempVolumeOverride == -1) {
                tempVolumeOverride = backupVolumeOverride;
            } else {
                backupVolumeOverride = tempVolumeOverride;
                tempVolumeOverride = -1;
            }
        }
        else if ((pressed & (KEY_DUP | KEY_DDOWN | KEY_DLEFT | KEY_DRIGHT)) && tempVolumeOverride != -1)
        {
            Draw_DrawString(10, posY, COLOR_WHITE, "\n                 ");
            if (pressed & KEY_DUP)
                tempVolumeOverride++;
            else if (pressed & KEY_DDOWN)
                tempVolumeOverride--;
            else if (pressed & KEY_DRIGHT)
                tempVolumeOverride+=10;
            else if (pressed & KEY_DLEFT)
                tempVolumeOverride-=10;

            if (tempVolumeOverride < 0)
                tempVolumeOverride = 0;
            if (tempVolumeOverride > 100)
                tempVolumeOverride = 100;
        }
    } while(!menuShouldExit);
}
