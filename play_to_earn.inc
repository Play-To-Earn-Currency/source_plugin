#if defined play_to_earn
    #endinput
#endif
#define play_to_earn

#include <sdktools_stringtables>
#include <SteamWorks>

public const char        DISCORD_INVITE[]      = "discord.gg/vGHxVsXc4Q";
static const char DATABASE_HTTP_IP[]    = "http://localhost:8000";
static const char PAYMENT_HTTP_IP[]     = "http://localhost:8001";
static const char SKIN_HTTP_IP[]        = "http://localhost:8002";
static const char SKINS_ID_PATH[]       = "addons/sourcemod/configs/skins_reader/skins_id.init";
static const char SKINS_NAME_PATH[]     = "addons/sourcemod/configs/skins_reader/skins_name.init";
static const char SKINS_DOWNLOAD_LIST[] = "addons/sourcemod/configs/skins_reader/downloads_list.ini";

// Configurations
public bool       enableSkinMenu = false;

//
// #REGION Increment Wallet
//

char       incrementWalletRequestBodies[MAXPLAYERS][256];
// Increment value to the player wallet
stock void IncrementWallet(
    int client,
    char[] valueToIncrement,
    char[] valueToShow = "0 PTE",
    char[] reason      = ", for Playing")
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when incrementing wallet");
        return;
    }

    char url[256];
    Format(url, sizeof(url), "%s/increment", DATABASE_HTTP_IP);
    Handle requestHandle = SteamWorks_CreateHTTPRequest(k_EHTTPMethodPUT, url);

    if (requestHandle == INVALID_HANDLE)
    {
        PrintToServer("[PTE-INC] Error while creating the http request.");
        return;
    }

    SteamWorks_SetHTTPCallbacks(requestHandle, OnIncrementRequest);

    char bodyStr[256];
    Format(bodyStr, sizeof(bodyStr),
           "{\"quantity\": \"%s\", \"valueToShow\": \"%s\", \"reason\": \"%s\", \"uniqueid\": %d}",
           valueToIncrement, valueToShow, reason, steamid);

    char from[64];
    GetGameFolderName(from, sizeof(from));

    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "Content-Type", "application/json");
    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "from", from);
    SteamWorks_SetHTTPRequestRawPostBody(requestHandle, "application/json", bodyStr, strlen(bodyStr));
    SteamWorks_SetHTTPRequestContextValue(requestHandle, client);

    IncrementWalletStartRequestWaitTimer(requestHandle, client, bodyStr);
}

stock void IncrementWalletStartRequestWaitTimer(Handle requestHandle, int client, const char[] bodyStr)
{
    DataPack pack = new DataPack();
    pack.WriteCell(client);
    pack.WriteCell(requestHandle);
    pack.WriteString(bodyStr);

    CreateTimer(0.5, IncrementWalletWaitForEmptyQueue, pack, TIMER_REPEAT);
}

stock Action IncrementWalletWaitForEmptyQueue(Handle timer, DataPack pack)
{
    pack.Reset();

    int    client        = pack.ReadCell();
    Handle requestHandle = pack.ReadCell();
    char   bodyStr[256];
    pack.ReadString(bodyStr, sizeof(bodyStr));

    if (incrementWalletRequestBodies[client][0] == EOS)
    {
        delete pack;
        strcopy(incrementWalletRequestBodies[client], sizeof(incrementWalletRequestBodies[]), bodyStr);
        SteamWorks_SendHTTPRequest(requestHandle);
        return Plugin_Stop;
    }
    return Plugin_Continue;
}

