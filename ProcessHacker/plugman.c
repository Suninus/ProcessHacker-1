/*
 * Process Hacker - 
 *   plugins
 * 
 * Copyright (C) 2010-2011 wj32
 * 
 * This file is part of Process Hacker.
 * 
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <phapp.h>
#include <settings.h>
#include <phplug.h>

#define STR_OR_DEFAULT(String, Default) ((String) ? (String) : (Default))

static HWND PluginsLv;
static PPH_PLUGIN SelectedPlugin;

INT_PTR CALLBACK PhpPluginsDlgProc(
    __in HWND hwndDlg,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

VOID PhShowPluginsDialog(
    __in HWND ParentWindowHandle
    )
{
    if (PhPluginsEnabled)
    {
        DialogBox(
            PhInstanceHandle,
            MAKEINTRESOURCE(IDD_PLUGINS),
            ParentWindowHandle,
            PhpPluginsDlgProc
            );
    }
    else
    {
        PhShowInformation(ParentWindowHandle,
            L"Plugins are not enabled. To use plugins enable them in Options and restart Process Hacker.");
    }
}

VOID PhpRefreshPluginDetails(
    __in HWND hwndDlg
    )
{
    PPH_STRING fileName;
    PH_IMAGE_VERSION_INFO versionInfo;

    if (SelectedPlugin)
    {
        fileName = SelectedPlugin->FileName;

        SetDlgItemText(hwndDlg, IDC_NAME, SelectedPlugin->Information.DisplayName ? SelectedPlugin->Information.DisplayName : L"(unnamed)");
        SetDlgItemText(hwndDlg, IDC_INTERNALNAME, SelectedPlugin->Name);
        SetDlgItemText(hwndDlg, IDC_AUTHOR, SelectedPlugin->Information.Author);
        SetDlgItemText(hwndDlg, IDC_FILENAME, fileName->Buffer);
        SetDlgItemText(hwndDlg, IDC_DESCRIPTION, SelectedPlugin->Information.Description);
        SetDlgItemText(hwndDlg, IDC_URL, SelectedPlugin->Information.Url);

        if (PhInitializeImageVersionInfo(&versionInfo, fileName->Buffer))
        {
            SetDlgItemText(hwndDlg, IDC_VERSION, PhGetStringOrDefault(versionInfo.FileVersion, L"Unknown"));
            PhDeleteImageVersionInfo(&versionInfo);
        }
        else
        {
            SetDlgItemText(hwndDlg, IDC_VERSION, L"Unknown");
        }

        ShowWindow(GetDlgItem(hwndDlg, IDC_OPENURL), SelectedPlugin->Information.Url ? SW_SHOW : SW_HIDE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_OPTIONS), SelectedPlugin->Information.HasOptions);
    }
    else
    {
        SetDlgItemText(hwndDlg, IDC_NAME, L"N/A");
        SetDlgItemText(hwndDlg, IDC_VERSION, L"N/A");
        SetDlgItemText(hwndDlg, IDC_INTERNALNAME, L"N/A");
        SetDlgItemText(hwndDlg, IDC_AUTHOR, L"N/A");
        SetDlgItemText(hwndDlg, IDC_URL, L"N/A");
        SetDlgItemText(hwndDlg, IDC_FILENAME, L"N/A");
        SetDlgItemText(hwndDlg, IDC_DESCRIPTION, L"N/A");

        ShowWindow(GetDlgItem(hwndDlg, IDC_OPENURL), SW_HIDE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_OPTIONS), FALSE);
    }
}

static COLORREF PhpPluginColorFunction(
    __in INT Index,
    __in PVOID Param,
    __in_opt PVOID Context
    )
{
    PPH_PLUGIN plugin = Param;

    if (plugin->Flags & PH_PLUGIN_FLAG_IS_CLR)
        return RGB(0xde, 0xff, 0x00);

    return PhSysWindowColor;
}

INT_PTR CALLBACK PhpPluginsDlgProc(
    __in HWND hwndDlg,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PPH_AVL_LINKS links;

            PluginsLv = GetDlgItem(hwndDlg, IDC_LIST);
            PhSetListViewStyle(PluginsLv, FALSE, TRUE);
            PhSetControlTheme(PluginsLv, L"explorer");
            PhAddListViewColumn(PluginsLv, 0, 0, 0, LVCFMT_LEFT, 200, L"Name");
            PhAddListViewColumn(PluginsLv, 1, 1, 1, LVCFMT_LEFT, 160, L"Author");
            PhSetExtendedListView(PluginsLv);
            ExtendedListView_SetItemColorFunction(PluginsLv, PhpPluginColorFunction);

            links = PhMinimumElementAvlTree(&PhPluginsByName);

            while (links)
            {
                PPH_PLUGIN plugin = CONTAINING_RECORD(links, PH_PLUGIN, Links);
                INT lvItemIndex;

                lvItemIndex = PhAddListViewItem(PluginsLv, MAXINT,
                    plugin->Information.DisplayName ? plugin->Information.DisplayName : plugin->Name, plugin);

                if (plugin->Information.Author)
                    PhSetListViewSubItem(PluginsLv, lvItemIndex, 1, plugin->Information.Author);

                links = PhSuccessorElementAvlTree(links);
            }

            ExtendedListView_SortItems(PluginsLv);

            SelectedPlugin = NULL;
            PhpRefreshPluginDetails(hwndDlg);
        }
        break;
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDCANCEL:
            case IDOK:
                EndDialog(hwndDlg, IDOK);
                break;
            case IDC_OPTIONS:
                {
                    if (SelectedPlugin)
                    {
                        PhInvokeCallback(PhGetPluginCallback(SelectedPlugin, PluginCallbackShowOptions), hwndDlg);
                    }
                }
                break;
            case IDC_CLEANUP:
                {
                    if (PhShowMessage(hwndDlg, MB_ICONQUESTION | MB_YESNO,
                        L"Do you want to clean up unused plugin settings?") == IDYES)
                    {
                        PhClearIgnoredSettings();
                    }
                }
                break;
            case IDC_OPENURL:
                {
                    if (SelectedPlugin)
                    {
                    }
                }
                break;
            }
        }
        break;
    case WM_NOTIFY:
        {
            LPNMHDR header = (LPNMHDR)lParam;

            switch (header->code)
            {
            case LVN_ITEMCHANGED:
                {
                    if (header->hwndFrom == PluginsLv)
                    {
                        if (ListView_GetSelectedCount(PluginsLv) == 1)
                            SelectedPlugin = PhGetSelectedListViewItemParam(PluginsLv);
                        else
                            SelectedPlugin = NULL;

                        PhpRefreshPluginDetails(hwndDlg);
                    }
                }
                break;
            case NM_CLICK:
                {
                    if (header->hwndFrom == GetDlgItem(hwndDlg, IDC_OPENURL))
                    {
                        if (SelectedPlugin)
                            PhShellExecute(hwndDlg, SelectedPlugin->Information.Url, NULL);
                    }
                }
                break;
            }
        }
        break;
    }

    REFLECT_MESSAGE_DLG(hwndDlg, PluginsLv, uMsg, wParam, lParam);

    return FALSE;
}
