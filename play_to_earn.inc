#if defined play_to_earn
    #endinput
#endif
#define play_to_earn

#include <SteamWorks>

public char        DISCORD_INVITE[32]   = "discord.gg/vGHxVsXc4Q";
static char DATABASE_HTTP_IP[32] = "http://localhost:8000";
static char PAYMENT_HTTP_IP[32]  = "http://localhost:8001";

//
// #REGION Increment Wallet
//

char        incrementWalletRequestBodies[MAXPLAYERS][256];
/// Increment value to the player wallet
stock void  IncrementWallet(
     int client,
     char[] valueToIncrement,
     char[] valueToShow = "0 PTE",
     char[] reason      = ", for Playing")
{
    if (!IsClientConnected(client) || IsFakeClient(client))
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

/// Try to register the player
stock void RegisterPlayer(const int client)
{
    if (!IsClientConnected(client) || IsFakeClient(client))
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

/// Updates a player wallet based on player client
stock void UpdateWallet(const int client, const char walletaddress[256])
{
    if (!IsClientConnected(client) || IsFakeClient(client))
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
// #REGION Show Menu
//

stock void ShowMenu(int client)
{
    if (!IsClientConnected(client) || IsFakeClient(client))
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

    char bodyStr[256];
    Format(bodyStr, sizeof(bodyStr),
           "{\"uniqueid\": %d}",
           steamid);

    char from[64];
    GetGameFolderName(from, sizeof(from));

    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "Content-Type", "application/json");
    SteamWorks_SetHTTPRequestHeaderValue(requestHandle, "from", from);

    SteamWorks_SendHTTPRequest(requestHandle);
}

public OnMenuBalanceReceived(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
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
    menu.Display(client, 8);
}

public int MenuHandler(Menu menu, MenuAction action, int client, int param)
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
            PrintToChat(client, "[PTE] You can register your wallet address in the chat, like: !wallet 0x123..., for more information you can check your discord: discord.gg/vGHxVsXc4Q");
            ShowMenu(client);
        }
        else if (StrEqual(info, "3"))
        {
            PrintToChat(client, "[PTE] Your steam id is: %d", GetSteamAccountID(client));
            ShowMenu(client);
        }
    }

    return 0;
}

public OnPaymentRequestReceived(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1, any data2)
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
// #REGION Utils
//

/// Returns any array containing all "client" from online players
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

/// Returns a int number containing the count of online players
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

//
// #ENDREGION Utils
//