stock void OnIncrementRequest(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
{
    int client = data1;

    if (eStatusCode != k_EHTTPStatusCode200OK)
    {
        incrementWalletRequestBodies[client][0] = EOS;
        PrintToChat(client, "[PTE] Cannot increment your wallet, contact server owner on: %s", DISCORD_INVITE);
        return;
    }

    if (incrementWalletRequestBodies[client][0] == EOS)
    {
        PrintToChat(client, "[PTE] Cannot increment your wallet, contact server owner on: %s", DISCORD_INVITE);
        PrintToServer("[PTE-INC] [Increment] ERROR: %d (bodySended index) have any invalid bodySended object: %s", client, incrementWalletRequestBodies[client]);
        return;
    }

    char bodySended[256];
    strcopy(bodySended, sizeof(bodySended), incrementWalletRequestBodies[client]);
    incrementWalletRequestBodies[client][0] = EOS;

    char valueToShow[32];
    char reason[32];

    ExtractJsonValue(bodySended, "valueToShow", valueToShow, sizeof(valueToShow));
    ExtractJsonValue(bodySended, "reason", reason, sizeof(reason));

    PrintToChat(client, "[PTE] You received: %s%s", valueToShow, reason);
}

//
// #ENDREGION Increment Wallet
//

//
// #REGION Register Player
//

// Try to register the player, should be called before any function in this include
stock void RegisterPlayer(const int client)
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when registering player");
        return;
    }

    char url[256];
    Format(url, sizeof(url), "%s/register", DATABASE_HTTP_IP);
    Handle requestHandle = SteamWorks_CreateHTTPRequest(k_EHTTPMethodPOST, url);

    if (requestHandle == INVALID_HANDLE)
    {
        PrintToServer("[PTE-INC] Error while creating the http request.");
        return;
    }

    SteamWorks_SetHTTPCallbacks(requestHandle, OnRegisterPlayerRequest);

    char bodyStr[256];
    Format(bodyStr, sizeof(bodyStr),
           "{\"uniqueid\": %d}",
           steamid);

    char from[64];
    GetGameFolderName(from, sizeof(from));

    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "Content-Type", "application/json");
    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "from", from);
    SteamWorks_SetHTTPRequestRawPostBody(requestHandle, "application/json", bodyStr, strlen(bodyStr));

    SteamWorks_SendHTTPRequest(requestHandle);
}

stock void OnRegisterPlayerRequest(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
{
}

//
// #ENDREGION Register Player
//

//
// #REGION Register Player
//

// Updates a player wallet based on player client
stock void UpdateWallet(const int client, const char walletaddress[256])
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when registering player");
        return;
    }

    char url[256];
    Format(url, sizeof(url), "%s/updatewallet", DATABASE_HTTP_IP);
    Handle requestHandle = SteamWorks_CreateHTTPRequest(k_EHTTPMethodPUT, url);

    if (requestHandle == INVALID_HANDLE)
    {
        PrintToServer("[PTE-INC] Error while creating the http request.");
        return;
    }

    SteamWorks_SetHTTPRequestContextValue(requestHandle, client);
    SteamWorks_SetHTTPCallbacks(requestHandle, OnCommandRegisterWalletRequest);

    char bodyStr[256];
    Format(bodyStr, sizeof(bodyStr),
           "{\"uniqueid\": %d, \"walletaddress\": \"%s\"}",
           steamid, walletaddress);

    char from[64];
    GetGameFolderName(from, sizeof(from));

    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "Content-Type", "application/json");
    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "from", from);
    SteamWorks_SetHTTPRequestRawPostBody(requestHandle, "application/json", bodyStr, strlen(bodyStr));

    SteamWorks_SendHTTPRequest(requestHandle);
}

stock void OnCommandRegisterWalletRequest(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
{
    int client = data1;

    if (eStatusCode != k_EHTTPStatusCode200OK)
    {
        PrintToChat(client, "[PTE] Cannot register your address, contact server owner on: %s", DISCORD_INVITE);
    }
    else {
        PrintToChat(client, "[PTE] Wallet changed!");
    }
}

//
// #ENDREGION Register Player
//

//
// #REGION Equip Skin
//

// Equip the skin to the player
stock void Simple_EquipSkin(const int client, const char[] skinid)
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when equip skin");
        return;
    }

    char url[256];
    Format(url, sizeof(url), "%s/equipskin", SKIN_HTTP_IP);
    Handle requestHandle = SteamWorks_CreateHTTPRequest(k_EHTTPMethodPOST, url);

    if (requestHandle == INVALID_HANDLE)
    {
        PrintToServer("[PTE-INC] Error while creating the http request.");
        return;
    }

    SteamWorks_SetHTTPRequestContextValue(requestHandle, client);
    SteamWorks_SetHTTPCallbacks(requestHandle, Simple_OnEquipSkinRequest);

    char bodyStr[256];
    Format(bodyStr, sizeof(bodyStr),
           "{\"uniqueid\": %d, \"skinid\": %s}",
           steamid, skinid);

    char from[64];
    GetGameFolderName(from, sizeof(from));

    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "Content-Type", "application/json");
    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "from", from);
    SteamWorks_SetHTTPRequestRawPostBody(requestHandle, "application/json", bodyStr, strlen(bodyStr));

    SteamWorks_SendHTTPRequest(requestHandle);
}

stock void Simple_OnEquipSkinRequest(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
{
    int client = data1;

    if (eStatusCode == k_EHTTPStatusCode200OK)
    {
        PrintToChat(client, "[PTE] Skin equipped successfully");
    }
    else if (eStatusCode == k_EHTTPStatusCode501NotImplemented) {
        PrintToChat(client, "[PTE] You don't have any wallet registered, you can register by using: !wallet 0x123..., for more information you can check your discord: %s", DISCORD_INVITE);
    }
    else {
        PrintToChat(client, "[PTE] Server error, contact server owner on: %s, code: %d", DISCORD_INVITE, eStatusCode);
    }
}

//
// #ENDREGION Equip Skin
//

//
// #REGION SIMPLE Equipped Skin
//

Function   equippedSkinRequestCallbacks[MAXPLAYERS];
stock void Simple_EquippedSkin(int client, Function callback)
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when equip skin");
        return;
    }

    char url[256];
    Format(url, sizeof(url), "%s/equippedskin?uniqueid=%d", SKIN_HTTP_IP, steamid);
    Handle requestHandle = SteamWorks_CreateHTTPRequest(k_EHTTPMethodGET, url);

    if (requestHandle == INVALID_HANDLE)
    {
        PrintToServer("[PTE-INC] Error while creating the http request.");
        return;
    }

    SteamWorks_SetHTTPRequestContextValue(requestHandle, client);
    SteamWorks_SetHTTPCallbacks(requestHandle, Simple_OnEquippedSkinRequest);

    char from[64];
    GetGameFolderName(from, sizeof(from));

    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "from", from);

    Simple_EquippedSkinStartRequestWaitTimer(requestHandle, client, callback);
}

stock void Simple_EquippedSkinStartRequestWaitTimer(Handle requestHandle, int client, Function callback)
{
    DataPack pack = new DataPack();
    pack.WriteCell(client);
    pack.WriteCell(requestHandle);
    pack.WriteFunction(callback);

    CreateTimer(0.5, Simple_EquippedSkinWaitForEmptyQueue, pack, TIMER_REPEAT);
}

stock Action Simple_EquippedSkinWaitForEmptyQueue(Handle timer, DataPack pack)
{
    pack.Reset();

    int      client        = pack.ReadCell();
    Handle   requestHandle = pack.ReadCell();
    Function callback      = pack.ReadFunction();

    if (equippedSkinRequestCallbacks[client] == INVALID_FUNCTION)
    {
        delete pack;
        equippedSkinRequestCallbacks[client] = callback;
        SteamWorks_SendHTTPRequest(requestHandle);
        return Plugin_Stop;
    }
    return Plugin_Continue;
}

stock void Simple_OnEquippedSkinRequest(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
{
    int      client   = data1;

    Function callback = equippedSkinRequestCallbacks[client];
    if (callback == INVALID_FUNCTION)
    {
        PrintToServer("[PTE-INC] [EquippedSkin] ERROR: %d (client) have any invalid callback object", client);
        return;
    }

    if (eStatusCode == k_EHTTPStatusCode200OK)
    {
        int bodySize = 0;
        if (!SteamWorks_GetHTTPResponseBodySize(hRequest, bodySize) || bodySize <= 0)
        {
            PrintToServer("[PTE-INC] Body response is empty");
            equippedSkinRequestCallbacks[client] = INVALID_FUNCTION;
            return;
        }

        char[] skinid = new char[bodySize + 1];
        if (!SteamWorks_GetHTTPResponseBodyData(hRequest, skinid, bodySize))
        {
            PrintToServer("[PTE-INC] Failed to receive equippedskin body response");
            equippedSkinRequestCallbacks[client] = INVALID_FUNCTION;
            return;
        }

        Call_StartFunction(INVALID_HANDLE, callback);
        Call_PushCell(client);
        Call_PushString(skinid);
        Call_Finish();
    }

    equippedSkinRequestCallbacks[client] = INVALID_FUNCTION;
}

//
// #ENDREGION SIMPLE Equipped Skin
//

//
// #REGION Show Menu
//

stock void ShowMenu(int client)
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when registering player");
        return;
    }

    char url[256];
    Format(url, sizeof(url), "%s/getbalance?uniqueid=%d", DATABASE_HTTP_IP, steamid);
    Handle requestHandle = SteamWorks_CreateHTTPRequest(k_EHTTPMethodGET, url);

    if (requestHandle == INVALID_HANDLE)
    {
        PrintToServer("[PTE] Error while creating the http request.");
        return;
    }

    SteamWorks_SetHTTPRequestContextValue(requestHandle, client);
    SteamWorks_SetHTTPCallbacks(requestHandle, OnMenuBalanceReceived);

    char from[64];
    GetGameFolderName(from, sizeof(from));

    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "from", from);
    SteamWorks_SendHTTPRequest(requestHandle);
}

stock OnMenuBalanceReceived(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
{
    int client = data1;

    if (eStatusCode != k_EHTTPStatusCode200OK)
    {
        PrintToServer("[PTE] Cannot receive balance from user, please check the http server");
        return;
    }

    int bodySize = 0;
    if (!SteamWorks_GetHTTPResponseBodySize(hRequest, bodySize) || bodySize <= 0)
    {
        PrintToServer("[PTE] Body response is empty");
        return;
    }

    char[] balance = new char[bodySize + 1];
    if (!SteamWorks_GetHTTPResponseBodyData(hRequest, balance, bodySize))
    {
        PrintToServer("[PTE] Failed to receive balance body response");
        return;
    }

    Menu menu = new Menu(MenuHandler);
    menu.SetTitle("Your Current Balance: %s PTE\nYou can reopen this menu in chat: !menu", balance);
    menu.AddItem("1", "Request Payment");
    menu.AddItem("2", "Register Wallet");
    menu.AddItem("3", "View Steam ID");
    if (enableSkinMenu)
    {
        menu.AddItem("4", "View Available Skins");
    }
    menu.Display(client, 8);
}

stock int MenuHandler(Menu menu, MenuAction action, int client, int param)
{
    if (action == MenuAction_Select)
    {
        char info[32];
        menu.GetItem(param, info, sizeof(info));

        if (StrEqual(info, "1"))
        {
            PrintToChat(client, "[PTE] Payment request send, to reopen this menu type: !menu");

            int  steamid = GetSteamAccountID(client);

            char url[256];
            Format(url, sizeof(url), "%s/requestpayment", PAYMENT_HTTP_IP);
            Handle requestHandle = SteamWorks_CreateHTTPRequest(k_EHTTPMethodPUT, url);

            if (requestHandle == INVALID_HANDLE)
            {
                PrintToServer("[PTE] Error while creating the http request.");
                return 0;
            }

            SteamWorks_SetHTTPRequestContextValue(requestHandle, client);
            SteamWorks_SetHTTPCallbacks(requestHandle, OnPaymentRequestReceived);

            SteamWorks_SetHTTPRequestContextValue(requestHandle, client);
            SteamWorks_SetHTTPCallbacks(requestHandle, OnMenuBalanceReceived);

            char bodyStr[256];
            Format(bodyStr, sizeof(bodyStr),
                   "{\"uniqueid\": %d}",
                   steamid);

            char from[64];
            GetGameFolderName(from, sizeof(from));

            SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "Content-Type", "application/json");
            SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "from", from);
            SteamWorks_SetHTTPRequestRawPostBody(requestHandle, "application/json", bodyStr, strlen(bodyStr));

            SteamWorks_SendHTTPRequest(requestHandle);
        }
        else if (StrEqual(info, "2"))
        {
            PrintToChat(client, "[PTE] You can register your wallet address in the chat, like: !wallet 0x123..., for more information you can check your discord: %s", DISCORD_INVITE);
            ShowMenu(client);
        }
        else if (StrEqual(info, "3"))
        {
            PrintToChat(client, "[PTE] Your steam id is: %d", GetSteamAccountID(client));
            ShowMenu(client);
        }
        else if (StrEqual(info, "4") && enableSkinMenu)
        {
            PrintToChat(client, "[PTE] Wait a second while the server is loading your available skins...");
            Simple_ShowSkinsMenu(client);
        }
    }

    return 0;
}

stock OnPaymentRequestReceived(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
{
    int client = data1;

    if (eStatusCode == k_EHTTPStatusCode200OK)
    {
        PrintToChat(client, "[PTE] Payment request successfully, check your wallet!");
    }
    else if (eStatusCode == k_EHTTPStatusCode429TooManyRequests) {
        PrintToChat(client, "[PTE] Payment server is overloaded, please wait a few minutes before trying again.");
    }
    else if (eStatusCode == k_EHTTPStatusCode409Conflict) {
        PrintToChat(client, "[PTE] Your request is still running, please wait a few minutes before trying again.");
    }
    else if (eStatusCode == k_EHTTPStatusCode404NotFound) {
        PrintToChat(client, "[PTE] User not found, contact server owner on: %s", DISCORD_INVITE);
    }
    else if (eStatusCode == k_EHTTPStatusCode405MethodNotAllowed) {
        PrintToChat(client, "[PTE] Wallet not registered, you can register by using: !wallet 0x123..., for more information you can check your discord: %s", DISCORD_INVITE);
    }
    else if (eStatusCode == k_EHTTPStatusCode406NotAcceptable) {
        PrintToChat(client, "[PTE] You don't have sufficient PTE to request a payment, please play more to earn PTE. >.<");
    }
    else {
        PrintToChat(client, "[PTE] Server error, contact server owner on: %s, code: %d", DISCORD_INVITE, eStatusCode);
    }
}

//
// #ENDREGION Show Menu
//

//
// #REGION Show SIMPLE Skins Menu
//

// Show the skin menu to the player
stock void Simple_ShowSkinsMenu(int client)
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when registering player");
        return;
    }

    char url[256];
    Format(url, sizeof(url), "%s/availableskins?uniqueid=%d", SKIN_HTTP_IP, steamid);
    Handle requestHandle = SteamWorks_CreateHTTPRequest(k_EHTTPMethodGET, url);

    if (requestHandle == INVALID_HANDLE)
    {
        PrintToServer("[PTE] Error while creating the http request.");
        return;
    }

    SteamWorks_SetHTTPRequestContextValue(requestHandle, client);
    SteamWorks_SetHTTPCallbacks(requestHandle, Simple_OnSkinMenuSkinsReceived);

    char from[64];
    GetGameFolderName(from, sizeof(from));

    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "from", from);
    SteamWorks_SendHTTPRequest(requestHandle);
}

stock Simple_OnSkinMenuSkinsReceived(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
{
    int client = data1;

    if (eStatusCode == k_EHTTPStatusCode501NotImplemented)
    {
        PrintToChat(client, "[PTE] Cannot view your skins because you don't have any wallet, you can register your wallet address in the chat, like: !wallet 0x123..., for more information you can check your discord: %s", DISCORD_INVITE);
        return;
    }

    if (eStatusCode != k_EHTTPStatusCode200OK)
    {
        PrintToServer("[PTE] Cannot receive skins from user, please check the http server");
        return;
    }

    int bodySize = 0;
    if (!SteamWorks_GetHTTPResponseBodySize(hRequest, bodySize) || bodySize <= 0)
    {
        PrintToServer("[PTE] Body response is empty");
        return;
    }

    char[] skins = new char[bodySize + 1];
    if (!SteamWorks_GetHTTPResponseBodyData(hRequest, skins, bodySize))
    {
        PrintToServer("[PTE] Failed to receive skins body response");
        return;
    }

    if (skins[0] == '[')
    {
        strcopy(skins, bodySize + 1, skins[1]);
    }

    int len = strlen(skins);
    if (len > 0 && skins[len - 1] == ']')
    {
        skins[len - 1] = '\0';
    }

    char parts[16][16];
    int  count = ExplodeString(skins, ",", parts, sizeof(parts), sizeof(parts[]));

    Menu menu  = new Menu(Simple_SkinMenuHandler);
    menu.SetTitle("Select your desired skin");
    for (int i = 0; i < count; i++)
    {
        char skinName[PLATFORM_MAX_PATH];
        if (GetSkinNameBySkinID(parts[i], skinName, sizeof(skinName)))
        {
            menu.AddItem(parts[i], skinName);
        }
        else {
            PrintToServer("[PTE-INC] ERROR: No name can be found for skinid: %s, verify in %s", parts[i], SKINS_NAME_PATH);
            menu.AddItem(parts[i], "No Name");
        }
    }
    menu.Display(client, 8);
}

stock int Simple_SkinMenuHandler(Menu menu, MenuAction action, int client, int param)
{
    if (action == MenuAction_Select)
    {
        char info[32];
        menu.GetItem(param, info, sizeof(info));

        Simple_EquipSkin(client, info);
    }

    return 0;
}

//
// #ENDREGION Show Skins Menu
//

//
// #REGION Utils
//

// Returns any array containing all "client" from online players
stock void GetOnlinePlayers(int[] onlinePlayers, int playerSize)
{
    int arrayIndex = 0;
    for (int i = 1; i < MaxClients; i += 1)
    {
        if (arrayIndex >= playerSize)
        {
            break;
        }

        int client = i;

        if (!IsValidClient(client))
        {
            continue;
        }

        onlinePlayers[arrayIndex] = client;
        arrayIndex++;
    }
}

// Returns a int number containing the count of online players
stock int GetOnlinePlayersCount()
{
    int count = 0;
    for (int i = 0; i < MaxClients; i += 1)
    {
        int client = i;

        if (!IsValidClient(client))
        {
            continue;
        }

        count++;
    }

    return count;
}

/**
 * Extracts the value of a given `key` from a simple JSON string `jsonStr`.
 *
 * @param jsonStr   JSON string (e.g. {"key": "value", "num": 123})
 * @param key       The key to extract the value from (e.g. "quantity")
 * @param out       Buffer where the extracted value will be stored
 * @param maxlen    Maximum size of the `out` buffer
 *
 * @return true if the key was found and value extracted, false otherwise
 */
bool ExtractJsonValue(const char[] jsonStr, const char[] key, char[] out, int maxlen)
{
    int keyPos = StrContains(jsonStr, key);
    if (keyPos == -1)
    {
        return false;    // key not found
    }

    // Position of value start: after key + ":"
    int valStart = keyPos + strlen(key);

    // Skip spaces, colons, quotes
    while (jsonStr[valStart] == ' ' || jsonStr[valStart] == ':' || jsonStr[valStart] == '"' || jsonStr[valStart] == '\'')
    {
        valStart++;
    }

    // Find end of value: quote, comma, or closing brace
    int valEnd = valStart;
    while (jsonStr[valEnd] != '"' && jsonStr[valEnd] != '\'' && jsonStr[valEnd] != ',' && jsonStr[valEnd] != '}' && valEnd < strlen(jsonStr))
    {
        valEnd++;
    }

    int length = valEnd - valStart;
    if (length <= 0 || length >= maxlen)
    {
        return false;    // invalid value or output buffer too small
    }

    // Clear output buffer
    strcopy(out, maxlen, "");

    // Manual copy of substring
    for (int i = 0; i < length; i++)
    {
        out[i] = jsonStr[valStart + i];
    }

    out[length] = '\0';    // Null-terminate the output string

    return true;
}

// Detects if the client is valid
stock bool IsValidClient(client)
{
    if (client <= 0 || client > MaxClients || !IsClientConnected(client) || IsFakeClient(client))
    {
        return false;
    }
    return IsClientInGame(client);
}

// Returns the model path by the skinid
stock bool GetSkinModelPathBySkinID(const char[] providedSkinID, char[] skinPathBuffer, int skinPathBufferSize)
{
    Handle file = OpenFile(SKINS_ID_PATH, "r");
    if (file == INVALID_HANDLE)
    {
        PrintToServer("[PTE-INC] [PreCacheModels] ERROR: Cannot open the file. did you forget to create it?");
        return false;
    }

    char line[PLATFORM_MAX_PATH + 65];
    char buffers[2][PLATFORM_MAX_PATH];

    while (ReadFileLine(file, line, sizeof(line)))
    {
        TrimString(line);
        int numParts = ExplodeString(line, ":", buffers, 2, PLATFORM_MAX_PATH);

        if (numParts != 2)
        {
            PrintToServer("[PTE-INC] [GetSkinModelPathBySkinID] ERROR: Invalid format: %s, you should follow the standard: 'id:path'", line);
            continue;
        }

        char id[64];
        char path[PLATFORM_MAX_PATH];

        strcopy(id, sizeof(id), buffers[0]);
        strcopy(path, sizeof(path), buffers[1]);

        // PrintToServer("[PTE-INC] ITERATION: %s == %s", providedSkinID, id);

        if (StrEqual(id, providedSkinID))
        {
            strcopy(skinPathBuffer, skinPathBufferSize, path);
            return true;
        }
    }
    CloseHandle(file);

    return false;
}

// Returns the skin name by the skinid
stock bool GetSkinNameBySkinID(const char[] providedSkinID, char[] skinNameBuffer, int skinNameBufferSize)
{
    Handle file = OpenFile(SKINS_NAME_PATH, "r");
    if (file == INVALID_HANDLE)
    {
        PrintToServer("[PTE-INC] [PreCacheModels] ERROR: Cannot open the file. did you forget to create it?");
        return false;
    }

    char line[PLATFORM_MAX_PATH + 65];
    char buffers[2][PLATFORM_MAX_PATH];

    while (ReadFileLine(file, line, sizeof(line)))
    {
        TrimString(line);
        int numParts = ExplodeString(line, ":", buffers, 2, PLATFORM_MAX_PATH);

        if (numParts != 2)
        {
            PrintToServer("[PTE-INC] [GetSkinModelPathBySkinID] ERROR: Invalid format: %s, you should follow the standard: 'id:path'", line);
            continue;
        }

        char id[64];
        char name[PLATFORM_MAX_PATH];

        strcopy(id, sizeof(id), buffers[0]);
        strcopy(name, sizeof(name), buffers[1]);

        // PrintToServer("[PTE-INC] ITERATION: %s == %s", providedSkinID, id);

        if (StrEqual(id, providedSkinID))
        {
            strcopy(skinNameBuffer, skinNameBufferSize, name);
            return true;
        }
    }
    CloseHandle(file);

    return false;
}

// Load available skins to downloads table, should be called OnMapStart
stock void LoadSkinsToDownloadTable()
{
    Handle file = OpenFile(SKINS_DOWNLOAD_LIST, "r");
    if (file == INVALID_HANDLE)
    {
        PrintToServer("[Skin Reader] [PreCacheModels] ERROR: Cannot open the file. did you forget to create it?");
        return;
    }

    char line[PLATFORM_MAX_PATH];

    while (ReadFileLine(file, line, sizeof(line)))
    {
        TrimString(line);

        // Ignore comments, for some reason is inverted
        if (!StrContains(line, "//"))
        {
            continue;
        }
        // Ignore spaces
        if (strlen(line) == 0)
        {
            continue;
        }

        AddFileToDownloadsTable(line)
    }
    CloseHandle(file);
}

//
// #ENDREGION Utils
//